#include <err.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core/filter.h"
#include "core/scanner.h"
#include "core/vocab.h"

static void
learn_sentence (struct vocab *v, char *s)
{
  char *w;
  while (w = strtok_r (s, " ", &s), w)
    vocab_add (v, w);
}

static void
learn_file (struct vocab *v, const char *path)
{
  struct scanner *s;
  char b[8192];

  s = scanner_new (path);
  if (s == NULL)
    return;
  while (scanner_readline (s, b, sizeof (b)) >= 0) {
    if (*b)
      learn_sentence (v, filter (b));
  }
  scanner_free (s);
}

int
main (int argc, char **argv)
{
  struct vocab *v;
  int i;

  v = vocab_new ();
  if (v == NULL)
    err (EXIT_FAILURE, "vocab_new");
  for (i = 1; i < argc; i++) {
    fprintf (stderr, "learning '%s' (%d/%d)\n", argv[i], i, argc - 1);
    learn_file (v, argv[i]);
  }
  fprintf (stderr, "vocab contains %zu words\n", v->len);
  vocab_shrink (v);
  fprintf (stderr, "vocab shrinked to %zu words\n", v->len);
  vocab_encode (v);
  vocab_save (v, "vocab.bin");
  vocab_print (v);
  vocab_free (v);
  return 0;
}
