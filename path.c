#include <stdlib.h>
#include <string.h>

#include "path.h"
#include "file.h"
#include "util.h"

path_entry_t* path_construct_entry(char* directory) {
  path_entry_t* entry = malloc(sizeof(path_entry_t));
  entry->directory = directory;
  entry->next = NULL;
  return entry;
}

path_t* path_parse(char* path_s) {
  path_t* path = malloc(sizeof(path_t));
  int num_entries = 0;
  int total_string_length = 0;

  char* tok_state;
  static char delim[] = { PATH_SEPARATOR_CHAR };
  char* result = strtok_r(path_s, delim, &tok_state);
  if (result) {
    path->head = path_construct_entry(result);
    num_entries++;
    total_string_length += strlen(result);
  }
  path_entry_t* prev = path->head;
  while (result) {
    result = strtok_r(NULL, delim, &tok_state);
    if (result) {
      path_entry_t* next = path_construct_entry(result);
      num_entries++;
      total_string_length += strlen(result);
      prev->next = next;
      prev = next;
    }
  }
  path->num_entries = num_entries;
  total_string_length += num_entries - 1; // for path separators
  path->total_string_length = total_string_length;
  return path;
}

void path_clean(path_t* path) {
  // remove duplicate path entries
  path_entry_t* existing;
  path_entry_t* curr;
  path_entry_t* prev;
  for (existing = path->head; existing; existing = existing->next) {
    prev = existing;
    for (curr = existing->next; curr; curr = curr->next) {
      if (strcmp(existing->directory, curr->directory) == 0) {
        print_verbose("Removing duplicate directory: %s\n", curr->directory);
        path->total_string_length -= strlen(curr->directory) + 1;
        path->num_entries--;
        prev->next = curr->next;
        free(curr);
        curr = prev;
      } else {
        prev = curr;
      }
    }
  }
}

/**
 * Returns 1 if there are warnings for the given directory,
 * 0 otherwise.
 */
int path_warnings_for_directory(char* dir) {
  if (!file_exists(dir)) {
    if (!directory_is_absolute(dir)) {
      print_warning("Directory `%s` is not an absolute directory and does not exist\n", dir);
    } else {
      print_warning("Directory `%s` does not exist\n", dir);
    }
  } else if (!directory_exists(dir)) {
    print_warning("File `%s` is not a directory\n", dir);
  } else if (!directory_readable(dir)) {
    print_warning("Directory `%s` not readable\n", dir);
  } else if (!directory_contains_executable_files(dir)) {
    print_warning("Directory `%s` contains no executable files\n", dir);
  } else {
    return 0;
  }
  return 1;
}

void path_warnings(path_t* path) {
  path_entry_t* curr;
  for (curr = path->head; curr; curr = curr->next) {
    path_warnings_for_directory(curr->directory);
  }
}

int path_add(path_t* path, char* directory) {
  print_verbose("Adding `%s` to path.\n", directory);
  int warnings = path_warnings_for_directory(directory);
  path_entry_t* path_entry = path_construct_entry(directory);
  path->num_entries++;
  path->total_string_length += strlen(directory);
  if (path->head) {
    path->total_string_length++;
  }
  path_entry->next = path->head;
  path->head = path_entry;
  return warnings;
}

int path_rm(path_t* path, char* directory) {
  print_verbose("Removing `%s` from path.\n", directory);
  path_entry_t* curr;
  path_entry_t* prev;
  int removed = 0;
  for (curr = path->head; curr; curr = curr->next) {
    if (directorycmp(directory, curr->directory) == 0) {
      path->total_string_length -= strlen(curr->directory);
      if (path->num_entries > 1) path->total_string_length--;
      path->num_entries--;
      if (curr == path->head) {
        path->head = curr->next;
      } else {
        prev->next = curr->next;
      }
      free(curr);
      curr = prev;
      removed++;
    }
    prev = curr;
  }
  return removed;
}

char* path_search(path_t* path, char* file) {
  path_entry_t* curr;
  for (curr = path->head; curr; curr = curr->next) {
    char* fullFile = file_join(curr->directory, file);
    if (file_is_executable(fullFile)) {
      return fullFile;
    }
    free(fullFile);
  }
  return NULL;
}

char* path_to_string(path_t* path) {
  char* pathStr = calloc(path->total_string_length + 1, sizeof(char));
  path_entry_t* curr;
  for (curr = path->head; curr; curr = curr->next) {
    strncat(pathStr, curr->directory, strlen(curr->directory));
    if (curr->next) {
      strncat(pathStr, (char[]){ PATH_SEPARATOR_CHAR }, 1);
    }
  }
  return pathStr;
}

void path_free(path_t* path) {
  path_entry_t* curr;
  path_entry_t* tmpEntry;
  for (curr = path->head; curr;) {
    tmpEntry = curr;
    curr = curr->next;
    free(tmpEntry);
  }
  free(path);
}
