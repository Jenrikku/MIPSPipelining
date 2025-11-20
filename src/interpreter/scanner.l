%option noyywrap
%option case-insensitive

%{
#include "parser.hpp"
#include <string>

extern void yyerror(const char *);
%}

comment		[\/@#;].*
ignore		[ \t\r]+
label		^.+\:
r			[$Rr][0-9][0-9]?
im			-?[1-9][0-9]*
lparen		\(
rparen		\)
separator	\,
next		\n+
id			[A-Z]+

%%
{comment}		{ } // Ignore comments
{ignore}		{ }

{label}			{
	char *str = strdup(yytext);
	str[strlen(str) - 1] = 0; // Remove colon (:)
	yylval.string = str;
	return LABEL;
}

{r}				{
	yylval.number = atoi(yytext + 1);
	return R;
}

{im}			{
	yylval.number = atoi(yytext);
	return IM;
}

{lparen}		{ return LPAREN; }
{rparen}		{ return RPAREN; }
{separator}		{ return SEPARATOR; }
{next}			{ return NEXT; }

{id}			{
	yylval.string = strdup(yytext);
	return ID;
}

.				{
	yyerror("Error. Incorrect data found: ");
	yyerror(yytext);
}
%%
