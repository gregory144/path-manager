
typedef struct node_t {
  struct node_t* next;
  char* val;
} node_t;

void turn_verbose_on();

void turn_warnings_on();

int warnings_are_on();

void print_verbose(char* s, ...);

void print_warning(char* s, ...);

void free_nodes(node_t* node);

node_t* find_best_matches(char* needle, node_t* haystack);

