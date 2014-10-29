#include "corpus.h"
#include "scanner.h"
#include "filter.h"
#include "log.h"
#include "mem.h"

#include <string.h>

#define resize(c,f,s) \
  do { \
    if ((s) < (c)->f.cap) \
      return -1; \
    (c)->f.cap = (s); \
    (c)->f.ptr = reallocarray ((c)->f.ptr, (c)->f.cap, sizeof ((c)->f.ptr[0])); \
    if ((c)->f.ptr == NULL) \
      return -1; \
  } while (0)

#define append(c,f,v) \
  do { \
    if ((c)->f.len >= (c)->f.cap) \
      if (resize_ ## f (c, (c)->f.cap << 2) != 0) \
        return -1; \
    (c)->f.ptr[(c)->f.len++] = v; \
  } while (0)

struct corpus *
corpus_new (struct vocab *v)
{
  struct corpus *c;

  c = calloc (1, sizeof (struct corpus));
  if (c == NULL)
    goto error;
  c->vocab = v;
  c->words.cap = 32768;
  c->words.ptr = calloc (c->words.cap, sizeof (size_t));
  if (c->words.ptr == NULL)
    goto error;
  c->sentences.cap = 1024;
  c->sentences.ptr = calloc (c->sentences.cap, sizeof (struct sentence *));
  if (c->sentences.ptr == NULL)
    goto error;
  return c;
error:
  if (c)
    corpus_free (c);
  return NULL;
}

void
corpus_free (struct corpus *c)
{
  free (c->sentences.ptr);
  free (c->words.ptr);
  free (c);
}

int
corpus_rebuild (struct corpus *c)
{
  size_t i;
  size_t j;

  for (i = j = 0; i < c->sentences.len; i += 1, j += 1 + c->words.ptr[j])
    c->sentences.ptr[i] = (struct sentence *) &c->words.ptr[j];
  return 0;
}

static int
resize_words (struct corpus *c, size_t cap)
{
  resize (c, words, cap);
  if (corpus_rebuild (c) != 0)
    return -1;
  return 0;
}

static int
resize_sentences (struct corpus *c, size_t cap)
{
  resize (c, sentences, cap);
  return 0;
}

int
corpus_grow (struct corpus *c, size_t w, size_t s)
{
  int r = 0;
  r |= resize_sentences (c, sizepow2 (s));
  r |= resize_words (c, sizepow2 (w));
  return r;
}

static int
add_sentence (struct corpus *c, char *s)
{
  size_t n = 0;
  size_t x = 0;
  char *w;

  append (c, sentences, (struct sentence *) c->words.ptr + c->words.len);
  append (c, words, 0);

  while (w = strtok_r (s, " ", &s), w) {
    if (vocab_get_index (c->vocab, w, &x) != 0)
      continue;
    append (c, words, x);
    n++;
  }
  if (n == 0) {
    c->sentences.len--;
    c->words.len--;
  }
  else {
    c->sentences.ptr[c->sentences.len - 1]->len = n;
  }
  return 0;
}

int
corpus_parse (struct corpus *c, const char *path)
{
  struct scanner *s;
  char b[8192] = { 0 };

  s = scanner_new (path);
  if (s == NULL)
    return -1;

  while (scanner_readline (s, b, sizeof (b)) >= 0) {
    if (*b)
      if (add_sentence (c, filter (b)) != 0)
        return -1;
  }
  scanner_free (s);
  return 0;
}
