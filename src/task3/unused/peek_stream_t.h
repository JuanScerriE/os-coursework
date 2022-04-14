#pragma once

#include <stdlib.h>

// NOTE: This data structure is not the same as the string
// as it is only meant to allow us to traverse a C string.
//
// So this data structure should only be used on the stack
// as it does not require any allocation.

typedef struct {
  char *arr;
  size_t pos;
  size_t len;
} peek_stream_t;

static inline peek_stream_t peek_stream_init(char *);

char peek_stream_peek(peek_stream_t *);
char peek_stream_next(peek_stream_t *);
int peek_stream_end(peek_stream_t *);
