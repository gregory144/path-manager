#ifndef FILE_H
#define FILE_H

#include "util.h"

#define DIR_SEPARATOR_CHAR '/'

typedef struct file_list_t {
  char* directory;
  char* filename;
  char* full_path;
  int executable;
  struct file_list_t* next;
} file_list_t;

bool file_exists(char* dir);

char* file_join(char* dir, char* file);

bool directory_exists(char* dir);

bool directory_readable(char* dir);

bool directory_is_absolute(char* dir);

bool directory_contains_executable_files(char* dir);

bool file_is_executable(char* file);

int directorycmp(char* dir1, char* dir2);

file_list_t* files_in_directories(node_t* directories);

void free_file_list(file_list_t* files);

#endif
