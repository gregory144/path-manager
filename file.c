#define _XOPEN_SOURCE 500 // for nftw

#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ftw.h>
#include <stdio.h>

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

file_list_t* list_directory(char* directory, bool only_files) {
  file_list_t* files = NULL;
  file_list_t* tail = NULL;
  DIR *dir = opendir(directory);
  struct dirent *dir_entry;
  if (dir) {
    while ((dir_entry = readdir(dir)) != NULL) {
      if (strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
        char* full_path = file_join(directory, dir_entry->d_name);
        bool include_file = true;
        if (only_files) {
          include_file = !directory_exists(full_path); // make sure it is not a directory
        }
        if (include_file) {
          file_list_t* new_file = malloc(sizeof(file_list_t));
          new_file->next = NULL;
          new_file->directory = strdup(directory);
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
    }
    closedir(dir);
  }
  return files;
}

file_list_t* files_in_directories(node_t* directories) {
  file_list_t* files = NULL;
  file_list_t* tail = NULL;
  file_list_t* curr_files = NULL;
  node_t* curr = NULL;
  for (curr = directories; curr; curr = curr->next) {
    curr_files = list_directory(curr->val, true);
    if (curr_files) {
      if (files) {
        tail->next = curr_files;
      } else {
        files = curr_files;
        tail = files;
      }
      while (tail->next) { tail = tail->next; }
    }
  }
  return files;
}

int compare_files(const void *a, const void *b) {
  file_list_t** a_match = (file_list_t**)a;
  file_list_t** b_match = (file_list_t**)b;
  return strcmp(b_match[0]->filename, a_match[0]->filename);
}

file_list_t* sort_files(file_list_t* files) {
  if (files) {
    int num_files;
    file_list_t* curr;
    // count the number of files
    for (curr = files, num_files = 0; curr; curr = curr->next, num_files++);
    // create an array of pointers to each file_list_t
    file_list_t** file_pointers = calloc(num_files, sizeof(file_list_t*));
    int i;
    for (curr = files, i = 0; curr; curr = curr->next, i++) {
      file_pointers[i] = curr;
    }
    // sort them
    qsort(file_pointers, num_files, sizeof(file_list_t*), compare_files);
    // reconstruct a linked list of file_list_t
    files = file_pointers[0];
    for (i = 0; i < num_files; i++) {
      curr = file_pointers[i];
      curr->next = (i + 1 < num_files) ? file_pointers[i + 1] : NULL;
    }
    free(file_pointers);
  }
  return files;
}

char* symlink_target(char* link) {
  int buf_size = 4096;
  char* buf = calloc(buf_size, sizeof(char));
  // TODO dynamically allocate buffer for correctness
  int readlink_ret = readlink(link, buf, (buf_size - 1) * sizeof(char));
  if (readlink_ret == sizeof(buf) || readlink_ret == 0) {
    fprintf(stderr, "Could not get symlink target\n");
  }
  return buf;
}

char* get_absolute_path(char* relative_path) {
  return realpath(relative_path, NULL);
}

bool mkdir_for_user(char* path) {
  mode_t process_mask = umask(0);
  int result_code = mkdir(path, S_IRWXU | S_IRWXG);
  umask(process_mask);
  return result_code == 0;
}

static int remove_file_in_walk(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
  return remove(fpath);
}

bool clear_dir(char* path) {
  if (!directory_exists(path)) {
    return true;
  }
  return nftw(path, remove_file_in_walk, 20, FTW_DEPTH | FTW_PHYS) == 0;
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
