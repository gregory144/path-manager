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

file_list_t* list_directory(char* directory, bool only_files);

file_list_t* files_in_directories(node_t* directories);

file_list_t* sort_files(file_list_t* files);

char* symlink_target(char* link);

char* get_absolute_path(char* relative_path);

bool mkdir_for_user(char* path);

bool clear_dir(char* path);

void free_file_list(file_list_t* files);

#endif
