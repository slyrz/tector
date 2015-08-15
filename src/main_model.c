#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "program.h"
#include "bundle.h"
#include "log.h"
#include "model.h"

static void create (void);
static void train (void);
static void generate (void);

struct program program = {
  .name = "model",
  .info = "manage language models",
  .commands = {
    { .name = "create", .args = "DIR", .opts = "iltvw", .main = create },
    { .name = "train", .args = "DIR TEXTFILE...", .main = train },
    { .name = "generate", .args = "DIR", .main = generate },
    {},
  },
};

static struct bundle *b;
static unsigned int iterations = 5;
static unsigned int layer = 50;
static unsigned int vector = 50;
static unsigned int window = 5;
static unsigned int type = MODEL_NN;

static void
create (void)
{
  if (b->model)
    fatal ("model exists");
  b->model = model_new (b->vocab, type);
  if (b->model == NULL)
    fatal ("model_new");
  b->model->size.layer = layer;
  b->model->size.vector = vector;
  b->model->size.window = window;
  b->model->size.iter = iterations;
  model_verify (b->model);
}

static void
train (void)
{
  struct corpus *c;
  char *arg;

  if (b->model == NULL)
    fatal ("model missing");
  c = corpus_new (b->vocab);
  if (c == NULL)
    fatal ("corpus_new");

  while (arg = program_poparg (), arg != NULL) {
    debug ("%s\n", arg);
    if (corpus_parse (c, arg) != 0)
      fatal ("corpus_parse");
    if (model_train (b->model, c) != 0)
      fatal ("model_train");
    corpus_clear (c);
  }
  corpus_free (c);
}

static void
generate (void)
{
  size_t n;
  size_t i;
  size_t j;

  if (b->model == NULL)
    fatal ("model missing");

  if (model_generate (b->model) != 0)
    fatal ("model_generate");

  n = b->model->size.vector;
  for (i = 0; i < b->vocab->len; i++) {
    printf ("%s ", b->vocab->entries[i].word);
    for (j = 0; j < n; j++)
      printf ("%f%c", b->model->embeddings[i * n + j], "\n "[j < (n - 1)]);
  }
}

int
main (int argc, char **argv)
{
  const char *typestr = NULL;

  program_init (argc, argv);
  program_getoptuint ('i', &iterations);
  program_getoptuint ('l', &layer);
  program_getoptuint ('v', &vector);
  program_getoptuint ('w', &window);
  program_getoptstr ('t', &typestr);

  if (typestr) {
    if (strcmp (typestr, "glove") == 0)
      type = MODEL_GLOVE;
    if (strcmp (typestr, "nn") == 0)
      type = MODEL_NN;
    if (strcmp (typestr, "svd") == 0)
      type = MODEL_SVD;
  }

  b = bundle_open (program_poparg ());
  if (b == NULL)
    fatal ("bundle_open");
  program_run ();
  bundle_save (b);
  bundle_free (b);
  return 0;
}
