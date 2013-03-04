#define PATH_SEPARATOR_CHAR ':'

typedef struct path_t {
  int num_entries;
  struct path_entry_t* head;
  int total_string_length;
} path_t;

typedef struct path_entry_t {
  char* directory;
  struct path_entry_t* next;
} path_entry_t;

path_t* path_parse(char* pathStr);

void path_clean(path_t* path);

void path_warnings(path_t* path);

int path_add(path_t* path, char* directory);

int path_rm(path_t* path, char* directory);

char* path_search(path_t* path, char* file);

char* path_to_string(path_t* path);

void path_free(path_t *path);
