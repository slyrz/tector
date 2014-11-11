#ifndef CORE_NEURAL_NETWORK_H
#define CORE_NEURAL_NETWORK_H

#include <stdlib.h>

#include "config.h"
#include "vocab.h"
#include "corpus.h"

#define MAX_WINDOW	50
#define MAX_LAYERS	10000

struct neural_network {
  struct vocab *v;
  struct corpus *c;
  struct {
    size_t vocab;
    size_t layer;
    size_t window;
  } size;
  float alpha;
  float *syn0;
  float *syn1;
  float *neu1;
  float *neu2;
};

struct neural_network *neural_network_new (struct vocab *v, size_t layer, size_t window);
void neural_network_free (struct neural_network *n);
int neural_network_alloc (struct neural_network *n);
int neural_network_train (struct neural_network *n, struct corpus *c);

#endif
