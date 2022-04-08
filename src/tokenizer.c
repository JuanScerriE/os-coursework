#include "peek_stream.h"
#include "token_vec.h"

#include <ctype.h>

struct token_vec *tokenise(char *line) {
  struct peek_stream stream;
  peek_stream_init(&stream, line);

  struct token_vec *vec = token_vec_new();

  char ch;
  struct token *tok;

  while (!peek_stream_end(&stream)) {
    ch = peek_stream_next(&stream);

    if (!isspace(ch) && ch != '\0') {
      tok = token_new();

      do {
        token_append(tok, ch);
        ch = peek_stream_next(&stream);
      } while (!isspace(ch) && ch != '\0');

      token_append(tok, '\0');
      token_vec_append(vec, tok);
    }
  }

  return vec;
}
