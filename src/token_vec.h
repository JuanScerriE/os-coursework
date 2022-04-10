#pragma once

#include <stdlib.h>

struct token {
  char *str;  /* pointer to string */
  size_t len; /* length of the current string */
  size_t alc; /* allocated size of the string */
};

void token_grow(struct token *, size_t);
void token_init(struct token *, size_t);
struct token *token_new(void);
void token_shallow_free(struct token *);
void token_deep_free(struct token *);
void token_append(struct token *, char);

struct token_vec {
  struct token *
      *arr;   /* pointer to array of token pointers */
  size_t len; /* length of the current array */
  size_t alc; /* allocated size of array */
};

void token_vec_grow(struct token_vec *, size_t);
void token_vec_init(struct token_vec *, size_t);
struct token_vec *token_vec_new(void);
void token_vec_shallow_free(struct token_vec *);
void token_vec_deep_free(struct token_vec *);
void token_vec_append(struct token_vec *, struct token *);
char **token_vec_get_list(struct token_vec *);
