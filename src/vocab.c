#include <string.h>

#include "core/options.h"
#include "core/filter.h"
#include "core/scanner.h"
#include "core/vocab.h"
#include "core/log.h"

struct program program = {
  .name = "vocab",
  .info = "manage vocabularies",
  .commands = {
    { .opts = "vm", .args = "TEXTFILE..." },
    { NULL },
  },
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

  vocab_save (v, vocab);
  vocab_free (v);
  return 0;
}
