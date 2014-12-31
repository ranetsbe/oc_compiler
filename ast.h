// $Id$
// Reid Anetsberger ~ ranetsbe@ucsc.edu
#ifndef __AST_H__
#define __AST_H__

#include <string>
#include <vector>
#include <unordered_set>
#include <cstdarg>
#include <limits>
#include "auxlib.h"
#include "ralib.h"
#include "astutils.h"

/*********************** terminal superclass ***********************/
class ast {
public:
  ast();
  ast(const char* lex);
  ast(int sym, const char* lex);
  ast(int sym, size_t fnum, size_t lnum, size_t offset, const char* lex);
  virtual ~ast();

/**** add methods ****/
  ast* add(ast* a);
  ast* add(ast* a, ast* b);
  ast* add(ast* a, ast* b, ast* c);
  ast* add(int n_args, ...);

/**** get methods ****/
  std::string getLex(); // return token lexinfo
  std::string getPos(); // return position string "(filenr.linenr.offset)"
  const char* getfp();  // return File Position "filename:linenr:offset:"

/**** print methods ****/
  void rec_dump(FILE* pipe, int depth); // dump subtree rooted at this
  virtual void dump_node(FILE* pipe);   // dump .ast data from this node
  void dump_tok(FILE* pipe);            // dump .tok data

/**** synthesize attributes ****/
  virtual void rec_typecheck();   // build symbol tables and syn. attributes
  virtual void absorb(ast* node); // copy filenr, linenr and offset from node

/*** codegen ***/
  virtual void dump_code(FILE* pipe); // dump i-code at the subtree rooted 
                                      // at this node
  virtual const char* rec_codegen(FILE* pipe); // generate i-code at each node

/**** node data ****/
  int symbol;                 // token symbol
  size_t filenr;              // index into filename stack
  size_t linenr;              // line number from source
  size_t offset;              // offset of token with current line
  const std::string* lexinfo; // lexical info assoc w/ token
  std::vector<ast*> children; // children nodes
};

/*********************** nonterminal superclasses ***********************/

class type_ast : public ast {
public:
  type_ast(const char* lex);
  std::string oil_name;
  std::string oil_type;
  SymbolTable* block_ptr;
};

class control_ast : public ast {
public:
  control_ast(const char* lex);
  std::string start;
  std::string end;
  int number;
  static int COUNT;
};

/*********************** nonterminals ***********************/

class root : public type_ast {
public:
  root();
  virtual void dump_code(FILE* pipe);
};

class vardecl : public type_ast {
public:
  vardecl(ast* type, ast* id, ast* op, ast* val);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
  void dump_globalcode(FILE* pipe);
  std::string getType();
  std::string getIdent();
};

class decl : public type_ast {
public:
  decl(ast* type, ast* id);
  std::string getType();
  std::string getIdent();
};

class structdef : public type_ast {
public:
  structdef(ast* id, ast* fld);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
  virtual void dump_globalcode(FILE* pipe);
  std::string getIdent();
};

class field : public type_ast {
public:
  field();
  virtual void dump_code(FILE* pipe);
};

class block : public control_ast {
public:
  block();
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
  virtual const char* rec_codegen(FILE* pipe);
  SymbolTable* block_ptr;
  SymbolTable* getBlk();
};


class func : public type_ast {
public:
  func(ast* type, ast* id, ast* p, ast* blk);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
  virtual void dump_globalcode(FILE* pipe);
  std::string getType();
  std::string getIdent();
  std::string getSig();
  SymbolTable* getBlk();
private:
  SymbolTable* block_ptr;
};

class funcreturn : public type_ast {
public:
  funcreturn();
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
};

class params : public type_ast {
public:
  params();
  virtual void rec_typecheck();
  std::string getSig();
};

class type : public type_ast {
public:
  type(ast* btype);
  type(ast* btype, ast* array);
  virtual void rec_typecheck();
  std::string getType();
};

class basetype : public type_ast {
public:
  basetype(ast* id);
  virtual void rec_typecheck();
  std::string getType();
};

class loop : public control_ast {
public:
  loop(ast* e, ast* stmt);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
};

class arguments : public type_ast {
public:
  arguments();
};

class ifelse : public control_ast {
public: 
  ifelse(ast* e, ast* stmt);
  ifelse(ast* e, ast* stmt1, ast* stmt2);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
};

/*** expressions can have "associated types" ***/

class expr : public type_ast {
public:
  expr(const char* lex);
  virtual void dump_node(FILE* pipe); // assoc types printed with debug 'z'
  std::string getType();
  std::string assoc_type;
};

class binop : public expr {
public:
  binop(ast* e1, ast* op, ast* e2);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
  virtual const char* rec_codegen(FILE* pipe);
};

class unop : public expr {
public:
  unop(ast* op, ast* e);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
  virtual const char* rec_codegen(FILE* pipe);
};

class alloc : public expr {
public:
  alloc(ast* btype);
  alloc(ast* btype, ast* tok, ast* e);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
  virtual const char* rec_codegen(FILE* pipe);
};

class call : public expr {
public:
  call(ast* id, ast* args);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
  virtual const char* rec_codegen(FILE* pipe);
};

class constant : public expr {
public:
  constant(ast* id);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
  virtual const char* rec_codegen(FILE* pipe);
};

class variable : public expr {
public:
  variable(ast* id);
  variable(ast* e1, ast* op, ast* e2);
  virtual void rec_typecheck();
  virtual void dump_code(FILE* pipe);
  virtual const char* rec_codegen(FILE* pipe);
};

//return current_scope pointer
SymbolTable* getCurrentScope();

#include "lyutils.h"


#endif
