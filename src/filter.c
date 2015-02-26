#include <stdio.h>

#include "core/options.h"
#include "core/scanner.h"
#include "core/filter.h"
#include "core/log.h"

struct program program = {
  .name = "filter",
  .info = "cleans text, peforms stemming stopword removal",
  .commands = {
    { .args = "TEXTFILE..." },
    { NULL },
  },
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
    s = scanner_open (argv[i]);
    if (s == NULL) {
      error ("scanner_new (%s)", argv[i]);
      continue;
    }
    while (scanner_readline (s, b, sizeof (b)) >= 0) {
      filter (b);
      if (*b)
        puts (b);
    }
    scanner_free (s);
  }
  return 0;
}
