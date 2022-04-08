#include "token_vec.h"
#include "util.h"

#define DEFAULT_SIZE 16

void token_grow(struct token *tok, size_t need) {
  tok->alc = tok->alc > 0 ? tok->alc : 1;

  while (tok->alc < need)
    tok->alc <<= 1;

  tok->str = erealloc(tok->str, sizeof(char) * tok->alc);
}

void token_init(struct token *tok, size_t est) {
  tok->str = NULL;
  tok->len = 0;
  tok->alc = 0;

  if (est > 0)
    token_grow(tok, est);
}

struct token *token_new(void) {
  struct token *tok = emalloc(sizeof(struct token));
  token_init(tok, DEFAULT_SIZE);
  return tok;
}

void token_shallow_free(struct token *tok) {
  if (tok != NULL) {
    free(tok);
  }
}

void token_deep_free(struct token *tok) {
  if (tok != NULL) {
    if (tok->str != NULL) {
      free(tok->str);
    }

    free(tok);
  }
}

void token_append(struct token *tok, char ch) {
  if (tok->len >= tok->alc)
    token_grow(tok, tok->alc + 1);

  tok->str[tok->len++] = ch;
}

/*-----------------------------------------------------------------*/

void token_vec_grow(struct token_vec *vec, size_t need) {
  vec->alc = vec->alc > 0 ? vec->alc : 1;

  while (vec->alc < need)
    vec->alc <<= 1;

  vec->arr = erealloc(vec->arr, sizeof(struct token *) * vec->alc);
}

void token_vec_init(struct token_vec *vec, size_t est) {
  vec->arr = NULL;
  vec->len = 0;
  vec->alc = 0;

  if (est > 0)
    token_vec_grow(vec, est);
}

struct token_vec *token_vec_new(void) {
  struct token_vec *vec = emalloc(sizeof(struct token_vec));
  token_vec_init(vec, DEFAULT_SIZE);
  return vec;
}

void token_vec_shallow_free(struct token_vec *vec) {
  if (vec != NULL) {
    free(vec);
  }
}

void token_vec_deep_free(struct token_vec *vec) {
  if (vec != NULL) {
    if (vec->arr != NULL) {
      for (size_t i = 0; i < vec->len; i++) {
        token_deep_free(vec->arr[i]);
      }

      free(vec->arr);
    }

    free(vec);
  }
}

void token_vec_append(struct token_vec *vec, struct token *tok) {
  if (vec->len >= vec->alc)
    token_vec_grow(vec, vec->alc + 1);

  vec->arr[vec->len++] = tok;
}

/*
 * Keep in mind that the list provided this function should be
 * freed independently of the the token vector. You should free
 * the individual strings and then you should free the array of
 * pointers.
 */

char **token_vec_get_list(struct token_vec *vec) {
  char **list = emalloc(sizeof(char *) * (vec->len + 1));

  for (size_t i = 0; i < vec->len; i++) {
    list[i] = emalloc(sizeof(char) * (strlen(vec->arr[i]->str) + 1));

    strncpy(list[i], vec->arr[i]->str, strlen(vec->arr[i]->str) + 1);
  }

  list[vec->len] = NULL;

  return list;
}

#undef DEFAULT_SIZE
