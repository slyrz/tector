#ifndef CORE_MODEL_H
#define CORE_MODEL_H

#include <stdlib.h>

#include "vocab.h"
#include "corpus.h"
#include "file.h"

enum {
  MODEL_NN,
};

struct model {
  const struct model_interface *i;
  struct vocab *v;
  int type;
  struct {
    size_t iter;
    size_t layer;
    size_t vector;
    size_t vocab;
    size_t window;
  } size;
  struct {
    unsigned allocated:1;
    unsigned changed:1;
  } state;
};

struct model_interface {
  size_t size;
  int (*init) (struct model *);
  int (*free) (struct model *);
  int (*alloc) (struct model *);
  int (*train) (struct model *, struct corpus *);
  int (*load) (struct model *, struct file *);
  int (*save) (struct model *, struct file *);
};

struct model *model_new (struct vocab *, int);
struct model *model_open (struct vocab *, const char *);
void model_free (struct model *);

int model_save (struct model *, const char *);
int model_train (struct model *, struct corpus *);

#endif
