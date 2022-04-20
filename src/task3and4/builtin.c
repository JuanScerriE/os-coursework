#include "builtin.h"

char cwd[PATH_MAX];

int builtin_exit(char **args) {
  UNUSED(args);

  // The OS will deallocate all memory on termination.
  // It honestly bugs me a bit that you are leaking memory
  // still. So I am not going to exit through exit(). I will
  // just return a special value which will indicate to the
  // shell that it needs to exit.
  // NOTE: Something similar should be done for malloc and
  // all other memory related operations becuase I do not
  // think we know how to recover. Currently we just use
  // continue running the program. Nothing bad will happen
  // it's just that the user won't be ablt to do anything
  // because he can't allocate anymore memory.
  /*exit(0)*/
  return EXIT_SHELL;
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

builtin_command_t builtins[] = {{"exit", &builtin_exit},
                                {"cd", &builtin_cd},
                                {"cwd", &builtin_cwd},
                                {"ver", &builtin_ver}};

size_t get_num_of_builtins(void) {
  return sizeof(builtins) / sizeof(builtin_command_t);
}

builtin_command_t get_builtin(size_t i) {
  return builtins[i];
}
