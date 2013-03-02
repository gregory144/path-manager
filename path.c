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
  char delims[] = ":";
  int num_entries = 0;
  int total_string_length = 0;

  char* tok_state;
  char* result = strtok_r(path_s, delims, &tok_state);
  if (result) {
    path->head = path_construct_entry(result);
    num_entries++;
    total_string_length += strlen(result);
  }
  path_entry_t* prev = path->head;
  while (result) {
    result = strtok_r(NULL, delims, &tok_state);
    if (result) {
      path_entry_t* next = path_construct_entry(result);
      num_entries++;
      total_string_length += strlen(result);
      prev->next = next;
      prev = next;
    }
  }
  path->num_entries = num_entries;
  total_string_length += num_entries - 1; // for ':' separators
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

void path_warnings(path_t* path) {
  path_entry_t* curr;
  for (curr = path->head; curr; curr = curr->next) {
    if (file_exists(curr->directory) != 0) {
      if (directory_is_absolute(curr->directory) != 0) {
        print_warning("Directory `%s` is not an absolute directory and does not exist\n", curr->directory);
      } else {
        print_warning("Directory `%s` does not exist\n", curr->directory);
      }
    } else if (directory_exists(curr->directory) != 0) {
      print_warning("File `%s` is not a directory\n", curr->directory);
    } else if (directory_readable(curr->directory) != 0) {
      print_warning("Directory `%s` not readable\n", curr->directory);
    } else if (directory_contains_executable_files(curr->directory) != 0) {
      print_warning("Directory `%s` contains no executable files\n", curr->directory);
    }
  }
}

char* path_to_string(path_t* path) {
  char* pathStr = calloc(path->total_string_length + 1, sizeof(char));
  path_entry_t* curr;
  for (curr = path->head; curr; curr = curr->next) {
    strncat(pathStr, curr->directory, strlen(curr->directory));
    if (curr->next) {
      strncat(pathStr, ":", 1);
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
