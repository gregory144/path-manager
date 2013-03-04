#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"
#include "util.h"

int main() {
  turn_verbose_on();
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
  path_add(path, "/home/gregory144/src");
  path_add(path, "/home/gregory144/src2");
  path_add(path, "/home/gregory144/src3");
  path_add(path, "test1");
  path_add(path, "test2");
  path_rm(path, "/usr/games");
  path_clean(path);
  path_warnings(path);
  char* searchResult = path_search(path, "exe1");
  if (searchResult) {
    printf("Found: %s\n", searchResult);
  }
  char* processed_path = path_to_string(path);
  printf("%s\n", processed_path);
  free(processed_path);
  path_free(path);
  free(path_s);
  return 0;
}

