#include "peek_stream.h"
#include "util.h"

void peek_stream_init(struct peek_stream *stream, char *str) {
  stream->str = str;
  stream->pos = 0;
  stream->len = strlen(str);
}

struct peek_stream *peek_stream_new(char *str) {
  struct peek_stream *stream = emalloc(sizeof(struct peek_stream));
  peek_stream_init(stream, str);
  return stream;
}

void peek_stream_shallow_free(struct peek_stream *stream) {
  if (stream != NULL) {
    free(stream);
  }
}

void peek_stream_deep_free(struct peek_stream *stream) {
  if (stream != NULL) {
    if (stream->str != NULL) {
      free(stream->str);
    }

    free(stream);
  }
}

/*-----------------------------------------------------------------*/

char peek_stream_peek(struct peek_stream *stream) {
  if (stream->pos < stream->len)
    return stream->str[stream->pos];

  return '\0';
}

char peek_stream_next(struct peek_stream *stream) {
  if (stream->pos < stream->len)
    return stream->str[stream->pos++];

  return '\0';
}

bool peek_stream_end(struct peek_stream *stream) {
  if (stream->pos < stream->len)
    return false;

  return true;
}
