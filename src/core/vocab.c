/**
 * This file incorporates derivative work covered by the
 * following copyright and permission notice:
 *
 * Copyright 2013 Google Inc. All Rights Reserved.
 */
#include "config.h"
#include "vocab.h"
#include "scanner.h"
#include "log.h"
#include "mem.h"
#include "hash.h"
#include "file.h"

#include <string.h>

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

void
vocab_free (struct vocab *v)
{
  mem_free (v->entries);
  mem_free (v->table);
  mem_free (v);
}

int
vocab_alloc (struct vocab *v)
{
  v->cap = reqcap (v->len, v->cap, 32768);
  v->entries = mem_realloc (v->entries, v->cap, sizeof (struct vocab_entry));
  if (v->entries == NULL)
    return -1;
  v->table = mem_realloc (v->table, v->cap, sizeof (struct vocab_entry *));
  if (v->table == NULL)
    return -1;
  mem_clear (v->entries + v->len, v->cap - v->len, sizeof (struct vocab_entry));
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
    j = v->entries[i].hash;
    for (;;) {
      if (v->table[j % v->cap] == NULL)
        break;
      j++;
    }
    v->table[j % v->cap] = &v->entries[i];
  }
  return 0;
}

struct vocab *
vocab_open (const char *path)
{
  struct vocab *v = NULL;
  struct file *f = NULL;

  f = file_open (path);
  if (f == NULL)
    return NULL;

  v = mem_alloc (1, sizeof (struct vocab));
  if (v == NULL)
    goto error;
  v->len = f->header.data[0];
  if (vocab_alloc (v) != 0)
    goto error;
  if (file_read (f, v->entries, v->len * sizeof (struct vocab_entry)) != 0)
    goto error;
  if (vocab_build (v) != 0)
    goto error;
  file_close (f);
  return v;
error:
  if (v)
    vocab_free (v);
  if (f)
    file_close (f);
  return NULL;
}

int
vocab_save (struct vocab *v, const char *path)
{
  struct file *f = NULL;

  f = file_create (path);
  if (f == NULL)
    return -1;
  f->header.data[0] = v->len;
  if (file_write (f, v->entries, v->len * sizeof (struct vocab_entry)) != 0)
    goto error;
  file_close (f);
  return 0;
error:
  if (f)
    file_close (f);
  return -1;
}

static int
parse_sentence (struct vocab *v, char *s)
{
  char *w;
  while (w = strtok_r (s, " ", &s), w)
    if (vocab_add (v, w) != 0)
      return -1;
  return 0;
}

int
vocab_parse (struct vocab *v, const char *path)
{
  struct scanner *s;
  char b[8192] = { 0 };
  int r = 0;

  s = scanner_new (path);
  if (s == NULL)
    return -1;
  while (scanner_readline (s, b, sizeof (b)) >= 0) {
    r = parse_sentence (v, b);
    if (r != 0)
      break;
  }
  scanner_free (s);
  return r;
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
  qsort (v->entries, v->len, sizeof (struct vocab_entry), cmp);
  while (v->len > 0) {
    if (v->entries[v->len - 1].count >= (uint32_t) min)
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
        || ((entry->hash == h) && (strcmp (entry->word, w) == 0)))
      break;
    i = (i + 1) % v->cap;
  }
  return i;
}

int
vocab_add (struct vocab *v, const char *w)
{
  struct vocab_entry entry;

  uint32_t h = hashstr (w);
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

  entry = (struct vocab_entry) {
    .hash = h,
    .count = 1,
  };
  strncpy (entry.word, w, MAX_WORD_LENGTH - 1);

  v->entries[v->len] = entry;
  v->table[i] = &v->entries[v->len];
  v->len++;
  return 0;
}

int
vocab_find (struct vocab *v, const char *w, size_t *p)
{
  size_t i = find (v, hashstr (w), w);

  if (v->table[i])
    *p = (size_t) (v->table[i] - v->entries);
  return -(v->table[i] == NULL);
}

int
vocab_encode (struct vocab *v)
{
  struct vocab_entry *entry;

  int32_t point[MAX_CODE_LENGTH];

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
    count[a] = v->entries[a].count;

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

  entry = v->entries;
  for (a = 0; a < v->len; a++) {
    entry->code = 1ull;
    for (b = a, i = 0; b != (v->len * 2 - 2); b = parent[b], i++) {
      entry->code <<= 1;
      entry->code |= (binary[b / 32] >> (b % 32)) & 1ull;
      point[i] = (int32_t) b;
    }
    entry->point[0] = (int32_t) v->len - 2;
    for (b = 0; b < i; b++)
      entry->point[i - b] = point[b] - (int32_t) v->len;
    entry++;
  }
cleanup:
  mem_free (count);
  mem_free (binary);
  mem_free (parent);
  return -r;
}

uint32_t
vocab_id (struct vocab *v)
{
  return hashptr (v->entries, v->len * sizeof (struct vocab_entry));
}
