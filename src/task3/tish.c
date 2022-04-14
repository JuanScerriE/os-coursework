#include "linenoise.h"
#include "parse_execute.h"
#include "tokeniser.h"

int main(void) {
  char *line;
  char **args;
  token_vec_t toks;

  if (getcwd(cwd, PATH_MAX) == NULL) {
    perror("getcwd");
    return EXIT_FAILURE;
  }

  while ((line = linenoise("tish $ ")) != NULL) {
    if (*line != '\0') {
      if (tokenise(&toks, line) != -1) {
        if (parse(toks) != -1) {
          execute(toks);
        }
      }

      token_vec_free(&toks);
    }

    free(line);
  }

  return 0;

fail_tish:
  free(line);
  token_vec_free(&toks);
  return EXIT_FAILURE;
}
