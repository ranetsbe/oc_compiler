%{
//$Id: scanner.l,v 1.4 2013/11/27 15:58:42 ranetsbe Exp ranetsbe $
// Reid Anetsberger ~ ranetsbe@ucsc.edu

#include "auxlib.h"
#include "lyutils.h"

#define YY_USER_ACTION  { scanner_useraction (); }
#define IGNORE(THING)   { }

%}

%option 8bit
%option debug
%option ecs
%option nodefault
%option nounput
%option noyywrap
%option perf-report
%option verbose
%option warn

LETTER          ([A-Za-z_])
DIGIT           ([0-9])
CHAR            ([^\\'\n]|\\[\\'"0nt])
STRING          ({CHAR}*)
INTEGER         ({DIGIT}+)
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
CHARCON         ('{CHAR}')
STRINGCON       (\"([^\\"\n]|\\[\\'"0nt])*\")

NOTINTEGER      ({INTEGER}{LETTER}+)
NOTCHARCON      ('{CHAR}{2,}')
UNTERMCHARCON   ('{CHAR}+[^\n])
NOTSTRCON       (\"[^\n\"]*\")
UNTERMSTRCON    (\"[^\n\"]*[^\n])

%%

"#".*           { scanner_include(); }
[ \t]+          { IGNORE (white space) }
\n              { scanner_newline(); }

"void"          { return yylval_token (VOID); }
"bool"          { return yylval_token (BOOL); }
"char"          { return yylval_token (CHAR); }
"int"           { return yylval_token (INT); }
"string"        { return yylval_token (STRING); }
"struct"        { return yylval_token (STRUCT); }
"if"            { return yylval_token (IF); }
"else"          { return yylval_token (ELSE); }
"while"         { return yylval_token (WHILE); }
"return"        { return yylval_token (RETURN); }
"new"           { return yylval_token (NEW); }
"false"         { return yylval_token (TOK_FALSE); }
"true"          { return yylval_token (TOK_TRUE); }
"null"          { return yylval_token (TOK_NULL); }
"ord"           { return yylval_token (ORD); }
"chr"           { return yylval_token (CHR); }

{INTEGER}       { return yylval_token (INTCON); }
{IDENT}         { return yylval_token (IDENT); }
{CHARCON}       { return yylval_token (CHARCON); }
{STRINGCON}     { return yylval_token (STRINGCON); }

"[]"            { return yylval_token (ARRAY); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }
";"             { return yylval_token (';'); }
","             { return yylval_token (','); }
"."             { return yylval_token ('.'); }
"=="            { return yylval_token (EQ); }
"="             { return yylval_token ('='); }
"!="            { return yylval_token (NE); }
"<="            { return yylval_token (LE); }
"<"             { return yylval_token (LT); }
">="            { return yylval_token (GE); }
">"             { return yylval_token (GT); }
"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"%"             { return yylval_token ('%'); }
"!"             { return yylval_token ('!'); }

{NOTINTEGER}    { scanner_badtoken ("unrecognized numeric", yytext);
                  return yylval_token (INTCON); }

{NOTCHARCON}    { scanner_badtoken ("unrecognized character constant", yytext);
                  return yylval_token (CHARCON); }

{UNTERMCHARCON} { scanner_badtoken ("unterminated character constant", yytext);
                  return yylval_token (CHARCON); }

{NOTSTRCON}     { scanner_badtoken ("unrecognized string constant", yytext);
                  return yylval_token (STRINGCON); }

{UNTERMSTRCON}  { scanner_badtoken ("unterminated string constant", yytext);
                  return yylval_token (STRINGCON); }

.               { scanner_badchar (*yytext); }

%%

