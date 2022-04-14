#include "tokenizer.h"

static inline bool isdelim(char ch, const char *delim) {
  for (size_t i = 0; i < strlen(delim); i++) {
    if (ch == delim[i])
      return true;
  }

  return false;
}

char **tokenise(char *line, const char *delim) {
  struct peek_stream stream;
  peek_stream_init(&stream, line);

  struct string_vec *vec = string_vec_new();

  char ch;
  struct string *tok;

  while (!peek_stream_end(&stream)) {
    ch = peek_stream_next(&stream);

    if (!isdelim(ch, delim) && ch != '\0') {
      tok = string_new();

      do {
        string_append(tok, ch);
        ch = peek_stream_next(&stream);
      } while (!isdelim(ch, delim) && ch != '\0');

      string_append(tok, '\0');
      string_vec_append(vec, tok);
    }
  }

  char **tokens = string_vec_get_list(vec);
  string_vec_deep_free(vec);

  return tokens;
}
