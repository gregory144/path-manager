#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"

int main() {
  // adds PATH entry to .bashrc
  char* orig_path = getenv("PATH");
  int path_len = 0;
  char* path_s = "";
  if (orig_path != NULL) {
    path_len = strlen(orig_path);
    path_s = strndup(orig_path, path_len);
  }
  path_t* path = path_parse(path_s);
  path_clean(path);
  char* processed_path = path_to_string(path);
  printf("%s\n", processed_path);
  free(processed_path);
  path_free(path);
  free(path_s);
  return 0;
}

