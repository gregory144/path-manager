#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include "path.h"
#include "util.h"
#include "file.h"
#include "match.h"

void display_usage() {
  printf("Usage: path\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  int c;
  int print = 1;
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
      {"verbose", no_argument,       0, 'v'},
      {"warn",    no_argument,       0, 'w'},
      {"quiet",   no_argument,       0, 'q'},
      {"help",    no_argument,       0, 'h'}
    };

    c = getopt_long(argc, argv, "-a:r:s:vwqh?", long_options, &option_index);
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
        // turn off print
        print = 0;

        tmp = malloc(sizeof(node_t));
        tmp->val = strdup(optarg);
        tmp->next = filesToSearch;
        filesToSearch = tmp;
        break;

      case 'v':
        turn_verbose_on();
        turn_warnings_on();
        break;

      case 'w':
        turn_warnings_on();
        break;

      case 'q':
        print = 0;
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

  char* orig_path = getenv("PATH");
  int path_len = 0;
  char* path_s = "";
  if (orig_path != NULL) {
    path_len = strlen(orig_path);
    path_s = strndup(orig_path, path_len);
    print_verbose("PATH=%s\n", path_s);
  } else {
    print_verbose("Unable to read PATH environment variable.\n");
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

  if (filesToSearch) {
    node_t* directories = path_directories(path);
    file_list_t* files = files_in_directories(directories);
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
    free_file_list(files);
    free_nodes(directories, 0);
  }

  if (warnings_are_on()) {
    path_warnings(path);
  }

  if (print) {
    char* processed_path = path_to_string(path);
    printf("%s\n", processed_path);
    free(processed_path);
  }

  free_nodes_and_vals(directoriesToAdd);
  free_nodes_and_vals(directoriesToRemove);
  free_nodes_and_vals(filesToSearch);
  path_free(path);
  free(path_s);
  return 0;
}

