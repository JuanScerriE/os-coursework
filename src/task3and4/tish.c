#include "builtin.h"
#include "interpreter.h"
#include "linenoise.h"

// NOTE: There are some improvements which can be made
// specifically, when Ctrl-C is pressed it kills whole
// shell.

int main(void) {
  char *line;

  // This is run once to set the cwd variable.
  if (getcwd(cwd, PATH_MAX) == NULL) {
    perror("getcwd");
    return EXIT_FAILURE;
  }


  // NOTE: This answers question c) and question d) from
  // task 3.
  while ((line = linenoise("tish $ ")) != NULL) {
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
