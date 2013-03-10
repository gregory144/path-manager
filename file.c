#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "file.h"
#include "util.h"

bool file_exists(char* dir_s) {
  struct stat s;
  return stat(dir_s, &s) != -1;
}

bool directory_exists(char* dir_s) {
  struct stat s;
  return stat(dir_s, &s) != -1 && S_ISDIR(s.st_mode);
}

bool directory_readable(char* dir_s) {
  DIR *dir = opendir(dir_s);
  if (dir) {
    closedir(dir);
    return 1;
  }
  return 0;
}

bool directory_is_absolute(char* dir_s) {
  return dir_s[0] == DIR_SEPARATOR_CHAR;
}

bool file_is_executable(char* file_s) {
  struct stat s;
  return stat(file_s, &s) == 0 && S_ISREG(s.st_mode) && (s.st_mode & 0111);
}

char* file_join(char* dir, char* file) {
  int dir_size = strlen(dir);
  if (dir[dir_size - 1] == DIR_SEPARATOR_CHAR) {
    dir_size--;
  }
  int file_size = strlen(file);
  char* joined = calloc(dir_size + 1 + file_size + 1, sizeof(char));
  strncat(joined, dir, dir_size);
  joined[dir_size] = DIR_SEPARATOR_CHAR;
  strncat(joined, file, file_size);
  return joined;
}

bool directory_contains_executable_files(char* dir_s) {
  DIR *dir = opendir(dir_s);
  struct dirent *entry;
  bool found = 0;
  if (dir) {
    while ((entry = readdir(dir)) != NULL) {
      char* filename = file_join(dir_s, entry->d_name);
      bool ret = file_is_executable(filename);
      free(filename);
      if (ret) {
        found = true;
        break;
      }
    }
    closedir(dir);
    return found;
  }
  return 0;
}

int directorycmp(char* dir1, char* dir2) {
  int dir1_len = strlen(dir1);
  int dir2_len = strlen(dir2);
  // ignore trailing slashes
  if (dir1[dir1_len - 1] == DIR_SEPARATOR_CHAR) {
    dir1_len--;
  }
  if (dir2[dir2_len - 1] == DIR_SEPARATOR_CHAR) {
    dir2_len--;
  }
  if (dir1_len == dir2_len) {
    return strncmp(dir1, dir2, dir1_len);
  }
  return strcmp(dir1, dir2);
}

file_list_t* files_in_directories(node_t* directories) {
  file_list_t* files = NULL;
  file_list_t* tail = NULL;
  file_list_t* new_file = NULL;
  node_t* curr = NULL;
  for (curr = directories; curr; curr = curr->next) {
    DIR *dir = opendir(curr->val);
    struct dirent *dir_entry;
    if (dir) {
      while ((dir_entry = readdir(dir)) != NULL) {
        char* full_path = file_join(curr->val, dir_entry->d_name);
        if (!directory_exists(full_path)) { // make sure it is not a directory
          new_file = malloc(sizeof(file_list_t));
          new_file->next = NULL;
          new_file->directory = strdup(curr->val);
          new_file->filename = strdup(dir_entry->d_name);
          new_file->full_path = full_path;
          new_file->executable = file_is_executable(full_path);
          if (!files) files = new_file;
          if (tail) tail->next = new_file;
          tail = new_file;
        } else {
          free(full_path);
        }
      }
      closedir(dir);
    }
  }
  return files;
}

void free_file_list(file_list_t* files) {
  file_list_t* entry;
  file_list_t* tmp;

  for (entry = files; entry;) {
    tmp = entry;
    entry = entry->next;
    free(tmp->filename);
    free(tmp->directory);
    free(tmp->full_path);
    free(tmp);
  }
}
