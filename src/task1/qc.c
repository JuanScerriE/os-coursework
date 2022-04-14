#include <unistd.h>

/* Imported to make testing easier. */
#include "../linenoise.h"
#include "../tokenizer.h"
#include "../util.h"

#define RD 0
#define WR 1

/*
 * @execute_pipeline This function executes a pipeline. This
 * is done by creating pipes and arranges them accordingly
 * to ensure that processes can communicate.
 * @param char **pipeline[] An array of commands whose
 * outputs become the inputs of the next process (if there
 * is a next process). The array is NULL terminated.
 * @return pid_t The function returns the pid of the last
 * forked process if successful. Otherwise it returns -1.
 */

static inline pid_t execute_pipeline(char **pipeline[]) {
  int current_fd[2];
  int previous_fd[2];
  pid_t pid;

  for (size_t i = 0; pipeline[i] != NULL; i++) {
    // Create a new pipe only if the current process is not
    // the last.
    if (pipeline[i + 1] != NULL) {
      if (pipe(current_fd) == -1)
        return -1;
    }

    if ((pid = fork()) == -1)
      return -1;

    if (pid == 0) {  // Child
      if (i > 0) {
        // Disconnect write-end of previous pipe.
        close(previous_fd[WR]);
        // Connect read-end of previous pipe to stdin.
        if (dup2(previous_fd[RD], STDIN_FILENO) == -1)
          return -1;

        close(previous_fd[RD]);
      }

      // Connect the write-end to a pipe only if the current
      // process is not the last.
      if (pipeline[i + 1] != NULL) {
        // Disconnect read-end of current pipe.
        close(current_fd[RD]);
        // Connect write-end of current pipe to stdout.
        if (dup2(current_fd[WR], STDOUT_FILENO) == -1)
          return -1;

        close(current_fd[WR]);
      }

      if (execvp(*pipeline[i], pipeline[i]) == -1)
        return -1;
    }

    // Only disconnect parent from previous pipe if there
    // are more than 1 pipe i.e. more than 1 process.
    if (i > 0) {
      close(previous_fd[RD]);
      close(previous_fd[WR]);
    }

    previous_fd[RD] = current_fd[RD];
    previous_fd[WR] = current_fd[WR];
  }

  return pid;
}

/* The main method is used for testing. */

int main(void) {
  char *line = NULL;
  size_t pipeline_size = 16;
  size_t pipeline_len = 0;
  char ***pipeline =
      emalloc(pipeline_size * sizeof(char **));

  while ((line = linenoise("(Enter to Stop) > ")) != NULL) {
    if (*line != '\0') {
      if (pipeline_len >= pipeline_size) {
        pipeline_size <<= 1;
        pipeline = erealloc(
            pipeline, pipeline_size * sizeof(char **));
      }

      pipeline[pipeline_len++] = tokenise(line, " \n\r\t");
    } else {
      free(line);
      break;
    }

    free(line);
  }

  if (pipeline_len >= pipeline_size) {
    pipeline_size <<= 1;
    pipeline =
        erealloc(pipeline, pipeline_size * sizeof(char **));
  }

  pipeline[pipeline_len] = NULL;

  if (execute_pipeline(pipeline) == -1) {
    perror("execute_pipeline");
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < pipeline_len; i++)
    string_arr_free(pipeline[i]);

  if (pipeline != NULL)
    free(pipeline);

  return EXIT_SUCCESS;
}
