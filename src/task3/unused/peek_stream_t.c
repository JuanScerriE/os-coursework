#include "peek_stream_t.h"

#include "util.h"

static inline peek_stream_t peek_stream_init(char *arr) {
  return (peek_stream_t){arr, 0, strlen(arr)};
}

/* ----------------------------------------------- */

char peek_stream_peek(peek_stream_t *stream) {
  if (stream != NULL && stream->pos < stream->len)
    return stream->arr[stream->pos];

  return '\0';
}

char peek_stream_next(peek_stream_t *stream) {
  if (stream != NULL && stream->pos < stream->len)
    return stream->arr[stream->pos++];

  return '\0';
}

int peek_stream_end(peek_stream_t *stream) {
  if (stream != NULL && stream->pos < stream->len)
    return 1;

  return 0;
}
