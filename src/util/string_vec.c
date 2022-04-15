#include "string_vec.h"

#include "util.h"

#define DEFAULT_SIZE 16

void string_grow(struct string *tok, size_t need) {
  tok->alc = tok->alc > 0 ? tok->alc : 1;

  while (tok->alc < need)
    tok->alc <<= 1;

  tok->str = erealloc(tok->str, sizeof(char) * tok->alc);
}

void string_init(struct string *tok, size_t est) {
  tok->str = NULL;
  tok->len = 0;
  tok->alc = 0;

  if (est > 0)
    string_grow(tok, est);
}

struct string *string_new(void) {
  struct string *tok = emalloc(sizeof(struct string));
  string_init(tok, DEFAULT_SIZE);
  return tok;
}

void string_shallow_free(struct string *tok) {
  if (tok != NULL) {
    free(tok);
  }
}

void string_deep_free(struct string *tok) {
  if (tok != NULL) {
    if (tok->str != NULL) {
      free(tok->str);
    }

    free(tok);
  }
}

void string_append(struct string *tok, char ch) {
  if (tok->len >= tok->alc)
    string_grow(tok, tok->alc + 1);

  tok->str[tok->len++] = ch;
}

/*-----------------------------------------------------------------*/

void string_vec_grow(struct string_vec *vec, size_t need) {
  vec->alc = vec->alc > 0 ? vec->alc : 1;

  while (vec->alc < need)
    vec->alc <<= 1;

  vec->arr = erealloc(vec->arr,
                      sizeof(struct string *) * vec->alc);
}

void string_vec_init(struct string_vec *vec, size_t est) {
  vec->arr = NULL;
  vec->len = 0;
  vec->alc = 0;

  if (est > 0)
    string_vec_grow(vec, est);
}

struct string_vec *string_vec_new(void) {
  struct string_vec *vec =
      emalloc(sizeof(struct string_vec));
  string_vec_init(vec, DEFAULT_SIZE);
  return vec;
}

void string_vec_shallow_free(struct string_vec *vec) {
  if (vec != NULL) {
    free(vec);
  }
}

void string_vec_deep_free(struct string_vec *vec) {
  if (vec != NULL) {
    if (vec->arr != NULL) {
      for (size_t i = 0; i < vec->len; i++) {
        string_deep_free(vec->arr[i]);
      }

      free(vec->arr);
    }

    free(vec);
  }
}

void string_vec_append(struct string_vec *vec,
                       struct string *tok) {
  if (vec->len >= vec->alc)
    string_vec_grow(vec, vec->alc + 1);

  vec->arr[vec->len++] = tok;
}

/*
 * Keep in mind that the list provided this function should
 * be freed independently of the the string vector. You
 * should free the individual strings and then you should
 * free the array of pointers.
 */

// FIXME: Add a way to get the size because the will be useful
// since it is already computed I don't need to recompute it
// that would be very wasteful.

char **string_vec_get_list(struct string_vec *vec) {
  char **list = emalloc(sizeof(char *) * (vec->len + 1));

  for (size_t i = 0; i < vec->len; i++) {
    list[i] = emalloc(sizeof(char) *
                      (strlen(vec->arr[i]->str) + 1));

    strncpy(list[i], vec->arr[i]->str,
            strlen(vec->arr[i]->str) + 1);
  }

  list[vec->len] = NULL;

  return list;
}

#undef DEFAULT_SIZE
