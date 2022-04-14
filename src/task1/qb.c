#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* Imported to make testing easier. */
#include "../linenoise.h"
#include "../tokenizer.h"
#include "../util.h"

#define RD 0
#define WR 1

/*
 * @fork_exec_pipe This function forks the running process.
 * Then it closes and binds file descriptors as is
 * necessary. Then it will replace the current process with
 * another process.
 * @param char **args A list consisting of the process name
 * and it's arguments.
 * @param int closefd The file descriptor which will be
 * closed upon fork(). This is generally used for closing
 * one of the two connections present to a pipe.
 * @param int newfd The file descriptor which is going to be
 * modified to point to the pipe which oldfd is pointing to.
 * @param int oldfd The file descriptor which is connected
 * to the pipe which we want newfd to point to. After
 * binding with dup2 oldfd will be closed.
 * @return pid_t The function returns the pid of the forked
 * process if successful. Otherwise it returns -1.
 * (Similarly to the functions is calls.)
 */

static inline pid_t fork_exec_pipe(char **args, int closefd,
                                   int oldfd, int newfd) {
  pid_t pid;

  if ((pid = fork()) == -1) {
    return -1;
  } else if (pid == 0) {  // Left Child
    close(closefd);

    if (dup2(oldfd, newfd) == -1)
      return -1;

    close(oldfd);

    if (execvp(args[0], args) == -1)
      return -1;
  }

  return pid;
}

/*
 * @fork_exec This function is a modified version of a
 * previous fork_exec which makes use of fork_exec_pipe to
 * actually fork and execute. However, it is the function
 * which creates the pipe allowing two processes to
 * communicate.
 * @param char **args1 A list consisting of the name of
 * process 1 and it's arguments.
 * @param char **args2 A list consisting of the name of
 * process 2 and it's arguments.
 * @return pid_t The function returns the pid of the forked
 * process if successful. Otherwise it returns -1.
 * (Similarly to the functions is calls.)
 */

pid_t fork_exec(char **args1, char **args2) {
  pid_t pid;
  int status;
  int fd[2];

  if (pipe(fd) == -1) {
    return -1;
  }

  pid =
      fork_exec_pipe(args1, fd[RD], fd[WR], STDOUT_FILENO);
  if (pid == -1)
    return -1;

  pid = fork_exec_pipe(args2, fd[WR], fd[RD], STDIN_FILENO);
  if (pid == -1)
    return -1;

  close(fd[RD]);
  close(fd[WR]);

  if (waitpid(pid, &status, 0) == -1)
    return -1;

  /* Return the pid of the second process. */
  return pid;
}

/* The main method is used for testing. */

int main(void) {
  char *line;
  char **args1;
  char **args2;

  if ((line = linenoise("1 > ")) != NULL) {
    if (*line != '\0')
      args1 = tokenise(line, " \n\r\t");

    free(line);
  }

  if ((line = linenoise("2 > ")) != NULL) {
    if (*line != '\0')
      args2 = tokenise(line, " \n\r\t");

    free(line);
  }

  if (fork_exec(args1, args2) == -1) {
    perror("fork_exec_pipe");
    return EXIT_FAILURE;
  }

  free(args1);
  free(args2);

  return EXIT_SUCCESS;
}
