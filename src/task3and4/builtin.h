#ifndef __BUILTIN_H
#define __BUILTIN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/limits.h>
#else
#define PATH_MAX 4096
#endif

#define EXIT_SHELL -2

extern char cwd[PATH_MAX];

#define AUTHOR "Juan Scerri"
#define VERSION "0.1"
#define MESSAGE                                    \
  "Async was successful. You can glitch into the " \
  "Backrooms."

#define UNUSED(...) (void)(__VA_ARGS__)

int builtin_cd(char **args);
int builtin_cwd(char **args);
int builtin_ver(char **args);
int builtin_exit(char **args);

typedef int (*builtin_t)(char **);

typedef struct {
  char *name;
  builtin_t func;
} builtin_command_t;

extern builtin_command_t builtins[];

size_t get_num_of_builtins(void);

#endif  // __BUILTIN_H
