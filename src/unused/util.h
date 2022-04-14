#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void die(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] == ':') {
    fputc(' ', stderr);
    perror(NULL);
  } else {
    fputc('\n', stderr);
  }

  exit(1);
}

static inline void *emalloc(size_t size) {
  void *ptr;

  if ((ptr = malloc(size)) == NULL)
    die("malloc:");

  return ptr;
}

static inline void *erealloc(void *ptr, size_t size) {
  if ((ptr = realloc(ptr, size)) == NULL) {
    die("realloc:");
  }

  return ptr;
}

static inline void string_arr_free(char **arr) {
  if (arr != NULL) {
    for (size_t i = 0; arr[i] != NULL; i++)
      free(arr[i]);

    free(arr);
  }
}
