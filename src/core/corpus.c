#include "corpus.h"
#include "scanner.h"
#include "filter.h"

#include <err.h>
#include <string.h>
#include <assert.h>

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

static void
corpus_rebuild_sentences (struct corpus *c)
{
  size_t i;
  size_t j;

  for (i = j = 0; i < c->sentences.len; i += 1, j += 1 + c->words.ptr[j])
    c->sentences.ptr[i] = (struct sentence *) &c->words.ptr[j];
}

#define corpus_resize(b,n,s) \
  do { \
    if ((n) < (b).cap) \
      err (EXIT_FAILURE, __func__); \
    (b).cap = (n); \
    (b).ptr = realloc ((b).ptr, (b).cap * (s)); \
    if ((b).ptr == NULL) \
      err (EXIT_FAILURE, __func__); \
  } while (0)

static void
corpus_grow_words (struct corpus *c)
{
  corpus_resize (c->words, c->words.cap << 2, sizeof (size_t));
  corpus_rebuild_sentences (c);
}

static void
corpus_grow_sentences (struct corpus *c)
{
  corpus_resize (c->sentences, c->sentences.cap << 2, sizeof (struct sentence *));
}

static void
corpus_write_word (struct corpus *c, size_t w)
{
  if (c->words.len >= c->words.cap)
    corpus_grow_words (c);
  c->words.ptr[c->words.len++] = w;
}

static void
corpus_write_sentence (struct corpus *c, struct sentence *s)
{
  if (c->sentences.len >= c->sentences.cap)
    corpus_grow_sentences (c);
  c->sentences.ptr[c->sentences.len++] = s;
}

static void
corpus_add_sentence (struct corpus *c, char *s)
{
  size_t n = 0;
  size_t x = 0;
  char *w;

  corpus_write_sentence (c, (struct sentence *) c->words.ptr + c->words.len);
  corpus_write_word (c, 0);

  while (w = strtok_r (s, " ", &s), w) {
    if (vocab_get_index (c->vocab, w, &x) != 0)
      continue;
    corpus_write_word (c, x);
    n++;
  }
  if (n == 0) {
    c->sentences.len--;
    c->words.len--;
  }
  else {
    c->sentences.ptr[c->sentences.len - 1]->len = n;
  }
}

void
corpus_load (struct corpus *c, const char *path)
{
  struct scanner *s;
  char b[8192] = { 0 };

  s = scanner_new (path);
  while (scanner_readline (s, b, sizeof (b)) >= 0) {
    if (*b) {
      filter (b);
      corpus_add_sentence (c, b);
    }
  }
  scanner_free (s);
}
