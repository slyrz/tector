#include "string.h"

void
nullterm (char *s, size_t l)
{
  if ((l > 0) && (s[l - 1] == ' '))
    l--;
  s[l] = '\0';
}
