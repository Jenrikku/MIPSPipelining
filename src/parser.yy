
// Note: Code that is added to .hpp
%code requires {
#include "parseraux.h"
using namespace parser;
}

%{

#include <stdlib.h>
#include <string>
#include <unordered_map>

#include "FlexLexer.h"
#include "parser.hpp"

yyFlexLexer *scanner;

int yylex() {
	return scanner->yylex();
}

extern void yyerror(const char * msg);

// Maps to prevent duplicated strings.
unordered_map<string, char *> labelMap;
unordered_map<string, char *> nameMap;

// Vector of all parsed instructions.
vector<instruction> code;

// Array used to manage incoming registers.
int *rlist = nullptr;

vector<instruction> parser::parse(std::istream *in) {
	scanner = new yyFlexLexer(in);
	yyparse();
	
	return code;
}

void parser::freeResources() {
	delete scanner;

	for (auto& pair : labelMap) {
		free(pair.second);
	}

	for (auto& pair : nameMap) {
		free(pair.second);
	}

	for (instruction instr : code) {
		if (instr.op.ptr) {
			switch (instr.op.type) {
				case optype::OPIM:
					delete (int *)instr.op.ptr;
					break;

				case optype::OPINDIRECT:
					delete (indirect *)instr.op.ptr;
					break;

				default:
					break;

				// Note: Labels were already freed.
			}
		}
	}

	labelMap.clear();
	nameMap.clear();
	code.clear();
	rlist = nullptr;
}

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
			string label = string($<string>1);

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
			string label = string($<string>1);

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
			string name = string($<string>1);

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
		|						{ $<op>$ = { }; }
		;

O:		  ID					{
			char *ptr = $<string>1;
			string label = string($<string>1);

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
			if ($<number>1 > 2) {
				yyerror("Error: Too many registers.");
				$<number>$ = $<number>1;
			} else {
				rlist[$<number>1] = $<number>3;
				$<number>$ = $<number>1 + 1;
			}
		}
		| R						{
			rlist = new int[3];
			rlist[0] = $<number>1;
			$<number>$ = 1;
		}
		;

%%

void yyerror(const char *error) {
  cerr << error << endl;
}
