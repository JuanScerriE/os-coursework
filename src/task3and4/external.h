#ifndef __EXTERNAL_H
#define __EXTERNAL_H

#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include "builtin.h"

pid_t exec_pipeline(char **pipeline[], int options,
                    char *infile, char *outfile,
                    bool append);

#endif  // __EXTERNAL_H
