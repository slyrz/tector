/**
 * This file incorporates derivative work covered by the
 * following copyright and permission notice:
 *
 * Copyright 2013 Google Inc. All Rights Reserved.
 */
#include "neural_network.h"
#include "exp.h"
#include "mem.h"
#include "log.h"

#include <string.h>
#include <stdint.h>
#include <math.h>

#define entry(v,s,p) \
  ((v)->entries[(s)->words[p]])

#define inrange(v,i,j) \
  (((v) >= (i)) && ((v) < (j)))

struct neural_network *
neural_network_new (struct vocab *v, size_t layer, size_t window)
{
  struct neural_network *n;

  n = mem_alloc (1, sizeof (struct neural_network));
  if (n == NULL)
    return NULL;

  n->v = v;
  n->size.vocab = v->len;
  n->size.layer = layer;
  n->size.window = window;
  n->alpha = 0.025f;
  if (neural_network_alloc (n) != 0)
    goto error;
  return n;
error:
  if (n)
    neural_network_free (n);
  return NULL;
}

void
neural_network_free (struct neural_network *n)
{
  mem_free (n->syn0);
  mem_free (n->syn1);
  mem_free (n->neu1);
  mem_free (n->neu2);
  mem_free (n);
}

int
neural_network_alloc (struct neural_network *n)
{
  size_t i;

  if ((n->size.window > MAX_WINDOW) || (n->size.layer > MAX_LAYERS))
    return -1;

  mem_free (n->syn0);
  mem_free (n->syn1);
  mem_free (n->neu1);
  mem_free (n->neu2);

  n->syn0 = mem_alloc (n->size.layer * n->size.vocab, sizeof (float));
  n->syn1 = mem_alloc (n->size.layer * n->size.vocab, sizeof (float));
  if ((n->syn0 == NULL) || (n->syn1 == NULL))
    goto error;

  n->neu1 = mem_alloc (n->size.layer, sizeof (float));
  n->neu2 = mem_alloc (n->size.layer, sizeof (float));
  if ((n->neu1 == NULL) || (n->neu2 == NULL))
    goto error;

  for (i = 0; i < n->size.layer * n->size.vocab; i++)
    n->syn0[i] = (float) (drand48 () - 0.5) / (float) n->size.layer;
  return 0;
error:
  mem_freenull (n->syn0);
  mem_freenull (n->syn1);
  mem_freenull (n->neu1);
  mem_freenull (n->neu2);
  return -1;
}

static inline size_t
subsample (struct sentence *dst, const struct sentence *src, size_t n)
{
  size_t i;

  /**
   * The sampling is loosely-based on Zipf's law, which states that the
   * probability of encountering a word is given by the word's rank alone:
   *
   *    P(r) = r^a
   *
   * where a is a number close to -1. So that's why this sampling method
   * solely works on the vocabulary indexes, and doesn't look up
   * the word counts stored in the entries.
   *
   * Here the square root is used to slow the decay down; the + 2 is used
   * to let the zero-based numbering start at sqrt(2); and the numerator
   * makes sure that the highest ranked word has a probability of 0.5 to
   * be drawn.
   */
  dst->len = 0;
  for (i = 0; i < src->len; i++) {
    if (dst->len >= n)
      break;
    if (drand48 () > (0.7071067811865476 / sqrt ((double) (src->words[i] + 2))))
      dst->words[dst->len++] = src->words[i];
  }
  return dst->len;
}

static inline void
hierarchical_softmax (struct neural_network *restrict n, struct vocab_entry *restrict e)
{
  const long long sl = (long long) n->size.layer;

  long long i, j;
  float f, g;

  uint64_t code = e->code;
  int32_t *point = e->point;

  while (code > 1) {
    j = point[0] * sl;
    f = 0.0f;
    for (i = 0; i < sl; i++)
      f += n->neu1[i] * n->syn1[i + j];
    f = exptabf (f);
    if (f >= 0.0f) {
      g = (1.0f - (float) (code & 1) - f) * n->alpha;
      for (i = 0; i < sl; i++)
        n->neu2[i] += g * n->syn1[i + j];
      for (i = 0; i < sl; i++)
        n->syn1[i + j] += g * n->neu1[i];
    }
    code >>= 1;
    point++;
  }
}

static inline void
train_bag_of_words (struct neural_network *restrict n, struct sentence *restrict s)
{
  const long long sw = (long long) n->size.window;
  const long long sl = (long long) n->size.layer;

  long long a, b, c, d, e, i;

  for (i = 0; i < (long long) s->len; i++) {
    b = (long long) lrand48 () % sw;
    d = 0;
    // in -> hidden
    for (a = b; a < (long long) sw * 2 + 1 - b; a++) {
      if (a == sw)
        continue;
      c = i + a - sw;
      if (inrange (c, 0, (long long) s->len)) {
        e = (long long) s->words[c] * sl;
        for (c = 0; c < sl; c++)
          n->neu1[c] += n->syn0[e + c];
        d++;
      }
    }
    if (d == 0)
      continue;
    for (c = 0; c < sl; c++)
      n->neu1[c] /= (float) d;
    hierarchical_softmax (n, n->v->entries + s->words[i]);
    // hidden -> in
    for (a = b; a < sw * 2 + 1 - b; a++) {
      if (a == sw)
        continue;
      c = i + a - sw;
      if (inrange (c, 0, (long long) s->len)) {
        e = (long long) s->words[c] * sl;
        for (c = 0; c < sl; c++)
          n->syn0[c + e] += n->neu2[c];
      }
    }
    mem_clear (n->neu1, (size_t) sl, sizeof (float));
    mem_clear (n->neu2, (size_t) sl, sizeof (float));
  }
}

int
neural_network_train (struct neural_network *n, struct corpus *c)
{
  struct sentence *s;
  size_t i;

  s = mem_alloc (512, sizeof (size_t));
  for (i = 0; i < c->sentences.len; i++) {
    if ((i & 0xfff) == 0)
      progress (i, c->sentences.len, "training");
    if (subsample (s, c->sentences.ptr[i], 511) == 0)
      continue;
    train_bag_of_words (n, s);
  }
  mem_free (s);
  return 0;
}
