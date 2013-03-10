#ifndef UTIL_H
#define UTIL_H

#define PATH_VERSION "0.1.0"

typedef enum { false, true } bool;

typedef struct node_t {
  struct node_t* next;
  char* val;
} node_t;

void turn_verbose_on();

void turn_warnings_on();

bool warnings_are_on();

void print_verbose(char* s, ...);

void print_warning(char* s, ...);

char* get_home_directory();

char* get_executable_directory();

bool install_in_shell();

void free_nodes(node_t* node);

#endif
