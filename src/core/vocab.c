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
hash (const char *restrict s)
{
  const unsigned char *w = (const unsigned char *) s;
  uint32_t h = 0x811c9dc5;
  while (*w)
    h = (h ^ *w++) * 0x1000193;
  return h;
}

struct vocab *
vocab_new (void)
{
  struct vocab *v;

  v = mem_alloc (1, sizeof (struct vocab));
  if (v == NULL)
    goto error;
  if (vocab_alloc (v) != 0)
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
  mem_free (v->pool);
  mem_free (v->table);
  mem_free (v);
}

int
vocab_alloc (struct vocab *v)
{
  v->cap = reqcap (v->len, v->cap, 32768);
  v->pool = mem_realloc (v->pool, v->cap, sizeof (struct vocab_entry));
  if (v->pool == NULL)
    return -1;
  v->table = mem_realloc (v->table, v->cap, sizeof (struct vocab_entry *));
  if (v->table == NULL)
    return -1;
  mem_clear (v->pool + v->len, v->cap - v->len, sizeof (struct vocab_entry));
  mem_clear (v->table, v->cap, sizeof (struct vocab_entry *));
  return 0;
}

int
vocab_build (struct vocab *v)
{
  size_t i;
  size_t j;

  mem_clear (v->table, v->cap, sizeof (struct vocab_entry *));
  for (i = 0; i < v->len; i++) {
    j = v->pool[i].hash;
    for (;;) {
      if (v->table[j % v->cap] == NULL)
        break;
      j++;
    }
    v->table[j % v->cap] = &v->pool[i];
  }
  return 0;
}

static inline float
load_factor (const struct vocab *v)
{
  return (float) v->len / (float) v->cap;
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

int
vocab_shrink (struct vocab *v, int min)
{
  if (min <= 0)
    return 0;
  qsort (v->pool, v->len, sizeof (struct vocab_entry), cmp);
  while (v->len > 0) {
    if (v->pool[v->len - 1].count >= (uint32_t) min)
      break;
    v->len--;
  }
  if (vocab_build (v) != 0)
    return -1;
  return 0;
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
  return i;
}

int
vocab_add (struct vocab *v, const char *w)
{
  struct vocab_entry *entry;

  uint32_t h = hash (w);
  size_t i = find (v, h, w);

  if (v->table[i]) {
    v->table[i]->count++;
    return 0;
  }

  if (load_factor (v) > 0.7f) {
    v->cap <<= 2;
    if (vocab_alloc (v) != 0)
      return -1;
    if (vocab_build (v) != 0)
      return -1;
    i = find (v, h, w);
  }

  entry = v->pool + v->len;
  entry->hash = h;
  entry->count = 1;
  entry->code = 0;
  strncpy (entry->data, w, MAX_WORD_LENGTH);
  entry->data[MAX_WORD_LENGTH - 1] = 0;

  v->table[i] = v->pool + v->len;
  v->len++;
  return 0;
}

struct vocab_entry *
vocab_get (struct vocab *v, const char *w)
{
  uint32_t h = hash (w);
  size_t i = find (v, h, w);
  return v->table[i];
}

int
vocab_get_index (struct vocab *v, const char *w, size_t *i)
{
  struct vocab_entry *entry;

  entry = vocab_get (v, w);
  if (entry == NULL)
    return -1;
  *i = (size_t) (entry - v->pool);
  return 0;
}

int
vocab_encode (struct vocab *v)
{
  struct vocab_entry *entry;

  int32_t point[MAX_CODE_LENGTH];
  uint64_t code;

  uint32_t a, b, i;
  uint32_t m1, m2;
  int64_t p1, p2;

  uint32_t *count;
  uint32_t *binary;
  uint32_t *parent;

  int r = 0;

  if (v->len == 0)
    return 0;

  if (v->len > INT32_MAX)
    return -1;

  count = mem_alloc (v->len * 2 + 1, sizeof (uint32_t));
  binary = mem_alloc (v->len / 16 + 1, sizeof (uint32_t));
  parent = mem_alloc (v->len * 2 + 1, sizeof (uint32_t));

  r |= (count == NULL);
  r |= (binary == NULL);
  r |= (parent == NULL);
  if (r)
    goto cleanup;

  for (a = 0; a < v->len; a++)
    count[a] = v->pool[a].count;

  for (; a < v->len * 2; a++)
    count[a] = 0x10000000;

  p1 = (int64_t) v->len - 1;
  p2 = (int64_t) v->len;
  for (a = 0; a < v->len - 1; a++) {
    if (p1 >= 0) {
      if (count[p1] < count[p2]) {
        m1 = (uint32_t) p1;
        p1--;
      }
      else {
        m1 = (uint32_t) p2;
        p2++;
      }
    }
    else {
      m1 = (uint32_t) p2;
      p2++;
    }
    if (p1 >= 0) {
      if (count[p1] < count[p2]) {
        m2 = (uint32_t) p1;
        p1--;
      }
      else {
        m2 = (uint32_t) p2;
        p2++;
      }
    }
    else {
      m2 = (uint32_t) p2;
      p2++;
    }
    count[v->len + a] = count[m1] + count[m2];
    parent[m1] = (uint32_t) v->len + a;
    parent[m2] = (uint32_t) v->len + a;
    binary[m2 / 32] |= 1u << (m2 % 32);
  }

  entry = v->pool;
  for (a = 0; a < v->len; a++) {
    code = 0;
    for (b = a, i = 0; b != (v->len * 2 - 2); b = parent[b], i++) {
      code |= ((binary[b / 32] >> (b % 32)) & 1) << i;
      point[i] = (int32_t) b;
    }
    entry->point[0] = (int32_t) v->len - 2;
    entry->code = 0;
    for (b = 0; b < i; b++) {
      entry->code |= (code & 1) << (i - b - 1);
      entry->point[i - b] = point[b] - (int32_t) v->len;
      code >>= 1;
    }
    entry++;
  }
cleanup:
  mem_free (count);
  mem_free (binary);
  mem_free (parent);
  return -r;
}
