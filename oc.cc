// $Id$
// Reid Anetsberger ~ ranetsbe@ucsc.edu

/* debug flags
 * x - options / flags / args
 * v - main
 * s - scanner
 * p - parser
 * z - symbol table
 * c - trace i-code generation
 * i - dump oil to stderr
 */

#include "oc.h"

using namespace std;

bool opt_D = false;                  // cpp define flag
string cpp_define = "-D";            // define this str
string cpp_command = "/usr/bin/cpp"; // cpp path
extern ast_ptr yyparse_ast;          // abstract syntax tree root pointer

// global symbol table
SymbolTable global_scope(NULL);

int main (int argc, char **argv) {
   // set executable name
   set_execname (argv[0]);

   // too few args
   if (argc < 2) {
      errprint_usage();
      EXIT();
   }

   // scan args and perform option & flag setup
   const char* fname = scan_opts (argc, argv);

   // setup basename
   char temp[strlen(fname) + 1];
   strcpy(temp, fname);
   char *bname = basename(temp);
   bname = remove_ext(bname, ".oc"); // file name w/o ext
   DEBUGF ('v', "bname=%s\n", bname);
   if (bname == NULL){
      errprintf ("%: error: \'%s\' must have extension \'.oc\'\n", fname);
      EXIT();
   }

   // open the oc file with cpp and assign the pipe to yyin
   cpp_popen (fname);
   
   // enable dump to .tok file 
   scanner_openpipe (bname);

   DEBUGF ('v', "filename = %s, yyin = %p, fileno (yyin) = %d\n",
           fname, yyin, fileno (yyin));

   // call yyparse to parse file
   yyparse();

   // close yyin
   cpp_pclose();
   scanner_closepipe();

   DEBUGSTMT ('s', dump_stringset (stderr); );

   // dump stringset to .str file
   dumpfile_str (bname);

   // dump abstract syntax tree to .ast file
   dumpfile_ast (bname);
   
   if (get_exitstatus() == EXIT_SUCCESS){
   
     // synthesize attributes, build symbol tables and perform typechecking
     yyparse_ast->rec_typecheck();
     
     DEBUGSTMT ('z', global_scope.dump(stderr, 0); );
     // dump symbol table to .sym file
     dumpfile_sym (bname);

     if (get_exitstatus() == EXIT_SUCCESS){
       DEBUGSTMT ('i', yyparse_ast->dump_code(stderr); );
       // dump intermediate code to .oil file
       dumpfile_oil(bname);
     }
     if (get_exitstatus() == EXIT_SUCCESS){
       char* data;
       asprintf(&data, "gcc -g -o %s -x c %s.oil oclib.c", bname, bname);
       system(data);
     }
   }
   
   // cleanup
   yylex_destroy();

   return get_exitstatus();
}


// open a pipe from CPP and assign it to yyin
void cpp_popen (const char* filename) {
   if (opt_D) {
      cpp_command.append (" ");
      cpp_command.append (cpp_define);
   }
   cpp_command.append (" ");
   cpp_command.append (filename);
   yyin = popen (cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (cpp_command.c_str());
      EXIT ();
   }
}


// close yyin
void cpp_pclose (void) {
   int pclose_rc = pclose (yyin);
   if (pclose_rc != 0) 
      errprintf("%: error: cpp closed with code \'%d\'.\n", 
                pclose_rc);
}


// scan options and setup oc
const char* scan_opts (int argc, char** argv) {
   // opterr = 0; // disable getopt(3) error messages
   yy_flex_debug = 0;
   yydebug = 0;
   while (true) {
      int opt = getopt (argc, argv, "@:D:ly");
      if (opt == EOF) break;
      switch (opt) {
         case '@': set_debugflags (optarg);                            break;
         case 'D': opt_D = true; cpp_define.append (optarg);           break;
         case 'l': yy_flex_debug = 1;                                  break;
         case 'y': yydebug = 1;                                        break;
         default: errprintf ("%: unrecognized option (%c)\n", optopt); break;
      }
   }
   if (optind >= argc || get_exitstatus() == EXIT_FAILURE) {
      errprint_usage();
      EXIT();
   }
   const char *filename = (optind == argc ? NULL : argv[argc - 1]);
   return filename;
}


// dump stringset to .str file
void dumpfile_str (char* bname) {
   string fname_str (bname);
   fname_str.append (".str");
   FILE *outfile_str = fopen (fname_str.c_str(), "w");
   dump_stringset (outfile_str);
   fclose (outfile_str);
}


// dump abstract syntax tree to .ast file
void dumpfile_ast (char* bname) {
   string fname_ast = (bname);
   fname_ast.append (".ast");
   FILE *outfile_ast = fopen (fname_ast.c_str(), "w");
   yyparse_ast->rec_dump(outfile_ast, 0);
   fclose (outfile_ast);
}


// dump symbol table to .sym file
void dumpfile_sym (char* bname) {
   // dump symbol table to .sym file
   string fname_sym = (bname);
   fname_sym.append (".sym");
   FILE *outfile_sym = fopen (fname_sym.c_str(), "w");
   global_scope.dump(outfile_sym, 0);
   fclose (outfile_sym);
}


// dump intermediate code to .oil file
void dumpfile_oil (char* bname) {
   string fname_oil (bname);
   fname_oil.append (".oil");
   FILE *outfile_oil = fopen (fname_oil.c_str(), "w");
   yyparse_ast->dump_code(outfile_oil);
   fclose (outfile_oil);
}
