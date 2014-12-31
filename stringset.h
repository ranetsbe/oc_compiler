//$Id: stringset.h,v 1.2 2013/10/28 03:56:36 ranetsbe Exp ranetsbe $
// Reid Anetsberger ~ ranetsbe@ucsc.edu

#ifndef __STRINGSET_H__
#define __STRINGSET_H__

#include <string>
#include <unordered_set>
#include <stdio.h>

#include "auxlib.h"

// insert a string to the stringset
const std::string* intern_stringset (const char* str);

// write the stringset to a file
void dump_stringset (FILE*);

#endif
