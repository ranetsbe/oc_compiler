// $Id$
// Reid Anetsberger ~ ranetsbe@ucsc.edu

#ifndef __LYUTILS_H__
#define __LYUTILS_H__

// Lex and Yacc interface utility.

#include <cstdio>

#include "ast.h"
#include "auxlib.h"

#define YYEOF 0

extern FILE* yyin;                // file pipe 
extern ast* yyparse_ast;          // ast root node
extern int yyin_linenr;           // linenr of current file
extern char* yytext;              // pointer to current line
extern int yy_flex_debug;         // flex debug flag
extern int yydebug;               // bison debug flag
extern size_t yyleng;

int yylex (void);
int yyparse (void);
void yyerror (const char* message);
int yylex_destroy (void);
const char* get_yytname (int symbol);
bool is_defined_token (int symbol);

void scanner_openpipe (char* fname);
void scanner_closepipe (void);
const std::string* scanner_filename (int filenr);
void scanner_newfilename (const char* filename);
void scanner_badchar (unsigned char bad);
void scanner_badtoken (const char* message, char* lexeme);
void scanner_newline (void);
void scanner_setecho (bool echoflag);
void scanner_useraction (void);
void free_ast (void);

// scanner action after recognizing a token
int yylval_token (int symbol);

void scanner_include (void);

typedef ast* ast_ptr;
#define YYSTYPE ast_ptr
#include "yyparse.h"

#endif
