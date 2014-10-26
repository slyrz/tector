#ifndef CORE_NEURAL_NETWORK_H
#define CORE_NEURAL_NETWORK_H

#include <stdlib.h>

#include "config.h"
#include "vocab.h"
#include "corpus.h"

struct neural_network {
  const struct vocab *restrict v;
  const struct corpus *restrict c;
  struct {
    size_t vocab;
    size_t layer;
    size_t window;
  } size;
  float *syn0;
  float *syn1;
};

struct neural_network *neural_network_new (struct vocab *v, size_t layer,
                                           size_t window);
void neural_network_free (struct neural_network *n);
void neural_network_train (struct neural_network *n, struct corpus *c);

#endif
