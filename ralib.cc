// Reid Anetsberger ~ ranetsbe@ucsc.edu
#include <limits>
#include <cstring>
#include "ralib.h"

using namespace std;

// return true if u and v are equal
bool stringcmp(string u, string v) {
  return (u.compare(v) == 0 ? true : false);
}

// return string representation of an integer n
string itos(int n) {
   const int max_size = std::numeric_limits<int>::digits10 + 2; // sign &
   char buffer[max_size] = {0};                                 // 0-terminator
   sprintf(buffer, "%d", n);
   return string(buffer);
}

// removes the file extension "ext" from the string "str"
char *remove_ext (char *str, const char *ext) {
   if (str == NULL) return NULL;
   char *tempstr = new char[strlen (str) + 1];
   char *lastdot;

   strcpy (tempstr, str);
   lastdot = strrchr (tempstr, '.');

   if (lastdot == NULL) {
      return NULL;
   }else {
      if (strcmp (lastdot, ext) == 0) {
         *lastdot = '\0';
         return tempstr;
      }else {
         *lastdot = '\0';
         return remove_ext (tempstr, ext);
      }
   }
}
