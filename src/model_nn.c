/**
 * This file incorporates derivative work covered by the
 * following copyright and permission notice:
 *
 * Copyright 2013 Google Inc. All Rights Reserved.
 */
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "config.h"
#include "exp.h"
#include "file.h"
#include "log.h"
#include "macros.h"
#include "mem.h"
#include "model.h"

struct nn {
  struct model base;
  float alpha;
  float *syn0;
  float *syn1;
  float *neu1;
  float *neu2;
};

int nn_init (struct model *);
int nn_free (struct model *);
int nn_load (struct model *, struct file *);
int nn_save (struct model *, struct file *);
int nn_alloc (struct model *);
int nn_train (struct model *, struct corpus *);
int nn_generate (struct model *);
int nn_verify (struct model *);

const struct model_interface interface_nn = {
  .size = sizeof (struct nn),
  .init = nn_init,
  .free = nn_free,
  .load = nn_load,
  .save = nn_save,
  .alloc = nn_alloc,
  .train = nn_train,
  .generate = nn_generate,
  .verify = nn_verify,
};

const float alpha = 0.05;

int
nn_init (struct model *base)
{
  base->size.layer = 64;
  base->size.vector = 64;
  base->size.window = 5;
  base->size.iter = 5;
  return 0;
}

int
nn_free (struct model *base)
{
  struct nn *m = (struct nn *) base;

  mem_free (m->syn0);
  mem_free (m->syn1);
  mem_free (m->neu1);
  mem_free (m->neu2);
  return 0;
}

int
nn_load (struct model *base, struct file *f)
{
  struct nn *m = (struct nn *) base;

  if (file_read (f, m->syn0, base->size.layer * base->size.vocab * sizeof (float)) != 0)
    goto error;
  if (file_read (f, m->syn1, base->size.layer * base->size.vocab * sizeof (float)) != 0)
    goto error;
  return 0;
error:
  return -1;
}

int
nn_save (struct model *base, struct file *f)
{
  struct nn *m = (struct nn *) base;

  if (file_write (f, m->syn0, base->size.layer * base->size.vocab * sizeof (float)) != 0)
    goto error;
  if (file_write (f, m->syn1, base->size.layer * base->size.vocab * sizeof (float)) != 0)
    goto error;
  return 0;
error:
  return -1;
}

int
nn_alloc (struct model *base)
{
  struct nn *m = (struct nn *) base;
  size_t i;

  mem_freenull (m->syn0);
  mem_freenull (m->syn1);
  mem_freenull (m->neu1);
  mem_freenull (m->neu2);

  m->syn0 = mem_alloc (base->size.layer * base->size.vocab, sizeof (float));
  m->syn1 = mem_alloc (base->size.layer * base->size.vocab, sizeof (float));
  if ((m->syn0 == NULL) || (m->syn1 == NULL))
    goto error;

  m->neu1 = mem_alloc (base->size.layer, sizeof (float));
  m->neu2 = mem_alloc (base->size.layer, sizeof (float));
  if ((m->neu1 == NULL) || (m->neu2 == NULL))
    goto error;

  for (i = 0; i < base->size.layer * base->size.vocab; i++)
    m->syn0[i] = (float) (drand48 () - 0.5) / (float) base->size.layer;
  return 0;
error:
  mem_freenull (m->syn0);
  mem_freenull (m->syn1);
  mem_freenull (m->neu1);
  mem_freenull (m->neu2);
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
hierarchical_softmax (struct nn *restrict m, struct vocab_entry *restrict e)
{
  const long long sl = (long long) m->base.size.layer;

  long long i, j;
  float f, g;

  uint64_t code = e->code;
  int32_t *point = e->point;

  while (code > 1) {
    j = point[0] * sl;
    f = 0.0f;
    for (i = 0; i < sl; i++)
      f += m->neu1[i] * m->syn1[i + j];
    f = exptabf (f);
    if (f >= 0.0f) {
      g = (1.0f - (float) (code & 1) - f) * m->alpha;
      for (i = 0; i < sl; i++)
        m->neu2[i] += g * m->syn1[i + j];
      for (i = 0; i < sl; i++)
        m->syn1[i + j] += g * m->neu1[i];
    }
    code >>= 1;
    point++;
  }
}

static inline void
train_bag_of_words (struct nn *restrict m, struct sentence *restrict s)
{
  const long long sw = (long long) m->base.size.window;
  const long long sl = (long long) m->base.size.layer;

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
          m->neu1[c] += m->syn0[e + c];
        d++;
      }
    }
    if (d == 0)
      continue;
    for (c = 0; c < sl; c++)
      m->neu1[c] /= (float) d;
    hierarchical_softmax (m, m->base.v->entries + s->words[i]);
    // hidden -> in
    for (a = b; a < sw * 2 + 1 - b; a++) {
      if (a == sw)
        continue;
      c = i + a - sw;
      if (inrange (c, 0, (long long) s->len)) {
        e = (long long) s->words[c] * sl;
        for (c = 0; c < sl; c++)
          m->syn0[c + e] += m->neu2[c];
      }
    }
    mem_clear (m->neu1, (size_t) sl, sizeof (float));
    mem_clear (m->neu2, (size_t) sl, sizeof (float));
  }
}

static inline void
alpha_decay (struct nn *restrict m, size_t i, size_t n)
{
  m->alpha = max (alpha * (1 - ((float) i / (float) (n + 1))), alpha * 0.0001);
}

int
nn_train (struct model *base, struct corpus *c)
{
  struct nn *m = (struct nn *) base;

  struct sentence *s;
  size_t i;
  size_t j;

  s = mem_alloc (512, sizeof (size_t));
  if (s == NULL)
    return -1;

  m->alpha = alpha;
  for (i = 0; i < base->size.iter; i++) {
    for (j = 0; j < c->sentences.len; j++) {
      if ((j & 0xfff) == 0) {
        progress (j, c->sentences.len, "train %zu/%zu", i + 1, base->size.iter);
        alpha_decay (m, i * c->sentences.len + j, base->size.iter * c->sentences.len);
      }
      if (subsample (s, c->sentences.ptr[j], 511) == 0)
        continue;
      train_bag_of_words (m, s);
    }
  }
  mem_free (s);
  return 0;
}

int
nn_generate (struct model *base)
{
  struct nn *m = (struct nn *) base;

  base->embeddings = m->syn0;
  return 0;
}

int
nn_verify (struct model *base)
{
  base->size.iter = max (base->size.iter, 1);
  if (base->size.iter < 5)
    warning ("consider using >= 5 iterations");

  /**
   * Vector and layer size should be the same. If they have different values,
   * use the largest for both.
   */
  base->size.vector = max (base->size.layer, base->size.vector);
  base->size.layer = base->size.vector;
  return 0;
}
