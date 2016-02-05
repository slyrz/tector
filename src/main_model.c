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
static void print (void);

struct program program = {
  .name = "model",
  .info = "manage language models",
  .commands = {
    { .name = "create", .args = "DIR", .opts = "iltvw", .main = create },
    { .name = "train", .args = "DIR TEXTFILE...", .main = train },
    { .name = "generate", .args = "DIR", .main = generate },
    { .name = "print", .args = "DIR", .main = print },
    {},
  },
};

static struct bundle *b;
static unsigned int iterations;
static unsigned int layer;
static unsigned int vector;
static unsigned int window;
static unsigned int type = MODEL_NN;

#define entry(x) [MODEL_ ## x] = #x
static const char *model_name[] = {
  entry (GLOVE),
  entry (NN),
  entry (SVD),
};
#undef entry

static void
create (void)
{
  if (b->model)
    fatal ("model exists");
  b->model = model_new (b->vocab, type);
  if (b->model == NULL)
    fatal ("model_new");
  if (layer)
    b->model->size.layer = layer;
  if (vector)
    b->model->size.vector = vector;
  if (window)
    b->model->size.window = window;
  if (iterations)
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
    info ("training on '%s'", arg);
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

static void
print (void)
{
  if (b->model == NULL)
    fatal ("model missing");

  printf ("Model %s\n",  model_name[b->model->type]);
  printf (" - Iterations: %zu\n", b->model->size.iter);
  printf (" - Layer: %zu dimensions\n", b->model->size.layer);
  printf (" - Vector: %zu dimensions\n", b->model->size.vector);
  printf (" - Window-Size: %zu\n", b->model->size.window);
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
    for (type = 0; type < NUM_MODELS; type++) {
      if (strcasecmp (typestr, model_name[type]) == 0)
        break;
    }
    if (type == NUM_MODELS)
      fatal ("unkown model %s", typestr);
  }

  b = bundle_open (program_poparg ());
  if (b == NULL)
    fatal ("bundle_open");
  program_run ();
  bundle_save (b);
  bundle_free (b);
  return 0;
}
