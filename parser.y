%{
// $Id: parser.y,v 1.4 2013/11/27 15:58:42 ranetsbe Exp ranetsbe $
// Reid Anetsberger ~ ranetsbe@ucsc.edu

#include <cassert>
#include "lyutils.h"
#include "ast.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

/* regular tokens */
%token VOID      BOOL      CHAR      INT       STRING
%token IF        ELSE      WHILE     RETURN    STRUCT
%token ARRAY     IDENT     TOK_TRUE  TOK_NULL  TOK_FALSE 
%token STRINGCON INTCON    CHARCON   ERROR

/* nonterminal and root */
%token NT ROOT

/* dangling else */
%right ELSE "then"

/* binary ops */
%right '='
%left  EQ NE LT LE GT GE
%left  '+' '-'
%left  '*' '/' '%'

/* unary ops */
%right POS NEG '!' ORD CHR
%left  NEW

/* other */
%left  ARRAY
%right '[' '.' '{' '('
%nonassoc "low"
%nonassoc "high"

%start program

%%

program   : program stmt        { $$ = $1->add($2); }
          | program function    { $$ = $1->add($2); }
          | program structdef   { $$ = $1->add($2); }
          | program error       { $$ = $1->add($2); }
          |                     { $$ = new root(); }
          ;

structdef : STRUCT IDENT '{' field '}' { $$ = new structdef($2, $4); }
          ;

field     : decl ';' field      { $$ = $3->add($1); }
          |                     { $$ = new field(); }
          ;

function  : type IDENT '(' params ')' block { $$ = new func($1, $2, $4, $6); }
          ;

params    : paramlist           { $$ = $1; }
          |                     { $$ = new params(); }
          ;

paramlist : paramlist ',' decl  { $$ = $1->add($3); }
          | decl                { $$ = new params(); $$->add($1); }
          ;

block     : '{' stmtseq '}'     { $$ = $2; }
          | '{' '}'             { $$ = new block(); }
          | ';'                 { $$ = new block(); }
          ;

stmtseq   : stmtseq stmt        { $$ = $1->add($2); } 
          | stmt                { $$ = new block(); $$->add($1); }
          ;

stmt      : block               { $$ = $1; }
          | vardecl             { $$ = $1; }
          | while               { $$ = $1; }
          | ifelse              { $$ = $1; }
          | return              { $$ = $1; }
          | expr ';'            { $$ = $1; }
          ;

vardecl   : type IDENT '=' expr ';' { $$ = new vardecl($1, $2, $3, $4); }
          ;

decl      : type IDENT          { $$ = new decl($1, $2); }
          ;

type      : basetype            { $$ = new type($1); }
          | basetype ARRAY      { $$ = new type($1, $2); }
          ;

basetype  : VOID                { $$ = new basetype($1); }
          | BOOL                { $$ = new basetype($1); }
          | CHAR                { $$ = new basetype($1); }
          | INT                 { $$ = new basetype($1); }
          | STRING              { $$ = new basetype($1); }
          | IDENT               { $$ = new basetype($1); }
          ;

while     : WHILE '(' expr ')' stmt { $$ = new loop($3, $5); }
          ;

ifelse    : IF '(' expr ')' stmt %prec "then" { $$ = new ifelse($3, $5); }
          | IF '(' expr ')' stmt ELSE stmt    { $$ = new ifelse($3, $5, $7); } 
          ;

return    : RETURN ';'          { $$ = new funcreturn(); $$->absorb($1); }
          | RETURN expr ';'     { $$ = new funcreturn(); $$->add($2); $$->absorb($1); }
          ;

expr      : variable            { $$ = $1; }
          | constant            { $$ = $1; }
          | allocator           { $$ = $1; }
          | call                { $$ = $1; }
          | unop                { $$ = $1; }
          | binop               { $$ = $1; }
          | '(' expr ')'        { $$ = $2; }
          ;

binop     : expr '=' expr       { $$ = new binop($1, $2, $3); }
          | expr EQ expr        { $$ = new binop($1, $2, $3); }
          | expr NE expr        { $$ = new binop($1, $2, $3); }
          | expr GT expr        { $$ = new binop($1, $2, $3); }
          | expr GE expr        { $$ = new binop($1, $2, $3); }
          | expr LT expr        { $$ = new binop($1, $2, $3); }
          | expr LE expr        { $$ = new binop($1, $2, $3); }
          | expr '+' expr       { $$ = new binop($1, $2, $3); }
          | expr '-' expr       { $$ = new binop($1, $2, $3); }
          | expr '*' expr       { $$ = new binop($1, $2, $3); }
          | expr '/' expr       { $$ = new binop($1, $2, $3); }
          | expr '%' expr       { $$ = new binop($1, $2, $3); }
          ;

unop      : '+' expr %prec POS  { $$ = new unop($1, $2); }
          | '-' expr %prec NEG  { $$ = new unop($1, $2); }
          | '!' expr %prec '!'  { $$ = new unop($1, $2); }
          | ORD expr %prec ORD  { $$ = new unop($1, $2); }
          | CHR expr %prec CHR  { $$ = new unop($1, $2); }
          ;

allocator : NEW basetype '(' expr ')' { $$ = new alloc($2, $3, $4); }
          | NEW basetype '[' expr ']' { $$ = new alloc($2, $3, $4); }
          | NEW basetype '(' ')'      { $$ = new alloc($2); }
          ;

call      : IDENT '(' args ')'  { $$ = new call($1, $3); }
          ;

args      : arglist             { $$ = $1; }
          |                     { $$ = new arguments(); }
          ;

arglist   : arglist ',' expr    { $$ = $1->add($3); }
          | expr                { $$ = new arguments(); $$->add($1); }
          ;

variable  : IDENT               { $$ = new variable($1); }
          | expr '[' expr ']'   { $$ = new variable($1, $2, $3); }
          | expr '.' IDENT      { $$ = new variable($1, $2, $3); }
          ;

constant  : INTCON              { $$ = new constant($1); }
          | CHARCON             { $$ = new constant($1); }
          | STRINGCON           { $$ = new constant($1); }
          | TOK_FALSE           { $$ = new constant($1); }
          | TOK_TRUE            { $$ = new constant($1); }
          | TOK_NULL            { $$ = new constant($1); }
          ;

%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}
