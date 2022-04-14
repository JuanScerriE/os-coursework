#include "builtin.h"

char cwd[PATH_MAX];

builtin_command_t builtins[] = {{"exit", &builtin_exit},
                                {"cd", &builtin_cd},
                                {"cwd", &builtin_cwd},
                                {"ver", &builtin_ver}};

int builtin_exit(char **args) {
  UNUSED(args);

  // This leaks memory if it is not relinquished by the OS.
  exit(0);
}

int builtin_cd(char **args) {
  int status;

  if (args[1] == NULL) {
    status = chdir(getenv("HOME"));
  } else {
    status = chdir(args[1]);
  }

  if (getcwd(cwd, PATH_MAX) == NULL) {
    return -1;
  }

  return status;
}

int builtin_cwd(char **args) {
  UNUSED(args);

  printf("%s\n", cwd);

  return 0;
}

int builtin_ver(char **args) {
  UNUSED(args);

  printf("Author: " AUTHOR
         "\n"
         "Version: " VERSION
         "\n"
         "Message: " MESSAGE "\n");

  return 0;
}

size_t get_num_of_builtins(void) {
  return sizeof(builtins) / sizeof(builtin_command_t);
}
