#include "mem.h"

#include <stdint.h>
#include <string.h>

size_t
sizepow2 (size_t n)
{
  size_t r = n;

  r--;
  r |= r >> 1;
  r |= r >> 2;
  r |= r >> 4;
  r |= r >> 8;
  r |= r >> 16;
  r++;
  return (r > n) ? r : n;
}

static inline int
overflow (size_t n, size_t s)
{
  const size_t t = 1ul << (sizeof (size_t) * 4);
  return (((n >= t) || (s >= t)) && (n > 0) && ((SIZE_MAX / n) < s));
}

void *
reallocarray (void *ptr, size_t len, size_t size)
{
  if (overflow (len, size) || (len == 0) || (size == 0))
    return NULL;
  return realloc (ptr, size * len);
}

void *
clearspace (void *ptr, size_t len, size_t cap, size_t size)
{
  return memset (ptr + (len * size), 0, (cap - len) * size);
}
