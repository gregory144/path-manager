#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

int path_verbose_on = 0;

int path_warnings_on = 0;

void turn_verbose_on() {
  path_verbose_on = 1;
}

void turn_warnings_on() {
  path_warnings_on = 1;
}

int warnings_are_on() {
  return path_warnings_on;
}

void print_verbose(char* s, ...) {
  if (path_verbose_on) {
    va_list va;
    va_start(va, s);
    vprintf(s, va);
    va_end(va);
  }
}

void print_warning(char* s, ...) {
  if (path_warnings_on) {
    printf("Warning: ");
    va_list va;
    va_start(va, s);
    vprintf(s, va);
    va_end(va);
  }
}

void free_nodes(node_t* node) {
  node_t* entry;
  node_t* tmp;

  for (entry = node; entry;) {
    tmp = entry;
    entry = entry->next;
    free(tmp->val);
    free(tmp);
  }
}

node_t* find_best_matches(char* needle, node_t* haystack) {
  // not implemented yet
  return NULL;
}
