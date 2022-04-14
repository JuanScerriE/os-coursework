#include "parse_execute.h"

// Again similar to the tokeniser I am using this global
// object and a number of helper functions to easily
// traverse the stream of tokens without a lot clutter and
// effort.

typedef struct {
  token_vec_t toks;
  size_t pos;
} token_stream_t;

static token_stream_t stream_g;

static inline void stream_init(token_vec_t toks) {
  stream_g.toks = toks;
  stream_g.pos = 0;
}

static inline token_t next(void) {
  if (stream_g.pos < stream_g.toks.len)
    return stream_g.toks.arr[stream_g.pos++];

  return (token_t){
      SEP, NULL};  // This is effectively the \0 of a token.
}

static inline token_t peek() {
  if (stream_g.pos < stream_g.toks.len)
    return stream_g.toks.arr[stream_g.pos];

  return (token_t){SEP, NULL};
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
size_t parse(token_vec_t toks) {
  stream_init(toks);

  bool start = true;
  token_type_t prev_tok_type;
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
          start = true;
          break;
      }
    }

    prev_tok_type = tok.type;
  }

  return toks.alc;

fail_parse:
  return -1;
}

// This method is expecting a syntactically correct
// collection of tokens.
//
// This will generate the necessary data structure to make
// use of our tokens with the fork_exec_pipe() function.
pid_t execute(token_vec_t toks) {
  stream_init(toks);
  pid_t pid;
  char *infile = NULL;
  char *outfile = NULL;
  bool append = false;
  size_t pipeline_size = 16;
  size_t pipeline_len = 0;
  size_t command_size = 16;
  size_t command_len = 0;
  char ***pipeline = NULL;

  token_t tok;

  while (!end()) {
    // Allocate memory for the pipeline.
    pipeline = malloc(sizeof(char **) * pipeline_size);
    if (pipeline == NULL) {
      perror("execute");
      goto fail_execute;
    }
    pipeline[pipeline_len] =
        calloc(command_size, sizeof(char *));
    if (pipeline[pipeline_len] == NULL) {
      perror("execute");
      goto fail_execute;
    }

    tok = next();

    // Construct the pipeline.
    while (tok.type == COMMAND ||
           tok.type == PIPE && !end()) {
      switch (tok.type) {
        case COMMAND:
          if (command_len >= command_size) {
            command_size += 16;
            pipeline[pipeline_len] =
                realloc(pipeline[pipeline_len],
                        command_size * sizeof(char *));
            if (pipeline[pipeline_len] == NULL) {
              perror("execute");
              goto fail_execute;
            }
          }

          pipeline[pipeline_len][command_len++] =
              tok.str.arr;
          break;
        case PIPE:
          if (command_len >= command_size) {
            command_size += 16;
            pipeline[pipeline_len] =
                realloc(pipeline[pipeline_len],
                        command_size * sizeof(char *));
            if (pipeline[pipeline_len] == NULL) {
              perror("execute");
              goto fail_execute;
            }
          }

          pipeline[pipeline_len++][command_len] = NULL;

          if (pipeline_len >= pipeline_size) {
            pipeline_size += 16;
            pipeline = realloc(
                pipeline, pipeline_size * sizeof(char **));
            if (pipeline == NULL) {
              perror("execute");
              goto fail_execute;
            }
          }

          command_size = 16;
          command_len = 0;
          pipeline[pipeline_len] =
              calloc(command_size, sizeof(char *));
          if (pipeline[pipeline_len] == NULL) {
            perror("execute");
            goto fail_execute;
          }
          break;
        default:
          // This should never be reached.
          break;
      }

      tok = next();
    }

    // NULL terminate the pipeline.
    pipeline_len++;
    if (pipeline_len >= pipeline_size) {
      pipeline_size += 16;
      pipeline = realloc(pipeline,
                         pipeline_size * sizeof(char **));
      if (pipeline == NULL) {
        perror("execute");
        goto fail_execute;
      }
    }

    pipeline[pipeline_len] = NULL;

    // Get the infile, outfile and append flag if any.
    while (!end() && tok.type != SEP) {
      if (tok.type == IN) {
        tok = next();
        infile = tok.str.arr;
      } else if (tok.type == OUT) {
        tok = next();
        outfile = tok.str.arr;
      } else if (tok.type == APPEND) {
        tok = next();
        outfile = tok.str.arr;
        append = true;
      } else {
        tok = next();
      }
    }

    // This is the execute section.
    bool found_builtin = false;

    // Check if the first command is a builtin.
    for (size_t i = 0; i <= get_num_of_builtins(); i++) {
      if (!strcmp(pipeline[0][0], builtins[i].name)) {
        found_builtin = true;
        pid = builtins[i].func(pipeline[0]);
      }
    }

    // Otherwise try to execute an external command
    if (found_builtin == false) {
      pid = fork_exec_pipe(pipeline, 0, infile, outfile,
                           append);
    }

    if (pid == -1) {
      perror("execute");
      goto fail_execute;
    }

    // Free up any memory used by the pipeline before
    // constructing a new one or returning from the
    // function.
    if (pipeline != NULL) {
      for (size_t i = 0; i < pipeline_len; i++) {
        if (pipeline[i] != NULL) {
          free(pipeline[i]);
        }
      }

      free(pipeline);
    }

    // Reset all the variables.
    pipeline_size = 16;
    pipeline_len = 0;
    command_size = 16;
    command_len = 0;
    infile = NULL;
    outfile = NULL;
    append = false;
  }

  return pid;

  // Again this is used to reduce clutter in the function.
fail_execute:
  if (pipeline != NULL) {
    for (size_t i = 0; i < pipeline_len; i++) {
      if (pipeline[i] != NULL)
        free(pipeline[i]);
    }

    free(pipeline);
  }
  return -1;
}

/* int main(void) { */
/*   token_vec_t toks; */
/*   char msg1[] = */
/*       "< heslj aldjf owijf asdlfkj < sdfsdf >>
 * .wendle;;;;"; */
/*   char msg2[] = ";;;heslj |aldjf owijf ;asdlfkj <
 * asdfa";
 */
/*   char msg3[] = */
/*       "\"This is a string\" asdf asdf asdf >> asdf >
 * asfd
 * "; */
/*   char msg4[] = */
/*       "\"This i\\\"s\\\\ a string\" asdf asdf asdf >>
 * asdf " */
/*       "> asfd"; */
/*   char msg5[] = */
/*       "\"This i\\\"s\\\\ \\a a string\" asdf asdf asdf
 * >>
 * " */
/*       "asdf > asfd"; */
/*   char msg6[] = "heslj |aldjf owijf ;asdlfk\"j <
 * as\"dfa"; */
/*   char msg7[] = "heslj |aldjf owijf ;asdlfk\"j <
 * asdfa";
 */
/*   char msg8[] = "heslj |aldjf owijf ;asdlfk \"j <
 * asd\"fa"; */
/*   char msg9[] = */
/*       "heslj |aldjf owijf ;asdlfk \"j < asd\"\"fa"; */
/*   tokenise(&toks, msg1); */
/*   parse(toks); */
/*   fprintf(stderr, "1------\n"); */
/*   token_vec_free(&toks); */
/*   tokenise(&toks, msg2); */
/*   parse(toks); */
/*   fprintf(stderr, "2------\n"); */
/*   token_vec_free(&toks); */
/*   tokenise(&toks, msg3); */
/*   parse(toks); */
/*   fprintf(stderr, "3------\n"); */
/*   token_vec_free(&toks); */
/*   tokenise(&toks, msg4); */
/*   parse(toks); */
/*   fprintf(stderr, "4------\n"); */
/*   token_vec_free(&toks); */
/*   tokenise(&toks, msg5); */
/*   parse(toks); */
/*   fprintf(stderr, "5------\n"); */
/*   token_vec_free(&toks); */
/*   tokenise(&toks, msg6); */
/*   parse(toks); */
/*   fprintf(stderr, "6------\n"); */
/*   token_vec_free(&toks); */
/*   tokenise(&toks, msg7); */
/*   parse(toks); */
/*   fprintf(stderr, "7------\n"); */
/*   token_vec_free(&toks); */
/*   tokenise(&toks, msg8); */
/*   parse(toks); */
/*   fprintf(stderr, "8------\n"); */
/*   token_vec_free(&toks); */
/*   tokenise(&toks, msg9); */
/*   parse(toks); */
/*   fprintf(stderr, "9------\n"); */
/*   token_vec_free(&toks); */
/*   return 0; */
/* } */
