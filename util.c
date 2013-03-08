#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

bool path_verbose_on = false;

bool path_warnings_on = false;

void turn_verbose_on() {
  path_verbose_on = true;
}

void turn_warnings_on() {
  path_warnings_on = true;
}

bool warnings_are_on() {
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

void free_nodes_and_vals(node_t* node) {
  free_nodes(node, 1);
}

void free_nodes(node_t* node, int freeVals) {
  node_t* entry;
  node_t* tmp;

  for (entry = node; entry;) {
    tmp = entry;
    entry = entry->next;
    if (freeVals) free(tmp->val);
    free(tmp);
  }
}
