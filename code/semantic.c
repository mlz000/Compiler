#ifndef __SEMANTIC_H
#define __SEMANTIC_H

#include "stdbool.h"
#include "syntax.tab.h"
#include "symbol.h"
#include "tree.c"
#include "assert.h"

#define first (t -> son)
#define second (t -> son -> son)
#define third (t -> son -> son -> son)

void WorkTree(Node *node);
void WorkSymbol(Node *node);
void WorkStruct(Node *node);
void WorkExp(Node *node);
void WorkCompSt(Node *node);
char *WorkType(Node *node);
void AddGlobalFuntionSymbol(Node *node, char *type);
void AddGlobalVariableSymbol(Node *node, char *type);
void PushStack();
void PopStack();
void CheckSymbol();
bool CheckStructSame(Struct *str1, Struct *str2);
bool CheckArraySame(ArrayField *arr1, ArrayField *arr2);
char *ErrorInfo[] = {
	"Undefined variable \"%s\"",
	"Undefined function \"%s\"",
	"Redefined variable \"%s\"", 
	"Redefined function \"%s\"",
	"Type mismatched for assignment",
	"The left-hand side of an assignment must be a variable",
	"Type mimatched for operands",
	"Type misatched for return",
	"Function Error!",
	"\"%s\" is not an array", 
	"\"%s\" is not a function",
	"It is not an integer",
	"Illegal use of \".\"",
	"Non-existent field \"%s\"",
	"Redefined or initialized field \"%s\"",
	"Duplicated name \"%s\"",
	"Undefined structure \"%s\"",
	"Undefined funciton \"%s\"",
	"Inconsistent declaration of funciton \"%s\"",
	"Function can't return array"
};

char *Two[] = {
	"ASSIGNOP", 
	"AND",
	"OR",
	"RELOP",
	"PLUS",
	"MINUS",
	"STAR",
	"DIV"
};

char *One[] = {
	"LP", "MINUS", "NOT"
};
Struct *NeedStruct = NULL;
ArrayField *NeedArray;
bool Check(char *s1, char *s2) {
	return (strcmp(s1, s2) == 0);
}

void Initall() {
	SymbolHead = NULL;
	StructHead = NULL;
}

void PrintError(int num, int LineNo, char *s) {
	printf("Error type %d at Line %d: ", num, LineNo);
	if (!Check(s, "QvQ"))	printf(ErrorInfo[num - 1], s);
	else printf(ErrorInfo[num - 1]);
	puts("");
}

void AddGlobalVariableSymbol(Node *node, char *Type) {
	assert(node != NULL);
	Node *t = node, *Son = node -> son;
	Symbol *Sym1 = NewSymbol(), *Sym2 = NULL;
	Struct *St = NULL;
	VariableInfo *Var = NewVariable();
	//ExtDecList -> VarDec COMMA ExtDecList
	do{
		if (Check(Son -> TokenName, "ExtDecList"))	AddGlobalVariableSymbol(Son, Type);
		Son = Son -> Next;
	}while (Son != t -> son);
	//VarDec
	ModifyVariable(Var, Son, Type);
	if (!Check(Son -> son -> TokenName, "ID"))	Sym1 -> IsArray = true;
	while (!Check(Son -> TokenName, "ID"))	Son = Son -> son;
	ModifySymbol(Sym1, Son -> Text, 1, 0, Son -> LineNo);
	Sym1 -> Variable = Var;
	Sym2 = FindSymbol(Sym1 -> SymbolName, 1);
	St = FindStruct(Sym1 -> SymbolName);
	if (Sym2 != NULL || St != NULL)	PrintError(3, Sym1 -> LineNo, Sym1 -> SymbolName);
	else AddToSymbolList(Sym1);
}

void AddLocalVariableSymbol(Node *node, char *Type) {
	//VarDec
	Node *t = node;
	Symbol *Sym1 = NewSymbol(), *Sym2 = NULL;
	Struct *Str = NULL;
	VariableInfo *Var = NewVariable();
	ModifyVariable(Var, t, Type);
	if (!Check(t -> son -> TokenName, "ID"))	Sym1 -> IsArray = true;
	while (!Check(t -> TokenName, "ID"))	t = t -> son;
	ModifySymbol(Sym1, t -> Text, 1, 0, t -> LineNo);
	Sym1 -> Variable = Var;
	Sym2 = FindSymbol(Sym1 -> SymbolName, 1);
	Str = FindStruct(Sym1 -> SymbolName);
	if (Sym2 != NULL || Str != NULL)	PrintError(3, Sym1 -> LineNo, Sym1 -> SymbolName);
	else AddToSymbolList(Sym1);
}

bool CheckStructSame(Struct *str1, Struct *str2) {
	if (str1 -> StructName != NULL && str2 -> StructName != NULL && Check(str1 -> StructName, str2 -> StructName))	return true;
	StructField *t1 = str1 -> Head, *t2 = str2 -> Head;
	if (t1 == NULL && t2 == NULL)	return true;
	else if (t1 == NULL || t2 == NULL)	return false;
	while (1) {
		if (t1 -> Type != t2 -> Type)	return false;
		if (t1 -> Type == 2) {
			if (!CheckStructSame(t1 -> SonStruct, t2 -> SonStruct))	return false;
		}
		if (t1 -> Type == 3) {
			if (!CheckArraySame(t1 -> SonArray, t2 -> SonArray))	return false;
		}
		t1 = t1 -> Next, t2 = t2 -> Next;
		if (t1 == str1 -> Head && t2 == str2 -> Head)	return true;
		else if (t1 == str1 -> Head || t2 == str2 -> Head)	return false;
	}
}

bool CheckArraySame(ArrayField *arr1, ArrayField *arr2) {
	ArrayField *t1 = arr1, *t2 = arr2;
	while (1) {
		if (t1 -> Type != t2 -> Type)	return false;
		if (t1 -> Type <= 1)	return true;
		else if (t1 -> Type == 2) {
			if (!CheckStructSame(t1 -> SonStruct, t2 -> SonStruct))	return false;
		}
		else if (t1 -> Type == 3) {
			t1 = t1 -> SonArray;
			t2 = t2 -> SonArray;
		}
	}
}

void AddGlobalFuntionSymbol(Node *node, char *Type) {
	assert(node != NULL);
	Node *t = node;
	FunctionInfo *tfunc = NewFunction();
	Symbol *Sym1 = NewSymbol(), *Sym2 = NewSymbol();
	VariableList *List1, *List2;
	bool DefOrExt = 0;
	if (Check(t -> Next -> TokenName, "CompSt"))	DefOrExt = 0;
	else if (Check(t -> Next -> TokenName, "SEMI"))	DefOrExt = 1;
	ModifyFunction(tfunc, t, Type);
	t = t -> son;
	while (!Check(t -> TokenName, "ID"))	t = t -> Next;
	ModifySymbol(Sym1, t -> Text, 0, DefOrExt, t -> LineNo);
	Sym1 -> Function = tfunc;
	Sym2 = FindSymbol(Sym1 -> SymbolName, 0);
	if (Sym2 != NULL && Sym2 -> DefOrExt == 0 && DefOrExt == 0) {
		PrintError(4, Sym1 -> LineNo, Sym1 -> SymbolName);
		return ;
	}
	else if (Sym2 != NULL && DefOrExt == 1) {
		if (Sym2 -> Function -> NumOfParameter != Sym1 -> Function -> NumOfParameter || Sym2 -> Function -> Type != Sym1 -> Function -> Type) {
			PrintError(19, Sym1 -> LineNo, Sym1 -> SymbolName);
			return ;
		}
		else {
			int i;
			List1 = Sym1 -> Function -> Head;
			List2 = Sym2 -> Function -> Head;
			for (i = 0; i < Sym1 -> Function -> NumOfParameter; ++i) {
				if (List1 -> Head -> Type != List2 -> Head -> Type) {
					PrintError(19, Sym1 -> LineNo, Sym1 -> SymbolName);
					return;
				}
				if (List1 -> Head -> Type == 2 && List2 -> Head -> Type == 2) {
					if (!CheckStructSame(List1 -> Head -> SonStruct, List2 -> Head -> SonStruct)) {
						PrintError(19, Sym1 -> LineNo, Sym1 -> SymbolName);
						return;
					}
				}
				List1 = List1 -> Next, List2 = List2 -> Next;
			}
		}
	}
	else if (Sym2 != NULL && Sym2 -> DefOrExt == 1 && DefOrExt == 0) {
		if (Sym2 -> Function -> NumOfParameter != Sym1 -> Function -> NumOfParameter || Sym2 -> Function -> Type != Sym1 -> Function -> Type) {
			PrintError(19, Sym1 -> LineNo, Sym1 -> SymbolName);
		}
		else {
			int i;
			List1 = Sym1 -> Function -> Head, List2 = Sym2 -> Function -> Head;
			for (i = 0; i < Sym1 -> Function -> NumOfParameter; ++i) {
				if (List1 -> Head -> Type != List2 -> Head -> Type)	PrintError(19, Sym1 -> LineNo, Sym1 -> SymbolName);
				List1 = List1 -> Next, List2 = List2 -> Next;
			}
		}
	}
	AddToSymbolList(Sym1);
}

char *WorkType(Node *node) {
	assert(node != NULL);
	Node *Son = node -> son;
	//specifier
	if (Check(Son -> TokenName, "TYPE"))	return Son -> Text;
	else if (Check(Son -> TokenName, "StructSpecifier")) {
		Son = Son -> son -> Next;
		if (Check(Son -> TokenName, "Tag"))	return Son -> son -> Text;
		else if (!Check(Son -> TokenName, "OptTag"))	return NULL;
		else return Son -> son -> Text;
	}
	else assert(0);//QvQ
}

void WorkSymbol(Node *node) {
	assert(node != NULL);
	Node *t = node, *Son = node -> son;
	if (Check(t -> TokenName, "ExtDef")) {
		//puts("OK!");
		while (!Check(Son -> TokenName, "FunDec") && !Check(Son -> TokenName, "ExtDecList")) {
			Son = Son -> Next;
			if (Son == t -> son)	break;
		}
		if (Check(Son -> TokenName, "ExtDecList")) {
			if (Check(second -> TokenName, "StructSpecifier") && Check(third -> Next -> TokenName, "Tag")) {
				if (FindStruct(third -> Next -> son -> Text) == NULL) {
					PrintError(17, third -> Next -> LineNo, third -> Next -> son -> Text);
				}
			}
			AddGlobalVariableSymbol(Son, WorkType(Son -> Prev));
		}
		else if (Check(Son -> TokenName, "FunDec"))	AddGlobalFuntionSymbol(Son, WorkType(Son -> Prev));
	}
}

void WorkStruct(Node *node) {
	assert(node != NULL);
	Node *t = node, *Son = node -> son;
	Struct *NowHead = NewStruct();
	StructField *f1, *f2;
	char *s = NULL;
	//StructSpecifier
	if (Check(t -> TokenName, "StructSpecifier") && !Check(Son -> Next -> TokenName, "Tag")) {
		if (Check(Son -> Next -> TokenName, "OptTag")) {
			s = Son -> Next -> son -> Text;
			if (FindSymbol(s, 1) != NULL || FindStruct(s) != NULL) {
				PrintError(16, Son -> Next -> son -> LineNo, s);
				return ;
			}
		}
		InsertStruct(t);
		NowHead = StructHead, f1 = StructHead -> Head;//structfield
		if (f1 != NULL) {
			do {
				f2 = f1 -> Next;
				while (f2 != NowHead -> Head) {
					if (Check(f1 -> StructFieldName, f2 -> StructFieldName))	PrintError(15, f1 -> LineNo, f1 -> StructFieldName);
					f2 = f2 -> Next;
				}
				f1 = f1 -> Next;
			}while (f1 != NowHead -> Head);
		}
		Node *t1, *t2;
		t1 = t -> son;
		while (!Check(t1 -> TokenName, "LC"))	t1 = t1 -> Next;
		t1 = t1 -> Next;
		if (Check(t1 -> TokenName, "DefList")) {
			do {
				t2 = t1 -> son -> son -> Next;//DecList
				do {
					if (Check(t2 -> son -> son -> Next -> TokenName, "ASSIGNOP"))	PrintError(15, t2 -> son -> son -> Next -> LineNo, "QvQ");
					t2 = t2 -> son -> Next -> Next;//DecList -> Dec COMMA DecList
				}while (Check(t2 -> TokenName, "DecList"));
				t1 = t1 -> son -> Next;	//DefList -> Def DefList
			}while (Check(t1 -> TokenName, "DefList"));
		}
	}
}

int WorkExpType(Node *node) {
	Symbol *Sym1 = NULL;
	ArrayField *Arr = NULL;
	Node *t = node -> son;
	int t1, t2, t3;
	if (Check(t -> TokenName, "INT"))	return 0;
	else if (Check(t -> TokenName, "FLOAT"))	return 1;
	else if (Check(t -> TokenName, "ID")) {
		if (Check(t -> Next -> TokenName, "LP")) {	//function
			Sym1 = FindSymbol(t -> Text, 0);
			if (Sym1 != NULL) {
				Struct *str = FindStruct(Sym1 -> Function -> FunctionType);
				if (str != NULL)	NeedStruct = str;
				else NeedStruct = NULL;
				return Sym1 -> Function -> Type;
			}
		}
		else {	//variable
			Sym1 = FindSymbol(t -> Text, 1);
			if (Sym1 != NULL) {
				if (Sym1 -> Variable -> Type == 2) {
					if (Sym1 -> Variable -> SonStruct == NULL)	NeedStruct = NULL;
					else NeedStruct = Sym1 -> Variable -> SonStruct;
				}
				else if (Sym1 -> Variable -> Type == 3) {
					if (Sym1 -> Variable -> SonArray == NULL)	NeedArray = NULL;
					else NeedArray = Sym1 -> Variable -> SonArray;
				}
				return Sym1 -> Variable -> Type;
			}
		}
	}
	else {
		bool two = 0, one = 0;
		int i;
		for (i = 0; i < 8; ++i)
			if (Check(t -> Next -> TokenName, Two[i])) {
				two = 1;
				break;
			}
		for (i = 0; i < 3; ++i)
			if (Check(t -> TokenName, One[i])) {
				one = 1;
				break;
			}
		if (two) {
			t1 = WorkExpType(t), t2 = WorkExpType(t -> Next -> Next);
			if (t1 == 0 && t2 == 0)	return 0;
			else if (t1 == 1 && t2 == 1)	return 1;
			else return -1;
		}
		else if (one)	return WorkExpType(t -> Next);
		else if (Check(t -> Next -> TokenName, "LB")) {	//array
			t3 = 0;
			Node *tmpnode = t;
			while (!Check(tmpnode -> son -> TokenName, "ID")) {
				++t3;
				tmpnode = tmpnode -> son;
			}
			Sym1 = FindSymbol(tmpnode -> son -> Text, 1);
			Arr = Sym1 -> Variable -> SonArray;
			for(i = 0; i < t3; ++i)	Arr = Arr -> SonArray;
			//struct
			if (Arr -> Type == 2)	NeedStruct = Arr -> SonStruct;
			else if (Arr -> Type == 3)	NeedArray = Arr -> SonArray;
			return Arr -> Type;
		}
		else if (Check(t -> Next -> TokenName, "DOT")) {
			StructField *tfield = NewStructField();
			if (WorkExpType(t) != 2 || NeedStruct == NULL)	return -1;//not struct
			tfield = NeedStruct -> Head;
			do {
				if (tfield == NULL)	return -1;
				if (Check(tfield -> StructFieldName, t -> Next -> Next -> Text)) {
					if (tfield -> Type == 2) {
						if (tfield -> SonStruct == NULL)	NeedStruct = NULL;
						else NeedStruct = tfield -> SonStruct;
					}
					else if (tfield -> Type == 3) {
						if (tfield -> SonArray == NULL)	NeedArray = NULL;
						else NeedArray = tfield -> SonArray;
					}
					return tfield -> Type;
				}
				else {
					tfield = tfield -> Next;
					if (tfield == NeedStruct -> Head)	return -1;
				}
			}while (1);
		}
	}
	return -1;
}

void CheckFunction() {
	Symbol *Sym1, *Sym2;
	SymbolStack *List = SymbolHead;
	if (List == NULL)	return;
	do {
		if (List -> Head != NULL) {
			Sym1 = List -> Head;
			do {
				//function
				if (!Sym1 -> FunctionOrVariable) {
					Sym2 = FindSymbol(Sym1 -> SymbolName, 0);
					if (Sym2 == NULL || Sym2 -> DefOrExt == 1)	PrintError(18, Sym1 -> LineNo, Sym1 -> SymbolName);
				}
				Sym1 = Sym1 -> Next;
			}while (Sym1 != List -> Head);
		}
		List = List -> Next;
	}while (List != SymbolHead);
}

void CheckLocalVariable() {
	if (SymbolHead == NULL)	return;
	Symbol *Sym1 = SymbolHead -> Head;
	if (Sym1 == NULL)	return ;
	do {
		if (Sym1 -> FunctionOrVariable) {
			VariableInfo *Var = Sym1 -> Variable;
			if (Var -> VariableName != NULL && !Check(Var -> VariableName, "int") && !Check(Var -> VariableName, "float") && FindStruct(Var -> VariableName) == NULL)	PrintError(17, Sym1 -> LineNo, Var -> VariableName);
		}
		Sym1 = Sym1 -> Next;
	}while (Sym1 != SymbolHead -> Head);
}

bool FindVariableID(Node *node) {
	Symbol *Sym1 = FindSymbol(node -> Text, 1);
	if (Sym1 != NULL)	return true;
	else {
		PrintError(1, node -> LineNo, node -> Text);
		return false;
	}
}

bool FindFunctionId(Node *node) {
	Symbol *Sym1 = FindSymbol(node -> Text, 0);
	Node *t = node;
	VariableList *Var = NULL;
	int cnt = 0;
	if (Sym1 == NULL) {
		if (FindSymbol(t -> Text, 1) != NULL) {
			PrintError(11, node -> LineNo, node -> Text);
			return false;
		}
		else {
			PrintError(2, node -> LineNo, node -> Text);
			return false;
		}
	}
	else{
		if (!Check(node -> Next -> Next -> TokenName, "Args")) {
			if (Sym1 -> Function -> NumOfParameter != 0)	PrintError(9, node -> LineNo, "QvQ");
		}
		else {//args
			t = node -> Next -> Next; 
			Var = Sym1 -> Function -> Head;
			while (Check(t -> TokenName, "Args")) {
				++cnt;
				//Args -> Exp COMMA Args
				t = t -> son -> Next -> Next;
			}
			if (cnt != Sym1 -> Function -> NumOfParameter)	PrintError(9, node -> LineNo, "QvQ");
			else {
				t = node -> Next -> Next;
				int i;
				for (i = 0; i < cnt; ++i) {
					int type1 = WorkExpType(t -> son), type2 = Var -> Head -> Type;
					if (type1 != type2) {
						PrintError(9, node -> LineNo, "QvQ");
						return false;
					}
					if (type1 == 2) {	//struct
						WorkExpType(t -> son);
						Struct *str = NeedStruct;
						if (!CheckStructSame(str, Var -> Head -> SonStruct)) {
							PrintError(9, node -> LineNo, "QvQ");
							return false;
						}
					}
					if (type1 == 3) {	//array
						WorkExpType(t -> son);
						ArrayField *tfiled = NeedArray;
						if (!CheckArraySame(tfiled, Var -> Head -> SonArray)) {
							PrintError(9, node -> LineNo, "QvQ");
							return false;
						}

					}
					t = t -> son -> Next -> Next;
					Var = Var -> Next;
				}
			}
		}
	}
	return true;
}

void WorkExp(Node *node) {
	Node *t = node -> son, *tmp;
	Symbol *Sym1 = NewSymbol();
	Struct *str1, *str2;
	ArrayField *arr1, *arr2;
	if (Check(t -> TokenName, "ID")) {
		//printf("AAA%s\n", t -> Text);
		if (!Check(t -> Next -> TokenName, "LP")) {
			if (FindVariableID(t)) { //can find
				Sym1 = FindSymbol(t -> Text, 1);
				if (!Sym1 -> IsArray && Check(node -> Next -> TokenName, "LB"))	PrintError(10, node -> LineNo, t -> Text);
			}
		}
		else FindFunctionId(t);
	}
	if (Check(t -> Next -> TokenName, "ASSIGNOP")) {
		bool id = Check(t -> son -> TokenName, "ID"), exp = Check(t -> son -> TokenName, "Exp");
		bool nid = Check(t -> son -> Next -> TokenName, "ID"), nlb = Check(t -> son -> Next -> TokenName, "LB"), ndot = Check(t -> son -> Next -> TokenName, "DOT");
		if (!((id && nid) || (exp && nlb) || (exp && ndot)))	PrintError(6, node -> LineNo, "QvQ");
		int t1 = WorkExpType(t), t2 = WorkExpType(t -> Next -> Next);
		if (~t1 && ~t2 && t1 != t2)	PrintError(5, node -> LineNo, "QvQ");
		if (t1 == 2 && t2 == 2) {
			WorkExpType(t);
			str1 = NeedStruct;
			WorkExpType(t -> Next -> Next);
			str2 = NeedStruct;
			if (!CheckStructSame(str1, str2))	PrintError(5, node -> LineNo, "QvQ");
		}
		if (t1 == 3 && t2 == 3) {
			WorkExpType(t);
			arr1 = NeedArray;
			WorkExpType(t -> Next -> Next);
			arr2 = NeedArray;
			if (!CheckArraySame(arr1, arr2))	PrintError(5, node -> LineNo, "QvQ");
		}
	}
	bool two = 0, one = 0;
	int i;
	for (i = 1; i < 8; ++i)
		if (Check(t -> Next -> TokenName, Two[i])) {
			two = 1;
			break;
		}
	for (i = 1; i < 3; ++i)
		if (Check(t -> TokenName, One[i])) {
			one = 1;
			break;
		}
	if (two) {
		int t1 = WorkExpType(t), t2 = WorkExpType(t -> Next -> Next);
		if (t1 == -1 || t2 == -1);//right
		else if (!((t1 == 0 && t2 == 0) || (t1 == 1 && t2 == 1)))	PrintError(7, node -> LineNo, "QvQ");//wrong
	}
	if (one && WorkExpType(t -> Next) >= 2)	PrintError(7, node -> LineNo, "QvQ");//array or struct
	if (Check(t -> Next -> TokenName, "LB") && WorkExpType(t -> Next -> Next) != 0)	PrintError(12, node -> LineNo, "QvQ");
	if (Check(t -> Next -> TokenName, "DOT")) {
		if (WorkExpType(t) != 2)	PrintError(13, node -> LineNo, "QvQ");
		else {
			//t:struct
			tmp = t;
			Struct *str = NewStruct();
			StructField *strf = NewStructField();
			while (!Check(tmp -> son -> TokenName, "ID")) {
				if (Check(tmp -> son -> TokenName, "LP"))	tmp = tmp -> son -> Next;
				else tmp = tmp -> son;
			}
			tmp = tmp -> son;
			if (Check(tmp -> Next -> TokenName, "LP"))	str = FindStruct(FindSymbol(tmp -> Text, 0) -> Function -> FunctionType);
			else str = FindStruct(FindSymbol(tmp -> Text, 1) -> Variable -> VariableName);
			if (str == NULL)	return;
			strf = str -> Head;
			do {
				if (strf == NULL) {
					PrintError(14, t -> Next -> Next -> LineNo, t -> Next -> Next -> Text); 
					break;
				}
				if (Check(strf -> StructFieldName, t -> Next -> Next -> Text))	break;
				else {
					strf = strf -> Next;
					if (strf == str -> Head) {
						PrintError(14, t -> Next -> Next -> LineNo, t -> Next -> Next -> Text);
						break;
					}
				}
			}while (1);
		}
	}
}

void WorkCompSt(Node *node) {
	Node *Func = node -> Prev, *Varlist; //ExtDec -> Specifier FunDec CompSt
	Node *t = node -> son -> Next, *Son = NewNode(), *ret = NewNode(); //DefList
	Struct *str;
	//printf("WorkCompst%s\n", Func -> TokenName);
	if (Check(Func -> TokenName, "FunDec") && Check(Func -> son -> Next -> Next -> TokenName, "VarList")) {
		Varlist = Func -> son -> Next -> Next;
		do {
			AddLocalVariableSymbol(Varlist -> son -> son -> Next, WorkType(Varlist -> son -> son));
			Varlist = Varlist -> son -> Next -> Next;
		}while (Check(Varlist -> TokenName, "VarList"));
	}
	while (Check(t -> TokenName, "DefList")) {
		Son = t -> son -> son -> Next;//DecList
		do {
			if (Check(Son -> TokenName, "DecList"))	AddLocalVariableSymbol(Son -> son -> son, WorkType(t -> son -> son));
			//Def -> Specifier FunDec SEMI
			else AddGlobalFuntionSymbol(Son, WorkType(t -> son -> son));
			Son = Son -> son -> Next -> Next;
		}while(Check(Son -> TokenName, "DecList"));
		t = t -> son -> Next;
	}
	int type;
	if (Check(node -> TokenName, "CompSt")) {
		ret = node -> son;
		while (!Check(ret -> TokenName, "StmtList")) {
			ret = ret -> Next;
			if (Check(ret -> TokenName, "RC"))	break;//need?
		}
		while (Check(ret -> TokenName, "StmtList")) {
			if (Check(ret -> son -> son -> TokenName, "RETURN")) {
				type = WorkExpType(ret -> son -> son -> Next);
				if (type != (FindSymbol(node -> Prev -> son -> Text, 0) -> Function -> Type))	PrintError(8, ret -> LineNo, "QvQ");
				else if (type == 2) {
					WorkExpType(ret -> son -> son -> Next);
					str = NeedStruct;
					if (FindSymbol(node -> Prev -> son -> Text, 0) -> Function -> FunctionType == NULL)	PrintError(8, ret -> LineNo, "QvQ");
					else if (!CheckStructSame(str, FindStruct(FindSymbol(node -> Prev -> son -> Text, 0) -> Function -> FunctionType)))	PrintError(8, ret -> LineNo, "QvQ");
				}
				else if (type == 3)	PrintError(20, ret -> LineNo, "QvQ");
			}
			ret = ret -> son -> Next;
		}
	}
}

void WorkTree(Node *node) {
	assert(node != NULL);
	Node *t = node, *Son = node -> son;
	if (Check(t -> TokenName, "ExtDef"))	WorkSymbol(t);
	if (Check(t -> TokenName, "StructSpecifier") && !Check(Son -> Next -> TokenName, "Tag"))	WorkStruct(t);
	if (Check(t -> TokenName, "CompSt")) {		//{
		PushStack();
		WorkCompSt(t);
		CheckLocalVariable();
	}
	if (Check(t -> TokenName, "Exp")) {
		WorkExp(t);
	}
	if (Son != NULL) {
		do {
			WorkTree(Son);
			Son = Son -> Next;
		}while (Son != t -> son);	//double linked list
	}
	if (Check(t -> TokenName, "CompSt"))	PopStack();	//}
}
#endif
