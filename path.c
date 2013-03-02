#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"

int main() {
  // adds PATH entry to .bashrc
  char* origPath = getenv("PATH");
  if (origPath != NULL) {
    int pathLen = strlen(origPath);
    char* pathStr = strndup(origPath, pathLen);
    path_t* path = parsePath(pathStr);
    int i = 0;
    while (i < path->numEntries) {
      printf("Path Entry: %s\n", path->entries[i]);
      i++;
    }
    char* currPath = getPath(path);
    printf("The current path is: %s\n", currPath);
    free(currPath);
    freePath(path);
    free(pathStr);
  } else {
    printf("PATH not set.\n");
  }
  return 0;
}

int countOccurences(char* path, char c) {
  int i, occurrences = 0;
  for (i = 1; path[i]; i++) {
    if (path[i] == c) {
      occurrences++;
    }
  }
  return occurrences;
}

path_t* parsePath(char* pathStr) {
  path_t* path = malloc(sizeof(path_t));
  char delims[] = ":";
  path->numEntries = countOccurences(pathStr, delims[0]) + 1;
  path->entries = malloc(path->numEntries * sizeof(char*));
  char* result = strtok(pathStr, delims);
  int i = 0;
  while (result) {
    path->entries[i] = result;
    result = strtok(NULL, delims);
    i++;
  }
  return path;
}

char* getPath(path_t* path) {
  int size = 0;
  int i = 0;
  while (i < path->numEntries) {
    size += strlen(path->entries[i]);
    size++;
    i++;
  }
  char* pathStr = malloc(size * sizeof(char));
  i = 0;
  while (i < path->numEntries) {
    strncat(pathStr, path->entries[i], strlen(path->entries[i]));
    i++;
    if (i < path->numEntries) {
      strncat(pathStr, ":", 1);
    }
  }
  return pathStr;
}

void freePath(path_t* path) {
  free(path->entries);
  free(path);
}
