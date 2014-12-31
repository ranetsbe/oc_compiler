// $Id$
// Reid Anetsberger ~ ranetsbe@ucsc.edu
#include <stdlib.h>
#include <cstring>
#include <cstdio>
#include <utility>
#include <list>

#include "stringset.h"
#include "lyutils.h"
#include "symtable.h"
#include "ast.h"

using namespace std;

extern SymbolTable global_scope;
SymbolTable* current_scope = &global_scope;
vector<structdef*> global_structdefs;
vector<vardecl*> global_vardecls;
vector<func*> global_funcs;

/*********************** superclass ***********************/
ast::ast(const char* lex) : symbol(NT), filenr(0), linenr(0), offset(0), 
         lexinfo(intern_stringset(lex)), children() {}
         
ast::ast(int sym, const char* lex) : symbol(sym), filenr(0), linenr(0), 
         offset(0), lexinfo(intern_stringset(lex)), children() {}

ast::ast(int sym, size_t fnum, size_t lnum, size_t offset, const char* lex) :
         symbol(sym), filenr(fnum), linenr(lnum), offset(offset),
         lexinfo(intern_stringset(lex)), children() {}

ast::~ast() {
  delete lexinfo;
  while(!children.empty()) {
    delete children.back();
    children.back() = NULL;
    children.pop_back();
  }
}

/**** add methods ****/
ast* ast::add(ast* a) {
  children.push_back(a);
  return this;
}

ast* ast::add(ast* a, ast* b) {
  children.push_back(a);
  children.push_back(b);
  return this;
}

ast* ast::add(ast* a, ast* b, ast* c) {
  children.push_back(a);
  children.push_back(b);
  children.push_back(c);
  return this;
}

ast* ast::add(int n_args, ...) {
   va_list ap;
   va_start(ap, n_args);
   for (int j = 0; j < n_args; j++)
      children.push_back(va_arg(ap, ast*));
   va_end(ap);
   return this;
}

/**** get methods ****/
// return lexinfo
string ast::getLex() {
  return string(*lexinfo);
}

// return position string "(filenr.linenr.offset)"
string ast::getPos() {
  char* data;
  asprintf(&data, "(%s.%s.%s)", itos(filenr).c_str(), itos(linenr).c_str(), 
           itos(offset).c_str());
  return string(data);
}

// return file position "filename:linenr:offset:"
const char* ast::getfp() {
  char* data;
  asprintf(&data, "%s:%s:%s:", scanner_filename(filenr)->c_str(), 
           itos(linenr).c_str(), itos(offset).c_str());
  return string(data).c_str();
}

/**** print methods ****/
void ast::rec_dump(FILE* pipe, int depth) {
  fprintf(pipe, "%*s", depth * 2, "");
  dump_node(pipe);
  fprintf (pipe, "\n");
  for (size_t i = 0; i < children.size(); i++) {
    children[i]->rec_dump(pipe, depth + 1);
  }
}

void ast::dump_node(FILE* pipe) {
  if (symbol == NT || symbol == ROOT)
    fprintf(pipe, "%s", lexinfo->c_str());
  else
    fprintf(pipe, "%s (%s)", get_yytname (symbol), lexinfo->c_str());
}

void ast::dump_tok(FILE* pipe) {
  fprintf(pipe, "%4lu %3lu.%.3lu  %3d  %-16s  (%s)\n", filenr, linenr,
          offset, symbol, get_yytname(symbol), lexinfo->c_str());
}

// recusively descend tree to build symbol tables and typecheck
void ast::rec_typecheck() {
  std::vector<ast*>::iterator it;
  for (it = children.begin(); it != children.end(); ++it) {
    (*it)->rec_typecheck();
  }
}

// inherit filenr, linenr and offset from node
void ast::absorb(ast* node) {
  filenr = node->filenr;
  linenr = node->linenr;
  offset = node->offset;
}

void ast::dump_code(FILE* pipe) { 
  DEBUGSTMT('c', fprintf(pipe, "/* DEBUG ~ ast dumpcode */\n"); ); 

  errprintf("ast::dump_code\n");
}

// recusively descend tree to generate intermediate code
const char* ast::rec_codegen(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* DEBUG ~ ast codegen */\n"); ); 
  return "[TODO]";
}

/***********************  nonterminal superclasses  ***********************/

type_ast::type_ast(const char* lex) : ast(lex), oil_name(""), oil_type(""),
                                      block_ptr(NULL) {}


int control_ast::COUNT = 0;
control_ast::control_ast(const char* lex) : ast(lex), start("undef"), end("undef") {
  number = COUNT++;
}


/***********************  AST root  ***********************/
root::root() : type_ast("program") { yyparse_ast = this; }

void root::dump_code(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* root */\n"); );


  fprintf(pipe, "#define __OCLIB_C__\n#include \"oclib.oh\"\n\n");
  // dump structs into global scope
  std::vector<structdef*>::iterator it;
  for (it = global_structdefs.begin(); it != global_structdefs.end(); ++it) {
    (*it)->dump_globalcode(pipe);
  }
  // dump global variables into global scope
  std::vector<vardecl*>::iterator it1;
  for (it1 = global_vardecls.begin(); it1 != global_vardecls.end(); ++it1) {
    (*it1)->dump_globalcode(pipe);
  }
  // dump function definitions
  std::vector<func*>::iterator it2;
  for (it2 = global_funcs.begin(); it2 != global_funcs.end(); ++it2) {
    (*it2)->dump_globalcode(pipe);
  }
  
  // dump everything else into __ocmain
  emit(pipe, "\nvoid __ocmain ()\n{\n");
  setIndent(true);
  std::vector<ast*>::iterator it3;
  for (it3 = children.begin(); it3 != children.end(); ++it3) {
    (*it3)->dump_code(pipe);
  }
  fprintf(pipe, "}\n");
  setIndent(false);
}


/***********************  variable declaration  ***********************/
vardecl::vardecl(ast* type, ast* id, ast* op, ast* val) : type_ast("vardecl") {
  add(4, type, id, op, val);
  absorb(id);
}

void vardecl::rec_typecheck() {
  ast::rec_typecheck();
  
  if(current_scope == &global_scope)
    global_vardecls.push_back(this); // add this ptr to global variables

  block_ptr = current_scope;
  string vartype = getType();
  string valtype = static_cast<expr*>(children[3])->getType();
  DEBUGSTMT('z', eprintf("vardecl\n"); );
  if (getType().compare("void") == 0)
    errprintf("%s error: variables of type void are not allowed.\n", 
              getfp());
  else if (!typecheck(vartype, valtype)) {
    errprintf("%s error: declared type (%s)", getfp(), vartype.c_str());
    errprintf(" is incompatible with assigned type (%s).\n", valtype.c_str());
  }
  current_scope->addSymbol(getIdent(), getType(), getPos());
}

void vardecl::dump_code(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* vardecl */\n"); ); 
  
  size_t blocknr = block_ptr->getNumber();
  type_ast *val = static_cast<type_ast*>(children[3]);
  if (blocknr != 0) { // emit "local scope" code
    oil_type = getOilType(getType());
    oil_name = mangle(blocknr, getIdent());
    emit(pipe, "%s %s = %s;\n", oil_type, oil_name, val->rec_codegen(pipe));
  }else { // variable declared globally, just assign the value
    emit(pipe, "%s = %s;\n", oil_name.c_str(), val->rec_codegen(pipe));
  }
}

void vardecl::dump_globalcode(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* ast::dump_globalcode */\n"); ); 
  
  oil_type = getOilType(getType());
  oil_name = mangle(0, getIdent());
  emit(pipe, "%s %s;\n", oil_type.c_str(), oil_name.c_str());
}

string vardecl::getType() {
  if (!children.empty())
    return static_cast<type*>(children[0])->getType();
  else return string("undef");
}

string vardecl::getIdent() { return children[1]->getLex(); }



/***********************  type  ***********************/
type::type(ast* btype) : type_ast("type") {
  add(btype);
  absorb(btype);
}

type::type(ast* btype, ast* array) : type_ast("type") {
  add(btype, array);
  absorb(btype);
}

string type::getType() {
  string temp = static_cast<basetype*>(children[0])->getType();
  if (children.size() == 2) 
    temp.append("[]");
  return temp;
}

void type::rec_typecheck() {
  ast::rec_typecheck();
  block_ptr = current_scope;
  DEBUGSTMT('z', eprintf("type\n"); );
  
  if (getType().compare("void[]") == 0)
    errprintf("%s error: undefined type (%s).\n", getfp(), 
              getType().c_str());
}



/***********************  basetype  ***********************/
basetype::basetype(ast* id) : type_ast("basetype") {
  add(id);
  absorb(id);
}

string basetype::getType() {
  return children[0]->getLex();
}

void basetype::rec_typecheck() {
  ast::rec_typecheck();
  block_ptr = current_scope;
  DEBUGSTMT('z', eprintf("basetype\n"); );

  ast* token = children[0];
  if (token->symbol==IDENT && !current_scope->lookup_usertype(token->getLex())) {
    errprintf("%s error: undefined usertype \"%s\".\n",
              token->getfp(), token->getLex().c_str());
  }
}



/***********************  declaration  ***********************/
decl::decl(ast* type, ast* id) : type_ast("decl") {
  add(type, id);
  absorb(id);
}

string decl::getType() {
  return static_cast<type*>(children[0])->getType();
}

string decl::getIdent() { return children[1]->getLex(); }



/***********************  struct definition  ***********************/
structdef::structdef(ast* id, ast* fld) : type_ast("structdef") {
  add(id, fld);
  absorb(id);
}

void structdef::rec_typecheck() {
  block_ptr = current_scope;
  DEBUGSTMT('z', eprintf("structdef\n"); );
  
  global_structdefs.push_back(this);
  string name = getIdent();
  ast* f = children[1]; // field node ptr
  if (!global_scope.lookup_usertype(name)) {
    vector<pair<string,string>>* fields = new vector<pair<string,string>>();
    std::vector<ast*>::iterator it; // field iterator
    for (it = f->children.begin(); it != f->children.end(); ++it) {
      fields->push_back(pair<string,string>(static_cast<decl*>(*it)->getIdent(),
                        static_cast<decl*>(*it)->getType()));
    }
    global_scope.define_usertype(name, fields);

  }else 
    errprintf("%s error: duplicate struct definition \"%s\".", 
              getfp(), getIdent().c_str());
}

void structdef::dump_code(FILE* pipe) {
    DEBUGSTMT('c', fprintf(pipe, "/* structdef */\n"); );
}

void structdef::dump_globalcode(FILE* pipe) {
  oil_name = children[0]->getLex();
  emit(pipe, "struct %s {\n", oil_name);
  setIndent(true);
  children[1]->dump_code(pipe);
  setIndent(false);
  fprintf(pipe, "};\n\n");
}

string structdef::getIdent() { return children[0]->getLex(); }



/***********************  field  ***********************/
field::field() : type_ast("field") {}

void field::dump_code(FILE* pipe) {
  std::vector<ast*>::iterator it;
  for (it = children.begin(); it < children.end(); ++it) {
    type* t = static_cast<type*>((*it)->children[0]);
    ast* id = (*it)->children[1];
    emit(pipe, "%s %s;\n", getOilType(t->getType()), id->getLex());
  }
}



/***********************  block  ***********************/
block::block() : control_ast("block"), block_ptr(NULL) {}

void block::rec_typecheck() {
  DEBUGSTMT('z', eprintf("block\n"); );

  if (children.size() > 0) {
    current_scope = current_scope->enterBlock();
    ast::rec_typecheck();
    current_scope = current_scope->leave();
  }
}

void block::dump_code(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* block */\n"); );
    
  std::vector<ast*>::iterator it;
  for (it = children.begin(); it != children.end(); ++it) {
    (*it)->dump_code(pipe);
  }
}

const char* block::rec_codegen(FILE* pipe) {
  dump_code(pipe);
  return "[block: error: use dump_code]";
}

SymbolTable* block::getBlk () { return block_ptr; }



/***********************  function declaration  ***********************/
func* func_ptr = NULL;

func::func(ast* type, ast* id, ast* p, ast* blk) : type_ast("function"), block_ptr(NULL) {
  add(4, type, id, p, blk);
  absorb(id);
}

void func::rec_typecheck() {
  global_funcs.push_back(this);
  children[0]->rec_typecheck();
  // check function defined in global scope
  if (current_scope != &global_scope)
    errprintf("%s error: functions must be defined in global scope.\n",getfp());
  // check if function name is previously defined
  if (global_scope.lookup(getIdent()).compare("undef") != 0) {
    errprintf("%s warning: duplicate function definition \"%s\".\n", 
              getfp(), getIdent().c_str());
  }
  // set current_scope to this function's block
  current_scope = current_scope->enterFunction(getIdent(), getSig(), getPos());
  block_ptr = current_scope;
  func_ptr = this; // function pointer enables return statement typechecking
  children[2]->rec_typecheck(); // enter parameter list into the new block
  children[3]->rec_typecheck(); // enter function block
  current_scope = current_scope->leave();
  func_ptr = NULL; // leaving function def, set function ptr to NULL
}

void func::dump_code(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* func */\n"); );
}

// all functions should be defined globally
void func::dump_globalcode(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* func */\n"); );

  // do not emit function prototypes
  if(children[3]->children.size() == 0)
    return;
    
  oil_type = getOilType(getType());
  oil_name = mangle(0, getIdent());
  emit(pipe, "%s\n%s(\n", oil_type, oil_name);
  setIndent(true);
  ast* params = children[2];
  std::vector<ast*>::iterator it;
  for (it = params->children.begin(); it != params->children.end(); ++it) {
    type* t = static_cast<type*>((*it)->children[0]);
    string idname = (*it)->children[1]->getLex();
    
    string argtype = getOilType(t->getType());
    string argid = mangle(block_ptr->getNumber(), idname);
    emit(pipe, "%s %s", argtype, argid); 
    if (it + 1 != params->children.end())
      fprintf(pipe, ",\n");
  }
  fprintf(pipe, ")\n{\n");
  children[3]->dump_code(pipe); // dump the block
  fprintf(pipe, "}\n\n");
  setIndent(false);
}

string func::getType() {
  return static_cast<type*>(children[0])->getType();
}

string func::getSig() {
  string temp = static_cast<type*>(children[0])->getType();
  temp.append("(");
  temp.append(static_cast<params*>(children[2])->getSig());
  temp.append(")");
  return temp;
}

string func::getIdent() { return children[1]->getLex(); }

SymbolTable* func::getBlk () { return block_ptr; }


/***********************  return  ***********************/
funcreturn::funcreturn() : type_ast("return") {}

void funcreturn::rec_typecheck() {
  ast::rec_typecheck();
  block_ptr = current_scope;
  DEBUGSTMT('z', eprintf("funcreturn\n"); );

  if (children.size() > 0) { // non-void return
    if (func_ptr == NULL) { // global return
      errprintf("%s error: global non-void return is not allowed.\n",
                 getfp());
    }else {
      expr* e = static_cast<expr*>(children[0]);
      string func_type = func_ptr->getType();
      if (func_type.compare(e->getType()) != 0)
        errprintf("%s error: return type (%s) must match function type (%s).\n",
                  getfp(), e->getType().c_str(), func_type.c_str());
    }
  }else { // void return
    if (func_ptr != NULL) { // global return
      string func_type = func_ptr->getType();
      if (func_type.compare("void") != 0)
        errprintf("%s error: void return in non-void function block (%s).\n",
                  getfp(), func_type.c_str());
    } 
    // else void global return
  }
}

void funcreturn::dump_code(FILE* pipe) {
  if (children.size() == 0) {
    emit(pipe, "return;\n");
  }else {
    emit(pipe, "return %s;\n", children[0]->rec_codegen(pipe));
  }
}


/***********************  while loop  ***********************/
loop::loop(ast* e, ast* stmt) : control_ast("while") {
  add(e, stmt);
}

void loop::rec_typecheck() {
  ast::rec_typecheck();
  DEBUGSTMT('z', eprintf("loop\n"); );
  
  expr* e = static_cast<expr*>(children[0]);
  if (!e->getType().compare("bool") == 0) {
    errprintf("%s error: loop expression must evaluate to type bool (%s).\n", 
              e->getfp(), e->getType().c_str());
  }
}

void loop::dump_code(FILE* pipe) {
  start = cmangle("while");
  end = cmangle("break");
  fprintf(pipe, "%s:;\n", start.c_str());
  expr* e = static_cast<expr*>(children[0]);
  ast* stmt = children[1];
  string ename = e->rec_codegen(pipe);
  emit(pipe, "if (!%s) goto %s;\n", ename, end); 
  stmt->dump_code(pipe);
  emit(pipe, "goto %s;\n", start);
  fprintf(pipe, "%s:;\n", end.c_str());
}



/***********************  parameter list  ***********************/
params::params() : type_ast("params") {}

string params::getSig() {
  string temp;
  for (size_t i = 0; i < children.size(); i++) {
    if (i > 0) temp.append(",");
      temp.append(static_cast<decl*>(children[i])->getType());
  }
  return temp;
}

void params::rec_typecheck() {
  block_ptr = current_scope;
  DEBUGSTMT('z', eprintf("params\n"); );

  for (size_t i = 0; i < children.size(); i++) {
    string type_str = static_cast<decl*>(children[i])->getType();
    string ident_str = static_cast<decl*>(children[i])->getIdent();
    string pos_str = static_cast<decl*>(children[i])->getPos();
    current_scope->addSymbol(ident_str, type_str, pos_str);
  }
}



/***********************  argument list  ***********************/
arguments::arguments() : type_ast("arguments") {}



/***********************  if then [else]  ***********************/
ifelse::ifelse(ast* e, ast* stmt1, ast* stmt2) : control_ast("ifelse") {
  add(e, stmt1, stmt2);
}

ifelse::ifelse(ast* e, ast* stmt) : control_ast("ifelse") {
  add(e, stmt);
}

void ifelse::rec_typecheck() {
  ast::rec_typecheck();
  DEBUGSTMT('z', eprintf("ifelse\n"); );

  expr* e = static_cast<expr*>(children[0]);
  if (!e->getType().compare("bool") == 0) {
    errprintf("%s error: branch expression evaluate to type bool (%s).\n", 
              e->getfp(), e->getType().c_str());
  }
}

void ifelse::dump_code(FILE* pipe) {
  DEBUGSTMT('c', eprintf("ifelse\n"); );

  string end = cmangle("fi");
  expr* e = static_cast<expr*>(children[0]);
  string ename = e->rec_codegen(pipe);
  
  ast* stmt1 = children[1];
  if (children.size() == 2) { // no else
    emit(pipe, "if (!%s) goto %s;\n", ename, end);
    stmt1->dump_code(pipe);
  }else { // is else
    string elselabel = cmangle("else");
    ast* stmt2 = children[2];
    emit(pipe, "if (!%s) goto %s;\n", ename, elselabel);
    stmt1->dump_code(pipe);
    emit(pipe, "goto %s;\n", end);
    fprintf(pipe, "%s:;\n", elselabel.c_str());
    stmt2->dump_code(pipe);
  }
  fprintf(pipe, "%s:;\n", end.c_str());
}



/***********************  expression types  ***********************/
// expressions have special type properties i.e. "associated types"
expr::expr(const char* lex) : type_ast(lex), assoc_type("undef") {}

string expr::getType() { return assoc_type; }

void expr::dump_node(FILE* pipe) {
  if (symbol == NT || symbol == ROOT) {
    fprintf(pipe, "%s", lexinfo->c_str());
    DEBUGX('z', fprintf (pipe, " - assoctype=%s", assoc_type.c_str()); );
  }
  else
    fprintf(pipe, "%s (%s)", get_yytname (symbol), lexinfo->c_str());
}



/***********************  binary operation  ***********************/
// form expr op expr
binop::binop(ast* e1, ast* op, ast* e2) : expr("binop") {
  add(e1, op, e2);
  absorb(op);
}

void binop::rec_typecheck() {
  ast::rec_typecheck();
  block_ptr = current_scope;
  DEBUGSTMT('z', eprintf("binop\n"); );

  bool binop_error = false;
  string l = static_cast<expr*>(children[0])->getType(); // left expr
  string r = static_cast<expr*>(children[2])->getType(); // right expr
  ast* op = children[1];

  /* need to check array types */
  switch (op->symbol) {
    case '+': assoc_type = "int"; 
              binop_error = !expectedcheck(l, r, "int");
      break;
    case '-': assoc_type = "int"; 
              binop_error = !expectedcheck(l, r, "int");
      break;
    case '*': assoc_type = "int"; 
              binop_error = !expectedcheck(l, r, "int");
      break;
    case '/': assoc_type = "int"; 
              binop_error = !expectedcheck(l, r, "int");
      break;
    case '%': assoc_type = "int"; 
              binop_error = !expectedcheck(l, r, "int");
      break;
    case '=': assoc_type = l; 
              if (!typecheck(l,r) || l.compare("null") == 0)
                binop_error = true;
      break;
    case EQ: assoc_type = "bool"; 
             if (!typecheck(l, r))
               binop_error = true;
      break; 
    case NE: assoc_type = "bool"; 
             if (!typecheck(l,r))
               binop_error = true;
      break;
    case LT: assoc_type = "bool"; 
             if (!typecheck(l,r) || !isPrimitive(l) || !isPrimitive(r))
               binop_error = true;
      break;
    case LE: assoc_type = "bool"; 
             if (!typecheck(l,r) || !isPrimitive(l) || !isPrimitive(r))
               binop_error = true;
      break;
    case GT: assoc_type = "bool"; 
             if (!typecheck(l,r) || !isPrimitive(l) || !isPrimitive(r))
               binop_error = true;
      break;
    case GE: assoc_type = "bool"; 
             if (!typecheck(l,r) || !isPrimitive(l) || !isPrimitive(r))
               binop_error = true;
      break;
  }

  if (binop_error)
    errprintf("%s error: binary operator undefined for types \"%s %s %s\".\n",
              getfp(), l.c_str(), op->getLex().c_str(), r.c_str());
}

void binop::dump_code(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* binop */\n"); ); 

  expr* e1 = static_cast<expr*>(children[0]);
  expr* e2 = static_cast<expr*>(children[2]);
  ast* op = children[1];

  emit(pipe, "%s %s %s;\n", e1->rec_codegen(pipe), op->getLex(),
       e2->rec_codegen(pipe));
}


const char* binop::rec_codegen(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* binop */\n"); );
  
  // assume temp variable must be declared
  oil_type = getOilType(assoc_type);
  expr* e1 = static_cast<expr*>(children[0]);
  expr* e2 = static_cast<expr*>(children[2]);
  ast* op = children[1];
  oil_name = getTypechar(oil_type);
  emit(pipe, "%s %s = %s %s %s;\n", oil_type, oil_name, e1->rec_codegen(pipe), 
       op->getLex(), e2->rec_codegen(pipe));

  return oil_name.c_str();
}



/***********************  unary operation  ***********************/
// form op expr
unop::unop(ast* op, ast* e) : expr("unop") {
  add(op, e);
  absorb(op);
}

void unop::rec_typecheck() {
  ast::rec_typecheck();
  block_ptr = current_scope;
  DEBUGSTMT('z', eprintf("unop\n"); );

  int op = children[0]->symbol;
  expr* e = static_cast<expr*>(children[1]);
  string etype = e->getType();
  switch(op) {
    case '!': if (!stringcmp(etype, "bool"))
                errprintf("%s error: operator ! is undefined for type (%s).\n",
                          getfp(), etype.c_str());
              assoc_type = "bool"; 
      break;
    case '+': if (!stringcmp(etype, "int"))
                errprintf("%s error: unary operator + is undefined for type %s.\n",
                          getfp(), etype.c_str());
              assoc_type = "int"; 
      break;
    case '-': if (!stringcmp(etype, "int"))
                errprintf("%s error: unary operator - is undefined for type %s.\n",
                          getfp(), etype.c_str());
              assoc_type = "int"; 
      break;
    case ORD: if (!stringcmp(etype, "char"))
                errprintf("%s error: operator ord is undefined for type %s.\n",
                          getfp(), etype.c_str());
              assoc_type = "int";
      break;
    case CHR: if (!stringcmp(etype, "int"))
                errprintf("%s error: operator chr is undefined for type %s.\n",
                          getfp(), etype.c_str());
              assoc_type = "char";
      break;
  }
}

void unop::dump_code(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* unop */\n"); ); 

  eprintf("%s warning: statement has no effect.\n", getfp());
}

const char* unop::rec_codegen(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* unop */\n"); ); 
  
  oil_type = getOilType(assoc_type);
  oil_name = getTypechar(oil_type);
  int op = children[0]->symbol;
  expr* e = static_cast<expr*>(children[1]);
  switch(op) {
    case '!': emit(pipe, "%s %s = !%s;\n", oil_type, oil_name,
                  e->rec_codegen(pipe));
      break;
    case '+': emit(pipe, "%s %s = +%s;\n", oil_type, oil_name,
                  e->rec_codegen(pipe));
      break;
    case '-': emit(pipe, "%s %s = -%s;\n", oil_type, oil_name,
                  e->rec_codegen(pipe));
      break;
    case ORD: emit(pipe, "%s %s = (int) %s;\n", oil_type, oil_name, 
                   e->rec_codegen(pipe));
      break;
    case CHR: emit(pipe, "%s %s = (int) %s;\n", oil_type, oil_name,
                   e->rec_codegen(pipe));
      break;
  }  
  return oil_name.c_str();
}


/***********************  allocator  ***********************/
// form NEW basetype()
alloc::alloc(ast* btype) : expr("allocator") {
  add(btype);
  absorb(btype);
}

// form NEW basetype(expr)  OR  NEW basetype[expr]
alloc::alloc(ast* btype, ast* tok, ast* e) : expr("allocator") {
  add(btype, tok, e);
  absorb(btype);
}

void alloc::rec_typecheck() {
  ast::rec_typecheck();
  block_ptr = current_scope;
  DEBUGSTMT('z', eprintf("alloc\n"); );
  
  basetype* base = static_cast<basetype*>(children[0]);
  if (children.size() > 2) {
    int op = children[1]->symbol;
    expr* e = static_cast<expr*>(children[2]);

    if (op == '(') { // NEW basetype(expr)
      assoc_type = e->getType();
    }else if (op == '[') { // NEW basetype[expr]
      if (!isBasetype(base->getType()))
        errprintf("%s error: invalid allocation type (%s).\n", getfp(),
                  base->getType().c_str());
      assoc_type = base->getType();
      assoc_type.append("[]");
      if (e->getType().compare("int") != 0) {
        errprintf("%s error: array size specifier ", getfp());
        errprintf("(%s) must be of type int.\n", e->getType().c_str());
      }
    }
  }else {
    assoc_type = base->getType();
  }
}


void alloc::dump_code(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* alloc */\n"); );

  eprintf("%s warning: statement has no effect.\n", getfp()); 
  eprintf("%s warning: memory leak.\n", getfp());
}

const char* alloc::rec_codegen(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* alloc */\n"); ); 
  
  
  oil_type = getOilType(assoc_type);
  oil_name = getTypechar(oil_type);
  if (children.size() == 1) {
    if (isUsertype(assoc_type)) { // struct
      emit(pipe, "struct %s %s = xcalloc (1, sizeof (struct %s));\n", oil_type,
           oil_name, oil_type);
    }else { // basic type
      emit(pipe, "%s %s = xcalloc (1, sizeof (%s));\n", oil_type, oil_name,
           oil_type);
    }
  }else {
    int op = children[1]->symbol;
    expr* e = static_cast<expr*>(children[2]);
    if (op == '(') {  // NEW basetype(expr)
      emit(pipe, "ubyte* %s = xcalloc (%s, sizeof (ubyte));\n", oil_name,
           e->rec_codegen(pipe));
    }else { // NEW basetype[expr]
      if (stringcmp(oil_type, "ubyte")) {  // char bool
        emit(pipe, "ubyte %s = xcalloc (%s, sizeof (ubyte));\n", oil_name,
             e->rec_codegen(pipe));
      }else if (stringcmp(oil_type, "int")) { // int
        emit(pipe, "int %s = xcalloc (%s, sizeof (int));\n", oil_name,
             e->rec_codegen(pipe));
      }else if (stringcmp(assoc_type, "string[]")) { // string
        emit(pipe, "ubyte** %s = xcalloc (%s, sizeof (ubyte*));\n", oil_name,
             e->rec_codegen(pipe));
      }else if (isArray(assoc_type)) {
        emit(pipe, "%s %s = xcalloc (%s, sizeof (%s));\n", oil_type, 
             oil_name, e->rec_codegen(pipe), oil_type);
      }else if (isUsertype(assoc_type)) { // usertype
        emit(pipe, "struct %s %s = xcalloc (%s, sizeof (struct %s*));\n",
             oil_type, oil_name, e->rec_codegen(pipe), oil_type);
      }else
        errprintf("codegen error: alloc oil_type: %s assoc_type: %s\n",
                  oil_type.c_str(), assoc_type.c_str()); 
    }
  }
  return oil_name.c_str();
}


/***********************  call  ***********************/
// form IDENT([expr[,expr]...])
call::call(ast* id, ast* args) : expr("call") {
  add(id, args);
  absorb(id);
}

void call::rec_typecheck() {
  ast::rec_typecheck();
  block_ptr = current_scope;

  // must check call signature matches function signature
  string id(children[0]->getLex());
  string sig = current_scope->lookup(id);
  vector<string> funcsig = global_scope.parseSig(sig);
  if (funcsig.size() > 0) {
    // make a list of arguments from the arguments to compare
    vector<string> callsig;
    ast* args = children[1];
    std::vector<ast*>::iterator it;
    for (it = args->children.begin(); it != args->children.end(); ++it) {
      callsig.push_back(static_cast<expr*>(*it)->getType());
    }
    // match arguments
    if (callsig.size() == funcsig.size() - 1) {
      bool args_matched = true;
      for (size_t i = 0; i < callsig.size(); i++) {
        if (callsig[i].compare(funcsig[i + 1]) != 0) {
          args_matched = false;
          break;
        }
      }
      if (args_matched) {
        assoc_type = funcsig[0];
      }else {
        errprintf("%s error: call to \"%s(%s)\" ", getfp(), 
                  id.c_str(), getArgList(callsig).c_str());
        errprintf("doesn't match function signature \"%s\".\n", sig.c_str());
      }
    }else {
      errprintf("%s error: call to \"%s(%s)\" ", getfp(), 
                id.c_str(), getArgList(callsig).c_str());
      errprintf("doesn't match function signature \"%s\".\n", sig.c_str());
    } 
  }else {
    errprintf("%s error: unknown function call to \"%s\".\n", getfp(),
              id.c_str());
  }
}

void call::dump_code(FILE* pipe) { // call as a statement
  DEBUGSTMT('c', fprintf(pipe, "/* call */\n"); ); 

  list<const char*> arglist;
  ast* args = children[1];
  std::vector<ast*>::iterator it;
  for (it = args->children.begin(); it != args->children.end(); ++it) {
    arglist.push_back((*it)->rec_codegen(pipe));
  }
  emit(pipe, "%s(", mangle(0, children[0]->getLex()));
  std::list<const char*>::iterator iter;
  for (iter = arglist.begin(); iter != arglist.end(); ) {
    fprintf(pipe, "%s", (*iter));
    if (++iter != arglist.end())
      fprintf(pipe, ", ");
  }
  fprintf(pipe, ");\n");
}

const char* call::rec_codegen(FILE* pipe) { // call part of another statement
  DEBUGSTMT('c', fprintf(pipe, "/* call */\n"); ); 

  list<const char*> arglist;
  ast* id = children[0];
  ast* args = children[1];
  std::vector<ast*>::iterator it;
  for (it = args->children.begin(); it != args->children.end(); ++it) {
    arglist.push_back((*it)->rec_codegen(pipe));
  }
  string functype = global_scope.parseSig(global_scope.lookup(id->getLex()))[0];
  string funcname = id->getLex();
  oil_type = getOilType(functype);
  oil_name = getTypechar(oil_type);
  string oil_call = mangle(0, funcname);
  oil_call.append("(");
  std::list<const char*>::iterator iter;
  for (iter = arglist.begin(); iter != arglist.end(); ) {
    oil_call.append(*iter);
    if (++iter != arglist.end()) {
      oil_call.append(", ");
    }
  }
  oil_call.append(")");
  emit(pipe, "%s %s = %s;\n", oil_type, oil_name, oil_call);
  return oil_name.c_str();
}



/***********************  variable  ***********************/
// form IDENT
variable::variable(ast* id) : expr("variable") {
  add(id);
  absorb(id);
}

// form expr[expr] or expr.IDENT
variable::variable(ast* e1, ast* op, ast* e2) : expr("variable") {
  add(e1, op, e2);
  absorb(op);
}

void variable::rec_typecheck() {
  ast::rec_typecheck();
  block_ptr = current_scope;
  DEBUGSTMT('z', eprintf("variable\n"); );

  if (children.size() == 1) { // form IDENT
    // lookup type
    assoc_type = current_scope->lookup(children[0]->getLex());
    if (assoc_type.compare("undef") == 0)
      errprintf("%s error: unknown identifier \"%s\".\n", getfp(),
             children[0]->getLex().c_str()); 
  }else {
    int op = children[1]->symbol;
    expr* e1 = static_cast<expr*>(children[0]);
    string etype = e1->getType();
    if (op == '[') { // expr[expr]

      // expression must be an array type or a string
      if (isArray(etype) || isString(etype)) {
        // check that array address type is int
        string aa_type = static_cast<expr*>(children[2])->getType();

        if (aa_type.compare("int") != 0) {
          errprintf("%s error: array address (%s) must be of type int.\n",
                    getfp(),  aa_type.c_str());
        }
        assoc_type = (isString(etype) ? "char" : parse_arraytype(etype));
      } else {
         errprintf("%s error: array access is undefined for non array type",
                   getfp());
         errprintf(" \"%s\".\n", etype.c_str());
         //assoc_type = etype;
      }

    } else if (op == '.') { // expr.IDENT
      string ident = children[2]->getLex();
      assoc_type = global_scope.lookup_fieldtype(etype, ident);
      if (assoc_type.compare("undef") == 0)
        errprintf("%s error: undefined field \"%s.%s\".\n", getfp(), 
                  etype.c_str(), ident.c_str());
    }
  }
}

void variable::dump_code(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* variable */\n"); ); 
  
  eprintf("%s warning: statement has no effect.\n", getfp());
}

const char* variable::rec_codegen(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* variable */\n"); ); 
  
  oil_type = getOilType(assoc_type);
  if (children.size() == 1) { // IDENT
    string idname = children[0]->getLex();
    oil_name = mangle(block_ptr->lookup_number(idname), idname);
  }else {
    expr* e1 = static_cast<expr*>(children[0]);
    if (children[1]->symbol == '[') { // expr[expr]
      expr* e2 = static_cast<expr*>(children[2]);
      char* data;
      asprintf(&data, "%s[%s]", e1->rec_codegen(pipe), e2->rec_codegen(pipe));
      oil_name = data;
      /*oil_name = getTypechar(oil_type);
      emit(pipe, "%s %s = %s;\n", oil_type, oil_name, data); */
    }else if (children[1]->symbol == '.') { // expr.IDENT
      ast* id = children[2];
      char* data;
      asprintf(&data, "%s->%s", e1->rec_codegen(pipe), id->getLex().c_str());
      oil_name = string(data);
    }
  }
  return oil_name.c_str();
}



/***********************  constant literal  ***********************/
// form ident
constant::constant(ast* id) : expr("constant") {
  add(id);
  absorb(id);
}

void constant::rec_typecheck() {
  block_ptr = current_scope;
  DEBUGSTMT('z', eprintf("constant\n"); );

  switch(children[0]->symbol) {
    case INTCON: assoc_type = "int";        break;
    case CHARCON: assoc_type = "char";      break;
    case STRINGCON: assoc_type = "string";  break;
    case TOK_FALSE: assoc_type = "bool";    break;
    case TOK_TRUE: assoc_type = "bool";     break;
    case TOK_NULL: assoc_type = "null";     break;
  }
}

void constant::dump_code(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* constant */\n"); ); 
  
  eprintf("%s warning: statement has no effect.\n", getfp());
}

const char* constant::rec_codegen(FILE* pipe) {
  DEBUGSTMT('c', fprintf(pipe, "/* constant */\n"); ); 
  
  string c = children[0]->getLex();
  if (stringcmp(c, "true")) {
    return "1";
  } else if (stringcmp(c, "false")) {
    return "0";
  } else if (stringcmp(c, "null")) {
    return "0";
  }
  return c.c_str();
}

// misc functions
SymbolTable* getCurrentScope () { return current_scope; }
