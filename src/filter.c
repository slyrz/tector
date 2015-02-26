#include <stdio.h>
#include <unistd.h>

#include "core/options.h"
#include "core/scanner.h"
#include "core/filter.h"
#include "core/log.h"

struct program program = {
  .name = "filter",
  .info = "cleans text, peforms stemming and stopword removal",
  .commands = {
    { .args = "TEXTFILE..." },
    { NULL },
  },
};

static int
run_filter (struct scanner *s)
{
  char b[8192] = { 0 };

  if (s == NULL)
    return -1;

  while (scanner_readline (s, b, sizeof (b)) >= 0) {
    filter (b);
    if (*b)
      puts (b);
  }
  scanner_free (s);
  return 0;
}

int
main (int argc, char **argv)
{
  int i;
  int k;

  k = options_parse (argc, argv);
  if (k == argc)
    run_filter (scanner_new (STDIN_FILENO));
  for (i = k; i < argc; i++)
    if (run_filter (scanner_open (argv[i])) != 0)
      error ("failed to open '%s'", argv[i]);
  return 0;
}
