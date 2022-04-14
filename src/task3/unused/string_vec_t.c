#include "string_vec_t.h"

#include "util.h"

#define DEFAULT_SIZE 16

// If return is -1 then there is an error. Otherwise return
// allocated space.
size_t string_vec_grow(string_vec_t *vec, size_t need) {
  if (vec == NULL)
    return -1;

  vec->alc = vec->alc > 0 ? vec->alc : 1;

  while (vec->alc < need)
    vec->alc <<= 1;

  vec->arr = realloc(vec->arr, sizeof(string_t) * vec->alc);

  if (vec->arr == NULL)
    return -1;
  else
    return vec->alc;
}

// If return is -1 then there is an error. Otherwise return
// allocated space.
size_t string_vec_init(string_vec_t *vec, size_t est) {
  if (vec == NULL)
    return -1;

  vec->arr = NULL;
  vec->len = 0;
  vec->alc = 0;

  if (est > 0)
    return string_vec_grow(vec, est);
  else
    return 0;
}

// If return is NULL then there is an error.
string_vec_t *string_arr_new(void) {
  string_vec_t *vec = malloc(sizeof(string_vec_t));
  if (vec == NULL)
    return NULL;
  if (string_vec_init(vec, DEFAULT_SIZE) == -1)
    return NULL;
  return vec;
}

// This obviously will return nothing.
// NOTE: Only call if the whole string_vec stucture was heap
// allocated.
void string_vec_drop(string_vec_t *vec) {
  if (vec != NULL) {
    if (vec->arr != NULL)
      free(vec->arr);

    free(vec);
  }
}

// This obviously will return nothing.
// NOTE: Call if the whole string_vec stucture was stack
// allocated.
void string_vec_free(string_vec_t *vec) {
  if (vec != NULL)
    if (vec->arr != NULL)
      free(vec->arr);
}

// If return is -1 then there is an error. Otherwise return
// allocated space.
size_t string_vec_append(string_vec_t *vec, string_t str) {
  if (vec == NULL)
    return -1;

  if (vec->len >= vec->alc)
    if (string_vec_grow(vec, vec->alc + 1) == -1)
      return -1;

  vec->arr[vec->len++] = str;

  return vec->alc;
}

// If return is NULL then there is an error.
char **string_vec_get_list(string_vec_t *vec) {
  if (vec == NULL)
    return NULL;

  // We add +1 to ensure the list can be NULL terminated.
  char **list = malloc(sizeof(char *) * (vec->len + 1));

  if (list == NULL)
    return NULL;

  for (size_t i = 0; i < vec->len; i++) {
    list[i] = string_to_char_pt(&(vec->arr[i]));

    if (list[i] == NULL) {
      char_pt_pt_drop(list);

      return NULL;
    }
  }

  list[vec->len] = NULL;

  return list;
}

#undef DEFAULT_SIZE
