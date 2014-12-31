// $Id: oc.h,v 1.4 2013/11/27 15:58:42 ranetsbe Exp ranetsbe $
// Reid Anetsberger ~ ranetsbe@ucsc.edu

#ifndef __OC_H__
#define __OC_H__

#include <iostream>
#include <string>
#include <cstdio>
#include <cassert>
#include <errno.h>
#include <cstdlib>
#include <cstring>
#include <libgen.h>
#include <unistd.h>

#include "ast.h"
#include "lyutils.h"
#include "stringset.h"
#include "yyparse.h"
#include "auxlib.h"
#include "symtable.h"
#include "ralib.h"

// open a pipe from CPP and assign it to yyin
void cpp_popen (const char* filename);

// close yyin
void cpp_pclose (void);

// scan options and setup oc
const char* scan_opts (int argc, char **argv);

// dump stringset to .str file
void dumpfile_str (char* bname);

// dump abstract syntax tree to .ast file
void dumpfile_ast (char* bname);

// dump symbol table to .sym file
void dumpfile_sym (char* bname);

// dump intermediate code to .oil file
void dumpfile_oil (char* bname);

#define EXIT()                                                        \
        eprintf ("%: compilation terminated\n");                      \
        exit (get_exitstatus());

#endif
