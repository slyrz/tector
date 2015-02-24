#include <string.h>

#include "core/options.h"
#include "core/filter.h"
#include "core/scanner.h"
#include "core/vocab.h"
#include "core/log.h"

struct command command = {
  .name = "vocab",
  .args = "TEXTFILE...",
  .opts = "vm",
};

static const char *vocab = "vocab.bin";
static int mincount = 10;

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

  for (i = k; i < argc; i++)
    vocab_parse (v, argv[i]);

  info ("vocab contains %zu words", v->len);
  vocab_shrink (v, mincount);
  info ("vocab shrinked to %zu words", v->len);

  vocab_encode (v);
  vocab_save (v, vocab);
  vocab_free (v);
  return 0;
}
