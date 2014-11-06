#include <string.h>

#include "core/options.h"
#include "core/filter.h"
#include "core/scanner.h"
#include "core/vocab.h"
#include "core/log.h"
#include "core/serialize.h"

struct command command = {
  .name = "vocab",
  .args = "TEXTFILE...",
  .opts = "vm",
};

static const char *vocab = "vocab.bin";
static int mincount = 10;

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
  char b[8192] = { 0 };

  s = scanner_new (path);
  if (s == NULL) {
    error ("scanner_new (%s)", path);
    return;
  }
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
  int k;

  k = options_parse (argc, argv);
  options_get_str ('v', &vocab);
  options_get_int ('m', &mincount);

  v = vocab_new ();
  if (v == NULL)
    fatal ("vocab_new");

  for (i = k; i < argc; i++) {
    info ("learning '%s'", argv[i]);
    learn_file (v, argv[i]);
  }

  info ("vocab contains %zu words", v->len);
  vocab_shrink (v, mincount);
  info ("vocab shrinked to %zu words", v->len);

  vocab_encode (v);
  vocab_save (v, vocab);
  vocab_free (v);
  return 0;
}
