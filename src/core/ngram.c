#include "ngram.h"

#include <stdio.h>
#include <string.h>

#define swapstr(x,y) \
  do { \
    const char *t = x; x = y; y = t; \
  } while (0)

void
bigram (char *dst, const char *a, const char *b)
{
  if (strcmp (a, b) > 0)
    swapstr (a, b);
  sprintf (dst, "%s_%s", a, b);
}

void
trigram (char *dst, const char *a, const char *b, const char *c)
{
  if (strcmp (a, c) > 0)
    swapstr (a, c);
  if (strcmp (a, b) > 0)
    swapstr (a, b);
  if (strcmp (b, c) > 0)
    swapstr (b, c);
  sprintf (dst, "%s_%s_%s", a, b, c);
}
