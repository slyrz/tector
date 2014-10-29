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

#define inrange(v,i,j) (long long) \
  (((v) >= (i)) && ((v) < (j)))

struct neural_network *
neural_network_new (struct vocab *v, size_t layer, size_t window)
{
  struct neural_network *n;
  size_t a;
  size_t b;

  n = calloc (1, sizeof (struct neural_network));
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
  free (n->syn0);
  free (n->syn1);
  free (n);
}

static int
tainted_size (size_t vocab, size_t layers, size_t window)
{
  if (window > MAX_WINDOW)
    return 1;
  if (layers > MAX_LAYERS)
    return 1;
  if (vocab > limitofsize (layers * window * sizeof (float)))
    return 1;
  return 0;
}

int
neural_network_alloc (struct neural_network *n)
{
  const size_t s = n->size.vocab * n->size.layer;

  if (tainted_size (n->size.vocab, n->size.layer, n->size.window))
    return -1;

  freenull (n->syn0);
  freenull (n->syn1);

  if (posix_memalign ((void **) &n->syn0, 128, s * sizeof (float)))
    goto error;
  if (posix_memalign ((void **) &n->syn1, 128, s * sizeof (float)))
    goto error;

  clearspace (n->syn0, 0, sizeof (float), s);
  clearspace (n->syn1, 0, sizeof (float), s);
  return 0;
error:
  freenull (n->syn0);
  freenull (n->syn1);
  return -1;
}

static void
worker (struct neural_network *restrict n, struct sentence **restrict s, size_t k)
{
  const float alpha = 0.025f;

  const size_t sw = n->size.window;
  const size_t sl = n->size.layer;

  long long a, b, c, d, e;

  float f;
  float g;

  uint64_t code;
  uint32_t *point;

  size_t i;
  size_t j;
  size_t x;

  float *restrict neu1;
  float *restrict neu2;

  unsigned short rnd[3] = { 1, 2, 3 };

  neu1 = calloc (sl, sizeof (float));
  neu2 = calloc (sl, sizeof (float));

  for (i = 0; i < k; i++) {
    // TODO: adjust alpha
    // cbow
    for (j = 0; j < s[i]->len; j++) {
      b = (long long) nrand48 (rnd) % sw;
      d = 0;
      // in -> hidden
      for (a = b; a < sw * 2 + 1 - b; a++) {
        if (a == sw)
          continue;
        c = j + a - sw;
        if (inrange (c, 0, (long long) s[i]->len)) {
          x = s[i]->words[c] * sl;
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
        f = 0;
        for (c = 0; c < sl; c++)
          f += neu1[c] * n->syn1[c + e];
        f = expf (f);
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
          x = s[i]->words[c] * sl;
          for (c = 0; c < sl; c++)
            n->syn0[c + x] += neu2[c];
        }
      }
      memset (neu1, 0, sl * sizeof (float));
      memset (neu2, 0, sl * sizeof (float));
    }
  }
  free (neu1);
  free (neu2);
}

int
neural_network_train (struct neural_network *n, struct corpus *c)
{
  worker (n, c->sentences.ptr, c->sentences.len);
  return 0;
}
