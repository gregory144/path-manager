#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "file.h"
#include "util.h"

int file_exists(char* dir_s) {
  struct stat s;
  int err = stat(dir_s, &s);
  if (err != -1) {
    return 0;
  }
  return 1;
}

int directory_exists(char* dir_s) {
  struct stat s;
  int err = stat(dir_s, &s);
  if (err != -1 && S_ISDIR(s.st_mode)) {
    return 0;
  }
  return 1;
}

int directory_readable(char* dir_s) {
  DIR *dir = opendir(dir_s);
  if (dir) {
    closedir(dir);
    return 0;
  } else {
    return errno;
  }
}

int directory_is_absolute(char* dir_s) {
  return dir_s[0] - '/';;
}

int file_is_executable(char* file_s) {
  print_verbose("Checking file: %s\n", file_s);
  struct stat s;
  if (!stat(file_s, &s)) {
    if (S_ISREG(s.st_mode) && s.st_mode & 0111) {
      return 0;
    }
  }
  return 1;
}

char* file_join(char* dir, char* file) {
  int dir_size = strlen(dir);
  if (dir[dir_size - 1] == '/') {
    dir_size--;
  }
  int file_size = strlen(file);
  char* joined = malloc((dir_size + 1 + file_size) * sizeof(char));
  strncat(joined, dir, dir_size);
  strncat(joined, "/", 1);
  strncat(joined, file, file_size);
  return joined;
}

int directory_contains_executable_files(char* dir_s) {
  DIR *dir = opendir(dir_s);
  struct dirent *entry;
  int found = 1;
  if (dir) {
    while ((entry = readdir(dir)) != NULL) {
      char* filename = file_join(dir_s, entry->d_name);
      if (file_is_executable(filename) == 0) {
        found = 0;
        break;
      }
    }
    closedir(dir);
    return found;
  } else {
    return errno;
  }
}
