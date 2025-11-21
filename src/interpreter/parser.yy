
// Note: Code that is added to .hpp
%code requires {
#include "interpreter.h"
}

%{

#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "FlexLexer.h"
#include "parser.hpp"

yyFlexLexer *scanner;

int yylex() {
	return scanner->yylex();
}

extern void yyerror(const char * msg);

// Maps to prevent duplicated strings.
std::unordered_map<std::string, char *> labelMap;
std::unordered_map<std::string, char *> nameMap;

// Vector of all parsed instructions.
std::vector<instruction> code;

// Array used to manage incoming registers.
int *rlist = nullptr;

%}

%union {
	char *string;
	int number;
	operand op;
	instruction instr;
};

%token LABEL
%token R
%token IM
%token LPAREN
%token RPAREN
%token SEPARATOR
%token NEXT
%token ID

%start E

%%

E:		  E NEXT L				{
			instruction i = $<instr>3;

			if (i.name) // Only add to code if not empty.
				code.push_back(i);
		}
		| L						{
			instruction i = $<instr>1;

			if (i.name) // Only add to code if not empty.
				code.push_back(i);
		}
		;

L:		  LABEL I				{
			instruction i = $<instr>2;
			std::string label = std::string($<string>1);

			if (labelMap.contains(label)) {
				i.label = labelMap[label];
				free($<string>1);
			} else {
				labelMap[label] = $<string>1;
				i.label = $<string>1;
			}

			$<instr>$ = i;
		}
		| LABEL NEXT I			{ // Allows declaring only label in line
			instruction i = $<instr>3;
			std::string label = std::string($<string>1);

			if (labelMap.contains(label)) {
				i.label = labelMap[label];
				free($<string>1);
			} else {
				labelMap[label] = $<string>1;
				i.label = $<string>1;
			}

			$<instr>$ = i;
		}
		| I						{ $<instr>$ = $<instr>1; }
		|						{ $<instr>$ = { }; } // Allows empty lines
		;

I:		  ID C					{
			instruction i = $<instr>2;
			std::string name = std::string($<string>1);

			if (nameMap.contains(name)) {
				i.name = nameMap[name];
				free($<string>1);
			} else {
				nameMap[name] = $<string>1;
				i.name = $<string>1;
			}

			$<instr>$ = i;
		}
		;

C:		  RLIST AUX				{
			$<instr>$ = { .rlist = rlist, .rcount = $<number>1, .op = $<op>2 };
		}
		| O						{ $<instr>$ = { .op = $<op>1 }; }
		|						{ $<instr>$ = { }; }
		;

AUX:	  SEPARATOR O			{ $<op>$ = $<op>2; }
		|
		;

O:		  ID					{
			char *ptr = $<string>1;
			std::string label = std::string($<string>1);

			if (labelMap.contains(label)) {
				free(ptr);
				ptr = labelMap[label];
			}

			$<op>$ = { .type = optype::OPLABEL, .ptr = ptr };
		}
		| IM LPAREN R RPAREN	{
			indirect *ptr = new indirect{ .im = $<number>1, .reg = $<number>3 };
			$<op>$ = { .type = optype::OPINDIRECT, .ptr = ptr };
		}
		| IM					{
			$<op>$ = { .type = optype::OPIM, .ptr = new int{$<number>1} };
		}
		;

RLIST:	  RLIST SEPARATOR R		{
			rlist[$<number>1] = $<number>3;
			$<number>$ = $<number>1 + 1;
		}
		| R						{
			rlist = new int[3];
			rlist[0] = $<number>1;
			$<number>$ = 1;
		}
		;

%%

void yyerror(const char *error) {
  std::cerr << error << std::endl;
}

void printInstr(instruction instr) {
	if (instr.label) std::cout << instr.label << ':';
	std::cout << '\t';
	if (instr.name) std::cout << instr.name << '\t';

	for (int i = 0; i < instr.rcount;) {
		std::cout << '$' << instr.rlist[i];
		if (++i < instr.rcount) std::cout << ", ";
	}

	if (instr.op.ptr) {
		if (instr.rcount) std::cout << ", ";

		switch (instr.op.type) {
			case optype::OPLABEL:
				std::cout << (char *)instr.op.ptr;
				break;

			case optype::OPINDIRECT: {
				indirect *indir = (indirect *)instr.op.ptr;
				std::cout << indir->im << "($" << indir->reg << ')';
				break;
			}

			case optype::OPIM:
				std::cout << *(int *)instr.op.ptr;
				break;

			default:
				std::cout << "UnkOP";
				break;
		}
	}

	std::cout << std::endl;
}

int main() {
	scanner = new yyFlexLexer(&std::cin);
	yyparse();

	std::for_each(code.begin(), code.end(), printInstr);
}
