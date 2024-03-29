#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* Imported to make testing easier. */
#include "../util/linenoise.h"
#include "../util/tokeniser.h"
#include "../util/util.h"

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

// The main method is used for testing.
int main(void) {
  char *line;
  char **args;

  if ((line = linenoise("> ")) != NULL) {
    if (*line != '\0') {
      args = tokenise(line, " \n\r\t");

      if (fork_exec(args) == -1) {
        perror("fork_exec");
        return EXIT_FAILURE;
      }

      string_arr_free(args);
    }

    free(line);
  }

  return EXIT_SUCCESS;
}
