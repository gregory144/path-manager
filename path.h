
typedef struct path_t {
  int numEntries;
  char** entries;
} path_t;

int countOccurences(char* str, char c);

path_t* parsePath(char* pathStr);

char* getPath(path_t* path);

void freePath(path_t *path);
