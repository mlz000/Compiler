%{
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "tree.c"
#include "stdarg.h"
#define YYSTYPE Node*
#define ToStr(x) #x
int Error;
int yylex();
void yyerror(char *msg);
Node *gao(char *s, int num, ...) { //varible
	Node *p = NewNode(); // root
	p -> IsToken = false;
	p -> TokenName = s;
	va_list ap;
	va_start(ap, num);
	int i;
	for (i = 0; i < num; ++i) {
		Node *q = va_arg(ap, Node *);
		if (!i)	p -> LineNo = q -> LineNo;
		if (q != NULL) AddSon(p, q);	
	}
	va_end(ap);
	return p;
}
%}

%token INT FLOAT ID
%token LC RC TYPE STRUCT RETURN IF WHILE SEMI COMMA
%right ASSIGNOP 
%left OR
%left AND  
%left RELOP
%left PLUS MINUS
%left DIV STAR
%right NOT
%left DOT LP RP LB RB
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
/* High-level Definitions */
Program : ExtDefList {
	$$ = gao(ToStr(Program), 1, $1);
	if (Error == 0)	PrintTree($$, 0);
} 
	;
ExtDefList : {$$ = NULL;}
	| ExtDef ExtDefList {
		$$ = gao(ToStr(ExtDefList), 2, $1, $2);
	}
	;
ExtDef : Specifier ExtDecList SEMI {$$ = gao(ToStr(ExtDef), 3, $1, $2, $3);}
	| Specifier SEMI{$$ = gao(ToStr(ExtDef), 2, $1, $2);}
	| Specifier FunDec CompSt {$$ = gao(ToStr(ExtDef), 3, $1, $2, $3);}
	;
ExtDecList : VarDec {$$ = gao(ToStr(ExtDecList), 1, $1);}
	| VarDec COMMA ExtDecList {$$ = gao(ToStr(ExtDecList), 3, $1, $2, $3);}
	;

/* Specifiers */
Specifier : TYPE {$$ = gao(ToStr(Specifier), 1, $1);}
	| StructSpecifier {gao(ToStr(Specifier), 1, $1);}
	;
StructSpecifier: STRUCT Tag { gao(ToStr(StructSpecifier), 2, $1, $2); }
	| STRUCT OptTag LC DefList RC { gao(ToStr(StructSpecifier), 5, $1, $2, $3, $4, $5); }
	;
OptTag: { $$ = NULL; }
	| ID { gao(ToStr(OptTag), 1, $1); }
	;
Tag: ID { gao(ToStr(Tag), 1, $1); }
   ;

/*Declarators*/
VarDec: ID { gao(ToStr(arDec), 1, $1); }
	| VarDec LB INT RB { gao(ToStr(VarDec), 4, $1, $2, $3, $4); }
	;
FunDec: ID LP VarList RP { gao(ToStr(FunDec), 4, $1, $2, $3, $4); }
	| ID LP RP { gao(ToStr(FunDec), 3, $1, $2, $3); }
	;
VarList: ParamDec COMMA VarList { gao(ToStr(VarList), 3, $1, $2, $3); }
	| ParamDec { gao(ToStr(VarList), 1, $1); }
    ;
ParamDec: Specifier VarDec { gao(ToStr(ParamDec), 2, $1, $2); }
 	;

/* Statements */
CompSt: LC DefList StmtList RC { gao(ToStr(CompSt), 4, $1, $2, $3, $4); }
	| error RC{gao(ToStr(CompSt), 2, $1, $2);}
	;
StmtList: { $$ = NULL; }
	| Stmt StmtList { gao(ToStr(StmtList), 2, $1, $2); }
	;
Stmt: Exp SEMI { gao(ToStr(Stmt), 2, $1, $2); }
	| CompSt { gao(ToStr(Stmt), 1, $1); }
	| RETURN Exp SEMI { gao(ToStr(Stmt), 3, $1, $2, $3); }
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { gao(ToStr(Stmt), 5, $1, $2, $3, $4, $5); }
	| IF LP Exp RP Stmt ELSE Stmt { gao(ToStr(Stmt), 7, $1, $2, $3, $4, $5, $6, $7); }
	| WHILE LP Exp RP Stmt { gao(ToStr(Stmt), 5, $1, $2, $3, $4, $5); }
	| error SEMI {gao(ToStr(Stmt), 2, $1, $2);}
	;

/* Local Definitions */
DefList: { $$ = NULL; }
	| Def DefList { gao(ToStr(DecList), 2, $1, $2); }
	;
Def: Specifier DecList SEMI { gao(ToStr(Def), 3, $1, $2, $3); }
	| Specifier error SEMI { gao(ToStr(Def), 3, $1, $2, $3);}
    ;
DecList: Dec { gao(ToStr(DecList), 1, $1); }
	| Dec COMMA DecList { gao(ToStr(DecList), 3, $1, $2, $3); }
Dec: VarDec { gao(ToStr(Dec), 1, $1); }
    | VarDec ASSIGNOP Exp { gao(ToStr(Dec), 3, $1, $2, $3); }
    ;

/* Expressions */
Exp: Exp ASSIGNOP Exp { gao(ToStr(Exp), 3, $1, $2, $3); }
   | Exp AND Exp { gao(ToStr(Exp), 3, $1, $2, $3); }
   | Exp OR Exp { gao(ToStr(Exp), 3, $1, $2, $3); }
   | Exp RELOP Exp { gao(ToStr(Exp), 3, $1, $2, $3); }
   | Exp PLUS Exp { gao(ToStr(Exp), 3, $1, $2, $3); }
   | Exp MINUS Exp { gao(ToStr(Exp), 3, $1, $2, $3); }
   | Exp STAR Exp { gao(ToStr(Exp), 3, $1, $2, $3); }
   | Exp DIV Exp { gao(ToStr(Exp), 3, $1, $2, $3); }
   | LP Exp RP { gao(ToStr(Exp), 3, $1, $2, $3); }
   | MINUS Exp { gao(ToStr(Exp), 2, $1, $2); }   
   | NOT Exp { gao(ToStr(Exp), 2, $1, $2); }   
   | ID LP Args RP { gao(ToStr(Exp), 4, $1, $2, $3, $4); }
   | ID LP RP { gao(ToStr(Exp), 3, $1, $2, $3); }
   | Exp LB Exp RB { gao(ToStr(Exp), 4, $1, $2, $3, $4); }
   | Exp DOT ID { gao(ToStr(Exp), 3, $1, $2, $3); }
   | ID { gao(ToStr(Exp), 1, $1); }
   | INT { gao(ToStr(Exp), 1, $1); }
   | FLOAT { gao(ToStr(Exp), 1, $1); }
   | error RP{ gao(ToStr(Exp), 2, $1, $2);}
   ;
Args: Exp COMMA Args { gao(ToStr(Args), 3, $1, $2, $3); }
	| Exp { gao(ToStr(Args), 1, $1); }
	;
%%

#include "lex.yy.c"
void yyerror(char *msg) {
	Error = 1;
	printf("Error type B at line %d : %s\n", yylineno, msg);
}
int main(int argc, char **argv) {
	if (argc <= 1)	return 1;
	Error = 0;
	FILE *f = fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]);
		return 1;
	}
	yyrestart(f);
	yyparse();
	return 0;
}