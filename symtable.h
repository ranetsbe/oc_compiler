// $Id: symtable.h,v 1.1 2013/11/27 15:58:42 ranetsbe Exp ranetsbe $
// Reid Anetsberger ~ ranetsbe@ucsc.edu
#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include <stdio.h>

#include <string>
#include <vector>
#include <utility>
#include <map>

using namespace std;
typedef std::pair<string,string> val; // val(type, pos)

class SymbolTable {

  // block number
  int number;

  SymbolTable* parent;

  // the mapping of identifiers to their types
  std::map<string,val>mapping;

  // subscopes are mapped by their unique number or in the case of
  // functions, the function name.
  std::map<string,SymbolTable*> subscopes;

  // global user defined types
  static std::map<string, std::map<string, string> > usertypes;

public:
  // Creates and returns a new symbol table.
  SymbolTable(SymbolTable* parent);

  // Creates a new empty table beneath the current table and returns it.
  SymbolTable* enterBlock();

  // Returns the parent block
  SymbolTable* leave();

  // Adds the function name as symbol to the current table
  // and creates a new empty table beneath the current one.
  //
  // Example: To enter the function "void add(int a, int b)",
  //          use "currentSymbolTable->enterFunction("add", "void(int,int)");
  SymbolTable* enterFunction(string name, string signature, string pos);

  // Add a symbol with the provided name and type to the current table.
  //
  // Example: To add the variable declaration "int i = 23;"
  //          use "currentSymbolTable->addSymbol("i", "int");
  void addSymbol(string name, string type, string pos);

  // Dumps the content of the symbol table and all its inner scopes
  // depth denotes the level of indention.
  //
  // Example: "global_symtable->dump(symfile, 0)"
  void dump(FILE* symfile, int depth);

  // Look up name in this and all surrounding blocks and return its type.
  //
  // Returns the "undef" if variable was not found
  string lookup(string name);

  // Looks through the symbol table chain to find the function which
  // surrounds the scope and returns its signature
  // or "" if there is no surrounding function.
  //
  // Use parentFunction(NULL) to get the parentFunction of the current block.
  string parentFunction(SymbolTable* innerScope);

  // return the number of this block
  int getNumber();
  
  // looks through the symbol table chain to find the block number where this
  // variable first appears
  //
  // returns -1 if not found
  int lookup_number(string name);

  // Running id number for symbol tables
  static int N;

  // Parses a function signature and returns all types as vector.
  // The first element of the vector is always the return type.
  //
  // Example: "SymbolTable::parseSig("void(int,int)")
  //          returns ["void", "int", "int"]
  static std::vector<string> parseSig(string signature);

  void debugstruct();

  // insert a struct into the usertypes table
  void define_usertype(std::string tname,  std::vector<val>* fields);

  // returns true if the type "tname" exists in usertypes
  bool lookup_usertype(std::string tname);

  // returns true if the field "tname.ident" exists in usertypes
  bool lookup_field(std::string tname, std::string ident);

  // returns the type of the field "tname.ident" if it exists
  // returns the empty string "undef" if not found
  string lookup_fieldtype(std::string tname, std::string ident);
};

#endif
