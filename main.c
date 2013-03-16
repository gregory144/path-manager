#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>

#include "path.h"
#include "util.h"
#include "file.h"
#include "match.h"

#define EXE_NAME "path"
#define ENV_VAR_NAME "PATH"

typedef enum { print_path, print_export, print_list, print_search, print_quiet, print_version } print_mode_t;

// cached copy of all files in the path
file_list_t* all_files = NULL;

void display_usage() {
  printf("\
Usage: %s [OPTION]...\n\
   or: `%s -e [--add DIRECTORY|--rm DIRECTORY]`\n\
", EXE_NAME, EXE_NAME);
  printf("Display/Search/Modify your %s environment variable.\n", ENV_VAR_NAME);
  printf("\
  [no options]           display all entries in your %s (one per line)\n\
  -a, --add=DIRECTORY    add the given directory to your %s environment\n\
                           variable\n\
  -r, --rm=DIRECTORY     remove the given directory from your %s environment\n\
                           variable\n\
  -l, --list             list all files in your %s\n\
  -s, --search=BASENAME  search your %s for the given executable filename;\n\
                           this will only search the basename (not the whole\n\
                           absolute path to a file)\n\
  -e, --export=METHOD    output the command to set the %s environment variable;\n\
                           METHOD is 'export' (default) or 'setenv'\n\
  -i, --install          setup bash to save any changes by %s over all future\n\
                           sessions; searches and modifies the first\n\
                           interactive and non-interactive bash startup\n\
                           scripts it can find (ex: .bash_profile, .bashrc)\n\
  -I, --install-global   setup bash to save any changes by %s over all future\n\
                           sessions for all users; search and modifies the\n\
                           first interactive and non-interactive bash startup\n\
                           scripts it can find (ex: /etc/profile.d,\n\
                           /etc/bash.bashrc); must have permissions to write\n\
                           these files\n\
  -w, --warn             show warnings\n\
  -q, --quiet            quiet output\n\
  -v, --version          show version information\n\
  -h, --help             show this help message\n\
  --verbose              show verbose output\n\
", ENV_VAR_NAME, ENV_VAR_NAME, ENV_VAR_NAME, ENV_VAR_NAME, ENV_VAR_NAME, ENV_VAR_NAME,
    EXE_NAME, EXE_NAME);
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
      {"warn",    no_argument,       0, 'w'},
      {"quiet",   no_argument,       0, 'q'},
      {"version", no_argument,       0, 'v'},
      {"help",    no_argument,       0, 'h'},
      {"verbose", no_argument,       0,  0 }
    };

    c = getopt_long(argc, argv, "-a:r:s:eiIlvwqh?", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {

      case 'a':
        {
          char* path = get_absolute_path(optarg);
          if (!path) {
            path = strdup(optarg);
          }
          modified = true;
          tmp = malloc(sizeof(node_t));
          tmp->val = path;
          tmp->next = dirs_to_add;
          dirs_to_add = tmp;
        }
        break;

      case 'r':
        {
          char* path = get_absolute_path(optarg);
          if (!path) {
            path = strdup(optarg);
          }
          modified = true;
          tmp = malloc(sizeof(node_t));
          tmp->val = path;
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
        print_mode = print_version;
        break;

      case 'w':
        turn_warnings_on();
        break;

      case 'q':
        print_mode = print_quiet;
        break;

      case 0:
        if( strcmp( "verbose", long_options[option_index].name ) == 0 ) {
          turn_verbose_on();
          turn_warnings_on();
        }
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
      fprintf(stderr, "`%s", orig_cmd);
      if (print_mode != print_export) {
        fprintf(stderr, " -e");
      }
      fprintf(stderr, "`\n");

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
      case print_version:
        printf("%s %s\n", EXE_NAME, PATH_VERSION);
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

