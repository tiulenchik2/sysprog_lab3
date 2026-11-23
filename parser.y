%{
#include "ast.h"
int yylex(void);
extern int yylineno;
void yyerror(const char *s) { fprintf(stderr, "Error line %d: %s\n", yylineno, s); exit(1); }
Node *root;
%}

%union { int num; char *str; Node *node; }

%token <num> NUM
%token <str> ID
%token INT IF ELSE WHILE PRINT EQ NE LE GE AND OR
%type <node> stmts stmt expr

%nonassoc IFX
%nonassoc ELSE
%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right UMINUS '!'

%%
program: stmts                          { root = mk("program", $1, 0, 0); }
       ;

stmts: /* empty */                      { $$ = 0; }
     | stmt stmts                       { $1->next = $2; $$ = $1; }
     ;

stmt: INT ID ';'                        { $$ = var("decl", $2, 0); }
    | INT ID '=' expr ';'               { $$ = var("decl", $2, $4); }
    | ID '=' expr ';'                   { $$ = var("=", $1, $3); }
    | PRINT '(' expr ')' ';'            { $$ = mk("print", $3, 0, 0); }
    | IF '(' expr ')' stmt %prec IFX    { $$ = mk("if", $3, $5, 0); }
    | IF '(' expr ')' stmt ELSE stmt    { $$ = mk("if", $3, $5, $7); }
    | WHILE '(' expr ')' stmt           { $$ = mk("while", $3, $5, 0); }
    | '{' stmts '}'                     { $$ = mk("block", $2, 0, 0); }
    ;

expr: NUM                               { $$ = num($1); }
    | ID                                { $$ = var("var", $1, 0); }
    | expr '+' expr                     { $$ = mk("+", $1, $3, 0); }
    | expr '-' expr                     { $$ = mk("-", $1, $3, 0); }
    | expr '*' expr                     { $$ = mk("*", $1, $3, 0); }
    | expr '/' expr                     { $$ = mk("/", $1, $3, 0); }
    | expr '%' expr                     { $$ = mk("%", $1, $3, 0); }
    | expr '<' expr                     { $$ = mk("<", $1, $3, 0); }
    | expr '>' expr                     { $$ = mk(">", $1, $3, 0); }
    | expr LE expr                      { $$ = mk("<=", $1, $3, 0); }
    | expr GE expr                      { $$ = mk(">=", $1, $3, 0); }
    | expr EQ expr                      { $$ = mk("==", $1, $3, 0); }
    | expr NE expr                      { $$ = mk("!=", $1, $3, 0); }
    | expr AND expr                     { $$ = mk("&&", $1, $3, 0); }
    | expr OR expr                      { $$ = mk("||", $1, $3, 0); }
    | '-' expr %prec UMINUS             { $$ = mk("neg", $2, 0, 0); }
    | '!' expr                          { $$ = mk("!", $2, 0, 0); }
    | '(' expr ')'                      { $$ = $2; }
    ;
%%
