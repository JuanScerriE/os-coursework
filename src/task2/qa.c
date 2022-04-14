#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define AUTHOR "Juan Scerri"
#define VERSION "0.1"
#define MESSAGE                                    \
  "Async was successful. You can glitch into the " \
  "Backrooms."

#define UNUSED(...) (void)(__VA_ARGS__)

int sh_cd(char **args) {
  return chdir(*args);
}

int sh_pwd(char **args) {
  UNUSED(args);
  printf("%s\n", getenv("PWD"));

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

int sh_exit(char **args) {
  UNUSED(args);
  exit(0);  // What about all the allocations?
}

typedef int (*builtin_t)(char **);

typedef struct {
  char *name;
  builtin_t func;
} builtin_command_t;

builtin_command_t builtins[] = {{"exit", &sh_exit},
                                {"cd", &sh_cd},
                                {"pwd", &sh_pwd},
                                {"ver", &sh_ver}};

static inline size_t get_num_of_builtins(void) {
  return sizeof(builtins) / sizeof(builtin_command_t);
}

int exec(void) {
  return 0;
}

int launch(char **args) {
  for (size_t i = 0; i < get_num_of_builtins(); i++) {
    if (!strcmp(*args, builtins[i].name)) {
      return builtins[i].func(args + 1);
    }
  }

  return exec();
}
