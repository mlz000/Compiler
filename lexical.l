%option yylineno
%{
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "tree.c"
#include "syntax.tab.h"

char *ToArray(const char *s) {
	char *p = (char *)malloc(strlen(s) + 1);
	strcpy(p, s);
	return p;
}

Node* AddNewNode(char* s) {
	Node *p = NewNode();
	p -> LineNo = yylineno;
	p -> IsToken = true; // leaf
	p -> TokenName = s;
	p -> Text = ToArray(yytext);
	return p;
}

void work(char *s) {
	yylval = AddNewNode(s);
}
%}

digit [0-9]
letter [_a-zA-Z]
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS \-
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT !
TYPE int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
INT 0|([1-9]{digit}*)
HEX 0[xX][0-9a-fA-F]+
OCT 0[0-7]+
FLOATNOR {digit}+\.{digit}+
FLOATSCI (({digit}*\.{digit}+)|({digit}+\.))[eE][\+-]?{digit}+
FLOAT {FLOATSCI}|{FLOATNOR}
ID {letter}({letter}|{digit})*
SPACE [ \t\n]
LINECOM \/\/
BLOCKCOM \/\*

%%

{LINECOM} {
	char c = input();
	while (c != '\n')	c = input();
}

{BLOCKCOM} {
	char a = '$', b;
	b = input();
	while (!(a == '*' && b == '/')) {
		a = b;
		b = input();
		if (b == -1)	break;
	}
}

{INT} {
	work("INT");
	yylval -> IntVal = atoi(yytext);
	return INT;
}

{FLOAT} {
	work("FLOAT");
	yylval -> FloatVal = atof(yytext);
	return FLOAT;
}

{HEX} {
	work("HEX");
	yylval -> IntVal = strtol(yytext, NULL, 16);
	return INT;
}

{OCT} {
	work("OCT");
	yylval -> IntVal = strtol(yytext, NULL, 8);
	return INT;
}

{SEMI} {
	work("SEMI");
	return SEMI;
}

{COMMA} {
	work("COMMA");
	return COMMA;
}

{ASSIGNOP} {
	work("ASSIGNOP");
	return ASSIGNOP;
}

{RELOP} {
	work("RELOP");
	return RELOP;
}

{PLUS} {
	work("PLUS");
	return PLUS;
}

{MINUS} {
	work("MINUS");
	return MINUS;
}

{STAR} {
	work("STAR");
	return STAR;
}

{DIV} {
	work("DIV");
	return DIV;
}

{AND} {
	work("AND");
	return AND;
}

{OR} {
	work("OR");
	return OR;
}

{DOT} {
	work("DOT");
	return DOT;
}

{NOT} {
	work("NOT");
	return NOT;
}

{TYPE} {
	work("TYPE");
	return TYPE;
}

{LP} {
	work("LP");
	return LP;
}

{RP} {
	work("RP");
	return RP;
}

{LB} {
	work("LB");
	return LB;
}

{RB} {
	work("RB");
	return RB;
}

{LC} {
	work("LC");
	return LC;
}

{RC} {
	work("RC");
	return RC;
}

{STRUCT} {
	work("STRUCT");
	return STRUCT;
}

{RETURN} {
	work("RETURN");
	return RETURN;
}

{IF} {
	work("IF");
	return IF;
}

{ELSE} {
	work("ELSE");
	return ELSE;
}

{WHILE} {
	work("WHILE");
	return WHILE;
}

{ID} {
	work("ID");
	return ID;
}

{SPACE} {}

. {
	Error = 1;
	printf("Error type A at line %d : Mysterious charater \"%s\"\n", yylineno, yytext);
}
%%