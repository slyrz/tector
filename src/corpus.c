#include "core/options.h"
#include "core/corpus.h"
#include "core/vocab.h"
#include "core/log.h"
#include "core/serialize.h"

struct command command = {
  .name = "corpus",
  .args = "TEXTFILE...",
  .opts = "cw",
};

static const char *corpus = "corpus.bin";
static const char *vocab = "vocab.bin";

int
main (int argc, char **argv)
{
  struct vocab *v;
  struct corpus *c;
  int i;
  int k;

  k = options_parse (argc, argv);
  options_get_str ('c', &corpus);
  options_get_str ('v', &vocab);

  v = vocab_new ();
  if (v == NULL)
    fatal ("vocab_new");
  if (vocab_load (v, vocab) != 0)
    fatal ("vocab_load");

  c = corpus_new (v);
  if (c == NULL)
    fatal ("corpus_new");

  for (i = k; i < argc; i++)
    corpus_parse (c, argv[i]);

  if (corpus_save (c, corpus) != 0)
    fatal ("corpus_save");
  corpus_free (c);
  vocab_free (v);
  return 0;
}
