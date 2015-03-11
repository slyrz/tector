#include <stdio.h>
#include <string.h>

#include "ngram.h"
#include "macros.h"

void
bigram (char *dst, const char *a, const char *b)
{
  if (strcmp (a, b) > 0)
    swap (a, b);
  sprintf (dst, "%s_%s", a, b);
}

void
trigram (char *dst, const char *a, const char *b, const char *c)
{
  if (strcmp (a, c) > 0)
    swap (a, c);
  if (strcmp (a, b) > 0)
    swap (a, b);
  if (strcmp (b, c) > 0)
    swap (b, c);
  sprintf (dst, "%s_%s_%s", a, b, c);
}
