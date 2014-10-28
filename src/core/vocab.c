/**
 * This file incorporates derivative work covered by the
 * following copyright and permission notice:
 *
 * Copyright 2013 Google Inc. All Rights Reserved.
 */
#include "config.h"
#include "vocab.h"
#include "serialize.h"
#include "log.h"
#include "mem.h"

#include <string.h>

static inline uint32_t
hash (const unsigned char *restrict w)
{
  uint32_t h = 0x811c9dc5;
  while (*w)
    h = (h ^ *w++) * 0x1000193;
  return h;
}

struct vocab *
vocab_new (void)
{
  struct vocab *v;

  v = calloc (1, sizeof (struct vocab));
  if (v == NULL)
    goto error;
  v->len = 0;
  v->cap = 32768;
  v->pool = calloc (v->cap, sizeof (struct vocab_entry));
  if (v->pool == NULL)
    goto error;
  v->table = calloc (v->cap, sizeof (struct vocab_entry *));
  if (v->table == NULL)
    goto error;
  return v;
error:
  if (v)
    vocab_free (v);
  return NULL;
}

struct vocab *
vocab_new_from_path (const char *path)
{
  struct vocab *v;

  v = vocab_new ();
  if (v)
    vocab_load (v, path);
  return v;
}

void
vocab_free (struct vocab *v)
{
  free (v->pool);
  free (v->table);
  free (v);
}

static inline float
load_factor (const struct vocab *v)
{
  return (float) v->len / (float) v->cap;
}

void
vocab_rebuild (struct vocab *v)
{
  size_t i;
  size_t j;

  clearspace (v->table, 0, v->cap, sizeof (struct vocab_entry *));
  for (i = 0; i < v->len; i++) {
    j = v->pool[i].hash;
    for (;;) {
      if (v->table[j % v->cap] == NULL)
        break;
      j++;
    }
    v->table[j % v->cap] = &v->pool[i];
  }
}

void
vocab_grow (struct vocab *v, size_t cap)
{
  cap = sizepow2 (cap);
  if (cap < v->cap)
    return;

  v->cap = cap;
  v->pool = reallocarray (v->pool, v->cap, sizeof (struct vocab_entry));
  if (v->pool == NULL)
    fatal ("reallocarray (v->pool)");

  v->table = reallocarray (v->table, v->cap, sizeof (struct vocab_entry *));
  if (v->table == NULL)
    fatal ("reallocarray (v->table)");

  clearspace (v->pool, v->len, v->cap, sizeof (struct vocab_entry));
  vocab_rebuild (v);
}

static int
cmp (const void *a, const void *b)
{
  const struct vocab_entry *x = (const struct vocab_entry *) a;
  const struct vocab_entry *y = (const struct vocab_entry *) b;

  /**
   * Sorting needs to happen in reversed order (from highest to lowest), so that the
   * lowest entries reside at the end of the array and can be stripped off by
   * decrementing length.
   */
  return (int) y->count - (int) x->count;
}

void
vocab_shrink (struct vocab *v)
{
  qsort (v->pool, v->len, sizeof (struct vocab_entry), cmp);
  while (v->len > 0) {
    if (v->pool[v->len - 1].count > 1)
      break;
    v->len--;
  }
  vocab_rebuild (v);
}

static inline size_t
find (struct vocab *v, uint32_t h, const char *w)
{
  struct vocab_entry *entry;
  size_t i;

  i = h % v->cap;
  for (;;) {
    entry = v->table[i];
    if ((entry == NULL)
        || ((entry->hash == h) && (strcmp (entry->data, w) == 0)))
      break;
    i = (i + 1) % v->cap;
  }
  return i % v->cap;
}

void
vocab_add (struct vocab *v, const char *w)
{
  struct vocab_entry *entry;

  uint32_t h = hash (w);
  size_t i = find (v, h, w);

  if (v->table[i]) {
    v->table[i]->count++;
    return;
  }

  if (load_factor (v) > 0.7f)
    vocab_grow (v, v->cap << 2);

  entry = v->pool + v->len;
  entry->hash = h;
  entry->count = 1;
  entry->code = 0;
  strncpy (entry->data, w, MAX_WORD_LENGTH);
  entry->data[MAX_WORD_LENGTH - 1] = 0;

  v->table[i] = v->pool + v->len;
  v->len++;
}

struct vocab_entry *
vocab_get (struct vocab *v, const char *w)
{
  uint32_t h = hash (w);
  size_t i = find (v, h, w);
  return v->table[i];
}

int
vocab_get_index (struct vocab *v, const char *w, size_t * i)
{
  struct vocab_entry *entry;

  entry = vocab_get (v, w);
  if (entry == NULL)
    return -1;
  *i = entry - v->pool;
  return 0;
}

void
vocab_encode (struct vocab *v)
{
  struct vocab_entry *entry;

  long long point[MAX_CODE_LENGTH];
  long long code;

  size_t a, b, i;
  long long m1, m2;
  long long p1, p2;

  long long *count;
  long long *binary;
  long long *parent;

  if (v->len == 0)
    return;

  count = calloc (v->len * 2 + 1, sizeof (long long));
  binary = calloc (v->len * 2 + 1, sizeof (long long));
  parent = calloc (v->len * 2 + 1, sizeof (long long));

  for (a = 0; a < v->len; a++)
    count[a] = v->pool[a].count;

  for (a = v->len; a < v->len * 2; a++)
    count[a] = 1e15;

  p1 = v->len - 1;
  p2 = v->len;
  for (a = 0; a < v->len - 1; a++) {
    if (p1 >= 0) {
      if (count[p1] < count[p2]) {
        m1 = p1;
        p1--;
      }
      else {
        m1 = p2;
        p2++;
      }
    }
    else {
      m1 = p2;
      p2++;
    }
    if (p1 >= 0) {
      if (count[p1] < count[p2]) {
        m2 = p1;
        p1--;
      }
      else {
        m2 = p2;
        p2++;
      }
    }
    else {
      m2 = p2;
      p2++;
    }
    count[v->len + a] = count[m1] + count[m2];
    parent[m1] = v->len + a;
    parent[m2] = v->len + a;
    binary[m2] = 1;
  }

  entry = v->pool;
  for (a = 0; a < v->len; a++) {
    code = 0ll;
    for (b = a, i = 0; b != (v->len * 2 - 2); b = parent[b], i++) {
      code |= binary[b] << i;
      point[i] = b;
    }

    entry->point[0] = v->len - 2;
    entry->code = 0ll;
    for (b = 0; b < i; b++) {
      entry->code |= (code & 1) << (i - b - 1);
      entry->point[i - b] = point[b] - v->len;
      code >>= 1;
    }
    entry++;
  }

  free (count);
  free (binary);
  free (parent);
}
