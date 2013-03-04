#define DIR_SEPARATOR_CHAR '/'

int file_exists(char* dir);

char* file_join(char* dir, char* file);

int directory_exists(char* dir);

int directory_readable(char* dir);

int directory_is_absolute(char* dir);

int directory_contains_executable_files(char* dir);

int file_is_executable(char* file);

int directorycmp(char* dir1, char* dir2);
