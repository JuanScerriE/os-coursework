#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>

#include "linenoise.h"
#include "token_vec.h"
#include "tokenizer.h"
#include "util.h"

#define UNUSED(...) (void)(__VA_ARGS__)

bool sh_exit(char **args) {
  UNUSED(args);

  return false;
}

char *builtins_str[] = {"exit", NULL};

bool (*builtins_func[])(char **) = {&sh_exit};

char *sh_launch(char **args, bool *continue_loop) {
  pid_t pid;
  int status;

  for (size_t i = 0; builtins_str[i] != NULL; i++) {
    if (!strcmp(args[0], builtins_str[i])) {
      *continue_loop = builtins_func[i](args);
      return args[0];
    }
  }

  if ((pid = fork()) == -1) {
    perror("fork");
  } else if (pid > 0) { // Parent
    if (waitpid(pid, &status, 0) == -1) {
      perror("waitpid");
    }

    *continue_loop = true;
  } else { // Child
    if (execvp(args[0], args) == -1) {
      die("execvp:");
    }
  }

  return args[0];
}

/* char *prompt_init(const char *prompt) { */
/*   size_t size = strlen(prompt); */
/*   char *dy_prompt = emalloc(sizeof(char) * size); */
/*   strncpy(dy_prompt, prompt, size); */
/*   return dy_prompt; */
/* } */

/* char *prompt_update(char *prompt, char *command) { */
/*   size_t prompt_size = strlen(prompt); */
/*   size_t command_size = strlen(command); */
/*   char *dy_prompt = emalloc(sizeof(char) * (prompt_size + command_size)); */
/*   strncpy(dy_prompt, command, command_size); */
/*   strncpy(dy_prompt + command_size, prompt, prompt_size); */
/*   return dy_prompt; */
/* } */

int main(void) {
  bool continue_loop = true;
  char *line;
  /* char *command; */
  char **args;
  struct token_vec *vec;

  /* char *prompt = prompt_init("> "); */

  while (continue_loop == true && (line = linenoise("> ")) != NULL) {
    if (*line != '\0') {
      vec = tokenise(line);
      args = token_vec_get_list(vec);

      /*command = */ sh_launch(args, &continue_loop);
      /* prompt = prompt_update(" > ", command); */

      /* for (int i = 0; args[i] != NULL; i++) { */
      /*   fprintf(stderr, "%d %s\n", i, args[i]); */
      /* } */

      string_arr_free(args);
      token_vec_deep_free(vec);
    }

    free(line);
  }

  /* free(prompt); */

  return 0;
}
