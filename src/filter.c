#include <stdio.h>

#include "core/scanner.h"
#include "core/filter.h"

int
main (int argc, char **argv)
{
  struct scanner *s;
  char b[8192];
  int i;

  for (i = 1; i < argc; i++) {
    s = scanner_new (argv[i]);
    if (s == NULL)
      continue;
    while (scanner_readline (s, b, sizeof (b)) >= 0) {
      if (*b)
        puts (filter (b));
    }
    scanner_free (s);
  }
  return 0;
}
