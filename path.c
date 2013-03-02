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
    path_entry_t* currEntry;
    for (currEntry = path->firstEntry; currEntry != NULL; currEntry = currEntry->next) {
      printf("Path Entry: %s\n", currEntry->directory);
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

path_entry_t* constructPathEntry(char* directory) {
  path_entry_t* entry = malloc(sizeof(path_entry_t));
  entry->directory = directory;
  entry->next = NULL;
  return entry;
}

path_t* parsePath(char* pathStr) {
  path_t* path = malloc(sizeof(path_t));
  char delims[] = ":";
  path->numEntries = countOccurences(pathStr, delims[0]) + 1;
  path->totalStringLength = strlen(pathStr);

  char* result = strtok(pathStr, delims);
  path->firstEntry = constructPathEntry(result);
  path_entry_t* prevEntry = path->firstEntry;
  while (result) {
    result = strtok(NULL, delims);
    if (result) {
      path_entry_t* nextEntry = constructPathEntry(result);
      prevEntry->next = nextEntry;
      prevEntry = nextEntry;
    }
  }
  return path;
}

void cleanPath(path_t* path) {
  // remove duplicate path entries
  path_entry_t* currEntry;
  for (currEntry = path->firstEntry; currEntry; currEntry = currEntry->next) {
    /*if (strcmp(iEntry, jEntry) == 0) {*/
    /*}*/
  }
}

char* getPath(path_t* path) {
  char* pathStr = calloc(path->totalStringLength + 1, sizeof(char));
  path_entry_t* currEntry;
  for (currEntry = path->firstEntry; currEntry; currEntry = currEntry->next) {
    strncat(pathStr, currEntry->directory, strlen(currEntry->directory));
    if (currEntry->next) {
      strncat(pathStr, ":", 1);
    }
  }
  return pathStr;
}

void freePath(path_t* path) {
  path_entry_t* currEntry;
  path_entry_t* tmpEntry;
  for (currEntry = path->firstEntry; currEntry;) {
    tmpEntry = currEntry;
    currEntry = currEntry->next;
    free(tmpEntry);
  }
  free(path);
}
