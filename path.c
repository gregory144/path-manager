#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"

int main() {
  // adds PATH entry to .bashrc
  char* origPath = getenv("PATH");
  int pathLen = 0;
  char* pathStr = "";
  if (origPath != NULL) {
    pathLen = strlen(origPath);
    pathStr = strndup(origPath, pathLen);
  }
  path_t* path = parsePath(pathStr);
  cleanPath(path);
  char* currPath = getPath(path);
  printf("%s\n", currPath);
  free(currPath);
  freePath(path);
  free(pathStr);
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
  path_entry_t* existingEntry;
  path_entry_t* currEntry;
  path_entry_t* prevEntry;
  for (existingEntry = path->firstEntry; existingEntry; existingEntry = existingEntry->next) {
    prevEntry = existingEntry;
    for (currEntry = existingEntry->next; currEntry; currEntry = currEntry->next) {
      if (strcmp(existingEntry->directory, currEntry->directory) == 0) {
        path->totalStringLength -= strlen(currEntry->directory) + 1;
        path->numEntries--;
        prevEntry->next = currEntry->next;
        free(currEntry);
        currEntry = prevEntry;
      } else {
        prevEntry = currEntry;
      }
    }
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
