#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

/* Imported to make testing easier. */
#include "../util/linenoise.h"
#include "../util/tokeniser.h"
#include "../util/util.h"

#define RD 0
#define WR 1

static inline int reopen(int fd, char *path, int flags,
                         mode_t mode) {
  /*
   * Open a file in the process creating a file descriptor.
   * This requires updating both the file descriptor table
   * of the process and the open file table.
   */
  int open_fd = open(path, flags, mode);
  if (open_fd == fd || open_fd < 0)
    return open_fd;

  /*
   * The fd will be updated to point to the same element in
   * the open file table as open_fd. Obviously, the element
   * in the open file table will be updated as well to
   * realise that is has two active pointers.
   */
  int dup_fd = dup2(open_fd, fd);

  /*
   * We close open_fd because we do not need the file
   * descriptor any more.
   */
  close(open_fd);

  return dup_fd == -1 ? dup_fd : fd;
}

static inline int redirect_input(char *input) {
  return reopen(STDIN_FILENO, input, O_RDONLY, S_IRUSR);
}

static inline int redirect_output(char *output,
                                  mode_t trunc_or_append) {
  return reopen(STDOUT_FILENO, output,
                O_RDWR | O_CREAT | trunc_or_append,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                    S_IROTH | S_IWOTH);
}

/*
 * @exec_pipeline_ex This function executes a pipeline. This
 * is done by creating pipes and arranges them accordingly
 * to ensure that processes can communicate. Moreover, it
 * can also change the input and ouput of a pipeline to
 * files.
 * @param char **pipeline[] An array of commands whose
 * outputs become the inputs of the next process (if there
 * is a next process). The array is NULL terminated.
 * @param int options This parameter is effectively the
 * options that the waitpid function will be given.
 * @param char *infile This is the file to which input is
 * bound if the pointer is not NULL.
 * @param char *outfile This is the file to which output is
 * bound if the pointer is not NULL.
 * @param bool append This variable
 * @return pid_t The function returns the pid of the last
 * forked process if successful. Otherwise it returns -1.
 */

static inline pid_t exec_pipeline_ex(char **pipeline[],
                                     int options,
                                     char *infile,
                                     char *outfile,
                                     bool append) {
  int current_fd[2];
  int previous_fd[2];
  int status;
  pid_t pid = -1;

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
      // If this is the first process.
      if (i == 0) {
        if (infile != NULL && redirect_input(infile) == -1)
          return -1;
      }

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

      // If this is the last process.
      if (pipeline[i + 1] == NULL) {
        if (outfile != NULL &&
            redirect_output(
                outfile, append ? O_APPEND : O_CREAT) == -1)
          return -1;
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

    // NOTE: This is the code we'd want to have if we do not
    // want to have zombies during a pipe.
    if (waitpid(pid, &status, options) == -1)
      return -1;

    // Possibility of handling status.
  }

  return pid;
}

// The main method is used for testing.

static inline void test1(void) {
  char *line = NULL;
  size_t pipeline_size = 16;
  size_t pipeline_len = 0;
  char ***pipeline =
      emalloc(pipeline_size * sizeof(char **));

  while ((line = linenoise("(Enter to Stop) 1 > ")) !=
         NULL) {
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

  // Use WNOHANG if you want it to not wait.
  // NOTE: We are reading from the terminals stdin and
  // ouputting to test.txt.
  if (exec_pipeline_ex(pipeline, 0, NULL, "test.txt",
                       true) == -1) {
    perror("exec_pipeline_ex");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < pipeline_len; i++)
    string_arr_free(pipeline[i]);

  if (pipeline != NULL)
    free(pipeline);
}

static inline void test2(void) {
  char *line = NULL;
  size_t pipeline_size = 16;
  size_t pipeline_len = 0;
  char ***pipeline =
      emalloc(pipeline_size * sizeof(char **));

  while ((line = linenoise("(Enter to Stop) 2 > ")) !=
         NULL) {
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

  // NOTE: We are reading from intest.txt and ouputting to
  // outtest.txt.
  // Use WNOHANG if you want it to not wait.
  if (exec_pipeline_ex(pipeline, 0, "intest.txt",
                       "outtest.txt", true) == -1) {
    perror("exec_pipeline_ex");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < pipeline_len; i++)
    string_arr_free(pipeline[i]);

  if (pipeline != NULL)
    free(pipeline);
}

int main(void) {
  test1();
  test2();
  return EXIT_SUCCESS;
}
