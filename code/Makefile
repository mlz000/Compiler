TESTFILE = $(shell find -name "*.cmm")

all:
	bison -d syntax.y
	flex lexical.l
	gcc syntax.tab.c -lfl -ly -o parser -g