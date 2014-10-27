#include <stdio.h>

#include "core/scanner.h"
#include "core/filter.h"
#include "core/log.h"

int
main (int argc, char **argv)
{
  struct scanner *s;
  char b[8192] = { 0 };
  int i;

  for (i = 1; i < argc; i++) {
    s = scanner_new (argv[i]);
    if (s == NULL) {
      error ("scanner_new (%s)", argv[i]);
      continue;
    }
    while (scanner_readline (s, b, sizeof (b)) >= 0) {
      if (*b)
        puts (filter (b));
    }
    scanner_free (s);
  }
  return 0;
}
