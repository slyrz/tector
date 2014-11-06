/**
 * This file incorporates derivative work covered by the
 * following copyright and permission notice:
 *
 * Copyright 2013 Google Inc. All Rights Reserved.
 */
#include "neural_network.h"
#include "exp.h"
#include "mem.h"

#include <string.h>
#include <stdint.h>

#define entry(v,s,i,j) \
  ((v)->pool[(s)[i]->words[j]])

#define inrange(v,i,j) \
  (((v) >= (i)) && ((v) < (j)))

struct neural_network *
neural_network_new (struct vocab *v, size_t layer, size_t window)
{
  struct neural_network *n;
  size_t a;
  size_t b;

  n = mem_alloc (1, sizeof (struct neural_network));
  if (n == NULL)
    return NULL;

  n->v = v;
  n->size.vocab = v->len;
  n->size.layer = layer;
  n->size.window = window;

  if (neural_network_alloc (n) != 0)
    goto error;

  for (a = 0; a < n->size.vocab; a++)
    for (b = 0; b < n->size.layer; b++)
      n->syn0[a * n->size.layer + b] = (float) (drand48 () - 0.5) / (float) n->size.layer;

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
  mem_free (n);
}

int
neural_network_alloc (struct neural_network *n)
{
  const size_t s = n->size.vocab * n->size.layer;

  if ((n->size.window > MAX_WINDOW) || (n->size.layer > MAX_LAYERS))
    return -1;

  mem_free (n->syn0);
  mem_free (n->syn1);

  n->syn0 = mem_align (s, sizeof (float), 128);
  n->syn1 = mem_align (s, sizeof (float), 128);
  if ((n->syn0 == NULL) || (n->syn1 == NULL))
    goto error;

  mem_clear (n->syn0, s, sizeof (float));
  mem_clear (n->syn1, s, sizeof (float));
  return 0;
error:
  mem_freenull (n->syn0);
  mem_freenull (n->syn1);
  return -1;
}

static void
worker (struct neural_network *restrict n, struct sentence **restrict s, size_t k)
{
  const float alpha = 0.025f;

  const long long sw = (long long) n->size.window;
  const long long sl = (long long) n->size.layer;

  long long a, b, c, d, e;

  float f;
  float g;

  uint64_t code;
  int32_t *point;

  long long i;
  long long j;
  long long x;

  float *neu1;
  float *neu2;

  unsigned short rnd[3] = { 1, 2, 3 };

  neu1 = mem_alloc ((size_t) sl, sizeof (float));
  neu2 = mem_alloc ((size_t) sl, sizeof (float));

  for (i = 0; i < (long long) k; i++) {
    // TODO: adjust alpha
    // cbow
    for (j = 0; j < (long long) s[i]->len; j++) {
      b = (long long) nrand48 (rnd) % sw;
      d = 0;
      // in -> hidden
      for (a = b; a < (long long) sw * 2 + 1 - b; a++) {
        if (a == sw)
          continue;
        c = j + a - sw;
        if (inrange (c, 0, (long long) s[i]->len)) {
          x = (long long) s[i]->words[c] * sl;
          for (c = 0; c < sl; c++)
            neu1[c] += n->syn0[x + c];
          d++;
        }
      }
      if (d == 0)
        continue;
      for (c = 0; c < sl; c++)
        neu1[c] /= (float) d;
      // hs
      code = entry (n->v, s, i, j).code;
      point = entry (n->v, s, i, j).point;
      while (code) {
        e = point[0] * sl;
        f = 0.0f;
        for (c = 0; c < sl; c++)
          f += neu1[c] * n->syn1[c + e];
        f = exptabf (f);
        if (f >= 0.0f) {
          g = (1.0f - (float) (code & 1) - f) * alpha;
          for (c = 0; c < sl; c++)
            neu2[c] += g * n->syn1[c + e];
          for (c = 0; c < sl; c++)
            n->syn1[c + e] += g * neu1[c];
        }
        code >>= 1;
        point++;
      }
      // hidden -> in
      for (a = b; a < sw * 2 + 1 - b; a++) {
        if (a == sw)
          continue;
        c = j + a - sw;
        if (inrange (c, 0, (long long) s[i]->len)) {
          x = (long long) s[i]->words[c] * sl;
          for (c = 0; c < sl; c++)
            n->syn0[c + x] += neu2[c];
        }
      }
      mem_clear (neu1, (size_t) sl, sizeof (float));
      mem_clear (neu2, (size_t) sl, sizeof (float));
    }
  }
  mem_free (neu1);
  mem_free (neu2);
}

int
neural_network_train (struct neural_network *n, struct corpus *c)
{
  worker (n, c->sentences.ptr, c->sentences.len);
  return 0;
}
