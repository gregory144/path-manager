#ifndef PATH_H
#define PATH_H

#include "util.h"

#define PATH_SEPARATOR_CHAR ':'

typedef enum { export_default, export_setenv } export_mode_t;

typedef struct path_t {
  int num_entries;
  struct path_entry_t* head;
  int total_string_length;
} path_t;

typedef struct path_entry_t {
  char* directory;
  struct path_entry_t* next;
  bool modifiable;
} path_entry_t;

path_t* path_load();

void path_clean(path_t* path);

bool path_warnings(path_t* path);

bool path_save(path_t* path);

void path_export(path_t* path, char* env_var_name, export_mode_t export_mode);

bool path_add(path_t* path, char* directory);

bool path_rm(path_t* path, char* directory);

char* path_search(path_t* path, char* file);

node_t* path_directories(path_t* path);

char* path_to_string(path_t* path);

void path_free(path_t *path);

#endif
