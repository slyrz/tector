#include "../src/core/vocab.h"
#include "../src/core/corpus.h"
#include "../src/core/neural_network.h"
#include "../src/core/serialize.h"
#include "../src/core/hash.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

int
main (void)
{
  struct corpus *c;
  struct vocab *v;
  struct neural_network *n;

  uint32_t x;
  uint32_t y;
  int i;

  v = vocab_new ();
  assert (v != NULL);
  assert (vocab_parse (v, "tests/testdata/corpus.txt") == 0);
  assert (vocab_shrink (v, 1) == 0);
  assert (vocab_encode (v) == 0);

  c = corpus_new (v);
  assert (c != NULL);
  assert (corpus_parse (c, "tests/testdata/corpus.txt") == 0);

  n = neural_network_new (v, 50, 5);
  assert (n != NULL);

  x = hashptr (n->syn0, n->size.vocab * n->size.layer * sizeof (float));
  assert (neural_network_train (n, c) == 0);
  y = hashptr (n->syn0, n->size.vocab * n->size.layer * sizeof (float));
  assert (x != y);

  assert (neural_network_save (n, "/tmp/neuralnetwork.bin") == 0);
  neural_network_free (n);

  n = neural_network_new (v, 50, 5);
  assert (n != NULL);
  assert (neural_network_load (n, "/tmp/neuralnetwork.bin") == 0);
  x = hashptr (n->syn0, n->size.vocab * n->size.layer * sizeof (float));
  assert (x == y);

  neural_network_free (n);
  corpus_free (c);
  vocab_free (v);
  return EXIT_SUCCESS;
}
