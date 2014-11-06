#include <stdio.h>

#include "core/options.h"
#include "core/scanner.h"
#include "core/filter.h"
#include "core/log.h"

struct command command = {
  .name = "filter",
  .args = "TEXTFILE...",
  .opts = "",
};

int
main (int argc, char **argv)
{
  struct scanner *s;
  char b[8192] = { 0 };
  int i;
  int k;

  k = options_parse (argc, argv);
  for (i = k; i < argc; i++) {
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
