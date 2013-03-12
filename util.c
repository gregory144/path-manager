#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <libgen.h>

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
  bool dont_dup = false;
  if (!home) {
    struct passwd* passwd_data = getpwuid(geteuid());
    if (passwd_data) {
      home = passwd_data->pw_dir;
    } else {
      home = getcwd(NULL, 0);
      dont_dup = true;
    }
  }
  return dont_dup ? home : strdup(home);
}

char* get_cmdline(int argc, char** argv) {
  int buf_len = 256;
  int max_len = buf_len - 3;
  char* buf = calloc(buf_len, sizeof(char));
  int i;
  int len_so_far = 0;
  for (i = 0; i < argc; i++) {
    char* arg = argv[i];
    int arg_len = strlen(arg);
    len_so_far = strlen(buf);
    if (len_so_far + arg_len <= max_len) {
      strncat(buf, arg, arg_len);
      if (i + 1 < argc) {
        strncat(buf, " ", 1);
      }
    } else {
      strncat(buf, arg, max_len - len_so_far);
      strncat(buf, "...", 3);
      break;
    }
  }
  return buf;
}

char* get_executable_file() {
  return symlink_target("/proc/self/exe");
}

int write_script(char* filename, char* script) {
  // TODO better error handling
  print_verbose("Installing in %s.\n", filename);
  FILE* file = fopen(filename, "a");
  int wrote = -1;
  if (file) {
    wrote = fprintf(file, "%s", script);
    fclose(file);
  }
  if (wrote == strlen(script)) {
    return true;
  } else {
    fprintf(stderr, "Failed to write to %s\n", filename);
    return false;
  }
}

char* bash_script() {

  char* executable_filename = get_executable_file();
  if (executable_filename) {
    char* version = PATH_VERSION;

    time_t ltime;
    time(&ltime);
    struct tm* timestamp = localtime(&ltime);
    char* timestamp_str = asctime(timestamp);

    char* bin_directory = strdup(executable_filename);
    bin_directory = dirname(bin_directory);

    //timestamp_str will include a newline char
    char* script = "\n# Added by PATH-%s at %s[[ -s \"%s\" ]] && `%s --add %s --export`\n";
    int script_length = strlen(script) + strlen(version) + strlen(timestamp_str) + (strlen(executable_filename) * 2) + strlen(bin_directory);
    char* script_buf = malloc((script_length + 1) * sizeof(char));
    sprintf(script_buf, script, version, timestamp_str, executable_filename, executable_filename, bin_directory);

    free(executable_filename);
    free(bin_directory);

    return script_buf;
  }
  return NULL;
}

bool install_in_shell(bool global) {
  // keep track of the number of files
  // that we've written to (should end up being 2)
  int wrote_files = 0;

  char* script = bash_script();
  if (script) {
    print_verbose("Writing \"%s\"\n", script);

    char *profile_filename = NULL;

    if (global) {
      //global mode - install in /etc/profile and /etc/bash.bashrc
      if (directory_exists("/etc/profile.d")) {
        profile_filename = "/etc/profile.d/path.sh";
      } else if (file_exists("/etc/profile")) {
        profile_filename = "/etc/profile";
      }
      if (profile_filename) {
        if (write_script(profile_filename, script)) wrote_files++;
      }
      if (file_exists("/etc/bash.bashrc")) {
        if (write_script("/etc/bash.bashrc", script)) wrote_files++;
      }
    } else {
      //user mode - install in profile, rc file
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
      if (write_script(profile_filename, script)) wrote_files++;
      free(profile_filename);

      char* rc_filename = file_join(home_dir, ".bashrc");
      if (write_script(rc_filename, script)) wrote_files++;
      free(rc_filename);

      free(home_dir);
    }

    free(script);
  }
  return wrote_files == 2;
}

void free_nodes(node_t* node) {
  node_t* entry;
  node_t* tmp;

  for (entry = node; entry;) {
    tmp = entry;
    entry = entry->next;
    free(tmp->val);
    free(tmp);
  }
}
