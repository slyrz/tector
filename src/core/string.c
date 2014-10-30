#include "string.h"
#include <stdio.h>

int
isupper (int c)
{
  return ((unsigned int) c - 'A') < 26;
}

int
islower (int c)
{
  return ((unsigned int) c - 'a') < 26;
}

int
isspace (int c)
{
  return (c <= ' ') || (c == 127);
}

int
isalpha (int c)
{
  return islower (c | 0x20);
}

int
isunicode (int c)
{
  return c & 0x80;
}

char
lowercase (int c)
{
  if (isupper (c))
    return (char) (c | 0x20);
  return (char) c;
}

char
uppercase (int c)
{
  if (islower (c))
    return (char) (c & 0x5f);
  return (char) c;
}

int
ordalpha (int c)
{
  c |= 0x20;
  if (islower (c))
    return 1 + (c - 'a');
  return 0;
}

void
nullterm (char *s, size_t l)
{
  if ((l > 0) && (s[l - 1] == ' '))
    l--;
  s[l] = '\0';
}

static inline int
log1024 (unsigned long v)
{
  int e = 0;
  e += (v >= 1024);
  v /= 1024;
  e += (v >= 1024);
  v /= 1024;
  e += (v >= 1024);
  v /= 1024;
  e += (v >= 1024);
  v /= 1024;
  return e;
}

void
formatsize (char *s, size_t v)
{
  const int w = log1024 (v);
  const size_t x = v >> (w * 10);
  sprintf (s, "%zu %sB", x, &"\0\0K\0M\0G\0T\0"[w << 1]);
}
