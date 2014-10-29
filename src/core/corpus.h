#ifndef CORE_CORPUS_H
#define CORE_CORPUS_H

#include <stdlib.h>
#include "vocab.h"

struct sentence {
  size_t len;
  size_t words[];
};

struct corpus {
  struct vocab *vocab;
  struct {
    size_t len;
    size_t cap;
    size_t *ptr;
  } words;
  struct {
    size_t len;
    size_t cap;
    struct sentence **ptr;
  } sentences;
};

struct corpus *corpus_new (struct vocab *v);
void corpus_free (struct corpus *c);

int corpus_parse (struct corpus *c, const char *path);
int corpus_grow (struct corpus *c, size_t words, size_t sentences);
int corpus_rebuild (struct corpus *c);

#endif
