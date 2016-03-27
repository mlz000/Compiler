#ifndef __TREE_H
#define __TREE_H
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

typedef struct Node {
	int LineNo, IntVal;
	bool IsToken;
	char *TokenName, *Text;
	float FloatVal;
	struct Node *son, *Next, *Prev;
}Node;

Node *NewNode() {
	Node *p = (Node *)malloc(sizeof(Node));
	p -> son = NULL;
	p -> Next = p -> Prev = p;
	return p;
}

void AddSon(struct Node *rt, struct Node *son) {
	if (rt -> son == NULL)	rt -> son = son;
	else {
		son -> Next = rt -> son;
		son -> Prev = rt -> son -> Prev;
		rt -> son -> Prev -> Next = son;
		rt -> son -> Prev = son;
	}
}

void PrintTree(struct Node *rt, int num) {
	int i;
	for (i = 0; i < num; ++i)	printf("  ");
	char *t = rt -> TokenName;
	printf("%s", t);
	if (rt -> IsToken) {
		if (strcmp(t, "ID") == 0) printf(": %s", rt -> Text);
		else if (strcmp(t, "INT") == 0)	printf(": %d", rt -> IntVal);
		else if (strcmp(t, "FLOAT") == 0) printf(": %.10lf", rt -> FloatVal);
		else if (strcmp(t, "TYPE") == 0) printf(": %s", rt -> Text);
		puts("");
	}	
	else {
		printf(" (%d)\n", rt -> LineNo);
		Node *p = rt -> son;
		do {
			PrintTree(p, num + 1);
			p = p -> Next;
		}while (p != rt -> son);
	}
}
#endif