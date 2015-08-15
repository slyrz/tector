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

struct svd {
  struct model base;
  float *cnt;
};

static int svd_init (struct model *);
static int svd_free (struct model *);
static int svd_load (struct model *, struct file *);
static int svd_save (struct model *, struct file *);
static int svd_alloc (struct model *);
static int svd_train (struct model *, struct corpus *);
static int svd_generate (struct model *);
static int svd_verify (struct model *);

const struct model_interface interface_svd = {
  .size = sizeof (struct svd),
  .init = svd_init,
  .free = svd_free,
  .load = svd_load,
  .save = svd_save,
  .alloc = svd_alloc,
  .train = svd_train,
  .generate = svd_generate,
  .verify = svd_verify,
};

static int
svd_init (struct model *base)
{
  base->size.vector = 64;
  base->size.layer = 320;
  base->size.window = 5;
  return 0;
}

static int
svd_free (struct model *base)
{
  struct svd *m = (struct svd *) base;

  mem_free (m->cnt);
  return 0;
}

static int
svd_load (struct model *base, struct file *f)
{
  struct svd *m = (struct svd *) base;

  return file_read (f, m->cnt, base->size.vocab * base->size.layer * sizeof (float));
}

static int
svd_save (struct model *base, struct file *f)
{
  struct svd *m = (struct svd *) base;

  return file_write (f, m->cnt, base->size.vocab * base->size.layer * sizeof (float));
}

static int
svd_alloc (struct model *base)
{
  struct svd *m = (struct svd *) base;

  m->cnt = mem_realloc (m->cnt, base->size.layer * base->size.vocab, sizeof (float));
  if (m->cnt == NULL)
    goto error;
  mem_clear (m->cnt, base->size.layer * base->size.vocab, sizeof (float));
  return 0;
error:
  mem_free (m->cnt);
  return -1;
}

static inline void
train (struct svd *restrict m, struct sentence *restrict s)
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

static int
svd_train (struct model *base, struct corpus *c)
{
  struct svd *m = (struct svd *) base;
  size_t i;

  for (i = 0; i < c->sentences.len; i++) {
    if ((i & 0xfff) == 0)
      progress (i, c->sentences.len, "training");
    train (m, c->sentences.ptr[i]);
  }
  return 0;
}

static int
svd_generate (struct model *base)
{
  struct svd *m = (struct svd *) base;

  const size_t sv = m->base.size.vocab;
  const size_t sl = m->base.size.layer;
  const size_t se = m->base.size.vector;
  size_t i;

  float *p;
  float *q;

  p = mem_alloc (sv * sl, sizeof (float));
  if (p == NULL)
    goto error;
  for (i = 0; i < sv * sl; i++)
    p[i] = sqrtf (m->cnt[i]);
  svd_topk (p, sv, sl, se, &q);
  if (q == NULL)
    goto error;
  if (base->embeddings)
    mem_free (base->embeddings);
  base->embeddings = q;
  mem_free (p);
  return 0;
error:
  mem_free (p);
  return -1;
}

static int
svd_verify (struct model *base)
{
  /**
   * The layer size must be twice as large as the vector size, but
   * should be larger.
   */
  base->size.layer = max (base->size.layer, 2 * base->size.vector);
  if (base->size.layer == 2 * base->size.vector)
    warning ("consider using a larger layer size (>= %zu)", 4 * base->size.vector);
  return 0;
}
