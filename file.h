#ifndef FILE_H
#define FILE_H

#include "util.h"

#define DIR_SEPARATOR_CHAR '/'

typedef struct file_list_t {
  char* directory;
  char* filename;
  struct file_list_t* next;
} file_list_t;

int file_exists(char* dir);

char* file_join(char* dir, char* file);

int directory_exists(char* dir);

int directory_readable(char* dir);

int directory_is_absolute(char* dir);

int directory_contains_executable_files(char* dir);

int file_is_executable(char* file);

int directorycmp(char* dir1, char* dir2);

file_list_t* files_in_directories(node_t* directories);

void free_file_list(file_list_t* files);

#endif
