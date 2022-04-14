#include "tokeniser.h"

// NOTE: I am using global variables for string and token
// because there is no need for these to be allocated on the
// heap. They are treated as normal variables by the C so
// when you assign a them to another the whole structure is
// copied.
//
// The only part of these which is worth keeping on the heap
// is the char * to the character array. Naturally, because
// it can grow and it's size is not known at compile time.

/* ------------------------------------------------- */

// This is a global string which is used to allocate new
// strings.
static string_t str_g;

// These methods are wrapper functions to reduce
// clutter in the main function. In fact they should
// be optimised out by the compiler.

static inline size_t w_str_init(void) {
  return string_init(&str_g, 16);
}

static inline size_t w_str_app(char ch) {
  return string_append(&str_g, ch);
}

static inline token_t w_emit_cmd_tok(void) {
  return emit_cmd_tok(str_g);
}

static inline void w_str_free(void) {
  string_free(&str_g);
}

/* ------------------------------------------------- */

// Similarly, this is a global variable used to allocate
// tokens.
static token_vec_t toks_g;

// And similarly these are wrappers to make writing easier
// and less cluttered.
static inline size_t w_tok_vec_init(void) {
  return token_vec_init(&toks_g, 16);
}

static inline size_t w_tok_vec_app(token_t tok) {
  return token_vec_append(&toks_g, tok);
}

/* ------------------------------------------------- */

// This structure is used to keep track of our position in
// the input character array across function calls.
// Naturally, this make the code less cluttered as well.

typedef struct {
  char *arr;
  size_t pos;
  size_t len;
} stream_t;

static stream_t stream_g;

// Similarly these are helper functions which make
// traversing one big character array is. Because we do not
// need to pass pointers around and we can keep track of our
// position in the character array across function calls.

static inline void stream_init(char *arr) {
  stream_g.arr = arr;
  stream_g.pos = 0;
  stream_g.len = strlen(arr);
}

static inline char next(void) {
  if (stream_g.pos < stream_g.len)
    return stream_g.arr[stream_g.pos++];

  return '\0';
}

static inline char peek() {
  if (stream_g.pos < stream_g.len)
    return stream_g.arr[stream_g.pos];

  return '\0';
}

static inline bool end(void) {
  return stream_g.pos >= stream_g.len ? true : false;
}

/* ------------------------------------------------- */

// This function is used to check if the @param ch is in the
// @param arr.
static inline bool is_in(char ch, const char *arr) {
  for (size_t i = 0; i < strlen(arr); i++) {
    if (ch == arr[i])
      return true;
  }

  return false;
}

// This function is used to consume a string as a whole
// command.
static inline size_t consume_str_as_cmd() {
  char ch;

  while (peek() != '"') {
    ch = next();

    if (end() || is_in(ch, "\r\v\f") || ch == '\0') {
      fprintf(
          stderr,
          "consume_string_as_command: Non-closed string\n");
      return -1;
    }

    // NOTE: Wait for the response of the lecturers.
    // FIXME: I have some issues with escaping I think they
    // are related to this and they need to be dealt with
    // depending on the expected behaviour.
    if (ch == '\\') {
      switch (ch = next()) {
        case '\\':
          if (w_str_app(ch) == -1)
            return -1;
          break;
        case '"':
          if (w_str_app(ch) == -1)
            return -1;
          break;
        default:
          fprintf(stderr,
                  "consume_string_as_command: Invalid "
                  "escape sequence\n");
          return -1;
          break;
      }
    } else {
      if (w_str_app(ch) == -1)
        return -1;
    }
  }

  // Null terminate the string
  if (w_str_app('\0') == -1)
    return -1;

  // Consume the double quote (")
  next();

  return str_g.alc;
}

// This is used just to consume a normal command.
// So nothing is escaped and the special characters stop us
// from continuing to consume characters.
static inline size_t consume_cmd(char first_ch) {
  if (w_str_app(first_ch) == -1)
    return -1;

  char ch;

  while (!end()) {
    ch = peek();

    if (is_in(ch, " \t\r\v\f|><;") || ch == '\0') {
      break;
    } else {
      if (w_str_app(next()) == -1)
        return -1;
    }
  }

  // Null terminate the string.
  if (w_str_app('\0') == -1)
    return -1;

  return str_g.alc;
}

// Again this is a simple wrapper which allows us to reduce
// clutter. Again this should be optimised away by the
// compiler.
static inline size_t init(char *arr) {
  stream_init(arr);

  if (w_tok_vec_init() == -1)
    return -1;

  if (w_str_init() == -1)
    return -1;

  return 0;
}

// This is the tokenise function which converts the user
// into from one big characters array into a number of
// tokens which can then be passed onto the parser for
// syntactic analysis.
size_t tokenise(token_vec_t *toks, char *arr) {
  if (init(arr) == -1) {
    goto fail_tokenise;
  }

  char ch;

  while (!end()) {
    ch = next();

    switch (ch) {
      // None of the characters should be newlines.
      case ' ':
      case '\t':
      case '\r':
      case '\v':
      case '\f':
        break;
      case '|':
        if (w_tok_vec_app(emit_tok(PIPE)) == -1)
          goto fail_tokenise;

        break;
      case '>':
        if (peek() == '>') {
          if (w_tok_vec_app(emit_tok(APPEND)) == -1)
            goto fail_tokenise;

          next();
        } else {
          if (w_tok_vec_app(emit_tok(OUT)) == -1)
            goto fail_tokenise;
        }
        break;
      case '<':
        if (w_tok_vec_app(emit_tok(IN)) == -1)
          goto fail_tokenise;

        break;
      case ';':
        if (w_tok_vec_app(emit_tok(SEP)) == -1)
          goto fail_tokenise;

        break;
      case '"':
        if (consume_str_as_cmd() == -1)
          goto fail_tokenise;

        if (w_tok_vec_app(w_emit_cmd_tok()) == -1)
          goto fail_tokenise;

        if (w_str_init() == -1)
          goto fail_tokenise;

        break;
      default:
        if (consume_cmd(ch) == -1)
          goto fail_tokenise;

        if (w_tok_vec_app(w_emit_cmd_tok()) == -1)
          goto fail_tokenise;

        if (w_str_init() == -1)
          goto fail_tokenise;

        break;
    }
  }

  w_str_free();
  *toks = toks_g;
  return toks_g.alc;

  // I am using labels to avoid repeating this pattern a
  // multitude of times.
fail_tokenise:
  w_str_free();
  *toks = toks_g;
  return -1;
}
