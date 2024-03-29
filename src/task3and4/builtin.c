#include "builtin.h"

char cwd[PATH_MAX];

// NOTE: This file answers Task 2 b)

int sh_exit(char **args) {
  UNUSED(args);

  // NOTE: I am returning -2 as the code which results in
  // the shell exiting. I am doing this because I do not
  // trust that the OS will actually free all the resources
  // allocated to the program.

  /* exit(0) */

  return EXIT_SHELL;
}

int sh_cd(char **args) {
  int status = 0;

  // If no argument is specified, 'cd' will change the
  // directory to the home directory of the current user if
  // the HOME environment variable is set, otherwise nothing
  // will happen.
  if (args[1] == NULL) {
    char *home = getenv("HOME");
    if (home != NULL)
      status = chdir(home);
  } else {
    status = chdir(args[1]);
  }

  if (status == -1) {
    perror("chdir");
    return -1;
  }

  // Update the cwd global.
  if (getcwd(cwd, PATH_MAX) == NULL) {
    perror("getcwd");
    return -1;
  }

  return status;
}

int sh_cwd(char **args) {
  UNUSED(args);

  printf("%s\n", cwd);

  return 0;
}

int sh_ver(char **args) {
  UNUSED(args);

  printf("Author: " AUTHOR
         "\n"
         "Version: " VERSION
         "\n"
         "Message: " MESSAGE "\n");

  return 0;
}

builtin_command_t builtins[] = {{"exit", &sh_exit},
                                {"cd", &sh_cd},
                                {"cwd", &sh_cwd},
                                {"ver", &sh_ver}};

size_t get_num_of_builtins(void) {
  return sizeof(builtins) / sizeof(builtin_command_t);
}
