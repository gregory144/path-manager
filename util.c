#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>

#include "util.h"
#include "file.h"

#define HOME_ENV_VAR "HOME"

bool path_verbose_on = false;

bool path_warnings_on = false;

void turn_verbose_on() {
  path_verbose_on = true;
}

void turn_warnings_on() {
  path_warnings_on = true;
}

bool warnings_are_on() {
  return path_warnings_on;
}

void print_verbose(char* s, ...) {
  if (path_verbose_on) {
    va_list va;
    va_start(va, s);
    vprintf(s, va);
    va_end(va);
  }
}

void print_warning(char* s, ...) {
  if (path_warnings_on) {
    printf("Warning: ");
    va_list va;
    va_start(va, s);
    vprintf(s, va);
    va_end(va);
  }
}

// result must be free'd
char* get_home_directory() {
  char* home = getenv(HOME_ENV_VAR);
  bool dontDup = false;
  if (!home) {
    struct passwd* passwd_data = getpwuid(geteuid());
    if (passwd_data) {
      home = passwd_data->pw_dir;
    } else {
      home = getcwd(NULL, 0);
      dontDup = true;
    }
  }
  return dontDup ? home : strdup(home);
}

char* get_executable_file() {
  int buf_size = 4096 * sizeof(char);
  char* buf = malloc(buf_size);
  // TODO dynamically allocate buffer for correctness
  int readlink_ret = readlink("/proc/self/exe", buf, buf_size - 1);
  if (readlink_ret < sizeof(buf)) {
    print_warning("Could not find my own directory");
  }
  return buf;
}

bool install_in_shell(bool global) {

  //user mode - install in profile, rc file
  //global mode - install in /etc/profile and /etc/bash.bashrc
  char *profile_filename = NULL;
  char *rc_filename = NULL;

  if (global) {
    if (directory_exists("/etc/conf.d")) {
      profile_filename = "/etc/conf.d/path.sh";
    } else if (file_exists("/etc/profile")) {
      profile_filename = "/etc/profile";
    }
    if (file_exists("/etc/bash.bashrc")) {
      rc_filename = "/etc/bash.bashrc";
    }
  } else {
    char* home_dir = get_home_directory();
    int i;
    char* profile_search_list[] = {".bash_profile", ".bash_login", ".profile"};
    for (i = 0; i < 3; i++) {
      char* filename = file_join(home_dir, profile_search_list[i]);
      if (file_exists(filename)) {
        profile_filename = filename;
        break;
      }
      free(filename);
    }
    if (!profile_filename) {
      profile_filename = file_join(home_dir, profile_search_list[0]);
    }

    char* filename = file_join(home_dir, ".bashrc");
    printf("Does %s exist?\n", filename);
    if (file_exists(filename)) {
      printf("Yes\n");
      rc_filename = filename;
    } else {
      free(filename);
    }

    free(home_dir);
  }

  char* executable_filename = get_executable_file();
  char* script = "\n[[ -s \"%s\" ]] && `%s --export`\n";
  int script_length = strlen(script) + (strlen(executable_filename) * 2);
  char* script_buf = malloc((script_length + 1) * sizeof(char));
  sprintf(script_buf, script, executable_filename, executable_filename);

  printf("Writing \"%s\"\n", script_buf);

  if (profile_filename) {
    // TODO better error handling
    print_verbose("Installing in %s.\n", profile_filename);
    FILE* profile_file = fopen(profile_filename, "a");
    if (profile_file) {
      int wrote = fprintf(profile_file, "%s", script_buf);
      printf("Wrote %d\n", wrote);
      fclose(profile_file);
    } else {
      print_verbose("Failed to write to %s\n", profile_filename);
    }
  }

  if (rc_filename) {
    print_verbose("Installing in %s.\n", rc_filename);
    FILE* rc_file = fopen(rc_filename, "a");
    if (rc_file) {
      int wrote = fprintf(rc_file, "%s", script_buf);
      printf("Wrote %d, %d\n", wrote, errno);
      fclose(rc_file);
    } else {
      print_verbose("Failed to write to %s\n", rc_filename);
    }
  }

  free(script_buf);
  free(executable_filename);
  free(profile_filename);
  return true;
}

void free_nodes_and_vals(node_t* node) {
  free_nodes(node, 1);
}

void free_nodes(node_t* node, int freeVals) {
  node_t* entry;
  node_t* tmp;

  for (entry = node; entry;) {
    tmp = entry;
    entry = entry->next;
    if (freeVals) free(tmp->val);
    free(tmp);
  }
}
