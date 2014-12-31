#include <vector>
#include <cstring>
#include <map>
#include "astutils.h"

using namespace std;

int t_count = 1; // temp variable counter
int c_count = 1; // control label counter
bool indent_flag = false; // set this to indent emits by 8 whitespace

// return the scope mangled name for a variable
std::string mangle(int blocknr, std::string id) {
  if (blocknr == 0) { // global variable
    string temp("__");
    temp.append(id.c_str());
    return temp;
  }else { // local
    string temp("_");
    temp.append(itos(blocknr).c_str());
    temp.append("_");
    temp.append(id.c_str());
    return temp;
  }
}

// return the label mangled name for a variable
std::string cmangle(string name) { 
  name.append("_");
  name.append(itos(c_count++));
  return name;
}

// return the temporary variable name for type t
string getTypechar(string t) {
  if (stringcmp(t, "ubyte")) {
    string temp("b");
    temp.append(itos(t_count++).c_str());
    return temp;
  }else if (stringcmp(t, "int")) { 
    string temp("i");
    temp.append(itos(t_count++).c_str());
    return temp;
  }else if (strrchr (t.c_str(), '*') != NULL){
    string temp("p");
    temp.append(itos(t_count++).c_str());
    return temp;
  }else
    return "[error]";
}

// return oil equivalent of t if t is a basetype
string getBasicOilType(string t) {
  if (stringcmp(t, "undef")) {
    return t;
  }else if (stringcmp(t, "int")) {
    return "int";
  }else if (stringcmp(t, "char") || stringcmp(t, "bool")) {
    return "ubyte";
  }else if (stringcmp(t, "string")) {
    return "ubyte*";
  }else if (isUsertype(t)) {
    string temp("struct ");
    temp.append(t);
    return temp;
  }
  return "[error]";
}

// return oil equivalent of type t
string getOilType(string t) {
  if (isBasetype(t)) { // return basic oil type (may be *)
    return getBasicOilType(t);
  }else if (isArray(t)) { // return basic oil type with * (may be **)
    string temp = parse_arraytype(t);
    temp = getBasicOilType(temp);
    temp.append("*");
    return temp;
  }else if (stringcmp(t, "void")) {
    return t;
  }
  return "undef_oil";
}

void setIndent(bool val) {
  indent_flag = val;
}

void emit(FILE* pipe, const char* format) {
  if (indent_flag)
    fprintf(pipe, "%*s", 8, "");
  fprintf(pipe, "%s", format);
}

void emit(FILE* pipe, const char* format, string s1) {
  if (indent_flag)
    fprintf(pipe, "%*s", 8, "");
  fprintf(pipe, format, s1.c_str());
}

void emit(FILE* pipe, const char* format, string s1, string s2) {
  if (indent_flag)
    fprintf(pipe, "%*s", 8, "");
  fprintf(pipe, format, s1.c_str(), s2.c_str());
}

void emit(FILE* pipe, const char* format, string s1, string s2, string s3) {
  if (indent_flag)
    fprintf(pipe, "%*s", 8, "");
  fprintf(pipe, format, s1.c_str(), s2.c_str(), s3.c_str());
}

void emit(FILE* pipe, const char* format, string s1, string s2, string s3,
          string s4) {
  if (indent_flag)
    fprintf(pipe, "%*s", 8, "");
  fprintf(pipe, format, s1.c_str(), s2.c_str(), s3.c_str(), s4.c_str());
}

void emit(FILE* pipe, const char* format, string s1, string s2, string s3,
          string s4, string s5) {
  if (indent_flag)
    fprintf(pipe, "%*s", 8, "");
  fprintf(pipe, format, s1.c_str(), s2.c_str(), s3.c_str(), s4.c_str(), s5.c_str());
}

// return true if s is bool int or char
bool isPrimitive(string s) {
  if (stringcmp(s, "bool") || stringcmp(s, "int") || stringcmp(s, "char"))
    return true;
  return false;
}

// return true if s.compare("string") == 0
bool isString(string s) {
  if (s.compare("string") == 0)
    return true;
  return false;
}

// return true if s is a usertype
bool isUsertype(string s) {
  if (getCurrentScope()->lookup_usertype(s))
    return true;
  return false;
}

// return true if s is bool int char string or usertype
bool isBasetype(string s) {
  if (isPrimitive(s) || isString(s) || isUsertype(s))
    return true;
  return false;
}

// return true if s has suffix "[]"
bool isArray(string s) {
  if (s.substr(s.size() - 2).compare("[]") == 0)
    return true;
  return false;
}

// return true if type1 and type2 are compatible
bool typecheck(string type1, string type2) {
  if (stringcmp(type1, type2))
    return true;
  else if (!isPrimitive(type1) && type2.compare("null") == 0)
    return true;
  else if (!isPrimitive(type2) && type1.compare("null") == 0)
    return true;
  return false;
}

// check that t1, t2 and expected are all the same
bool expectedcheck(string t1, string t2, const char* expected) {
  if (stringcmp(t1, t2) && t1.compare(expected) == 0)
    return true;
  return false;
}

// returns the elements of "args" seperated by a ','
string getArgList(vector<string> args) {
  string list;
  std::vector<string>::iterator it;
  for (it = args.begin(); it < args.end(); ++it) {
    list.append(*it);
    if (it + 1 != args.end()) list.append(",");
  }
  return list;
}

// return the result of element access on an array type
string parse_arraytype(string arraytype) {
  // remove the trailing "[]"
  string basetype = arraytype.substr(0, arraytype.size() - 2);
  return basetype;
}

