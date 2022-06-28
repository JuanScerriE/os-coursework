#include <errno.h>
#include <signal.h>

#include "builtin.h"
#include "interpreter.h"
#include "linenoise.h"

void consume_sigint(int signum) {
  UNUSED(signum);

  write(STDOUT_FILENO, "\n", 1);
}

int main(void) {
  // Disable SIGINT
  signal(SIGINT, consume_sigint);

  char *line;

  // This is run once to set the cwd variable.
  if (getcwd(cwd, PATH_MAX) == NULL) {
    perror("getcwd");
    return EXIT_FAILURE;
  }

  // NOTE: This answers question c) and question d) from
  // task 3.
  for (;;) {
    line = linenoise("tish $ ");

    // Ignore raw Ctrl-C bytes
    if (line == NULL) {
      if (errno == EAGAIN) {
        continue;
      } else {
        break;
      }
    }

    if (*line != '\0') {
      int ret = interpret(line);

      if (ret == -1) {
        // Do nothing.
      } else if (ret == EXIT_SHELL) {
        goto exit_shell;
      }
    }

    free(line);
  }

  return EXIT_SUCCESS;

exit_shell:
  free(line);

  return EXIT_SUCCESS;
}
