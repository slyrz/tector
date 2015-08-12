#ifndef CORE_BUNDLE_H
#define CORE_BUNDLE_H

#include <stdlib.h>

#include "vocab.h"
#include "model.h"

/**
 * Bundle is just a wrapper to load/save a vocab and a model with a single
 * function call.
 */
struct bundle {
  struct vocab *vocab;
  struct model *model;
  struct {
    char *bundle;
    char *model;
    char *vocab;
  } path;
};

struct bundle *bundle_open (const char *);
void bundle_free (struct bundle *);
int bundle_save (struct bundle *);

#endif
