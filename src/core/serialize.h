#ifndef CORE_SERIALIZE_H
#define CORE_SERIALIZE_H

#include "vocab.h"
#include "corpus.h"
#include "neural_network.h"

int vocab_load (struct vocab *v, const char *path);
int vocab_save (struct vocab *v, const char *path);
int corpus_load (struct corpus *c, const char *path);
int corpus_save (struct corpus *c, const char *path);
int neural_network_load (struct neural_network *n, const char *path);
int neural_network_save (struct neural_network *n, const char *path);

#endif
