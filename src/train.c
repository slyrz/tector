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
  .opts = "ilnvw",
  .args = "TEXTFILE...",
};

static const char *neuralnetwork = "neuralnetwork.bin";
static const char *vocab = "vocab.bin";
static size_t iterations = 10;
static size_t layers = 50;
static size_t window = 5;

int
main (int argc, char **argv)
{
  struct corpus *c;
  struct neural_network *n;
  struct vocab *v;

  int i;
  int j;
  int k;

  k = options_parse (argc, argv);
  options_get_str ('n', &neuralnetwork);
  options_get_str ('v', &vocab);
  options_get_size_t ('i', &iterations);
  options_get_size_t ('l', &layers);
  options_get_size_t ('w', &window);

  v = vocab_open (vocab);
  if (v == NULL)
    fatal ("vocab_new");

  n = neural_network_new (v, layers, window);
  if (n == NULL)
    fatal ("neural_network_new");

  for (i = k; i < argc; i++) {
    c = corpus_new (v);
    if (c == NULL)
      fatal ("corpus_new");
    if (corpus_parse (c, argv[i]) != 0)
      fatal ("corpus_parse");
    for (j = 0; j < iterations; j++)
      neural_network_train (n, c);
    corpus_free (c);
  }

  for (i = 0; i < 10; i++) {
    printf ("%s:\n", v->entries[i].word);
    for (j = 0; j < n->size.layer; j++)
      printf ("%f ", (double) n->syn0[i * n->size.layer + j]);
    putchar ('\n');
  }

  /**
   * TODO: ...
   * if (neural_network_save (n, neuralnetwork) != 0)
   *   fatal ("neural_network_save");
   */

  neural_network_free (n);
  vocab_free (v);
  return 0;
}
