#pragma once

#include <stdlib.h>

#include "util.h"

typedef struct {
  char *arr;  /* pointer to character vector */
  size_t len; /* length of the current string */
  size_t alc; /* allocated size of the string */
} string_t;

int string_grow(string_t *, size_t);
int string_init(string_t *, size_t);
string_t *string_new(void);
void string_drop(string_t *);
void string_free(string_t *);
int string_append(string_t *, char);
char *string_to_char_pt(string_t *);
