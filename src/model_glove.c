/**
 * This file incorporates derivative work covered by the
 * following copyright and permission notice:
 *
 * Copyright 2014 The Board of Trustees of The Leland Stanford Junior
 * University. All Rights Reserved.
 */
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "config.h"
#include "file.h"
#include "log.h"
#include "mem.h"
#include "model.h"
#include "macros.h"
#include "linalg.h"

struct glove {
  struct model base;
  float *cnt;
};

int glove_init (struct model *);
int glove_free (struct model *);
int glove_load (struct model *, struct file *);
int glove_save (struct model *, struct file *);
int glove_alloc (struct model *);
int glove_train (struct model *, struct corpus *);
int glove_generate (struct model *);

const struct model_interface interface_glove = {
  .size = sizeof (struct glove),
  .init = glove_init,
  .free = glove_free,
  .load = glove_load,
  .save = glove_save,
  .alloc = glove_alloc,
  .train = glove_train,
  .generate = glove_generate,
};

int
glove_init (struct model *base)
{
  (void) base;
  return 0;
}

int
glove_free (struct model *base)
{
  struct glove *m = (struct glove *) base;

  mem_free (m->cnt);
  return 0;
}

int
glove_load (struct model *base, struct file *f)
{
  struct glove *m = (struct glove *) base;

  return file_read (f, m->cnt, base->size.vocab * base->size.layer * sizeof (float));
}

int
glove_save (struct model *base, struct file *f)
{
  struct glove *m = (struct glove *) base;

  return file_write (f, m->cnt, base->size.vocab * base->size.layer * sizeof (float));
}

int
glove_alloc (struct model *base)
{
  struct glove *m = (struct glove *) base;

  m->cnt = mem_realloc (m->cnt, base->size.layer * base->size.vocab, sizeof (float));
  if (m->cnt == NULL)
    goto error;
  mem_clear (m->cnt, base->size.layer * base->size.vocab, sizeof (float));
  return 0;
error:
  mem_freenull (m->cnt);
  return -1;
}

static inline void
train (struct glove *restrict m, struct sentence *restrict s)
{
  const long long sw = (long long) m->base.size.window;
  const long long sl = (long long) m->base.size.layer;

  long long i;
  long long j;
  long long k;
  long long x;

  for (i = 0; i < (long long) s->len; i++) {
    x = (long long) s->words[i] * sl;
    for (j = 0; j < (long long) sw * 2 + 1; j++) {
      if (j == sw)
        continue;
      k = i + j - sw;
      if (inrange (k, 0, (long long) s->len))
        m->cnt[x + (m->base.v->entries[s->words[k]].hash % sl)] += 1.0 / fabs ((float) (k - i));
    }
  }
}

int
glove_train (struct model *base, struct corpus *c)
{
  struct glove *m = (struct glove *) base;
  size_t i;

  for (i = 0; i < c->sentences.len; i++) {
    if ((i & 0xfff) == 0)
      progress (i, c->sentences.len, "training");
    train (m, c->sentences.ptr[i]);
  }
  return 0;
}

int
glove_generate (struct model *base)
{
  struct glove *m = (struct glove *) base;

  const long long sv = (long long) m->base.size.vocab;
  const long long sl = (long long) m->base.size.layer;
  const long long se = (long long) m->base.size.vector;

  const float xmax = 100.0f;
  const float alpha = 0.75f;
  const float eta = 0.05f;

  float *vec0;
  float *vec1;
  float *grd0;
  float *grd1;
  float *bia0;
  float *bia1;

  float dd, df;
  float t0, t1;
  float cost;

  long long i;
  long long j;
  long long k;

  int r = 0;

  vec0 = mem_alloc ((size_t) (se * sv), sizeof (float));
  vec1 = mem_alloc ((size_t) (se * sl), sizeof (float));
  grd0 = mem_alloc ((size_t) (se * sv), sizeof (float));
  grd1 = mem_alloc ((size_t) (se * sl), sizeof (float));
  bia0 = mem_alloc ((size_t) (sv * 2), sizeof (float));
  bia1 = mem_alloc ((size_t) (sv * 2), sizeof (float));

  if (vec0 == NULL || vec1 == NULL)
    goto error;
  if (grd0 == NULL || grd1 == NULL)
    goto error;
  if (bia0 == NULL || bia1 == NULL)
    goto error;

  for (i = 0; i < se * sv; i++)
    vec0[i] = (float) (drand48 () - 0.5) / (float) se;
  for (i = 0; i < se * sl; i++)
    vec1[i] = (float) (drand48 () - 0.5) / (float) se;
  for (i = 0; i < se * sv; i++)
    grd0[i] = 1.0;
  for (i = 0; i < se * sl; i++)
    grd1[i] = 1.0;

  cost = 0.0;
  for (i = 0; i < sv; i++) {
    for (j = 0; j < sl; j++) {
      if (m->cnt[i * sl + j] == 0.0)
        continue;

      dd = 0.0;
      for (k = 0; k < se; k++)
        dd += vec0[i * se + k] * vec1[j * se + k];
      dd += bia0[2 * i] + bia1[2 * j] - logf (m->cnt[i * sl + j]);
      if (m->cnt[i * sl + j] > xmax)
        df = dd;
      else
        df = dd * powf (m->cnt[i * sl + j] / xmax, alpha);
      cost += (dd * df) / 2.0;

      df *= eta;
      for (k = 0; k < se; k++) {
        t0 = df * vec1[j * se + k];
        t1 = df * vec0[i * se + k];
        vec0[i * se + k] -= t0 / sqrtf (grd0[i]);
        vec1[j * se + k] -= t1 / sqrtf (grd1[j]);
        grd0[i] += t0 * t0;
        grd1[j] += t1 * t1;
      }
      bia0[2 * i + 0] -= df / sqrtf (grd0[i]);
      bia1[2 * j + 0] -= df / sqrtf (grd1[j]);
      bia0[2 * i + 1] += df * df;
      bia1[2 * j + 1] += df * df;
    }
  }

  base->embeddings = vec0;
  goto done;
error:
  r = -1;
  mem_free (vec0);
done:
  mem_free (vec1);
  mem_free (grd0);
  mem_free (grd1);
  mem_free (bia0);
  mem_free (bia1);
  return r;
}
