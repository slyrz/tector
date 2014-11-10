#include "../src/core/vocab.h"
#include "../src/core/corpus.h"
#include "../src/core/neural_network.h"
#include "../src/core/serialize.h"
#include "../src/core/options.h"
#include "../src/core/log.h"

#include <stdlib.h>

struct command command = {
  .name = "load",
  .args = "",
  .opts = "cvn",
};

static const char *corpus = NULL;
static const char *neuralnetwork = NULL;
static const char *vocab = NULL;

int
main (int argc, char **argv)
{
  struct vocab *v;
  struct corpus *c;
  struct neural_network *n;

  options_parse (argc, argv);
  options_get_str ('c', &corpus);
  options_get_str ('n', &neuralnetwork);
  options_get_str ('v', &vocab);

  if (vocab) {
    v = vocab_new ();
    if (vocab_load (v, vocab) != 0)
      fatal ("vocab_load");
    if (corpus) {
      c = corpus_new (v);
      if (corpus_load (c, corpus) != 0)
        fatal ("corpus_load");
      if (neuralnetwork) {
        n = neural_network_new (v, 50, 5);
        if (neural_network_load (n, neuralnetwork) != 0)
          fatal ("neural_network_load");
        neural_network_free (n);
      }
      corpus_free (c);
    }
    vocab_free (v);
  }
  return EXIT_SUCCESS;
}
