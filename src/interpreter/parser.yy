
// Note: Code that is added both to .hpp and .cpp
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

%}

%union {
	char *string;
	int *array;
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

C:		  RLIST AUX				{ $<instr>$ = { .rlist = $<array>1, .op = $<op>2 }; }
		| O						{ $<instr>$ = { .op = $<op>1 }; }
		|						{ $<instr>$ = { }; }
		;

AUX:	  SEPARATOR O			{ $<op>$ = $<op>2; }
		|
		;

O:		  LABEL					{
			char *ptr = $<string>1;
			std::string label = std::string($<string>1);

			if (labelMap.contains(label)) {
				free(ptr);
				ptr = labelMap[label];
			}

			$<op>$ = { .type = optype::OPLABEL, .ptr = ptr };
			std::cout << "LABEL as operand: " << $<string>1;
		}
		| IM LPAREN R RPAREN	{
			indirect *ptr = new indirect{ .im = $<number>1, .reg = $<number>3 };
			$<op>$ = { .type = optype::OPINDIRECT, .ptr = ptr };
			std::cout << "INDIRECT: " << $<number>1 << " + $" << $<number>3;
		}
		| IM					{
			$<op>$ = { .type = optype::OPIM, .ptr = new int{$<number>1} };
			std::cout << "IM: " << $<number>1;
		}
		;

RLIST:	  RLIST SEPARATOR R		{
			int oldsize = sizeof($<array>1) / sizeof(int);

			$<array>$ = new int[oldsize + 1];
			std::memcpy($<array>$, $<array>1, oldsize);
			delete[] $<array>1;

			$<array>$[oldsize] = $<number>3;
			std::cout << "R: $" << $<number>3;
		}
		| R						{
			$<array>$ = new int[1];
			$<array>$[0] = $<number>1;
			std::cout << "R: $" << $<number>1;
		}
		;

%%

void yyerror(const char *error) {
  std::cerr << error << std::endl;
}

int main() {
  scanner = new yyFlexLexer(&std::cin);
}
