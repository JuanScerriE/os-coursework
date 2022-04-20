#include <stdlib.h>

#include "token_t.h"

typedef struct {
  token_t *arr; /* pointer to vector of token pointers */
  size_t len;   /* length of the current vector */
  size_t alc;   /* allocated size of vector */
} token_vec_t;

int token_vec_grow(token_vec_t *, size_t);
int token_vec_init(token_vec_t *, size_t);
token_vec_t *token_vec_new(void);
void token_vec_drop(token_vec_t *);
void token_vec_free(token_vec_t *);
int token_vec_append(token_vec_t *, token_t);
char **token_vec_to_list(token_vec_t *);
void print_token_vec(token_vec_t *vec);
