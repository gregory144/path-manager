
typedef struct path_t {
  int numEntries;
  struct path_entry_t* firstEntry;
  int totalStringLength;
} path_t;

typedef struct path_entry_t {
  char* directory;
  struct path_entry_t* next;
} path_entry_t;

path_t* parsePath(char* pathStr);

void cleanPath(path_t* path);

char* getPath(path_t* path);

void freePath(path_t *path);
