#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>

#include "path.h"
#include "util.h"
#include "file.h"
#include "match.h"

#define ENV_VAR_NAME "PATH"

typedef enum { print_path, print_export, print_list, print_search, print_quiet } print_mode_t;

// cached copy of all files in the path
file_list_t* all_files = NULL;

void display_usage() {
  printf("Usage: path\n");
  exit(EXIT_FAILURE);
}

file_list_t* get_all_files(path_t* path) {
  if (!all_files) {
    node_t* directory_names = path_directories(path);
    all_files = files_in_directories(directory_names);
    free_nodes(directory_names);
  }
  return all_files;
}

void free_all_files() {
  if (all_files) {
    free_file_list(all_files);
  }
}

int main(int argc, char **argv) {
  int c;
  print_mode_t print_mode = print_path;
  export_mode_t export_mode = export_default;
  bool install = false;
  bool install_global = false;
  bool modified = false;
  bool failed = false;

  node_t* dirs_to_add = NULL;
  node_t* dirs_to_remove = NULL;
  node_t* files_to_search = NULL;
  node_t* tmp;

  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"add",     required_argument, 0, 'a'},
      {"rm",      required_argument, 0, 'r'},
      {"search",  required_argument, 0, 's'},
      {"export",  optional_argument, 0, 'e'},
      {"install", no_argument,       0, 'i'},
      {"install-global", no_argument,0, 'I'},
      {"list",    no_argument,       0, 'l'},
      {"verbose", no_argument,       0, 'v'},
      {"warn",    no_argument,       0, 'w'},
      {"quiet",   no_argument,       0, 'q'},
      {"help",    no_argument,       0, 'h'}
    };

    c = getopt_long(argc, argv, "-a:r:s:eiIlvwqh?", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {

      case 'a':
        if (optarg[0] != '/') {
          fprintf(stderr, "Could not add %s to %s, it must be an absolute path.\n", optarg, ENV_VAR_NAME);
          failed = true;
        } else {
          modified = true;
          tmp = malloc(sizeof(node_t));
          tmp->val = strdup(optarg);
          tmp->next = dirs_to_add;
          dirs_to_add = tmp;
        }
        break;

      case 'r':
        if (optarg[0] != '/') {
          fprintf(stderr, "Could not remove %s from %s, it must be an absolute path.\n", optarg, ENV_VAR_NAME);
          failed = true;
        } else {
          modified = true;
          tmp = malloc(sizeof(node_t));
          tmp->val = strdup(optarg);
          tmp->next = dirs_to_remove;
          dirs_to_remove = tmp;
        }
        break;

      case 's':
        print_mode = print_search;

        tmp = malloc(sizeof(node_t));
        tmp->val = strdup(optarg);
        tmp->next = files_to_search;
        files_to_search = tmp;
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

      case 'I':
        install_global = true;
        break;

      case 'i':
        install = true;
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
        display_usage();
        failed = true;
        break;
    }
  }

  if (!failed) {

    if (install || install_global) {
      install_in_shell(install_global);
    }

    path_t* path = path_load(ENV_VAR_NAME);

    node_t* entry;

    for (entry = dirs_to_add; entry; entry = entry->next) {
      print_verbose("Adding `%s` to %s\n", tmp->val, ENV_VAR_NAME);
      int added = path_add(path, entry->val);
      if (!added) {
        print_warning("Directory `%s` not added to %s due to warnings. Use --force to override.\n", entry->val, ENV_VAR_NAME);
      }
    }

    for (entry = dirs_to_remove; entry; entry = entry->next) {
      print_verbose("Removing `%s` from .\n", entry->val, ENV_VAR_NAME);
      path_rm(path, entry->val);
    }

    path_clean(path);

    if (warnings_are_on()) {
      path_warnings(path);
    }

    // give a warning if the user doesn't export using command substitution
    if ((modified || print_mode == print_export) && isatty(fileno(stdout))) {
      fprintf(stderr, "In order to save %s for the current shell, use command substitution, like:\n", ENV_VAR_NAME);
      char* orig_cmd = get_cmdline(argc, argv);
      fprintf(stderr, "`%s`\n", orig_cmd);
      free(orig_cmd);
    }

    switch (print_mode) {
      case print_search:
        if (files_to_search) {
          file_list_t* files = get_all_files(path);
          for (entry = files_to_search; entry; entry = entry->next) {
            print_verbose("Searching for: `%s`\n", entry->val);
            char* search_result = path_search(path, entry->val);
            if (search_result) {
              printf("exact: %s\n", search_result);
            } else {
              print_warning("`%s` not found in %s.\n", entry->val, ENV_VAR_NAME);
              match_t* matches = find_best_matches(entry->val, files);
              match_t* curr_match;
              int i;
              for (i = 0; i < 10; i++) {
                curr_match = &matches[i];
                if (curr_match && curr_match->metric > 0.4) {
                  printf("almost: %s", curr_match->file->full_path);
                  if (!curr_match->file->executable) {
                    printf(" (not executable)");
                  }
                  printf("\n");
                }
              }
              free(matches);
            }
            free(search_result);
          }
        }
        break;
      case print_export: {
          path_export(path, ENV_VAR_NAME, export_mode);
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
      case print_path:
      default: {
          path_entry_t* entry;

          for (entry = path->head; entry; entry = entry->next) {
            printf("%s\n", entry->directory);
          }
        }
        break;
    }

    if (modified) {
      path_save(path);
    }

    path_free(path);
  }

  free_all_files();
  free_nodes(dirs_to_add);
  free_nodes(dirs_to_remove);
  free_nodes(files_to_search);

  if (failed) {
    free_nodes(dirs_to_add);
    free_nodes(dirs_to_remove);
    free_nodes(files_to_search);
    exit(EXIT_FAILURE);
  }
  return 0;
}

