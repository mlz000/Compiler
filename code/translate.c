#ifndef __TRANSLATE_H
#define __TRANSLATE_H

#include "stdio.h"
#include "syntax.tab.h"
#include "intercode.c"
#include "tree.c"
#include "symbol.h"
#include "assert.h"

char *WorkType(Node *TypeNode);
int WorkSize(int type, ArrayField *arr, Struct *str);
Struct *ReStruct(Node *node);
ArrayField *ReArray(Node *node);
InterCode *TransExtDef(Node *node);
InterCode *TransCompSt(Node *node);
InterCode *TransDefList(Node *node);
InterCode *TransDef(Node* InfoNode);
InterCode *TransStmtList(Node *node);
InterCode *TransStmt(Node *node);
InterCode *TransExp(Node *node, Operand **place);
InterCode *TransStruct(Node *node, Operand *place);
InterCode *TransArray(Node *node, Operand *place);
InterCode *TransArgs(Node *node, OperandList **list);
InterCode *TransCond(Node *node, Operand *labeltrue, Operand *labelfalse);

bool check(char *s1, char *s2) {
	return (strcmp(s1, s2) == 0);
}

Node *sson(Node *node) {
	return node -> son -> son;
}

Node *ssson(Node *node) {
	return node -> son -> son -> son;
}

InterCode *TransExtDef(Node *node){
    InterCode *ic = NewInterCode();
    InterCode *p = NULL;
    if(check(node -> son -> Next-> TokenName, "FunDec")){
        Operand *op = NewOperand();
        op -> kind = Function, op -> name = node -> son -> Next -> son -> Text;
        ModifyInterCodeValue(ic, FUNCTION, op, NULL, NULL);
        AddInterCode(ic, &p);        
	}
    return p;
}

InterCode *TransCompSt(Node *node){
    Node *t = node;
    Node *son = node -> son;
    InterCode* p = NULL;
    if(check(t -> Prev->TokenName, "FunDec")
    && check(t -> Prev -> son -> Next -> Next -> TokenName, "VarList")){
        Node *tnode = t -> Prev -> son -> Next -> Next;
        do{
			Symbol *sym = NewSymbol();
            if(check(sson(tnode) -> Next -> son -> TokenName, "ID"))
            sym = FindSymbol(sson(tnode) -> Next -> son -> Text, 1);//find            
            else	sym = FindSymbol(sson(sson(tnode) -> Next) -> Text, 1);
            Operand *op = NewOperand();
            op -> kind = VarNo, op -> no = GetSymbolNo(sym);
            InterCode *ic = NewInterCode();
           	ic -> kind = PARAM, ic -> res = op;
            AddInterCode(ic, &p);
            tnode = tnode -> son -> Next -> Next;
        }while(check(tnode -> TokenName, "VarList"));
    }    
    if(check(son -> Next -> TokenName, "DefList")){
        AddInterCode(TransDefList(son -> Next), &p);
        if(check(son -> Next -> Next -> TokenName, "StmtList"))	AddInterCode(TransStmtList(son -> Next -> Next), &p);
    }
    else if(check(son -> Next -> TokenName, "StmtList"))	AddInterCode(TransStmtList(son -> Next),&p);
    return p;
}

InterCode *TransDef(Node *node){
    Node *t = node, *son = t -> son, *need = son -> Next;//declist!
    InterCode *p = NULL;
    do{
        if(check(ssson(need) -> TokenName, "ID")){
            Symbol *sym = NewSymbol();
            sym = FindSymbol(ssson(need) -> Text, 1);
            if(sym -> Variable -> Type == 2){
                sym -> HaveAddr = true;
                Operand *op1 = NewOperand();
                op1 -> kind = VarNo, op1 -> no = GetSymbolNo(sym);
                InterCode *ic1 = NewInterCode();
                ic1 -> kind = DEC, ic1 -> res = op1;
                ic1 -> size = WorkSize(sym -> Variable -> Type, sym -> Variable -> SonArray, sym -> Variable -> SonStruct);
                AddInterCode(ic1, &p);
            }
        }
        else{
            Node *tnode = ssson(need);
            while(!check(tnode -> TokenName, "ID")) tnode = tnode -> son;
            Symbol *sym = FindSymbol(tnode -> Text, 1);
            Operand *op1 = NewOperand();
            op1 -> kind = VarNo, op1 -> no = GetSymbolNo(sym); 
            InterCode *ic1 = NewInterCode();
            ic1 -> kind = DEC, ic1 -> res = op1;
            ic1 -> size = WorkSize(sym -> Variable -> Type, sym -> Variable -> SonArray, sym -> Variable -> SonStruct);
            AddInterCode(ic1, &p);
        }        
        if(check(sson(need) -> Next -> TokenName, "ASSIGNOP"))	AddInterCode(TransExp(need -> son, NULL), &p);
        need = need -> son -> Next -> Next;
    }while(check(need -> TokenName, "DecList"));
    return p;  
}

InterCode *TransDefList(Node *node){
    InterCode *p = NULL;
    Node *t = node, *son = node;
    do{    
        AddInterCode(TransDef(t -> son), &p);
        t = t -> son -> Next;
    }while(check(t ->TokenName, "DefList"));
    return p;
}

InterCode *TransStmtList(Node *node){
	Node *t = node;
    InterCode* p = NULL;
    do{        
        AddInterCode(TransStmt(t -> son), &p);
        t = t -> son -> Next;
    }while(check(t -> TokenName, "StmtList"));
    return p;
}

InterCode *TransStmt(Node *node){
    Node *t = node;
    InterCode *p = NULL;
    if(check(t -> son -> TokenName, "Exp"))	AddInterCode(TransExp(t -> son, NULL), &p);
    else if(check(t -> son -> TokenName, "CompSt"))	AddInterCode(TransCompSt(t -> son), &p);
    else if(check(t -> son -> TokenName, "RETURN")){
        Operand *op1 = NewTmp();
        AddInterCode(TransExp(t -> son -> Next, &op1), &p);
        InterCode *ic = NewInterCode();
        ic -> kind = RET, ic -> res = op1;
        AddInterCode(ic, &p);
		//puts("WTF!");//debug
    }
	else if(check(t -> son -> TokenName, "IF")) {
		if (!check(t -> son -> Prev -> Prev -> TokenName, "ELSE")){
			Operand *label1 = NewLabel(), *label2 = NewLabel();
			AddInterCode(TransCond(t -> son -> Next -> Next, label1, label2), &p);
			InterCode *ic1 = NewInterCode(), *ic2 = NewInterCode();
			ic1 -> kind = ic2 -> kind = LABEL;
			ic1 -> res = label1, ic2 -> res = label2;
			AddInterCode(ic1, &p);
			AddInterCode(TransStmt(t -> son -> Prev), &p);
			AddInterCode(ic2, &p);
		}
		else {	//else
			Operand *op1 = NewLabel(), *op2 = NewLabel(), *op3 = NewLabel();
			AddInterCode(TransCond(t -> son -> Next -> Next, op1, op2), &p);
			InterCode *ic1 = NewInterCode(), *ic2 = NewInterCode(), *ic3 = NewInterCode(), *ic4 = NewInterCode();
			ic1 -> kind = ic3 -> kind = ic4 -> kind =  LABEL, ic2 -> kind = GOTO;
			ic1 -> res = op1, ic2 -> res = ic4 -> res = op3, ic3 -> res = op2;
			AddInterCode(ic1, &p);
			AddInterCode(TransStmt(t -> son -> Prev -> Prev -> Prev), &p);
			AddInterCode(ic2, &p), AddInterCode(ic3, &p);
			AddInterCode(TransStmt(t -> son -> Prev), &p);
			AddInterCode(ic4, &p);
		}
	}
    else if(check(t -> son -> TokenName, "WHILE")){
        Operand *op1 = NewLabel(), *op2 = NewLabel(), *op3 = NewLabel();
        InterCode *ic1 = NewInterCode(), *ic2 = NewInterCode(), *ic3 = NewInterCode(), *ic4 = NewInterCode();
        ic1 -> kind = ic2 -> kind = ic4 -> kind = LABEL, ic3 -> kind = GOTO;
        ic1 -> res = ic3 -> res = op1, ic2 -> res = op2, ic4 -> res = op3;
        AddInterCode(ic1, &p);
        AddInterCode(TransCond(t -> son -> Next -> Next, op2, op3), &p);
        AddInterCode(ic2, &p);
        AddInterCode(TransStmt(t -> son -> Prev), &p);
        AddInterCode(ic3, &p);
        AddInterCode(ic4, &p);
    }
    return p;
}

InterCode *TransExp(Node *node, Operand **place){
    //puts("OK?");//debug
	Node *t = node;
    InterCode *p = NULL;
    if(check(t -> son -> TokenName, "INT")){
        if(place == NULL) return NULL;
        Operand *op = ConstantOperand(t -> son -> IntVal);
        *place = op;
    }
    else if(check(t -> son -> TokenName, "ID") 
    && check(t -> son -> Next -> TokenName, "ID")){
       if(place == NULL) return NULL;
       Symbol *sym = FindSymbol(t -> son -> Text, 1);	//var
       if(sym -> IdNo != -1){
           Operand *op1 = NewOperand();
           op1 -> kind = VarNo, op1 -> no = sym -> IdNo;
           if(sym -> HaveAddr){
               char buf[50];
               op1 -> Text = (char *)malloc(32);
               sprintf(buf, "&v%d", op1 -> no);
               strcpy(op1 -> Text, buf);
           }
           *place = op1;
       }
       else{
           Operand *op1 = NewOperand();
           op1 -> kind = VarNo, op1 -> no = GetSymbolNo(sym);
           InterCode *ic1 = NewInterCode();
           if(sym -> HaveAddr) ic1 -> kind = ADDR;
           else ic1 -> kind = ASSIGN;
           ic1 -> res = *place, ic1 -> op1 = op1;
           AddInterCode(ic1, &p);
       }
    }
    else if(check(t -> son -> TokenName, "ID")
    && check(t -> son -> Next -> TokenName, "LP")
    && check(t -> son -> Next -> Next -> TokenName, "RP")){
        Symbol *sym = FindSymbol(t -> son -> Text, 0);	//fun
        if(check(t -> son -> Text, "read")){            
            if(place != NULL){
                InterCode *ic1 = NewInterCode();
                ic1 -> kind = READ, ic1 -> res = *place;
                AddInterCode(ic1, &p);
            }            
        }
        else{            
            if(place != NULL){
                InterCode *ic = NewInterCode();
                ic -> kind = CALL, ic -> res = *place;
                Operand *op = NewOperand();
                op -> kind = Function, op -> no = GetSymbolNo(sym);
                ic -> op1 = op;
                AddInterCode(ic, &p);               
            }            
        }
    }
    else if(check(t -> son -> TokenName, "ID")
    && check(t -> son -> Next -> TokenName, "LP")
    && check(t -> son -> Next -> Next -> TokenName, "Args")){        
        Symbol *sym = FindSymbol(t -> son -> Text, 0);	//func
        OperandList *list = NULL; 
        AddInterCode(TransArgs(t -> son -> Next -> Next, &list), &p);
        if(check(t -> son -> Text, "write")){
            InterCode *ic = NewInterCode();
            ic -> kind = WRITE, ic -> res = list -> op;   
            AddInterCode(ic, &p);         
        }
        else{
            OperandList *tlist = list;
            do{
                InterCode *ic = NewInterCode();
                ic -> kind = ARG, ic -> res = tlist -> op;
                AddInterCode(ic, &p);
                tlist = tlist -> Next;
            }while(tlist != list);
            if(place != NULL){
                InterCode *ic = NewInterCode();
                ic -> kind = CALL, ic -> res = *place;
                Operand *op = NewOperand();
                op -> kind = Function, op -> name = sym -> SymbolName;
                ic -> op1 = op;
                AddInterCode(ic, &p);
            }            
        }
    }
    else if(check(t -> son -> Next -> TokenName, "ASSIGNOP")){
        if(check(sson(t) -> TokenName, "ID")){
            Symbol *sym = FindSymbol(sson(t) -> Text, 1);	//var
            Operand *op1 = NewTmp();
            AddInterCode(TransExp(t -> son -> Next -> Next, &op1), &p);
            Operand *op2 = NewOperand();
            op2 -> kind = VarNo, op2 -> no = GetSymbolNo(sym);
            InterCode *ic = NewInterCode();
            ic -> kind = ASSIGN, ic -> res = op2, ic -> op1 = op1;
            AddInterCode(ic, &p);
            if(place != NULL){
                InterCode *ic2 = NewInterCode();
                ic2 -> kind = ASSIGN, ic2 -> res = *place, ic2 -> op1 = op2;
                AddInterCode(ic2, &p);
            }
        }
        else if(check(sson(t) -> Next -> TokenName, "DOT")){
            Operand *t1 = NewTmp(), *t2 = NewTmp();
            AddInterCode(TransExp(t -> son -> Next -> Next, &t1), &p);
            AddInterCode(TransExp(t -> son, &t2), &p);
            InterCode *ic = NewInterCode();
            ic = p -> Prev, ic -> kind = ASSIGN, ic -> res = NewOperand();
            char buf[50];
            ic -> res -> Text = (char *)malloc(32);
            sprintf(buf, "*t%d", ic -> op1 -> no);
            strcpy(ic -> res -> Text, buf);
            ic -> op1 = t1;
            if(place != NULL){
                InterCode *ic2 = NewInterCode();
                ic2 -> kind = ASSIGN, ic2 -> res = *place, ic2 -> op1 = t2;
                AddInterCode(ic2, &p);
            }            
        }
        else if(check(sson(t) -> Next -> TokenName, "LB")){
            Operand *t1 = NewTmp(), *t2 = NewTmp();
            AddInterCode(TransExp(t -> son -> Next -> Next, &t1), &p);
            AddInterCode(TransExp(t -> son, &t2), &p);
            InterCode *ic = NewInterCode();
            ic = p -> Prev, ic -> kind = ASSIGN, ic -> res = NewOperand();
            char buf[50];
            ic -> res -> Text = (char *)malloc(32);
            sprintf(buf,"*t%d", ic -> op1 -> no);
            strcpy(ic -> res -> Text, buf);
            ic -> op1 = t1;
            if(place != NULL){
                InterCode *ic2 = NewInterCode();
                ic2 -> kind = ASSIGN, ic2 ->res = *place, ic2 -> op1 = t2;
                AddInterCode(ic2, &p);
            } 
        }
    }
    else if(check(t -> son -> Next -> TokenName, "PLUS")){
        Operand *t1 = NewTmp(), *t2 = NewTmp();
        AddInterCode(TransExp(t -> son, &t1), &p);
        AddInterCode(TransExp(t -> son -> Prev, &t2), &p);
        if(place != NULL){
            InterCode *ic = NewInterCode();
            ic -> kind = ADD, ic -> res = *place, ic -> op1 = t1, ic -> op2 = t2;
            AddInterCode(ic, &p);
        }
    }
    else if(check(t -> son -> Next -> TokenName,"MINUS")){
        Operand *t1 = NewTmp(), *t2 = NewTmp();
        AddInterCode(TransExp(t -> son, &t1), &p);
        AddInterCode(TransExp(t -> son -> Prev, &t2), &p);
        if(place != NULL){
            InterCode *ic = NewInterCode();
            ic -> kind = SUB, ic -> res = *place, ic -> op1 = t1, ic -> op2 = t2;
            AddInterCode(ic, &p);
        }
    }
    else if(check(t -> son -> Next -> TokenName, "STAR")){
        Operand *t1 = NewTmp(), *t2 = NewTmp();
		AddInterCode(TransExp(t -> son, &t1), &p);
        AddInterCode(TransExp(t -> son -> Prev, &t2), &p);
        if(place != NULL){
            InterCode *ic = NewInterCode();
            ic -> kind = MUL, ic -> res = *place, ic -> op1 = t1, ic -> op2 = t2;
            AddInterCode(ic, &p);
        }
    }
    else if(check(t -> son -> Next -> TokenName, "DIV")){
        Operand *t1 = NewTmp(), *t2 = NewTmp();
        AddInterCode(TransExp(t -> son, &t1), &p);
        AddInterCode(TransExp(t -> son -> Prev, &t2), &p);
        if(place != NULL){
            InterCode *ic = NewInterCode();
			ic -> kind = DIVI, ic -> res = *place, ic -> op1 = t1, ic -> op2 = t2;
            AddInterCode(ic, &p);
        }
    }
    else if(check(t -> son -> TokenName, "MINUS")){
        Operand* t1 = NewTmp();
        AddInterCode(TransExp(t -> son -> Next, &t1), &p);
        if(place != NULL){
            if(t1 -> kind == Const){
                Operand *t2 = NewTmp();
                t2 -> kind = Const, t2 -> val = -(t1 -> val), *place = t2; 
            }
            else{
                InterCode *ic = NewInterCode();
                ic -> kind = SUB, ic -> res = *place, ic -> op1 = ConstantOperand(0), ic -> op2 = t1;
                AddInterCode(ic, &p);
            }
            
        }
    }
    else if(check(t -> son -> Next -> TokenName, "RELOP")
    || check(t -> son -> TokenName, "NOT")
    || check(t -> son -> Next -> TokenName, "AND")
    || check(t -> son -> Next -> TokenName, "OR")){
        Operand *label1 = NewLabel(), *label2 = NewLabel();
		if(place != NULL){
			InterCode *ic = NewInterCode();
			ic -> kind = ASSIGN, ic -> res = *place, ic -> op1 = ConstantOperand(0);
			AddInterCode(ic, &p);
		}
        AddInterCode(TransCond(t, label1, label2), &p);
        InterCode *ic2 = NewInterCode();
        ic2 -> kind = LABEL, ic2 -> res = label1;
      	AddInterCode(ic2, &p);
		if(place != NULL){
			InterCode *ic3 = NewInterCode();
			ic3 -> kind = ASSIGN, ic3 -> res = *place, ic3 -> op1 = ConstantOperand(1);
			AddInterCode(ic3, &p);
		}
        InterCode *ic4 = NewInterCode();
        ic4 -> kind = LABEL, ic4 -> res = label2;
        AddInterCode(ic4, &p);
    }
    else if(check(t -> son -> TokenName, "LP"))	AddInterCode(TransExp(t -> son -> Next, place), &p);
    else if(check(t -> son -> Next -> TokenName, "LB")) {
		//puts("WT!");//debug
		AddInterCode(TransArray(t, *place), &p);
	}
    else if(check(t -> son -> Next -> TokenName, "DOT"))	AddInterCode(TransStruct(t, *place), &p);
    else printf("%d\n%s\n", t -> LineNo, t -> son -> Next -> TokenName);
    return p;
}

InterCode *TransArray(Node *node, Operand *place){
	//puts("TransArray!");//debug
	Node *t = node;
    InterCode *p = NULL;
    ArrayField *arr = ReArray(t -> son);
    int size = WorkSize(arr -> Type, arr -> SonArray, arr -> SonStruct);
    Operand *t1 = NewTmp(), *t2 = NewTmp(), *t3 = NewTmp(), *t4 = NewTmp();/*t4 = t1[t2] */
    AddInterCode(TransExp(t -> son, &t1), &p);
    AddInterCode(TransExp(t -> son -> Next -> Next, &t2), &p);
    if(t2 -> kind == Const){
        InterCode *ic = NewInterCode();
        ic -> kind = ADD, ic -> res = t4, ic -> op1 = t1, ic -> op2 = ConstantOperand(size * t2 -> val);
        AddInterCode(ic, &p);
    }
	else{
		InterCode *ic1 = NewInterCode(), *ic2 = NewInterCode();
		ic1 -> kind = MUL, ic2 -> kind = ADD;
		ic1 -> res = t3, ic2 -> res = t4;
		ic1 -> op1 = t2, ic2 -> op1 = t1;
		ic1 -> op2 = ConstantOperand(size), ic2 -> op2 = t3;
		AddInterCode(ic1, &p), AddInterCode(ic2, &p);
	}
	if(place != NULL){
		InterCode *ic = NewInterCode();
		if(arr -> Type <= 1) ic -> kind = GETPOINT;
		else ic -> kind = ASSIGN;
		ic -> res = place, ic -> op1 = t4;
		AddInterCode(ic, &p);
	}
	return p;    
}

InterCode *TransStruct(Node *node, Operand *place){
	//puts("AAAA");//debug
    Node *t = node;
    InterCode *p = NULL;
    int size = 0;
    Struct *str = ReStruct(node);
    StructField *strfield = str -> Head;
    while(!check(t -> son -> Prev -> Text, strfield -> StructFieldName)){
        size = size + WorkSize(strfield -> Type, strfield -> SonArray, strfield -> SonStruct);
        strfield = strfield -> Next;
    }
    Operand *t1 = NewTmp(), *t2 = NewTmp();
    AddInterCode(TransExp(t -> son, &t1), &p);
    InterCode *ic = NewInterCode();
    ic -> kind = ADD, ic -> res = t2, ic -> op1 = t1, ic -> op2 = ConstantOperand(size);
    AddInterCode(ic, &p);
    if(place != NULL){
        InterCode *ic2 = NewInterCode();
        if(strfield -> Type <= 1) ic2 -> kind = GETPOINT;
        else ic2 -> kind = ASSIGN;
		ic2 -> res = place, ic2 -> op1 = t2;
        AddInterCode(ic2, &p);
    }    
    return p;
}

InterCode *TransArgs(Node *node, OperandList **list){
    InterCode *p = NULL;
    Node *t = node;
    if(check(t -> son -> Next -> TokenName, "Exp")){
        Operand *t1 = NewTmp();
        AddInterCode(TransExp(t -> son, &t1), &p);
        if(*list == NULL)	*list = NewOperandList(t1);
        else{
            OperandList *t2 = NewOperandList(t1);
            t2 -> Next = *list, t2 -> Prev = (*list) -> Prev;
            (*list) -> Prev -> Next = t2;
            (*list) -> Prev = t2;
        }
    }
    else if(check(t -> son -> Next -> TokenName, "COMMA")){
        Operand *t1 = NewTmp(); 
        AddInterCode(TransExp(t, &t1), &p);
        if(*list == NULL)	*list = NewOperandList(t1);
        else{
            OperandList *t2 = NewOperandList(t1);
            t2 -> Next = *list;
            t2 -> Prev = (*list) -> Prev;
            (*list) -> Prev -> Next = t2;
            (*list) -> Prev = t2;
        }
        AddInterCode(TransArgs(t -> son -> Prev, &(*list)), &p);
    }
    return p;
}

InterCode *TransCond(Node *node, Operand *labeltrue, Operand *labelfalse){
    InterCode* p = NULL;
    Node *t = node;
    if(check(t -> son -> TokenName,"NOT"))	AddInterCode(TransCond(t -> son -> Next, labelfalse, labeltrue), &p);
    else if(check(t -> son -> Next -> TokenName, "RELOP")){
        Operand *t1 = NewTmp(), *t2 = NewTmp();
        AddInterCode(TransExp(t -> son, &t1), &p);        
        AddInterCode(TransExp(t -> son -> Prev, &t2), &p);
        char *op = GetRelop(t -> son -> Next);        
        InterCode *ic1 = NewInterCode(), *ic2 = NewInterCode();
        ic1 -> kind = IFGOTO, ic2 -> kind = GOTO;
		ic1 -> res = labeltrue, ic2 -> res = labelfalse;
		ic1 -> op1 = t1; 
		ic1 -> op2 = t2; 
		ic1 -> relop = op;
        AddInterCode(ic1, &p), AddInterCode(ic2, &p);
    }
    else if(check(t -> son -> Next -> TokenName, "AND")){
        Operand *label1 = NewLabel();
        AddInterCode(TransCond(t -> son, label1, labelfalse), &p);
        InterCode *ic = NewInterCode();
        ic -> kind = LABEL, ic -> res = label1;
        AddInterCode(ic, &p);
        AddInterCode(TransCond(t -> son -> Prev, labeltrue, labelfalse), &p);
    }
    else if(check(t -> son -> Next -> TokenName, "OR")){
        Operand *label1 = NewLabel();
        AddInterCode(TransCond(t -> son, labeltrue, label1), &p);
        InterCode *ic = NewInterCode();
        ic -> kind = LABEL, ic -> res = label1;
        AddInterCode(ic, &p);
        AddInterCode(TransCond(t -> son -> Prev, labeltrue, labelfalse), &p);   
    }
    else{
        Operand *t1= NewTmp();
        AddInterCode(TransExp(t, &t1), &p);
        InterCode *ic1 = NewInterCode(), *ic2 = NewInterCode();
        ic1 -> kind = IFGOTO, ic2 -> kind = GOTO;
        ic1 -> res = labeltrue, ic2 -> res = labelfalse;
        ic1 -> op1 = t1;   
        ic1 -> op2 = ConstantOperand(0);
        ic1 -> relop = "!=";
        AddInterCode(ic1, &p), AddInterCode(ic2, &p);
    }
    return p;
}
#endif
