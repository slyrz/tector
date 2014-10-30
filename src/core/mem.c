#include "mem.h"
#include "log.h"
#include "string.h"

#include <stdint.h>
#include <string.h>
#include <malloc.h>

static size_t used = 0;
static size_t objects = 0;

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

static void
logusage (void)
{
  char size[32];

  if (objects)
    debug ("%zu object%s using %s memory", objects, &"s"[objects == 1], formatsize (size, used));
  else
    debug ("all memory freed");
}

enum mode {
  ADD,
  SUB,
};

static void
bookkeep (int mode, void *ptr)
{
  const size_t size = malloc_usable_size (ptr);

  switch (mode) {
    case ADD:
      used += size;
      objects += !!size;
      break;
    case SUB:
      used -= size;
      objects -= !!size;
      break;
  }
  logusage ();
}

static inline int
valid (size_t n, size_t s)
{
  const size_t t = 1ul << (sizeof (size_t) * 4);

  if ((n == 0) || (s == 0))
    return 0;
  if ((n >= t) || (s >= t))
    return ((SIZE_MAX / s) >= n);
  return 1;
}

void *
mem_alloc (size_t nmemb, size_t size)
{
  void *ptr = NULL;

  if (valid (nmemb, size))
    if (ptr = calloc (nmemb, size), ptr != NULL)
      bookkeep (ADD, ptr);
  return ptr;
}

void *
mem_align (size_t nmemb, size_t size, size_t alignment)
{
  void *ptr = NULL;

  if (valid (nmemb, size))
    if (posix_memalign (&ptr, alignment, nmemb * size) == 0)
      bookkeep (ADD, ptr);
  return ptr;
}

void *
mem_realloc (void *ptr, size_t nmemb, size_t size)
{
  if (valid (nmemb, size)) {
    bookkeep (SUB, ptr);
    if (ptr = realloc (ptr, nmemb * size), ptr != NULL)
      bookkeep (ADD, ptr);
    return ptr;
  }
  return NULL;
}

void
mem_free (void *ptr)
{
  bookkeep (SUB, ptr);
  free (ptr);
}

void
mem_clear (void *ptr, size_t nmemb, size_t size)
{
  memset (ptr, 0, nmemb * size);
}
