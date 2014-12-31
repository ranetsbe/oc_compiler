// $Id$
// Reid Anetsberger ~ ranetsbe@ucsc.edu

/* abstract syntax tree utility file */

#ifndef __ASTUTILS_H__
#define __ASTUTILS_H__

#include <string>
#include "symtable.h"
#include "auxlib.h"
#include "ralib.h"

// return the scope mangled name for a variable
std::string mangle(int blocknr, std::string id);

// return the label mangled name for a variable
std::string cmangle(std::string name);

// return the temporary variable name for type t
std::string getTypechar(std::string t);

// return oil equivalent of t if t is not an advanced type
std::string getBasicOilType(std::string t);

// return oil equivalent of t
std::string getOilType(std::string t);

// emit helper functions
void setIndent(bool val);

void emit(FILE* pipe, const char* format);

void emit(FILE* pipe, const char* format, std::string s1);

void emit(FILE* pipe, const char* format, std::string s1, std::string s2);

void emit(FILE* pipe, const char* format, std::string s1, std::string s2, 
          std::string s3);
          
void emit(FILE* pipe, const char* format, std::string s1, std::string s2, 
          std::string s3, std::string s4);
          
void emit(FILE* pipe, const char* format, std::string s1, std::string s2, 
          std::string s3, std::string s4, std::string s5);

// return true if s is: bool, int or char
bool isPrimitive(std::string s);

// return true if s.compare("string") == 0
bool isString(std::string s);

// return true if s is a usertype
bool isUsertype(std::string s);

// return true if s is: bool, int, char, string or usertype
bool isBasetype(std::string s);

// return true if s has suffix "[]"
bool isArray(std::string s);

// return true if type1 and type2 are compatible
bool typecheck(std::string type1, std::string type2);

// test if t1, t2 and expected are compatible types
// return true if compatible
bool expectedcheck(std::string t1, std::string t2, const char* expected);

// returns the elements of "args" seperated by a ','
std::string getArgList(std::vector<std::string> args);

// return the result of element access on an array type
std::string parse_arraytype(std::string arraytype);

#include "ast.h"

#endif // __ASTUTILS_H__
