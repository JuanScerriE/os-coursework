// Again similar to the tokeniser I am using this global
// object and a number of helper functions to easily
// traverse the stream of tokens without a lot clutter and
// effort.

#include <stdbool.h>
#include <stdlib.h>

typedef enum {
  STRING,
  PIPE,    // |
  IN,      // <
  OUT,     // >
  APPEND,  // >>
  SEP,     // ;
} TokenType;

typedef struct {
  TokenType type;
  struct {
    char *str;
    size_t len;
  };
} Token;

typedef struct {
  char *stream;
  size_t pos;
  size_t len;
} CharStream;

static CharStream charStream;

static inline void initCharStream(char *stream, size_t len) {
  charStream.stream = stream;
  charStream.len = len;
}

typedef struct {
  Token *stream;
  size_t pos;
  size_t len;
} TokenStream;

static TokenStream tokenStream;

static inline void initTokenStream(char *stream, size_t len) {
  charStream.stream = stream;
  charStream.len = len;
}

typedef struct {
  char ***pipeline;
  char *infile;
  char *outfile;
  bool append;
} Statement;

typedef struct {
  // These are the final product which will be executed.
  struct {
    Statement *get;
    size_t len;
  } statement;

  //
} InterpreterState;

static InterpreterState state;

static token_stream_t stream_g;

static string_t str_g;

// These methods are wrapper functions to reduce
// clutter in the main function. In fact they should
// be optimised out by the compiler.

static inline int w_str_init(void) {
  return string_init(&str_g, 16);
}

static inline int w_str_app(char ch) {
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
static inline int w_tok_vec_init(void) {
  return token_vec_init(&toks_g, 16);
}

static inline int w_tok_vec_app(token_t tok) {
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
static inline int consume_str_as_cmd() {
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

  return 0;
}

// This is used just to consume a normal command.
// So nothing is escaped and the special characters stop us
// from continuing to consume characters.
static inline int consume_cmd(char first_ch) {
  if (w_str_app(first_ch) == -1)
    return -1;

  char ch;

  while (!end()) {
    ch = peek();

    if (is_in(ch, " \t\r\v\f|><\n;") || ch == '\0') {
      break;
    } else {
      if (w_str_app(next()) == -1)
        return -1;
    }
  }

  // Null terminate the string.
  if (w_str_app('\0') == -1)
    return -1;

  return 0;
}

// Again this is a simple wrapper which allows us to reduce
// clutter. Again this should be optimised away by the
// compiler.
static inline int init(char *arr) {
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
//
// NOTE: This is handling the requests of task 3 and task 4.
// More specifically, this is handling parts question a)
// form task 3 and and question a), b) and c) from task 4.
int tokenise(token_vec_t *toks, char *arr) {
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
      case '\n':
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
  return 0;

  // I am using labels to avoid repeating this pattern a
  // multitude of times.
fail_tokenise:
  w_str_free();
  *toks = toks_g;
  return -1;
}

static inline void stream_init(token_vec_t toks) {
  stream_g.toks = toks;
  stream_g.pos = 0;
}

static inline token_t next(void) {
  if (stream_g.pos < stream_g.toks.len)
    return stream_g.toks.arr[stream_g.pos++];

  // This is effectively the \0 of a token.
  return emit_tok(SEP);
}

static inline token_t peek() {
  if (stream_g.pos < stream_g.toks.len)
    return stream_g.toks.arr[stream_g.pos];

  return emit_tok(SEP);
}

static inline bool end(void) {
  return stream_g.pos >= stream_g.toks.len ? true : false;
}

/* ------------------------------------------------- */

// This function checks that the tokens follow a correct
// syntactical order. Specifically, the largest thing a this
// shell can execute is the following:
//
// {pipeline} [infile] [outfile]
//
// Where the pipeline must be at least one command and
// infile and outfile are optional. But they must be in that
// order.
//
// NOTE: This does not really mimic the behaviour of other
// shells because in other shell the redirection operators
// take precedence over pipes. But without using an AST that
// would have been much more complicated to implement.
//
// NOTE: Again note that this is handling both the requests
// of task 3 and task 4. Similarly to the tokeniser this is
// handling a the part of syntatic analysis described in
// part a) for task 3. Moreover, it also handles the final
// part of c) from task 4.
// Let us make this return the number of statements. in one
// line.
int parse(token_vec_t toks, size_t *num_of_statements) {
  stream_init(toks);

  bool start = true;
  token_type_t prev_tok_type = SEP;
  token_t tok;

  while (!end()) {
    tok = next();

    // To make sure that the end is handled correctly.
    // Because we cannot rely on previous to for example
    // handle an incorrect redirection like: "ls -al >"
    if (end())
      prev_tok_type = tok.type;

    // The first token must always be a command or a
    // separator.
    if (start == true) {
      switch (tok.type) {
        case COMMAND:
          start = false;
          break;
        case SEP:
          (*num_of_statements)++;
          start = true;
          break;
        default:
          fprintf(
              stderr,
              "parse: A line must start with a command\n");
          goto fail_parse;
          break;
      }
    } else {
      // We decide what is appropriate depending on
      // our previous token type
      switch (prev_tok_type) {
        case COMMAND:
          switch (tok.type) {
            case COMMAND:
            case PIPE:
            case IN:
            case OUT:
            case APPEND:
              break;
            case SEP:
              (*num_of_statements)++;
              start = true;
              break;
          }
          break;
        case PIPE:
          switch (tok.type) {
            case COMMAND:
              break;
            default:
              fprintf(stderr,
                      "parse: | must be followed by a "
                      "command\n");
              goto fail_parse;
              break;
          }
          break;
        case IN:
          if (tok.type != COMMAND) {
            fprintf(
                stderr,
                "parse: < must be followed by a file\n");
            goto fail_parse;
          } else
            tok = next();

          switch (tok.type) {
            case OUT:
            case APPEND:
              break;
            case SEP:
              start = true;
              break;
            default:
              fprintf(stderr,
                      "parse: < must be followed by a file "
                      "and a > or >> or ;\n");
              goto fail_parse;
              break;
          }
          break;
        case OUT:
        case APPEND:
          if (tok.type != COMMAND) {
            fprintf(stderr,
                    "parse: > or >> must be followed by a "
                    "file\n");
            goto fail_parse;
          } else
            tok = next();

          switch (tok.type) {
            case SEP:
              (*num_of_statements)++;
              start = true;
              break;
            default:
              fprintf(stderr,
                      "parse: > or >> must be followed by "
                      "a file and a ;\n");
              goto fail_parse;
              break;
          }
          break;
        case SEP:
          (*num_of_statements)++;
          start = true;
          break;
      }
    }

    prev_tok_type = tok.type;
  }

  return 0;

fail_parse:
  return -1;
}

typedef struct {
  char ***pipeline;
  char *infile;
  char *outfile;
  bool append;
} statement_t;

// This method is expecting a syntactically correct
// collection of tokens.
//
// This will generate the necessary data structure to make
// use of our tokens with the fork_exec_pipe() function.
/* statement_t *generate_statements(token_vec_t toks) { */
/*   size_t statements_size = 1; */
/*   statement_t *statements = */
/*       malloc(sizeof(statement_t) * statements_size); */

/*   stream_init(toks); */
/*   pid_t pid = -1; */
/*   char *infile = NULL; */
/*   char *outfile = NULL; */
/*   bool append = false; */
/*   size_t pipeline_size = 16; */
/*   size_t pipeline_len = 0; */
/*   size_t command_size = 16; */
/*   size_t command_len = 0; */
/*   char ***pipeline = NULL; */

/*   token_t tok; */

/*   while (!end()) { */
/*     // Allocate memory for the pipeline. */
/*     pipeline = malloc(sizeof(char **) * pipeline_size);
 */
/*     if (pipeline == NULL) { */
/*       perror("execute"); */
/*       goto fail_execute; */
/*     } */
/*     pipeline[pipeline_len] = */
/*         calloc(command_size, sizeof(char *)); */
/*     if (pipeline[pipeline_len] == NULL) { */
/*       perror("execute"); */
/*       goto fail_execute; */
/*     } */

/*     tok = next(); */

/*     // Construct the pipeline. */
/*     while (tok.type == COMMAND || */
/*            (tok.type == PIPE && !end())) { */
/*       switch (tok.type) { */
/*         case COMMAND: */
/*           if (command_len >= command_size) { */
/*             command_size += 16; */
/*             pipeline[pipeline_len] = */
/*                 realloc(pipeline[pipeline_len], */
/*                         command_size * sizeof(char *));
 */
/*             if (pipeline[pipeline_len] == NULL) { */
/*               perror("execute"); */
/*               goto fail_execute; */
/*             } */
/*           } */

/*           pipeline[pipeline_len][command_len++] = */
/*               tok.str.arr; */
/*           break; */
/*         case PIPE: */
/*           if (command_len >= command_size) { */
/*             command_size += 16; */
/*             pipeline[pipeline_len] = */
/*                 realloc(pipeline[pipeline_len], */
/*                         command_size * sizeof(char *));
 */
/*             if (pipeline[pipeline_len] == NULL) { */
/*               perror("execute"); */
/*               goto fail_execute; */
/*             } */
/*           } */

/*           pipeline[pipeline_len++][command_len] = NULL;
 */

/*           if (pipeline_len >= pipeline_size) { */
/*             pipeline_size += 16; */
/*             pipeline = realloc( */
/*                 pipeline, pipeline_size * sizeof(char
 * **)); */
/*             if (pipeline == NULL) { */
/*               perror("execute"); */
/*               goto fail_execute; */
/*             } */
/*           } */

/*           command_size = 16; */
/*           command_len = 0; */
/*           pipeline[pipeline_len] = */
/*               calloc(command_size, sizeof(char *)); */
/*           if (pipeline[pipeline_len] == NULL) { */
/*             perror("execute"); */
/*             goto fail_execute; */
/*           } */
/*           break; */
/*         default: */
/*           // This should never be reached. */
/*           break; */
/*       } */

/*       tok = next(); */
/*     } */

/*     // NULL terminate the pipeline. */
/*     pipeline_len++; */
/*     if (pipeline_len >= pipeline_size) { */
/*       pipeline_size += 16; */
/*       pipeline = realloc(pipeline, */
/*                          pipeline_size * sizeof(char
 * **)); */
/*       if (pipeline == NULL) { */
/*         perror("execute"); */
/*         goto fail_execute; */
/*       } */
/*     } */

/*     pipeline[pipeline_len] = NULL; */

/*     // Get the infile, outfile and append flag if any. */
/*     while (!end() && tok.type != SEP) { */
/*       if (tok.type == IN) { */
/*         tok = next(); */
/*         infile = tok.str.arr; */
/*       } else if (tok.type == OUT) { */
/*         tok = next(); */
/*         outfile = tok.str.arr; */
/*       } else if (tok.type == APPEND) { */
/*         tok = next(); */
/*         outfile = tok.str.arr; */
/*         append = true; */
/*       } else { */
/*         tok = next(); */
/*       } */
/*     } */

/*     // This is the execute section. */
/*     // NOTE: This section deals with question b) and */
/*     // question c) from task 3. */
/*     bool found_builtin = false; */

/*     // Check if the first command is a builtin. */
/*     for (size_t i = 0; i < get_num_of_builtins(); i++) {
 */
/*       if (!strcmp(pipeline[0][0], builtins[i].name)) { */
/*         found_builtin = true; */
/*         pid = builtins[i].func(pipeline[0]); */
/*       } */
/*     } */

/*     // Otherwise try to execute an external command */
/*     if (found_builtin == false) { */
/*       pid = fork_exec_pipe(pipeline, 0, infile, outfile,
 */
/*                            append); */
/*     } */

/*     if (pid == EXIT_SHELL) */
/*       goto fail_execute; */

/*     if (pid == -1) { */
/*       goto fail_execute; */
/*     } */

/*     // Free up any memory used by the pipeline before */
/*     // constructing a new one or returning from the */
/*     // function. */
/*     if (pipeline != NULL) { */
/*       for (size_t i = 0; i < pipeline_len; i++) { */
/*         if (pipeline[i] != NULL) { */
/*           free(pipeline[i]); */
/*         } */
/*       } */

/*       free(pipeline); */
/*     } */

/*     // Reset all the variables. */
/*     pipeline_size = 16; */
/*     pipeline_len = 0; */
/*     command_size = 16; */
/*     command_len = 0; */
/*     infile = NULL; */
/*     outfile = NULL; */
/*     append = false; */
/*   } */

/*   return pid; */

/*   // Again this is used to reduce clutter in the
 * function. */
/* fail_execute: */
/*   if (pipeline != NULL) { */
/*     for (size_t i = 0; i < pipeline_len; i++) { */
/*       if (pipeline[i] != NULL) */
/*         free(pipeline[i]); */
/*     } */

/*     free(pipeline); */
/*   } */
/*   if (pid == EXIT_SHELL) */
/*     return -2; */
/*   else */
/*     return -1; */
/* } */
