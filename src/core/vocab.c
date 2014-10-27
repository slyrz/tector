/**
 * This file incorporates derivative work covered by the
 * following copyright and permission notice:
 *
 * Copyright 2013 Google Inc. All Rights Reserved.
 */
#include "config.h"

#include <err.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "vocab.h"

static inline uint32_t
hash (const char *restrict w)
{
  uint32_t h = 0x811c9dc5;
  while (*w)
    h = (h ^ *w++) * 0x1000193;
  return h;
}

static size_t
next_pow2 (size_t n)
{
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return ++n;
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
vocab_load_factor (const struct vocab *v)
{
  return (float) v->len / (float) v->cap;
}

static void
vocab_rebuild_table (struct vocab *v)
{
  size_t i;
  size_t j;

  memset (v->table, 0, v->cap * sizeof (struct vocab_entry *));
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
  if (cap < v->cap)
    return;

  v->cap = cap;
  v->pool = realloc (v->pool, v->cap * sizeof (struct vocab_entry));
  if (v->pool == NULL)
    goto error;
  v->table = realloc (v->table, v->cap * sizeof (struct vocab_entry *));
  if (v->table == NULL)
    goto error;
  memset (v->pool + v->len, 0, (v->cap - v->len) * sizeof (struct vocab_entry));
  vocab_rebuild_table (v);
  return;
error:
  err (EXIT_FAILURE, "vocab_grow failed");
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
  vocab_rebuild_table (v);
}

static inline size_t
vocab_find (struct vocab *v, uint32_t h, const char *w)
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
  size_t i = vocab_find (v, h, w);

  if (v->table[i]) {
    v->table[i]->count++;
    return;
  }

  if (vocab_load_factor (v) > 0.7)
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
  size_t i = vocab_find (v, h, w);
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

static inline void
vocab_entry_print (struct vocab_entry *entry, int i)
{
  printf (" [%6d] = { 0x%08x, %4d, '%s' }\n", i, entry->hash, entry->count,
          entry->data);
}

void
vocab_print (const struct vocab *v)
{
  size_t i;

  printf ("Len:    %zu\n", v->len);
  printf ("Cap:    %zu\n", v->cap);
  printf ("Load:   %f\n", vocab_load_factor (v));
  for (i = 0; i < v->len; i++) {
    vocab_entry_print (&v->pool[i], i);
  }
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

#define io(func,fd,ptr,size) \
  do { \
    if (func (fd, ptr, size) != (ssize_t) (size)) goto error; \
  } while (0)

void
vocab_load (struct vocab *v, const char *path)
{
  size_t len = 0;
  size_t cap = v->cap;
  int fd;

  fd = open (path, O_RDONLY);
  if (fd == -1)
    goto error;
  io (read, fd, &len, sizeof (size_t));
  if (len > cap) {
    cap = next_pow2 (len);
    vocab_grow (v, cap);
  }
  io (read, fd, v->pool, sizeof (struct vocab_entry) * len);
  close (fd);
  v->len = len;
  v->cap = cap;
  vocab_rebuild_table (v);
  return;
error:
  warn ("failed writing vocab to file '%s'", path);
}

void
vocab_save (struct vocab *v, const char *path)
{
  int fd;

  fd = open (path, O_CREAT | O_TRUNC | O_WRONLY, 0666);
  if (fd == -1)
    goto error;
  io (write, fd, &v->len, sizeof (size_t));
  io (write, fd, v->pool, sizeof (struct vocab_entry) * v->len);
  close (fd);
  return;
error:
  warn ("failed writing vocab to file '%s'", path);
}
