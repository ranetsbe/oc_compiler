// $Id$
// Reid Anetsberger ~ ranetsbe@ucsc.edu

/* identifier/label name mangling utility file */

#ifndef __MANGLER_H__
#define __MANGLER_H__

#include <string>
#include "auxlib.h"
#include "ralib.h"

int label_count; // control flow label number
int temp_count; // temporary variable number

// mangle global name
char* g_mangle(std::string name);

// mangle local name
char* l_mangle(std::string name);

// mangle control label
char* c_mangle(std::string label);

// mangle temporary variable
char* t_mangle(std::string temp);

int getLabelCount();
int getTempCount();

#endif // __MANGLER_H__
