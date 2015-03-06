#include <stdio.h>
#include <stdlib.h>

#include "core/program.h"
#include "core/bundle.h"
#include "core/log.h"
#include "core/model.h"

static void create (int argc, char **argv);
static void train (int argc, char **argv);
static void generate (int argc, char **argv);

struct program program = {
  .name = "model",
  .info = "manage language models",
  .commands = {
    { .name = "create", .args = "DIR", .opts = "ilvw", .main = create },
    { .name = "train", .args = "DIR TEXTFILE...", .main = train },
    { .name = "generate", .args = "DIR", .main = generate },
    { NULL },
  },
};

static struct bundle *b;
static int iterations = 10;
static int layer = 50;
static int vector = 50;
static int window = 5;

static void
create (int argc, char **argv)
{
  if (b->model)
    fatal ("model exists");
  b->model = model_new (b->vocab, MODEL_NN);
  if (b->model == NULL)
    fatal ("model_new");
  b->model->size.layer = layer;
  b->model->size.vector = vector;
  b->model->size.window = window;
}

static void
train (int argc, char **argv)
{
  struct corpus *c;
  int i;

  if (b->model == NULL)
    fatal ("model missing");
  c = corpus_new (b->vocab);
  if (c == NULL)
    fatal ("corpus_new");
  for (i = 0; i < argc; i++)
    if (corpus_parse (c, argv[i]) != 0)
      fatal ("corpus_parse");
  if (model_train (b->model, c) != 0)
    fatal ("model_train");
}

static void
generate (int argc, char **argv)
{
  puts ("generate");
}

int
main (int argc, char **argv)
{
  program_init (argc, argv);
  program_getoptint ('i', &iterations);
  program_getoptint ('l', &layer);
  program_getoptint ('w', &window);

  b = bundle_open (argv[0]);
  if (b == NULL)
    fatal ("bundle_open");
  program_run ();
  bundle_save (b);
  bundle_free (b);
  return 0;
}
