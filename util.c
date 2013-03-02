#include <stdarg.h>
#include <stdio.h>

int path_verbose_on = 0;

void turn_verbose_on() {
  path_verbose_on = 1;
}

void print_verbose(char* s, ...) {
  if (path_verbose_on) {
    va_list va;
    va_start(va, s);
    vprintf(s, va);
    va_end(va);
  }
}

void print_warning(char*s, ...) {
  printf("Warning: ");
  va_list va;
  va_start(va, s);
  vprintf(s, va);
  va_end(va);
}
