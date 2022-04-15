#pragma once

#include <stdlib.h>

struct string {
  char *str;  /* pointer to string */
  size_t len; /* length of the current string */
  size_t alc; /* allocated size of the string */
};

void string_grow(struct string *, size_t);
void string_init(struct string *, size_t);
struct string *string_new(void);
void string_shallow_free(struct string *);
void string_deep_free(struct string *);
void string_append(struct string *, char);

struct string_vec {
  struct string *
      *arr;   /* pointer to array of string pointers */
  size_t len; /* length of the current array */
  size_t alc; /* allocated size of array */
};

void string_vec_grow(struct string_vec *, size_t);
void string_vec_init(struct string_vec *, size_t);
struct string_vec *string_vec_new(void);
void string_vec_shallow_free(struct string_vec *);
void string_vec_deep_free(struct string_vec *);
void string_vec_append(struct string_vec *,
                       struct string *);
char **string_vec_get_list(struct string_vec *);
