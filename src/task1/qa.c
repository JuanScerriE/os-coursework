#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* Imported to make testing easier. */
#include "../linenoise.h"
#include "../token_vec.h"
#include "../tokenizer.h"
#include "../util.h"

/*
 * @fork_exec The function forks the running process and
 * replaces it with another program.
 * @param char **args A list consisting of the process name
 * and it's arguments.
 * @return pid_t The function returns the pid of the forked
 * process if successful. Otherwise it returns -1.
 * (Similarly to the functions is calls.)
 */

pid_t fork_exec(char **args) {
  pid_t pid;
  int status;

  if ((pid = fork()) == -1) {
    return -1;
  } else if (pid > 0) {  // Parent
    if (waitpid(pid, &status, 0) == -1) {
      return -1;
    }
  } else {  // Child
    if (execvp(args[0], args) == -1) {
      return -1;
    }
  }

  return pid;
}

/* The main method is used for testing. */

int main(int argc, char **argv) {
  char *line;
  char **args;
  struct token_vec *vec;

  if ((line = linenoise("> ")) != NULL) {
    if (*line != '\0') {
      vec = tokenise(line);
      args = token_vec_get_list(vec);

      if (fork_exec(args) == -1) {
        perror("fork_exec");
        return EXIT_FAILURE;
      }

      string_arr_free(args);
      token_vec_deep_free(vec);
    }

    free(line);
  }

  return EXIT_SUCCESS;
}
