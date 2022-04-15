#pragma once

#include <stdio.h>

#include "string_t.h"

typedef enum {
  COMMAND,
  PIPE,    // |
  IN,      // <
  OUT,     // >
  APPEND,  // >>
  SEP,     // ;
} token_type_t;

typedef struct {
  token_type_t type;
  union {
    string_t str;
    void *nothing;  // This is used to represent nothing.
  };
} token_t;

token_t emit_tok(token_type_t);
token_t emit_cmd_tok(string_t);

void token_free(token_t *);
void print_token(token_t);
