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
    (c)->f.ptr = mem_realloc ((c)->f.ptr, (c)->f.cap, sizeof ((c)->f.ptr[0])); \
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

  c = mem_alloc (1, sizeof (struct corpus));
  if (c == NULL)
    goto error;
  c->vocab = v;
  if (corpus_alloc (c) != 0)
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
  mem_free (c->sentences.ptr);
  mem_free (c->words.ptr);
  mem_free (c);
}

int
corpus_build (struct corpus *c)
{
  size_t i;
  size_t j;

  for (i = j = 0; i < c->sentences.len; i += 1, j += 1 + c->words.ptr[j]) {
    if (j >= c->words.len)
      return -1;
    c->sentences.ptr[i] = (struct sentence *) &c->words.ptr[j];
  }
  return 0;
}

int
corpus_alloc (struct corpus *c)
{
  const size_t s = reqcap (c->sentences.len, c->sentences.cap, 1024);
  const size_t w = reqcap (c->words.len, c->words.cap, 32768);

  resize (c, words, w);
  resize (c, sentences, s);
  return 0;
}

static int
resize_words (struct corpus *c, size_t cap)
{
  resize (c, words, cap);
  if (corpus_build (c) != 0)
    return -1;
  return 0;
}

static int
resize_sentences (struct corpus *c, size_t cap)
{
  resize (c, sentences, cap);
  return 0;
}

static int
add_sentence (struct corpus *c, char *s)
{
  size_t n = 0;
  size_t x = 0;
  char *w;

  /**
   * The order of these appends is crucial - the other way around might produce
   * a dangling pointer if words get resized.
   */
  append (c, words, 0);
  append (c, sentences, (struct sentence *) c->words.ptr + c->words.len - 1);

  while (w = strtok_r (s, " ", &s), w) {
    if (vocab_find (c->vocab, w, &x) != 0)
      continue;
    append (c, words, x);
    n++;
  }
  if (n <= 1) {
    c->words.len -= 1 + n;
    c->sentences.len--;
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
