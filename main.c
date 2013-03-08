#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include "path.h"
#include "util.h"
#include "file.h"
#include "match.h"

#define ENV_VAR_NAME "PATH"

typedef enum { print_default, print_export, print_list, print_search, print_quiet } print_mode_t;

typedef enum { export_default, export_setenv } export_mode_t;


// cached copy of all files in the path
node_t* directory_names = NULL;
file_list_t* all_files = NULL;

void display_usage() {
  printf("Usage: path\n");
  exit(EXIT_FAILURE);
}

file_list_t* get_all_files(path_t* path) {
  if (!all_files) {
    directory_names = path_directories(path);
    all_files = files_in_directories(directory_names);
  }
  return all_files;
}

void free_all_files() {
  if (all_files) {
    free_file_list(all_files);
    free_nodes(directory_names, 0);
  }
}

int main(int argc, char **argv) {
  int c;
  print_mode_t print_mode = print_default;
  export_mode_t export_mode = export_default;
  node_t* directoriesToAdd = NULL;
  node_t* directoriesToRemove = NULL;
  node_t* filesToSearch = NULL;
  node_t* tmp;

  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"add",     required_argument, 0, 'a'},
      {"rm",      required_argument, 0, 'r'},
      {"search",  required_argument, 0, 's'},
      {"export",  optional_argument, 0, 'e'},
      {"list",    no_argument,       0, 'l'},
      {"verbose", no_argument,       0, 'v'},
      {"warn",    no_argument,       0, 'w'},
      {"quiet",   no_argument,       0, 'q'},
      {"help",    no_argument,       0, 'h'}
    };

    c = getopt_long(argc, argv, "-a:r:s:elvwqh?", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {

      case 'a':
        tmp = malloc(sizeof(node_t));
        tmp->val = strdup(optarg);
        print_verbose("Adding: %s\n", tmp->val);
        tmp->next = directoriesToAdd;
        directoriesToAdd = tmp;
        break;

      case 'r':
        tmp = malloc(sizeof(node_t));
        tmp->val = strdup(optarg);
        print_verbose("Removing: %s\n", tmp->val);
        tmp->next = directoriesToRemove;
        directoriesToRemove = tmp;
        break;

      case 's':
        print_mode = print_search;

        tmp = malloc(sizeof(node_t));
        tmp->val = strdup(optarg);
        tmp->next = filesToSearch;
        filesToSearch = tmp;
        break;

      case 'e':
        print_mode = print_export;
        if (optarg) {
          if (strcmp(optarg, "setenv") == 0) {
            export_mode = export_setenv;
          } else {
            export_mode = export_default;
          }
        }
        break;

      case 'l':
        print_mode = print_list;
        break;

      case 'v':
        turn_verbose_on();
        turn_warnings_on();
        break;

      case 'w':
        turn_warnings_on();
        break;

      case 'q':
        print_mode = print_quiet;
        break;

      default:
      case 'h':
      case '?':
        free_nodes_and_vals(directoriesToAdd);
        free_nodes_and_vals(directoriesToRemove);
        free_nodes_and_vals(filesToSearch);
        display_usage();
        break;
    }
  }

  char* orig_path = getenv(ENV_VAR_NAME);
  int path_len = 0;
  char* path_s = "";
  if (orig_path != NULL) {
    path_len = strlen(orig_path);
    path_s = strndup(orig_path, path_len);
    print_verbose("%s=%s\n", ENV_VAR_NAME, path_s);
  } else {
    print_verbose("Unable to read %s environment variable.\n", ENV_VAR_NAME);
  }

  path_t* path = path_parse(path_s);

  node_t* entry;

  for (entry = directoriesToAdd; entry; entry = entry->next) {
    int added = path_add(path, entry->val);
    if (!added) {
      print_warning("Directory `%s` not added to the path due to warnings. Use --force to override.\n", entry->val);
    }
  }

  for (entry = directoriesToRemove; entry; entry = entry->next) {
    path_rm(path, entry->val);
  }

  path_clean(path);

  if (warnings_are_on()) {
    path_warnings(path);
  }

  switch (print_mode) {
    case print_search:
      if (filesToSearch) {
        file_list_t* files = get_all_files(path);
        for (entry = filesToSearch; entry; entry = entry->next) {
          print_verbose("Searching for: `%s`\n", entry->val);
          char* searchResult = path_search(path, entry->val);
          if (searchResult) {
            printf("exact: %s\n", searchResult);
          } else {
            print_warning("`%s` not found in path.\n", entry->val);
            match_t* matches = find_best_matches(entry->val, files);
            match_t* currMatch;
            int i;
            for (i = 0; i < 10; i++) {
              currMatch = &matches[i];
              if (currMatch && currMatch->metric > 0.4) {
                printf("almost: %s", currMatch->file->full_path);
                if (!currMatch->file->executable) {
                  printf(" (not executable)");
                }
                printf("\n");
              }
            }
            free(matches);
          }
          free(searchResult);
        }
      }
      break;
    case print_export: {
        char* processed_path = path_to_string(path);
        char* cmd_name = "export";
        char* key_value_separator = "=";
        if (export_mode == export_setenv) {
          cmd_name = "setenv";
          key_value_separator = " ";
        }
        printf("%s %s%s%s\n", cmd_name, ENV_VAR_NAME, key_value_separator, processed_path);
        free(processed_path);
      }
      break;
    case print_list: {
        file_list_t* files = get_all_files(path);
        file_list_t* entry = NULL;

        for (entry = files; entry; entry = entry->next) {
          if (entry->executable) {
            printf("%s\n", entry->full_path);
          }
        }
      }
      break;
    case print_quiet:
      break;
    case print_default:
    default: {
        char* processed_path = path_to_string(path);
        printf("%s\n", processed_path);
        free(processed_path);
      }
      break;
  }

  free_all_files();
  free_nodes_and_vals(directoriesToAdd);
  free_nodes_and_vals(directoriesToRemove);
  free_nodes_and_vals(filesToSearch);
  path_free(path);
  free(path_s);
  return 0;
}

