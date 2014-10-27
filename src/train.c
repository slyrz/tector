#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "core/filter.h"
#include "core/scanner.h"
#include "core/vocab.h"
#include "core/corpus.h"
#include "core/neural_network.h"
#include "core/log.h"

const size_t dimensionality = 50;
const size_t window = 5;

int
main (int argc, char **argv)
{
  struct corpus *c;
  struct neural_network *n;
  struct vocab *v;

  size_t i;
  size_t j;

  v = vocab_new_from_path ("vocab.bin");
  if (v == NULL)
    fatal ("vocab_new_from_path");

  c = corpus_new (v);
  if (c == NULL)
    fatal ("corpus_new");

  for (i = 1; i < (size_t) argc; i++)
    corpus_parse (c, argv[i]);

  n = neural_network_new (v, dimensionality, window);
  if (n == NULL)
    fatal ("neural_network_new");

  neural_network_train (n, c);

  for (i = 0; i < 10; i++) {
    printf ("%s:\n", v->pool[i].data);
    for (j = 0; j < n->size.layer; j++)
      printf ("%lf ", n->syn0[i * n->size.layer + j]);
    putchar ('\n');
  }

  corpus_free (c);
  neural_network_free (n);
  vocab_free (v);
  return 0;
}
