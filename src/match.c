#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "match.h"
#include "file.h"
#include "path.h"
#include "util.h"

int num_letter_pairs(char* s) {
  return strlen(s) - 1;
}

double best_match_metric(char* a, char* b) {
  int a_len = strlen(a);
  int b_len = strlen(b);

  int intersection_size = 0;
  int union_size = (a_len - 1) + (b_len - 1) ;

  int a_index, b_index;
  for (a_index = 0; a_index < a_len - 1; a_index++) {
    for (b_index = 0; b_index < b_len - 1; b_index++) {
      // TODO doesn't take into account repeated pairs
      if (a[a_index] == b[b_index] && a[a_index + 1] == b[b_index + 1]) {
        intersection_size++;
      }
    }
  }
  return (2.0 * intersection_size) / union_size;
}

int compare_matches(const void *a, const void *b) {
  match_t* a_match = (match_t*)a;
  match_t* b_match = (match_t*)b;
  if (a_match->metric == b_match->metric) return b_match->file->executable - a_match->file->executable;
  return (b_match->metric - a_match->metric) * 100;
}

match_t* find_best_matches(char* needle, file_list_t* haystack) {
  match_t* matches;
  file_list_t* entry = NULL;
  int haystack_size = 0;
  for (entry = haystack; entry; entry = entry->next, haystack_size++);
  print_verbose("Checking %d files\n", haystack_size);
  matches = calloc(haystack_size, sizeof(match_t));

  int i;
  for (entry = haystack, i = 0; entry; entry = entry->next, i++) {
    matches[i].metric = best_match_metric(needle, entry->filename);
    matches[i].file = entry;
  }
  qsort(matches, haystack_size, sizeof(match_t), compare_matches);
  return matches;
}
