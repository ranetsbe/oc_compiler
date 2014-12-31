// $Id: symtable.cc,v 1.1 2013/11/27 15:58:42 ranetsbe Exp ranetsbe $
// Reid Anetsberger ~ ranetsbe@ucsc.edu

#include "auxlib.h"
#include "symtable.h"

using namespace std;

// Creates and returns a new symbol table.
//
// Use "new SymbolTable(NULL)" to create the global table
SymbolTable::SymbolTable(SymbolTable* parent) {
  // Set the parent (this might be NULL)
  this->parent = parent;
  // Assign a unique number and increment the global N
  this->number = SymbolTable::N++;
}

// Creates a new empty table beneath the current table and returns it.
SymbolTable* SymbolTable::enterBlock() {
  // Create a new symbol table beneath the current one
  SymbolTable* child = new SymbolTable(this);
  // Convert child->number to a string stored in buffer
  char buffer[10];
  sprintf(&buffer[0], "%d", child->number);
  // Use the number as ID for this symbol table
  // to identify all child symbol tables
  this->subscopes[buffer] = child;
  // Return the newly created symbol table
  return child;
}

// Returns the parent block
SymbolTable* SymbolTable::leave() {
  return parent;
}

// adds the function name as symbol to the current table
// and creates a new empty table beneath the current one
SymbolTable* SymbolTable::enterFunction(string name, string signature, string pos) {
  this->addSymbol(name, signature, pos);
  SymbolTable* child = new SymbolTable(this);
  this->subscopes[name] = child;
  return child;
}

// add a symbol with the provided name and type to the current table
void SymbolTable::addSymbol(string name, string type, string pos) {
  this->mapping[name] = val(type, pos);
}

// dumps the content of the symbol table and all its inner scopes
// depth denotes the level of indention.
void SymbolTable::dump(FILE* symfile, int depth) {
  std::map<string,val>::iterator it;
  for (it = this->mapping.begin(); it != this->mapping.end(); ++it) {
    const char* name = it->first.c_str();
    const char* type = it->second.first.c_str();
    const char* pos = it->second.second.c_str();
    // Print the symbol as "name (pos) {blocknumber} type"
    // indented by 3 spaces for each level
    fprintf(symfile, "%*s%s %s {%d} %s\n", 3*depth, "", name, pos, this->number, type);
    // If the symbol we just printed is actually a function
    // then we can find the symbol table of the function by the name
    if (this->subscopes.count(name) > 0) {
      // And recursively dump the functions symbol table
      // before continuing the iteration
      this->subscopes[name]->dump(symfile, depth + 1);
    }
  }
  // Create a new iterator for <string,SymbolTable*>
  std::map<string,SymbolTable*>::iterator i;
  // Iterate over all the child symbol tables
  for (i = this->subscopes.begin(); i != this->subscopes.end(); ++i) {
    // If we find the key of this symbol table in the symbol mapping
    // then it is actually a function scope which we already dumped above
    if (this->mapping.count(i->first) < 1) {
      // Otherwise, recursively dump the (non-function) symbol table
      i->second->dump(symfile, depth + 1);
    }
  }
}

// Look up name in this and all surrounding blocks and return its type.
//
// Returns "undef" if variable was not found
string SymbolTable::lookup(string name) {
  // Look up "name" in the identifier mapping of the current block
  if (this->mapping.count(name) > 0) {
    // If we found an entry, just return its type
    return this->mapping[name].first;
  }
  // Otherwise, if there is a surrounding scope
  if (this->parent != NULL) {
    // look up the symbol in the surrounding scope
    // and return its reported type
    return this->parent->lookup(name);
  } else {
    // Return "undef" if the global symbol table has no entry
    //errprintf("Unknown identifier: %s\n", name.c_str());
    return "undef";
  }
}

// Looks through the symbol table chain to find the function which
// surrounds the scope and returns its signature
// or "undef" if there is no surrounding function.
//
// Use parentFunction(NULL) to get the parentFunction of the current block.
string SymbolTable::parentFunction(SymbolTable* innerScope) {
  // Create a new <string,SymbolTable*> iterator
  std::map<string,SymbolTable*>::iterator it;
  // Iterate over all the subscopes of the current scopes
  for (it = this->subscopes.begin(); it != this->subscopes.end(); ++it) {
    // If we find the requested inner scope and if its name is a symbol
    // in the identifier mapping
    if (it->second == innerScope && this->mapping.count(it->first) > 0) {
      // Then it must be the surrounding function, so return its type/signature
      return this->mapping[it->first].first;
    }
  }
  // If we did not find a surrounding function
  if (this->parent != NULL) {
    // Continue the lookup with the parent scope if there is one
    return this->parent->parentFunction(this);
  }
  // If there is no parent scope, return "undef"
  //errprintf("Could not find surrounding function\n");
  return "undef";
}

// looks through the symbol table chain to find the block number where this
// variable first appears
//
// returns -1 if not found
int SymbolTable::lookup_number(string name) {
  // Look up "name" in the identifier mapping of the current block
  if (this->mapping.count(name) > 0) {
    return this->number;
  }
  if (this->parent != NULL) {
    return this->parent->lookup_number(name);
  } else {
    // Return -1 if the variable is not found
    return -1;
  }
}


// return the number of this block
int SymbolTable::getNumber() { return number; }

// initialize running block ID to 0
int SymbolTable::N(0);

// Parses a function signature and returns all types as vector.
// The first element of the vector is always the return type.
//
// Example: "SymbolTable::parseSig("void(int,int)")
//          returns ["void", "int", "int"]
vector<string> SymbolTable::parseSig(string sig) {
  // Initialize result string vector
  vector<string> results;
  // Find first opening parenthesis
  size_t left = sig.find_first_of('(');
  if (left == string::npos) {
    // Print error and return empty results if not a function signature
    //errprintf("%s is not a function\n", sig.c_str());
    return results;
  }
  // Add return type
  results.push_back(sig.substr(0, left));
  // Starting with the position of the left parenthesis
  // Find the next comma or closing parenthesis at each step
  // and stop when the end is reached.
  for (size_t i = left + 1; i < sig.length()-1; i = sig.find_first_of(",)", i) + 1) {
    // At each step, get the substring between the current position
    // and the next comma or closing parenthesis
    results.push_back(sig.substr(i, sig.find_first_of(",)", i) - i));
  }
  return results;
}

// initialize usertypes map
map<string, map<string, string> > SymbolTable::usertypes;

void SymbolTable::debugstruct(){
  eprintf("\nDEBUGSTRUCT\n");
  std::map<string,map<string,string>>::iterator it = usertypes.begin();
  for(; it != usertypes.end(); ++it) {
     eprintf("struct name: %s\n", it->first.c_str());
     std::map<string,string>::iterator iter;
     for (iter = it->second.begin(); iter != it->second.end(); ++iter){
        eprintf("  field: %s %s\n", iter->first.c_str(), iter->second.c_str());
     }
  }
  eprintf("\n");
}

// insert a struct and its fields into the defined_types table
/* TODO: does not check for duplicate ident declarations */
void SymbolTable::define_usertype(string tname, std::vector<val>* fields) {
   map<string, string> *set = new map<string, string>();
   std::vector<val>::iterator it;
   for (it = fields->begin(); it != fields->end(); ++it) {
      set->insert (*it);
   }
   usertypes[tname] = *set;
}

// returns true if the type "tname" exists in usertypes
bool SymbolTable::lookup_usertype(string tname) {
  if (usertypes.find(tname) != usertypes.end()) 
    return true;
  return false;
}

// returns true if the field "tname.ident" exists in usertypes
bool SymbolTable::lookup_field(string tname, string ident) {
  std::map<string, map<string,string> >::iterator it = usertypes.find(tname);

  if (it != usertypes.end() && it->second.find(ident) != it->second.end())
    return true;
  return false;
}

// returns the type of the field "tname.ident" if it exists
// returns the empty string ("") if not found
string SymbolTable::lookup_fieldtype(string tname, string ident) {
  std::map<string, map<string,string> >::iterator it = usertypes.find(tname);
  if (it != usertypes.end() && it->second.find(ident) != it->second.end())
    return it->second.find(ident)->second;
  return "undef";
}


