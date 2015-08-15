#include <stdio.h>
#include <unistd.h>

#include "program.h"
#include "filter.h"
#include "log.h"
#include "scanner.h"

struct program program = {
  .name = "filter",
  .info = "cleans text, peforms stemming and stopword removal",
  .commands = {
    { .args = "TEXTFILE..." },
    {},
  },
};

static int
filter_lines (struct scanner *s)
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
  char *arg;

  program_init (argc, argv);

  arg = program_poparg ();
  if (arg == NULL)
    filter_lines (scanner_new (STDIN_FILENO));
  while (arg) {
    if (filter_lines (scanner_open (arg)) != 0)
      error ("failed to open '%s'", arg);
    arg = program_poparg ();
  }
  return 0;
}
