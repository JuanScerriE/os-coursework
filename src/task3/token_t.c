#include "token_t.h"

token_t emit_tok(token_type_t type) {
  return (token_t){type, NULL};
}

token_t emit_cmd_tok(string_t str) {
  return (token_t){COMMAND, str};
}

void token_free(token_t *tok) {
  if (tok != NULL) {
    if (tok->type == COMMAND) {
      string_free(&tok->str);
    }
  }
}

/* This was used for debuging purposes. */
void print_token(token_t tok) {
  switch (tok.type) {
    case COMMAND:
      fprintf(stderr, "[%s {%ld}]", tok.str.arr,
              tok.str.len);
      break;
    case PIPE:
      fprintf(stderr, "[|]");
      break;
    case IN:
      fprintf(stderr, "[<]");
      break;
    case OUT:
      fprintf(stderr, "[>]");
      break;
    case APPEND:
      fprintf(stderr, "[>>]");
      break;
    case SEP:
      fprintf(stderr, "[;]");
      break;
  }
}
