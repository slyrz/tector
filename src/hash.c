#include "hash.h"

#define fnv_init(h) \
  h = 2166136261

#define fnv_update(h,v) \
  h = (h ^ (v)) * 16777619

uint32_t
hashstr (const char *restrict ptr)
{
  const uint8_t *w = (const uint8_t *) ptr;
  uint32_t h;

  fnv_init (h);
  while (*w)
    fnv_update (h, *w++);
  return h;
}

uint32_t
hashptr (const void *restrict ptr, size_t n)
{
  const uint8_t *w = (const uint8_t *) ptr;
  uint32_t h;

  fnv_init (h);
  while (n--)
    fnv_update (h, *w++);
  return h;
}
