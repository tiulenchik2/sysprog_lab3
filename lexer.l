%{
#include "ast.h"
#include "parser.tab.h"
%}
%option noyywrap yylineno
%%
"int"       return INT;
"if"        return IF;
"else"      return ELSE;
"while"     return WHILE;
"print"     return PRINT;
"=="        return EQ;
"!="        return NE;
"<="        return LE;
">="        return GE;
"&&"        return AND;
"||"        return OR;
[0-9]+      { yylval.num = atoi(yytext); return NUM; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return ID; }
"//".*      ;
[ \t\r\n]+  ;
.           return yytext[0];
%%
