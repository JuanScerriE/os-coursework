#include "builtin.h"
#include "linenoise.h"
#include "parse_execute.h"
#include "tokeniser.h"

int main(void) {
  char *line;
  token_vec_t toks;
  size_t num_of_statements;

  // This is run once to set the cwd variable.
  if (getcwd(cwd, PATH_MAX) == NULL) {
    perror("getcwd");
    return EXIT_FAILURE;
  }

  // NOTE: There are some improvements which can be made
  // specifically, when Ctrl-C is pressed it kills whole
  // shell and that can be handled by adding an appropriate
  // signal handler.
  //
  // Moreover, on exiting some memory is leaked. Although
  // this is cleaned up by the OS. I would be better if the
  // program it self cleaned it up. This could be done
  // through exit handlers but it requires some changes in
  // the current structure of the code making it non-trivial
  // to implement.
  //
  // Or we could just return a special value like -2 which
  // indicates that we have to terminate the program.
  // but that would be defeating the point of exit().
  //
  // NOTE: NOTE: I decided to do this instead. I am
  // returning -2 as the code which results in the shell
  // exiting.

  // NOTE: This is question c) and question d) from task 3.
  while ((line = linenoise("tish $ ")) != NULL) {
    if (*line != '\0') {
      if (tokenise(&toks, line) != -1) {
        if (parse(toks, &num_of_statements) != -1) {
          /*if (execute(toks) == EXIT_SHELL)
            goto exit_main;*/
        }

        fprintf(stderr, "NUM_OF_STATMETNS: %lu\n", num_of_statements);
      }

      token_vec_free(&toks);
    }

    line = NULL;
  }

  return EXIT_SUCCESS;

//exit_main:
  token_vec_free(&toks);
  free(line);
  return EXIT_SUCCESS;
}
