#ifndef __SYMBOL_H
#define __SYMBOL_H

#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "tree.c"
#include "semantic.c"

char *WorkType(Node *node);
//struct
struct Struct *NewStruct();
struct StructField *NewStructField();
struct StructField *CreateNewStructField(Node *node, char *Type);
struct StructField *CreateNewStructFieldList(Node *node);
void InsertStruct(Node *node);
void AddToStructList(struct Struct *str);
struct Struct *FindStruct(char *s);
//variable
struct VariableInfo *NewVariable();
struct VariableList *NewVariableList();
void ModifyVariable(struct VariableInfo *Var, Node *node, char *Type);
//array
struct ArrayField *NewArrayField(Node *node, char *Type);
void ModifyArraySize(Node *node, struct VariableInfo *Var);
//function
struct FunctionInfo *NewFunction(); 
void AddToFunctionList(struct FunctionInfo *func, struct VariableInfo *Var);
void ModifyFunction(struct FunctionInfo *func, Node *node, char *Type);
//symbol
struct Symbol *NewSymbol();
struct Symbol *FindSymbol(char *s, int Type);
struct SymbolStack *NewSymbolStack();
void AddToSymbolList(struct Symbol *sym);
void ModifySymbol(struct Symbol *sym, char *s, bool a, bool b, int LineNo);
void PushStack();
void PopStack();
//Struct
typedef struct Struct {
	char *StructName;
	struct StructField *Head;
	struct Struct *Prev, *Next;
}Struct;
Struct *StructHead;// head

typedef struct StructField {
	char *StructFieldName, *TypeName;
	int Type;
	int LineNo;
	union {
		struct ArrayField *SonArray;
		struct Struct *SonStruct;
	};
	struct StructField *Prev, *Next;
}StructField;
//Array
typedef struct ArrayField {
	char *ArrayFieldName;
	int Type, Size;
	union {
		struct ArrayField *SonArray;
		struct Struct *SonStruct;
	};
}ArrayField;	
//Variable
typedef struct VariableInfo {
	char *VariableName;
	int Type;		//0:int 1:float 2:struct 3:array
	union{
		struct ArrayField *SonArray;
		struct Struct *SonStruct;
	};
}VariableInfo;

typedef struct VariableList {
	VariableInfo *Head;
	struct VariableList *Prev, *Next;
}VariableList;
//Function
typedef struct FunctionInfo {
	char *FunctionType;
	int Type;//0: int 1: float 2: struct
	int NumOfParameter;
	VariableList *Head;
}FunctionInfo;
//Symbol
typedef struct Symbol {
	char *SymbolName;
	bool FunctionOrVariable, DefOrExt, IsArray;
	int LineNo;
	union {
		struct FunctionInfo *Function;
		struct VariableInfo *Variable;
	};
	struct Symbol *Prev, *Next;
}Symbol;

typedef struct SymbolStack {
	Symbol *Head;
	struct SymbolStack *Prev, *Next;
}SymbolStack;
SymbolStack *SymbolHead;//head

//struct
Struct *NewStruct() {
	Struct *t = (Struct *)malloc(sizeof(Struct));
	t -> StructName = NULL;
	t -> Prev = t -> Next = t;
	return t;
}


StructField *NewStructField() {
	StructField *t = (StructField *)malloc(sizeof(StructField));
	t -> StructFieldName = NULL, t -> TypeName = NULL;
	t -> LineNo = -1;
	t -> SonArray = (ArrayField *)malloc(sizeof(ArrayField));
	t -> SonStruct = NewStruct();
	t -> Prev = t -> Next = t;
	return t;
}

StructField *CreateNewStructField(Node *node, char *Type) {
	//Dec -> VarDec | VarDec ASSIGNOP Exp
	StructField *st = (StructField *)malloc(sizeof(StructField));
	Node *t = node -> son;
	struct VariableInfo *Var = NewVariable();
	while (t -> son -> TokenName != "ID")	t = t -> son;
	st -> StructFieldName = t -> son -> Text;
	ModifyVariable(Var, node -> son, Type);
	st -> TypeName = Var -> VariableName;
	st -> Type = Var -> Type;
	st -> SonArray = Var -> SonArray;
	st -> SonStruct = Var -> SonStruct;
	st -> LineNo = node -> LineNo;
	st -> Prev = st -> Next = st;
	return st;
}

void AddToStructList(Struct *str) {
	if (StructHead == NULL)	StructHead = str;
	else {//double linked list
		str -> Prev = StructHead -> Prev;
		str -> Next = StructHead;
		StructHead -> Prev -> Next = str;
		StructHead -> Prev = str;
	}
}

void InsertStruct(Node *node) {
	//StructSpecifier
	Node *Son = node -> son;
	Struct *tst = NewStruct();
	if (Son -> Next -> TokenName == "OptTag")	tst -> StructName = Son -> Next -> son -> Text;
	//printf("sda %s\n", tst -> StructName);
	while (Son -> TokenName != "LC")	Son = Son -> Next;
	if (Son -> Next -> TokenName == "DefList") {
		Son = Son -> Next;
		tst -> Head = CreateNewStructFieldList(Son);//list
		AddToStructList(tst);
	}
	else {	//Deflist is null
		tst -> Head = NULL;
		AddToStructList(tst);
	}
}

Struct *FindStruct(char *s) {
	Struct *t = StructHead;
	if (s == NULL || t == NULL)	return NULL;
	do {
		if (t -> StructName == NULL)	continue;
		if (strcmp(t -> StructName, s) == 0)	return t;
		t = t -> Next;
	}while (t != StructHead);
	return NULL;
}

StructField *CreateNewStructFieldList(Node *node) {
	if (node == NULL)	return NULL;
	StructField *tlist = NULL, *tfield = NewStructField();
	//DefList->Def DefLIst, Def -> Specifier DecList
	Node *t = node, *Son = node -> son -> son -> Next;
	char *s;
	do {
		s = WorkType(t -> son -> son);//Sepcifier
		Son = t -> son -> son -> Next; //DecList
		do {
			//DecList -> Dec
			tfield = CreateNewStructField(Son -> son, s);//Dec
			if (tlist == NULL)	tlist = tfield;
			else {//double linked list
				tfield -> Prev = tlist -> Prev;
				tfield -> Next = tlist;
				tlist -> Prev -> Next = tfield;
				tlist -> Prev = tfield;
			}
			//DecList -> Dec COMMA DecList
			Son = Son -> son -> Next -> Next;
		}while(Son -> TokenName == "DecList");
		t = t -> son -> Next;
	}while(t -> TokenName == "DefList");
	return tlist;
}
//variable
VariableInfo *NewVariable() {
	VariableInfo *t = (VariableInfo *)malloc(sizeof(VariableInfo));
	t -> Type = -1;	//init
	t -> SonArray = (ArrayField *)malloc(sizeof(ArrayField));
	t -> SonStruct = NewStruct();
	return t;
}

VariableList *NewVariableList() {
	VariableList *t = (VariableList *)malloc(sizeof(VariableList));
	t -> Prev = t -> Next = t;
	return t;
}

void ModifyVariable(VariableInfo *Var, Node *node, char *Type) {
	char *s = node -> son -> TokenName;
	bool Int = (strcmp(Type, "int") == 0), Float = (strcmp(Type, "float") == 0);
	if (s == "VarDec")	{
		Var -> Type = 3;	//array
		Var -> SonArray = NewArrayField(node, Type);
		ModifyArraySize(node, Var);
	}
	else if (Type == NULL || (Int + Float == 0)) Var -> Type = 2, Var -> SonStruct = FindStruct(Type);	//OptTag
	else if (s == "ID") {
		if (Int)	Var -> Type = 0;
		else if (Float)	Var -> Type = 1;
	}
	if (Type == NULL)	Var -> VariableName = NULL;
	else Var -> VariableName = Type;
}
//Array
ArrayField *NewArrayField(Node *node, char *Type) {
	assert(node != NULL);
	ArrayField *t = (ArrayField *)malloc(sizeof(ArrayField));
	Node *Son = node -> son;
	bool Int = (strcmp(Type, "int") == 0), Float = (strcmp(Type, "float") == 0);
	if (Son -> son -> TokenName == "ID") {
		if (Int)	t -> Type = 0, t -> ArrayFieldName = "int";
		else if (Float)	t -> Type = 1, t -> ArrayFieldName = "float";
	}
	else if (!Int && !Float) {
		t -> Type = 2;	//strcut
		t -> SonStruct = FindStruct(Type);
		t -> ArrayFieldName = Type;
	}
	else if (Son -> son -> TokenName == "VarDec") {
		t -> Type = 3;	//array
		t -> ArrayFieldName = "array";
		t -> SonArray = NewArrayField(Son, Type);
	}
	return t;
}

void ModifyArraySize(Node *node, VariableInfo *Var) {
	int l = 0, i;
	Node *t = node;
	ArrayField *tarr = Var -> SonArray;
	//VarDec -> VarDec LB INT RB
	while (t -> son -> TokenName != "ID") {
		++l;//dimension of array
		t = t -> son;
	}
	int *a = (int *)malloc(l * sizeof(int));
	t = node;
	for (i = 0; i < l; ++i, t = t -> son)	a[l - i - 1] = t -> son -> Next -> Next -> IntVal;
	for (i = 0; i < l; ++i, tarr = tarr -> SonArray)	tarr -> Size = a[i];
}
//Function
FunctionInfo *NewFunction() {
	FunctionInfo *t = (FunctionInfo *)malloc(sizeof(FunctionInfo));
	t -> FunctionType = NULL, t -> Type = -1, t -> NumOfParameter = 0, t -> Head = NULL;
	return t;
}

void AddToFunctionList(FunctionInfo *func, VariableInfo *Var) {
	VariableList *t = NewVariableList();
	t -> Head = Var;
	if (func -> Head == NULL)	func -> Head = t;
	else {//double linked list
		t -> Prev = func -> Head -> Prev;
		t -> Next = func -> Head;
		func -> Head -> Prev -> Next = t;
		func -> Head -> Prev = t;
	}
}

void ModifyFunction(FunctionInfo *func, Node *node, char *Type) {
	assert(node != NULL);
	int NumOfPara = 0;
	Node *t = node, *Son = node -> son;
	char *s = NULL;
	bool Int = (strcmp(Type, "int") == 0), Float = (strcmp(Type, "float") == 0);
	if (Type == NULL)	func -> FunctionType = NULL, func -> Type = 2;	//struct no name
	else {
		func -> FunctionType = Type;
		if (Int)	func -> Type = 0;
		else if (Float)	func -> Type = 1;
		else func -> Type = 2;
	}
	Son = t -> son -> Next -> Next; //Varlist
	while (Son -> TokenName == "VarList") {
		VariableInfo *Var = NewVariable();
		s = WorkType(Son -> son -> son);
		ModifyVariable(Var, Son -> son -> son -> Next, s);
		AddToFunctionList(func, Var);
		func -> NumOfParameter++;
		Son = Son -> son -> Next -> Next; //VarList -> ParamDec COMMA CarList
	}
}
//Symbol
SymbolStack *NewSymbolStack() {
	SymbolStack *t = (SymbolStack *)malloc(sizeof(SymbolStack));
	t -> Head = NULL;
	t -> Next = t -> Prev = t;
	return t;
}

void AddToSymbolList(struct Symbol *sym) {
	assert(sym != NULL);
	if (SymbolHead == NULL) {
		SymbolHead = NewSymbolStack();
		SymbolHead -> Head = sym;
	}
	else if (SymbolHead -> Head == NULL)	SymbolHead -> Head = sym;
	else {	//double linked list
		sym -> Prev = SymbolHead -> Head -> Prev;
		sym -> Next = SymbolHead -> Head;
		SymbolHead -> Head -> Prev -> Next = sym;
		SymbolHead -> Head -> Prev = sym;
	}
}

Symbol *NewSymbol() {
	Symbol *t = (Symbol *)malloc(sizeof(Symbol));
	t -> IsArray = false;
	t -> Prev = t -> Next = t;
	return t;
}

Symbol *FindSymbol(char *s, int Type) {
	if (SymbolHead == NULL)	return NULL;
	SymbolStack *t = SymbolHead;
	Symbol *Sym1 = NULL, *Sym2 = NULL;
	bool ok = 0;
	do {
		if (t -> Head != NULL) {
			Sym1 = t -> Head;
			do {
				bool f = (strcmp(Sym1 -> SymbolName, s) == 0);
				if (Type == 0 && Sym1 -> FunctionOrVariable == 0 && f) {  	//Funcion and matched
					if (Sym1 -> DefOrExt == 0)	return Sym1;	//def
					else if (!ok)	ok = 1, Sym2 = Sym1;
				}
				else if (Type == 1 && Sym1 -> FunctionOrVariable == 1 && f)	return Sym1;//Variable and matched
				Sym1 = Sym1 -> Next;
			}while (Sym1 != t -> Head);
		}
		t = t -> Next;
	}while (t != SymbolHead);
	if (Type == 0 && Sym2 != NULL)	return Sym2;
	return NULL;
}

void PushStack() {
	assert(SymbolHead != NULL);
	SymbolStack *Head = NewSymbolStack();
	Head -> Next = SymbolHead;
	Head -> Prev = SymbolHead -> Prev;
	SymbolHead -> Prev -> Next = Head;
	SymbolHead -> Prev = Head;
	SymbolHead = Head;
}

void PopStack() {
	SymbolHead -> Next -> Prev = SymbolHead -> Prev;
	SymbolHead -> Prev -> Next = SymbolHead -> Next;
	SymbolHead = SymbolHead -> Next;
}

void ModifySymbol(Symbol *sym, char *s, bool a, bool b, int LineNo) {
	sym -> SymbolName = s;
	sym -> FunctionOrVariable = a, sym -> DefOrExt = b;
	sym -> LineNo = LineNo;
}

#endif
