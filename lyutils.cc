// $Id: lyutils.cc,v 1.4 2013/11/27 15:58:42 ranetsbe Exp ranetsbe $
// Reid Anetsberger ~ ranetsbe@ucsc.edu
// c file for interfacing with flex and bison

#include <vector>
#include <stack>
#include <string>
#include <cassert>
#include <ctype.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "lyutils.h"
#include "auxlib.h"

using namespace std;

FILE* pipe_tok = stderr;
ast* yyparse_ast = NULL;
int scan_linenr = 1;
int scan_offset = 0;
bool scan_echo = false;
vector<string> included_filenames;
stack<ast*> lexemes;

void scanner_openpipe (char* fname) {
   string fname_tok (fname);
   fname_tok.append (".tok");
   pipe_tok = fopen (fname_tok.c_str(), "w");
   if (pipe_tok == NULL) {
      errprintf ("%: error: failed to dump file \"%s\"\n", fname_tok.c_str());
      pipe_tok = stderr;
   }
}

void scanner_closepipe (void) {
   int fclose_rc = fclose (pipe_tok);
   if (fclose_rc != 0) 
      errprintf("%: error: pipe_tok closed with code \'%d\'.\n", fclose_rc);
}

const string* scanner_filename (int filenr) {
   return &included_filenames.at(filenr);
}

void scanner_newfilename (const char* filename) {
   included_filenames.push_back (filename);
}

void scanner_newline (void) {
   ++scan_linenr;
   scan_offset = 0;
}

void scanner_setecho (bool echoflag) {
   scan_echo = echoflag;
}

void scanner_useraction (void) {
   if (scan_echo) {
      if (scan_offset == 0) printf (";%5d: ", scan_linenr);
      printf ("%s", yytext);
   }
   scan_offset += yyleng;
}

void yyerror (const char* message) {
   assert (not included_filenames.empty());
   errprintf ("%:%s: %d: %s\n", included_filenames.back().c_str(),
              scan_linenr, message);
}

void scanner_badchar (unsigned char bad) {
   char char_rep[16];
   sprintf (char_rep, isgraph (bad) ? "%c" : "\\%03o", bad);
   errprintf ("%:%s: %d: invalid source character (%s)\n",
              included_filenames.back().c_str(), scan_linenr, char_rep);
}

void scanner_badtoken (const char* message, char* lexeme) {
   errprintf ("%:%s: %d: %s (%s)\n", included_filenames.back().c_str(),
              scan_linenr, message, lexeme);
}

int yylval_token (int symbol) {
   int offset = scan_offset - yyleng;
   yylval = new ast (symbol, included_filenames.size() - 1, scan_linenr, 
                     offset, yytext);
   yylval->dump_tok (pipe_tok);
   lexemes.push(yylval);  // keep track of lexemes for deletion
   return symbol;
}

void scanner_include (void) {
   scanner_newline();
   char filename[strlen (yytext) + 1];
   int linenr;
   int scan_rc = sscanf (yytext, "# %d \"%[^\"]\"", &linenr, filename);
   if (scan_rc != 2) {
      errprintf ("%: %d: [%s]: invalid directive, ignored\n", scan_rc, yytext);
   }else {
      fprintf (pipe_tok, ";# %d \"%s\"\n", linenr, filename);
      scanner_newfilename (filename);
      scan_linenr = linenr - 1;
      DEBUGF ('s', "filename=%s, scan_linenr=%d\n",
              included_filenames.back().c_str(), scan_linenr);
   }
}

void free_ast (void) {
   while (!lexemes.empty()) {
      lexemes.pop();
   }
}
