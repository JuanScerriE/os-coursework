#include "token_vec_t.h"

#include "util.h"

#define DEFAULT_SIZE 16

// If return is -1 then there is an error. Otherwise return
// allocated space.
size_t token_vec_grow(token_vec_t *vec, size_t need) {
  if (vec == NULL) {
    fprintf(stderr, "token_vec_grow: NULL pointer\n");
    return -1;
  }

  vec->alc = vec->alc > 0 ? vec->alc : 1;

  while (vec->alc < need)
    vec->alc <<= 1;

  vec->arr = realloc(vec->arr, sizeof(token_t) * vec->alc);

  if (vec->arr == NULL) {
    perror("token_vec_grow");
    return -1;
  } else {
    return vec->alc;
  }
}

// If return is -1 then there is an error. Otherwise return
// allocated space.
size_t token_vec_init(token_vec_t *vec, size_t est) {
  if (vec == NULL) {
    fprintf(stderr, "token_vec_init: NULL pointer\n");
    return -1;
  }

  vec->arr = NULL;
  vec->len = 0;
  vec->alc = 0;

  if (est > 0)
    return token_vec_grow(vec, est);
  else
    return 0;
}

// If return is NULL then there is an error.
token_vec_t *token_vec_new(void) {
  token_vec_t *vec = malloc(sizeof(token_vec_t));
  if (vec == NULL) {
    perror("token_arr_new");
    return NULL;
  }

  if (token_vec_init(vec, DEFAULT_SIZE) == -1)
    return NULL;

  return vec;
}

// This obviously will return nothing.
// NOTE: Only call if the whole token_vec stucture was heap
// allocated.
void token_vec_drop(token_vec_t *vec) {
  if (vec != NULL) {
    if (vec->arr != NULL)
      free(vec->arr);

    free(vec);
  }
}

// This obviously will return nothing.
// NOTE: Call if the whole token_vec stucture was stack
// allocated.
void token_vec_free(token_vec_t *vec) {
  if (vec != NULL) {
    if (vec->arr != NULL) {
      for (size_t i = 0; i < vec->len; i++)
        token_free(vec->arr + i);

      free(vec->arr);
    }
  }
}

// If return is -1 then there is an error. Otherwise return
// allocated space.
size_t token_vec_append(token_vec_t *vec, token_t str) {
  if (vec == NULL) {
    fprintf(stderr, "token_vec_append: NULL pointer\n");
    return -1;
  }

  if (vec->len >= vec->alc)
    if (token_vec_grow(vec, vec->alc + 1) == -1)
      return -1;

  vec->arr[vec->len++] = str;

  return vec->alc;
}

// If return is NULL then there is an error.
char **token_vec_to_list(token_vec_t *vec) {
  if (vec == NULL) {
    fprintf(stderr, "token_vec_to_list: NULL pointer\n");
    return NULL;
  }

  // We add +1 to ensure the list can be NULL terminated.
  char **list = malloc(sizeof(char *) * (vec->len + 1));

  if (list == NULL) {
    perror("token_vec_to_list");
    return NULL;
  }

  for (size_t i = 0; i < vec->len; i++) {
    list[i] = string_to_char_pt(&(vec->arr[i].str));

    if (list[i] == NULL) {
      char_pt_pt_drop(list);

      return NULL;
    }
  }

  list[vec->len] = NULL;

  return list;
}

void print_token_vec(token_vec_t *vec) {
  for (int i = 0; i < vec->len; i++) {
    print_token(vec->arr[i]);
  }

  fprintf(stderr, "\n");
}

#undef DEFAULT_SIZE
