#pragma once

#include "string_t.h"

typedef struct {
  string_t *arr; /* pointer to vector of string pointers */
  size_t len;    /* length of the current vector */
  size_t alc;    /* allocated size of vector */
} string_vec_t;

size_t string_vec_grow(string_vec_t *, size_t);
size_t string_vec_init(string_vec_t *, size_t);
string_vec_t *string_vec_new(void);
void string_vec_drop(string_vec_t *);
void string_vec_free(string_vec_t *);
size_t string_vec_append(string_vec_t *, string_t);
char **string_vec_to_list(string_vec_t *);
