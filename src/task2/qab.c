#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../util/linenoise.h"
#include "../util/tokeniser.h"
#include "../util/util.h"

#ifdef __linux__
#include <linux/limits.h>
#else
#define PATH_MAX 4096
#define NAME_MAX 255
#endif

char cwd[PATH_MAX];

#define AUTHOR "Juan Scerri"
#define VERSION "0.1"
#define MESSAGE                                    \
  "Async was successful. You can glitch into the " \
  "Backrooms."

#define UNUSED(...) (void)(__VA_ARGS__)

int sh_cd(char **args) {
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

int sh_exit(char **args) {
  UNUSED(args);

  // This leaks memory if it is not relinquished by the OS.
  exit(0);
}

typedef int (*builtin_t)(char **);

typedef struct {
  char *name;
  builtin_t func;
} builtin_command_t;

builtin_command_t builtins[] = {{"exit", &sh_exit},
                                {"cd", &sh_cd},
                                {"cwd", &sh_cwd},
                                {"ver", &sh_ver}};

static inline size_t get_num_of_builtins(void) {
  return sizeof(builtins) / sizeof(builtin_command_t);
}

int sh_launch(char **args) {
  pid_t pid;
  int status;

  for (size_t i = 0; i < get_num_of_builtins(); i++) {
    if (!strcmp(args[0], builtins[i].name)) {
      return builtins[i].func(args);
    }
  }

  if ((pid = fork()) == -1) {
    perror("fork");
  } else if (pid > 0) {  // Parent
    if (waitpid(pid, &status, 0) == -1) {
      perror("waitpid");
    }
  } else {  // Child
    if (execvp(args[0], args) == -1) {
      die("execvp:");
    }
  }

  return pid;
}

/* Again main is being used for testing. */

int main(void) {
  char *line;
  char **args;
  if (getcwd(cwd, PATH_MAX) == NULL) {
    perror("getcwd");
    return EXIT_FAILURE;
  }

  while ((line = linenoise("> ")) != NULL) {
    if (*line != '\0') {
      args = tokenise(line, " \n\r\t");

      if (sh_launch(args) == -1)
        perror("sh_launch");
      string_arr_free(args);
    }

    free(line);
  }

  return 0;
}
