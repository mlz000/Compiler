#ifndef __INTERCODE_H
#define __INTERCODE_H
#include <stdio.h>

typedef enum {
	LabelNo, TmpNo, VarNo, Const, Function
}OpKind;

typedef enum {
	LABEL, FUNCTION, ASSIGN, ADD, SUB, MUL, DIVI, ADDR,
GETPOINT, SETPOINT, GOTO, IFGOTO, RET, DEC, ARG, CALL, PARAM, READ, WRITE}InterCodeKind;

typedef struct Operand{
    char *Text;
    OpKind kind;
    union{
        int no;
        int val;
        char *name;
    };    
}Operand;

typedef struct OperandList{
    Operand *op;
    struct OperandList *Prev, *Next;//two directed list
}OperandList;

typedef struct InterCode{
    InterCodeKind kind;
    Operand *res, *op1, *op2;    
    union{
        char *relop;
        int size;
    };
    struct InterCode *Prev, *Next;
}InterCode;

int tmpcnt, varcnt, labelcnt;

Operand *NewOperand(){
    Operand *p = (Operand*)malloc(sizeof(Operand));
    p -> Text = NULL;
    p -> kind = -1;
    p -> no = 0;
}

Operand *ConstantOperand(int x){
    Operand *p = NewOperand();
    p -> kind = Const;
    p -> val = x;
    return p;
}

Operand *NewTmp(){
    Operand *p  = NewOperand();
    p -> kind = TmpNo;
    p -> no = tmpcnt;
    ++tmpcnt;
    return p;
}

Operand *NewLabel(){
    Operand *p = NewOperand();
    p -> kind = LabelNo;
    p -> no = labelcnt;
  	++labelcnt;
    return p;
}

OperandList *NewOperandList(Operand *op){
    OperandList *t =  (OperandList*)malloc(sizeof(OperandList));
    t -> op = op;
    t -> Next = t -> Prev = t;  
    return t;  
}

int GetSymbolNo(Symbol *p){
    if(p -> IdNo == -1)	p -> IdNo = varcnt++;
    return p -> IdNo;
}

char *PrintOperand(Operand *t){
    if (t == NULL) return;
    if (t -> Text != NULL) return t -> Text;
    char buf[100];
    t -> Text = (char *)malloc(32);
	int now = t -> kind;
    if (now == LabelNo)	sprintf(buf, "label%d", t -> no);
	else if (now ==  TmpNo)	sprintf(buf, "t%d", t -> no);
	else if (now == VarNo)	sprintf(buf, "v%d", t -> no);
	else if (now == Const)	sprintf(buf, "#%d", t -> val);
	else if (now == Function)	sprintf(buf, "%s", t -> name);
	strcpy(t -> Text, buf);
    return t -> Text;
}

const char *Inter_Code[] = {
    "LABEL %s :",
    "FUNCTION %s :",
    "%s := %s",
    "%s := %s + %s",
    "%s := %s - %s",
    "%s := %s * %s",
    "%s := %s / %s",
    "%s := &%s",
    "%s := *%s",
    "*%s := %s",  
    "GOTO %s",
    "",
    "RETURN %s",
    "",
    "ARG %s",
    "%s := CALL %s",
    "PARAM %s",
    "READ %s",
    "WRITE %s",
};

void InitInterCode(){
    labelcnt = varcnt = tmpcnt = 1;
}

InterCode *NewInterCode(){
    InterCode *p = (InterCode*)malloc(sizeof(InterCode));
    p -> res = NULL;
	p -> op1 = NULL;
	p -> op2 = NULL;
	p -> relop = NULL;
    p -> size = 0;
    p -> Prev = p -> Next = p;
    return p;
}

void ModifyInterCodeValue(InterCode *p, InterCodeKind kind, Operand *res, Operand *op1, Operand *op2){
    p -> kind = kind;
    p -> res = res;
    p -> op1 = op1;
    p -> op2 = op2;
}

void PrintInterCode(InterCode *t, FILE *file){
    char buf[100];
    int now = t -> kind;
	if (now == IFGOTO)	sprintf(buf, "IF %s %s %s GOTO %s",PrintOperand(t -> op1), t -> relop, PrintOperand(t -> op2), PrintOperand(t -> res));
    else if (now == DEC)	sprintf(buf, "DEC %s %d", PrintOperand(t -> res), t -> size);
    else sprintf(buf, Inter_Code[t -> kind], PrintOperand(t -> res), PrintOperand(t -> op1), PrintOperand(t -> op2));
    fprintf(file, "%s\n", buf);
    printf("%s\n", buf);
}

void PrintInterCodeTable(InterCode *ICHead){
    if(ICHead == NULL) return;    
    InterCode *t = ICHead;
    FILE *file = fopen("out.ir", "w");
    if(!file) return;
    do{        
        PrintInterCode(t, file);
        t = t -> Next;
    }while(t != ICHead);
}

void AddInterCode(InterCode *p, InterCode **ICHead){
    if (p == NULL) return;
    if (*ICHead == NULL)	*ICHead = p;            
    else{//two directed 
        InterCode *t = p -> Prev;
        p -> Prev -> Next = *ICHead;
        p -> Prev = (*ICHead) -> Prev;
        (*ICHead) -> Prev -> Next = p;
        (*ICHead) -> Prev = t;
    }
}

char *GetRelop(Node *node){
    return node -> Text;
}

#endif
