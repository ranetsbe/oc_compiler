//$Id: stringset.cc,v 1.2 2013/10/28 03:56:36 ranetsbe Exp ranetsbe $
// Reid Anetsberger ~ ranetsbe@ucsc.edu

// Use cpp to scan a file and print line numbers.
// Inserts strings from the file into the stringset adt

#include <iostream>
#include <unordered_set>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
//#include <wait.h>

#include "stringset.h"
#include "auxlib.h"
using namespace std;

typedef std::unordered_set<std::string> stringset;
typedef stringset::const_iterator stringset_itor;
typedef stringset::const_local_iterator stringset_bucket_itor;

stringset set;                               // hashtable

// insert string to stringset
const string* intern_stringset (const char* str) {
   pair<stringset_itor,bool> handle = set.insert (str);
   return &*handle.first;
}

// write the stringset to a file
void dump_stringset (FILE* out) {
   size_t max_bucket_size = 0;
   for (size_t bucket = 0; bucket < set.bucket_count(); ++bucket) {
      bool need_index = true;
      size_t curr_size = set.bucket_size (bucket);
      if (max_bucket_size < curr_size) max_bucket_size = curr_size;
      for (stringset_bucket_itor itor = set.cbegin (bucket);
           itor != set.cend (bucket); ++itor) {
         if (need_index) fprintf (out, "stringset[%4lu]: ", bucket);
                    else fprintf (out, "          %4s   ", "");
         need_index = false;
         const string* str = &*itor;
         fprintf (out, "%22lu %p->\"%s\"\n", set.hash_function()(*str),
                  str, str->c_str());
      }
   }
   fprintf (out, "load_factor = %.3f\n", set.load_factor());
   fprintf (out, "bucket_count = %lu\n", set.bucket_count());
   fprintf (out, "max_bucket_size = %lu\n", max_bucket_size);
}
