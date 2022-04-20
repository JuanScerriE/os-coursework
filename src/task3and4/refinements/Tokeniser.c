#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../builtin.h"
#include "../external.h"

#define INIT_SIZE 8
#define GROW_SIZE 8

/* #define DEBUG */

/* ------------------------------------------- */
/* Types */
/* ------------------------------------------- */

typedef struct {
  char *str;
  size_t pos;
  size_t len;
} CharStream;

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
  size_t len;
  char *str;
} Token;

typedef struct {
  Token *toks;
  size_t pos;
  size_t len;
} TokenStream;

typedef struct {
  struct {
    char ***arr;
    size_t len;
  } pipeline;
  char *infile;
  char *outfile;
  bool append;
} Statement;

typedef struct {
  struct {
    Token *arr;
    size_t len;
    size_t alc;
  } Tokens;
  struct {
    Statement *arr;
    size_t len;
  } Statements;
} InterpreterState;

/* ------------------------------------------- */
/* Static Globals */
/* ------------------------------------------- */

static CharStream CS;
static TokenStream TS;
static InterpreterState IS;

/* ------------------------------------------- */
/* Debug Functions */
/* ------------------------------------------- */

#ifdef DEBUG
static inline void printTokens(void) {
  if (IS.Tokens.arr != NULL) {
    fprintf(stderr, "TOKENS:\n");
    for (size_t i = 0; i < IS.Tokens.len; i++) {
      switch (IS.Tokens.arr[i].type) {
        case STRING:
          fprintf(stderr, "  [%s]\n", IS.Tokens.arr[i].str);
          break;
        case PIPE:
          fprintf(stderr, "  [|]\n");
          break;
        case IN:
          fprintf(stderr, "  [<]\n");
          break;
        case OUT:
          fprintf(stderr, "  [>]\n");
          break;
        case APPEND:
          fprintf(stderr, "  [>>]\n");
          break;
        case SEP:
          fprintf(stderr, "  [;]\n");
          break;
      }
    }
  }
}

static inline void printStatement(size_t i) {
  if (i < IS.Statements.len && IS.Statements.arr != NULL) {
    fprintf(stderr, "STATEMENT %lu:\n", i);

    fprintf(stderr,
            "  infile: %s\n  outfile: %s\n  append: %u\n  "
            "-->\n",
            IS.Statements.arr[i].infile,
            IS.Statements.arr[i].outfile,
            IS.Statements.arr[i].append);

    if (IS.Statements.arr[i].pipeline.arr != NULL) {
      for (size_t j = 0;
           j < IS.Statements.arr[i].pipeline.len; j++) {
        fprintf(stderr, "  COMMAND %lu:\n", j);
        for (size_t k = 0;
             IS.Statements.arr[i].pipeline.arr[j][k] !=
             NULL;
             k++) {
          fprintf(stderr, "    STRING %lu: %s\n", k,
                  IS.Statements.arr[i].pipeline.arr[j][k]);
        }
      }
    }
  }
}

static inline void printStatements(void) {
  for (size_t i = 0; i < IS.Statements.len; i++) {
    printStatement(i);
  }
}
#endif

/* ------------------------------------------- */
/* Char Stream Helpers */
/* ------------------------------------------- */

static inline void initCS(char *str, size_t len) {
  CS.str = str;
  CS.pos = 0;
  CS.len = len;
}

static inline size_t getCSLen(void) {
  return CS.len;
}

static inline char CSNext(void) {
  if (CS.pos < CS.len)
    return CS.str[CS.pos++];

  return '\0';
}

static inline char CSPeek(void) {
  if (CS.pos < CS.len)
    return CS.str[CS.pos];

  return '\0';
}

static inline bool isCSEnd(void) {
  return CS.pos >= CS.len ? true : false;
}

/* ------------------------------------------- */
/* Token Stream Helpers */
/* ------------------------------------------- */

static inline void initTS(void) {
  TS.toks = IS.Tokens.arr;
  TS.pos = 0;
  TS.len = IS.Tokens.len;
}

static inline size_t getTSLen(void) {
  return TS.len;
}

static inline Token TSNext(void) {
  if (TS.pos < TS.len)
    return TS.toks[TS.pos++];

  return (Token){SEP, 0, NULL};
}

static inline Token TSPeek(void) {
  if (TS.pos < TS.len)
    return TS.toks[TS.pos];

  return (Token){SEP, 0, NULL};
}

static inline bool isTSEnd(void) {
  return TS.pos >= TS.len ? true : false;
}

/* ------------------------------------------- */
/* Global Interpreter State Tokens Helpers */
/* ------------------------------------------- */

static inline int initTokens(void) {
  IS.Tokens.len = 0;
  IS.Tokens.alc = INIT_SIZE;
  IS.Tokens.arr = malloc(sizeof(Token) * IS.Tokens.alc);

  if (IS.Tokens.arr == NULL) {
    perror("initTokens");
    return -1;
  } else {
    return 0;
  }
}

// We cannot use this always because we might use the
// pointers in the tokens later on to avoid copying
// increasing performance.
static inline void cleanTokens(void) {
  if (IS.Tokens.arr != NULL) {
    // Free the allocated strings in the individual tokens.
    for (size_t i = 0; i < IS.Tokens.len; i++)
      if (IS.Tokens.arr[i].str != NULL) {
        free(IS.Tokens.arr[i].str);
      }

    free(IS.Tokens.arr);
  }

  IS.Tokens.arr = NULL;
  IS.Tokens.len = 0;
  IS.Tokens.alc = INIT_SIZE;
}

static inline int addToken(Token tok) {
  if (IS.Tokens.len >= IS.Tokens.alc) {
    IS.Tokens.alc += GROW_SIZE;
    Token *tmp = realloc(IS.Tokens.arr,
                         sizeof(Token) * IS.Tokens.alc);

    if (tmp == NULL) {
      perror("addToken");
      return -1;
    } else {
      IS.Tokens.arr = tmp;
    }
  }

  IS.Tokens.arr[IS.Tokens.len++] = tok;
  return 0;
}

/* ------------------------------------------- */
/* Global Interpreter State Statements Helpers */
/* ------------------------------------------- */

static inline void initStatements(void) {
  IS.Statements.len = 0;
  IS.Statements.arr = NULL;
}

static inline void cleanStatements(void) {
  if (IS.Statements.arr != NULL) {
    for (size_t i = 0; i < IS.Statements.len; i++) {
      if (IS.Statements.arr[i].pipeline.arr != NULL) {
        for (size_t j = 0;
             j < IS.Statements.arr[i].pipeline.len; j++) {
          if (IS.Statements.arr[i].pipeline.arr[j] != NULL)
            free(IS.Statements.arr[i].pipeline.arr[j]);
        }

        free(IS.Statements.arr[i].pipeline.arr);
      }
    }

    free(IS.Statements.arr);
  }

  IS.Statements.arr = NULL;
  IS.Statements.len = 0;
}

/* ------------------------------------------- */
/* Tokeniser Helpers */
/* ------------------------------------------- */

static inline int emitPipe(void) {
  CSNext();
  Token tok = {PIPE, 0, NULL};
  return addToken(tok);
}

static inline int emitOutOrAppend(void) {
  CSNext();
  Token tok = {OUT, 0, NULL};

  if (CSPeek() == '>') {
    tok.type = APPEND;
    CSNext();
  }

  return addToken(tok);
}

static inline int emitIn(void) {
  CSNext();
  Token tok = {IN, 0, NULL};
  return addToken(tok);
}

static inline int emitSep(void) {
  CSNext();
  Token tok = {SEP, 0, NULL};
  return addToken(tok);
}

static inline int emitQuotedString(void) {
  CSNext();
  Token tok = {STRING, 0, NULL};
  size_t alc = INIT_SIZE;
  tok.str = malloc(sizeof(char) * alc);

  if (tok.str == NULL) {
    perror("emitQuotedString");
    return -1;
  }

  char ch = CSPeek();

  while (ch != '"') {
    ch = CSNext();

    if (isCSEnd()) {
      fprintf(stderr,
              "emitQuotedString: Non-closed string\n");
      return -1;
    }

    if (ch == '\\') {
      switch (ch = CSNext()) {
        case '\\':
        case '"':
          // DO NOTHING
          break;
        default:
          fprintf(stderr,
                  "emitQuotedString: Invalid escape "
                  "sequence\n");
          return -1;
      }
    }

    if (tok.len >= alc) {
      alc += GROW_SIZE;
      char *tmp = realloc(tok.str, sizeof(char) * alc);

      if (tmp == NULL) {
        perror("emitQuotedString");
        return -1;
      } else {
        tok.str = tmp;
      }
    }

    tok.str[tok.len++] = ch;

    ch = CSPeek();
  }

  CSNext();

  if (tok.len >= alc) {
    alc += GROW_SIZE;
    char *tmp = realloc(tok.str, sizeof(char) * alc);

    if (tmp == NULL) {
      perror("emitQuotedString");
      return -1;
    } else {
      tok.str = tmp;
    }
  }

  tok.str[tok.len] = '\0';

  return addToken(tok);
}

static inline int emitString(void) {
  Token tok = {STRING, 0, NULL};
  size_t alc = INIT_SIZE;
  tok.str = malloc(sizeof(char) * alc);

  if (tok.str == NULL) {
    perror("emitString");
    return -1;
  }

  char ch = CSPeek();

  while (ch != ' ' && ch != '\t' && ch != '\r' &&
         ch != '\v' && ch != '\f' && ch != '|' &&
         ch != '<' && ch != '>' && ch != '\n' &&
         ch != ';' && ch != '"' && ch != '\0') {
    ch = CSNext();

    if (ch == '\\') {
      switch (ch = CSNext()) {
        case '\\':
        case '"':
          // DO NOTHING
          break;
        default:
          fprintf(stderr,
                  "emitString: Invalid escape sequence\n");
          return -1;
      }
    }

    if (tok.len >= alc) {
      alc += GROW_SIZE;
      char *tmp = realloc(tok.str, sizeof(char) * alc);

      if (tmp == NULL) {
        perror("emitString");
        return -1;
      } else {
        tok.str = tmp;
      }
    }

    tok.str[tok.len++] = ch;

    ch = CSPeek();
  }

  if (tok.len >= alc) {
    alc += GROW_SIZE;
    char *tmp = realloc(tok.str, sizeof(char) * alc);

    if (tmp == NULL) {
      perror("emitString");
      return -1;
    } else {
      tok.str = tmp;
    }
  }

  tok.str[tok.len] = '\0';

  return addToken(tok);
}

/* ------------------------------------------- */
/* Tokeniser */
/* ------------------------------------------- */

static inline int tokenise(char *str) {
  initCS(str, strlen(str));
  if (initTokens() == -1)
    goto fail_tokenise;

  char ch;

  while (!isCSEnd()) {
    ch = CSPeek();

#ifdef DEBUG
    fprintf(stderr, "Character: %c\n", ch);
#endif

    switch (ch) {
        // WHITE SPACE
      case ' ':
      case '\t':
      case '\r':
      case '\v':
      case '\f':
        ch = CSNext();
        break;
        // PIPE
      case '|':
        if (emitPipe() == -1)
          goto fail_tokenise;
        break;
        // OUT or APPEND
      case '>':
        if (emitOutOrAppend() == -1)
          goto fail_tokenise;
        break;
        // IN
      case '<':
        if (emitIn() == -1)
          goto fail_tokenise;
        break;
        // SEP
      case '\n':
      case ';':
        if (emitSep() == -1)
          goto fail_tokenise;
        break;
        // (QUOTED) STRING
      case '"':
        if (emitQuotedString() == -1)
          goto fail_tokenise;
        break;
        // STRING
      default:
        if (emitString() == -1)
          goto fail_tokenise;
        break;
    }
  }

#ifdef DEBUG
  printTokens();
#endif
  return 0;

fail_tokenise:
  cleanTokens();
  return -1;
};

/* ------------------------------------------- */
/* Parser */
/* ------------------------------------------- */

static inline int parse(void) {
  initStatements();
  initTS();

  bool start = true;
  TokenType prevTokType = SEP;
  Token tok;

  while (!isTSEnd()) {
    tok = TSNext();

    // To make sure that the end is handled correctly.
    // Because we cannot rely on previous to for example
    // handle an incorrect redirection like: "ls -al >"
    if (isTSEnd())
      prevTokType = tok.type;

    // The first token must always be a command or a
    // separator.
    if (start == true) {
      switch (tok.type) {
        case STRING:
          start = false;
          break;
        case SEP:
          IS.Statements.len++;
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
      switch (prevTokType) {
        case STRING:
          switch (tok.type) {
            case STRING:
            case PIPE:
            case IN:
            case OUT:
            case APPEND:
              break;
            case SEP:
              IS.Statements.len++;
              start = true;
              break;
          }
          break;
        case PIPE:
          switch (tok.type) {
            case STRING:
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
          if (tok.type != STRING) {
            fprintf(
                stderr,
                "parse: < must be followed by a file\n");
            goto fail_parse;
          } else
            tok = TSNext();

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
          if (tok.type != STRING) {
            fprintf(stderr,
                    "parse: > or >> must be followed by a "
                    "file\n");
            goto fail_parse;
          } else
            tok = TSNext();

          switch (tok.type) {
            case SEP:
              IS.Statements.len++;
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
          IS.Statements.len++;
          start = true;
          break;
      }
    }

    prevTokType = tok.type;
  }

  IS.Statements.len++;

#ifdef DEBUG
  fprintf(stderr, "NUM_OF_STATEMENTS: %lu\n",
          IS.Statements.len);
#endif
  return 0;

fail_parse:
  // These two cleaning mechanisms are fine for this
  // particular stage. Especially cleanTokens since it frees
  // everything.
  cleanStatements();
  cleanTokens();
  return -1;
}

/* ------------------------------------------- */
/* Generate Statements */
/* ------------------------------------------- */

static inline int genStatements(void) {
  IS.Statements.arr =
      malloc(sizeof(Statement) * IS.Statements.len);

  if (IS.Statements.arr == NULL) {
    perror("getStatements");
    goto fail_genStatements;
  }

  size_t x = 0;  // Used to count pipes.
  size_t y = 0;  // Used to count strings.
  size_t z = 0;  // Used to populate pipelines.

  for (size_t i = 0; i < IS.Statements.len; i++) {
    /* Count the number of commands separated by pipes. */
    IS.Statements.arr[i].pipeline.len = 0;
    IS.Statements.arr[i].infile = NULL;
    IS.Statements.arr[i].outfile = NULL;
    IS.Statements.arr[i].append = false;

    while (x < IS.Tokens.len &&
           IS.Tokens.arr[x].type != SEP) {
      if (IS.Tokens.arr[x].type == PIPE)
        IS.Statements.arr[i].pipeline.len++;

      /* Populate infile, outfile and append properties. */
      if (IS.Tokens.arr[x].type == IN)
        IS.Statements.arr[i].infile =
            IS.Tokens.arr[++x].str;

      if (IS.Tokens.arr[x].type == OUT) {
        IS.Statements.arr[i].outfile =
            IS.Tokens.arr[++x].str;
        IS.Statements.arr[i].append = false;
      }

      if (IS.Tokens.arr[x].type == APPEND) {
        IS.Statements.arr[i].outfile =
            IS.Tokens.arr[++x].str;
        IS.Statements.arr[i].append = true;
      }

      x++;
    }

    x++;
    IS.Statements.arr[i].pipeline.len++;

    IS.Statements.arr[i].pipeline.arr =
        malloc(sizeof(char **) *
               (IS.Statements.arr[i].pipeline.len + 1));

    if (IS.Statements.arr[i].pipeline.arr == NULL) {
      perror("getStatements");
      goto fail_genStatements;
    }

    IS.Statements.arr[i]
        .pipeline.arr[IS.Statements.arr[i].pipeline.len] =
        NULL;
#ifdef DEBUG
    fprintf(stderr, "PIPELINE_SIZE: %lu\n",
            IS.Statements.arr[i].pipeline.len);
#endif

    /* Count the number of strings per command per pipe. */

    for (size_t j = 0;
         j < IS.Statements.arr[i].pipeline.len; j++) {
      size_t commandSize = 0;

      while (y < IS.Tokens.len &&
             IS.Tokens.arr[y].type != SEP) {
        if (IS.Tokens.arr[y].type == STRING) {
          commandSize++;
        } else if (IS.Tokens.arr[y].type == PIPE) {
          break;
        } else {
          while (y < IS.Tokens.len &&
                 IS.Tokens.arr[y].type != SEP)
            y++;

          break;
        }
        y++;
      }

      y++;

#ifdef DEBUG
      fprintf(stderr, "y: %lu\n", y);
      fprintf(stderr, "COMMAND_SIZE: %lu\n", commandSize);
#endif

      IS.Statements.arr[i].pipeline.arr[j] =
          malloc(sizeof(char *) * (commandSize + 1));

      if (IS.Statements.arr[i].pipeline.arr[j] == NULL) {
        perror("getStatements");
        goto fail_genStatements;
      }

      IS.Statements.arr[i].pipeline.arr[j][commandSize] =
          NULL;

      for (size_t k = 0; k < commandSize; k++) {
        IS.Statements.arr[i].pipeline.arr[j][k] =
            IS.Tokens.arr[z++].str;
      }

      z = y;

#ifdef DEBUG
      fprintf(stderr, "z: %lu\n", z);
#endif
    }
  }

#ifdef DEBUG
  printStatements();
#endif
  return 0;

fail_genStatements:
  cleanStatements();
  cleanTokens();
  return -1;
}

/* ------------------------------------------- */
/* Exposed Function */
/* ------------------------------------------- */

static inline int execute(void) {
  int ret = 0;

  for (size_t i = 0; i < IS.Statements.len; i++) {
    bool foundBuiltin = false;

#ifdef DEBUG
    fprintf(stderr, "NUM_OF_BUILTINS: %lu\n",
            get_num_of_builtins());
#endif

    // Check if the first command is a builtin.
    for (size_t j = 0; j < get_num_of_builtins(); j++) {
#ifdef DEBUG
      fprintf(stderr, "FIRST_COMMAND: %s\n",
              IS.Statements.arr[i].pipeline.arr[0][0]);
      fprintf(stderr, "BUILTIN_PTR: %p\n", &builtins[j]);
      fprintf(stderr, "BUILTIN: %s\n", builtins[j].name);
#endif
      if (!strcmp(IS.Statements.arr[i].pipeline.arr[0][0],

                  builtins[j].name)) {
#ifdef DEBUG
        fprintf(stderr, "VERIFY REACHED\n");
#endif
        foundBuiltin = true;
        ret = builtins[j].func(
            IS.Statements.arr[i].pipeline.arr[0]);

        if (ret == -1) {
          goto stop_execute;
        } else if (ret == EXIT_SHELL) {
          goto stop_execute;
        } else {
          ret = 0;
        }
      }
    }

    // Otherwise try to execute an external command.
    if (foundBuiltin == false) {
      ret =
          fork_exec_pipe(IS.Statements.arr[i].pipeline.arr,
                         0, IS.Statements.arr[i].infile,
                         IS.Statements.arr[i].outfile,
                         IS.Statements.arr[i].append);
      if (ret == -1) {
        goto stop_execute;
      } else if (ret == EXIT_SHELL) {
        goto stop_execute;
      } else {
        ret = 0;
      }
    }
  }

stop_execute:
  cleanStatements();
  cleanTokens();
  return ret;
}

/* ------------------------------------------- */
/* Exposed Function */
/* ------------------------------------------- */

int interpret(char *str) {
  if (tokenise(str) == -1)
    return -1;

  if (parse() == -1)
    return -1;

  if (genStatements() == -1) {
    return -1;
  }

  int ret = execute();

  if (ret == -1 || ret == EXIT_SHELL) {
    return ret;
  }

  return 0;
}

#undef INIT_SIZE
#undef GROW_SIZE
