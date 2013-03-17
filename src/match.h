#ifndef MATCH_H
#define MATCH_H

#include "file.h"

typedef struct match_t {
  file_list_t* file;
  double metric;
} match_t;

match_t* find_best_matches(char* needle, file_list_t* haystack);

#endif
