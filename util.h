#ifndef UTIL_H
#define UTIL_H

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

void free_nodes_and_vals(node_t* node);

void free_nodes(node_t* node, int freeVals);

#endif
