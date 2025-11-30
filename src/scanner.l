%option noyywrap
%option case-insensitive

%{
#include "parser.hpp"
#include <string>
#include <cctype>

extern void yyerror(const char *);

void toUpper(char *str) {
	while (*str) {
		*str++ = std::toupper((unsigned char) *str);
	}
}
%}

comment		[\/@#;].*
ignore		[ \t\r]+
label		^.+\:
r			[$R][0-9][0-9]?
im			-?[0-9]+
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
	toUpper(str); // Convert to uppercase
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
	char *str = strdup(yytext);
	toUpper(str); // Convert to uppercase
	yylval.string = str;
	return ID;
}

.				{
	yyerror("Error. Incorrect data found: ");
	yyerror(yytext);
}
%%
