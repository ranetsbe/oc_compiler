// $Id$
// Reid Anetsberger ~ ranetsbe@ucsc.edu
#ifndef __RALIB_H__
#define __RALIB_H__

#include <string>

// return true if u and v are equal
bool stringcmp(std::string u, std::string v);

// return string representation of an integer n
std::string itos(int n);

// returns a copy of str without the suffix ext
// returns NULL if the suffix is not found
char *remove_ext (char *str, const char *ext);

#endif
