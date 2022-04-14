#pragma once

#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

pid_t fork_exec_pipe(char **pipeline[], int options,
                     char *infile, char *outfile,
                     bool append);
