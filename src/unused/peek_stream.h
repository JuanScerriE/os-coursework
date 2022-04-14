#pragma once

#include <stdbool.h>
#include <stdlib.h>

struct peek_stream {
  char *str;
  size_t pos;
  size_t len;
};

void peek_stream_init(struct peek_stream *stream,
                      char *str);
struct peek_stream *peek_stream_new(char *str);
void peek_stream_shallow_free(struct peek_stream *stream);
void peek_stream_deep_free(struct peek_stream *stream);

char peek_stream_peek(struct peek_stream *stream);
char peek_stream_next(struct peek_stream *stream);
bool peek_stream_end(struct peek_stream *stream);
