#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h> // for basename
#include <unistd.h> // for symlink

#include "path.h"
#include "file.h"
#include "util.h"

char* get_path_directory() {
  char* home_dir = get_home_directory();
  char* path_directory = file_join(home_dir, ".path");
  free(home_dir);
  return path_directory;
}

path_entry_t* path_construct_entry(char* directory, bool modifiable) {
  path_entry_t* entry = malloc(sizeof(path_entry_t));
  entry->directory = strdup(directory);
  entry->next = NULL;
  entry->modifiable = modifiable;
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
    path->head = path_construct_entry(result, false);
    num_entries++;
    total_string_length += strlen(result);
  }
  path_entry_t* prev = path->head;
  while (result) {
    result = strtok_r(NULL, delim, &tok_state);
    if (result) {
      path_entry_t* next = path_construct_entry(result, false);
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

path_t* path_load(char* env_var_name) {
  // load the default path from the environment
  char* orig_path = getenv(env_var_name);
  char* path_s = "";
  if (orig_path != NULL) {
    path_s = strdup(orig_path);
  } else {
    print_verbose("Unable to read %s environment variable.\n", env_var_name);
  }

  path_t* path = path_parse(path_s);
  free(path_s);

  // load the path from ~/.path
  char* path_directory = get_path_directory();
  if (directory_exists(path_directory)) {
    // TODO must load them in order!
    file_list_t* files = list_directory(path_directory, false);
    files = sort_files(files);

    file_list_t* curr;
    for (curr = files; curr; curr = curr->next) {
      char* target = symlink_target(curr->full_path);
      path_add(path, target);
      free(target);
    }
    free_file_list(files);
  }

  free(path_directory);

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
 * Returns true if there are warnings for the given directory,
 * false otherwise.
 */
bool path_warnings_for_directory(char* dir) {
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
    return false;
  }
  return true;
}

bool path_warnings(path_t* path) {
  bool warnings = false;
  path_entry_t* curr;
  for (curr = path->head; curr; curr = curr->next) {
    warnings |= path_warnings_for_directory(curr->directory);
  }
  return warnings;
}

bool path_save(path_t* path) {
  char* path_directory = get_path_directory();
  //clear out directory
  if (!clear_dir(path_directory)) {
    fprintf(stderr, "Failed to remove directory %s\n", path_directory);
  } else {
    if (!mkdir_for_user(path_directory)) {
      fprintf(stderr, "Failed to create directory %s\n", path_directory);
    } else {
      path_entry_t* curr;
      int i;
      for (curr = path->head, i = 0; curr; curr = curr->next, i++) {
        // dont link a file that is already in ~/.path
        if (curr->modifiable && strncmp(curr->directory, path_directory, strlen(path_directory)) != 0) {
          char i_buf[50];
          snprintf(i_buf, 50, "%d", i);
          char* symlink_filename = file_join(path_directory, i_buf);
          print_verbose("Linking %s to %s\n", symlink_filename, curr->directory);
          if (symlink(curr->directory, symlink_filename) != 0) {
            fprintf(stderr, "Failed to save path entry %s to %s\n", curr->directory, path_directory);
          }
          free(symlink_filename);
        }
      }
    }
  }

  free(path_directory);
  return true;
}

void path_export(path_t* path, char* env_var_name, export_mode_t export_mode) {
  char* processed_path = path_to_string(path);
  char* cmd_name = "export";
  char* key_value_separator = "=";
  if (export_mode == export_setenv) {
    cmd_name = "setenv";
    key_value_separator = " ";
  }
  printf("%s %s%s%s\n", cmd_name, env_var_name, key_value_separator, processed_path);
  free(processed_path);
}

bool path_add(path_t* path, char* directory) {
  // remove the file if it's already on the path first
  path_rm(path, directory);
  bool warnings = path_warnings_for_directory(directory);
  path_entry_t* path_entry = path_construct_entry(directory, true);
  path->num_entries++;
  path->total_string_length += strlen(directory);
  if (path->head) {
    path->total_string_length++;
  }
  path_entry->next = path->head;
  path->head = path_entry;
  return warnings;
}

bool path_rm(path_t* path, char* directory) {
  path_entry_t* curr;
  path_entry_t* prev = NULL;
  bool removed = 0;
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
      free(curr->directory);
      free(curr);
      if (prev) {
        curr = prev;
      } else {
        curr = path->head;
      }
      removed = true;
    }
    prev = curr;
  }
  return removed;
}

node_t* path_search(path_t* path, char* file) {
  path_entry_t* path_entry;
  node_t* head = NULL;
  node_t* tail = NULL;
  node_t* curr = NULL;
  for (path_entry = path->head; path_entry; path_entry = path_entry->next) {
    char* full_file = file_join(path_entry->directory, file);
    if (file_is_executable(full_file)) {
      curr = malloc(sizeof(node_t));
      curr->val = full_file;
      curr->next = NULL;
      if (!head) head = curr;
      if (tail) tail->next = curr;
      tail = curr;
    } else {
      free(full_file);
    }
  }
  return head;
}

node_t* path_directories(path_t* path) {
  path_entry_t* path_entry = NULL;
  node_t* head = NULL;
  node_t* tail = NULL;
  node_t* curr = NULL;
  for (path_entry = path->head; path_entry; path_entry = path_entry->next) {
    curr = malloc(sizeof(node_t));
    curr->val = strdup(path_entry->directory);
    curr->next = NULL;
    if (!head) head = curr;
    if (tail) tail->next = curr;
    tail = curr;
  }
  return head;
}

char* path_to_string(path_t* path) {
  char* path_str = calloc(path->total_string_length + 1, sizeof(char));
  path_entry_t* curr;
  for (curr = path->head; curr; curr = curr->next) {
    strncat(path_str, curr->directory, strlen(curr->directory));
    if (curr->next) {
      strncat(path_str, (char[]){ PATH_SEPARATOR_CHAR }, 1);
    }
  }
  return path_str;
}

void path_free(path_t* path) {
  path_entry_t* curr;
  path_entry_t* tmp_entry;
  for (curr = path->head; curr;) {
    tmp_entry = curr;
    curr = curr->next;
    free(tmp_entry->directory);
    free(tmp_entry);
  }
  free(path);
}
