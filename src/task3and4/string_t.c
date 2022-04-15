#include "string_t.h"

#define DEFAULT_SIZE 16

// If return is -1 then there is an error. Otherwise return
// allocated space.
int string_grow(string_t *str, size_t need) {
  if (str == NULL) {
    fprintf(stderr, "string_grow: NULL pointer\n");
    return -1;
  }

  str->alc = str->alc > 0 ? str->alc : 1;

  while (str->alc < need)
    str->alc <<= 1;

  str->arr = realloc(str->arr, sizeof(char) * str->alc);

  if (str->arr == NULL) {
    perror("string_grow");
    return -1;
  } else {
    return 0;
  }
}

// If return is -1 then there is an error. Otherwise return
// allocated space.
int string_init(string_t *str, size_t est) {
  if (str == NULL) {
    fprintf(stderr, "string_init: NULL pointer\n");
    return -1;
  }

  str->arr = NULL;
  str->len = 0;
  str->alc = 0;

  if (est > 0)
    return string_grow(str, est);
  else
    return 0;
}

// If return is NULL then there is an error.
string_t *string_new(void) {
  string_t *str = malloc(sizeof(string_t));
  if (str == NULL) {
    perror("string_new");
    return NULL;
  }

  if (string_init(str, DEFAULT_SIZE) == -1)
    return NULL;

  return str;
}

// This obviously will return nothing.
// NOTE: Only call if the whole string stucture was heap
// allocated.
void string_drop(string_t *str) {
  if (str != NULL) {
    if (str->arr != NULL)
      free(str->arr);

    free(str);
  }
}

// This obviously will return nothing.
// NOTE: Call if the whole string stucture was stack
// allocated.
void string_free(string_t *str) {
  if (str != NULL)
    if (str->arr != NULL)
      free(str->arr);
}

// If return is -1 then there is an error. Otherwise return
// allocated space.
int string_append(string_t *str, char ch) {
  if (str == NULL) {
    fprintf(stderr, "string_append: NULL pointer\n");
    return -1;
  }

  if (str->len >= str->alc)
    if (string_grow(str, str->alc + 1) == -1)
      return -1;

  str->arr[str->len++] = ch;

  return 0;
}

// If return is NULL then there is an error.
char *string_to_char_pt(string_t *str) {
  if (str == NULL) {
    fprintf(stderr, "string_to_char_pt: NULL pointer\n");
    return NULL;
  }

  char *char_pt = malloc(sizeof(char) * str->len);

  if (char_pt == NULL) {
    perror("string_to_char_pt");
    return NULL;
  }

  strncpy(char_pt, str->arr, str->len);

  return char_pt;
}

#undef DEFAULT_SIZE
