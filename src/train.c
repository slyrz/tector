#include <stdio.h>
#include <stdlib.h>

#include "core/corpus.h"
#include "core/filter.h"
#include "core/log.h"
#include "core/neural_network.h"
#include "core/options.h"
#include "core/serialize.h"
#include "core/vocab.h"

struct command command = {
  .name = "train",
  .args = "",
  .opts = "cilntvw",
};

static const char *corpus = "corpus.bin";
static const char *neuralnetwork = "neuralnetwork.bin";
static const char *vocab = "vocab.bin";
static int threads = 4;
static int iterations = 10;
static size_t layers = 50;
static size_t window = 5;

int
main (int argc, char **argv)
{
  struct corpus *c;
  struct neural_network *n;
  struct vocab *v;

  size_t i;
  size_t j;

  options_parse (argc, argv);
  options_get_str ('c', &corpus);
  options_get_str ('n', &neuralnetwork);
  options_get_str ('v', &vocab);
  options_get_int ('i', &iterations);
  options_get_int ('t', &threads);
  options_get_size_t ('l', &layers);
  options_get_size_t ('w', &window);

  v = vocab_new ();
  if (v == NULL)
    fatal ("vocab_new");
  if (vocab_load (v, vocab) != 0)
    fatal ("vocab_load");

  c = corpus_new (v);
  if (c == NULL)
    fatal ("corpus_new");
  if (corpus_load (c, corpus) != 0)
    fatal ("corpus_load");

  n = neural_network_new (v, layers, window);
  if (n == NULL)
    fatal ("neural_network_new");
  neural_network_train (n, c);

  for (i = 0; i < 10; i++) {
    printf ("%s:\n", v->entries[i].word);
    for (j = 0; j < n->size.layer; j++)
      printf ("%lf ", (double) n->syn0[i * n->size.layer + j]);
    putchar ('\n');
  }

  if (neural_network_save (n, neuralnetwork) != 0)
    fatal ("neural_network_save");

  corpus_free (c);
  neural_network_free (n);
  vocab_free (v);
  return 0;
}
