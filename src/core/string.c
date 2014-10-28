#include "string.h"

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
