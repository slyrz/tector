#include <stdio.h>
#include <stdlib.h>

#include "core/corpus.h"
#include "core/filter.h"
#include "core/log.h"
#include "core/model.h"
#include "core/options.h"
#include "core/vocab.h"

static int create (int argc, char **argv);
static int train (int argc, char **argv);
static int generate (int argc, char **argv);

struct program program = {
  .name = "model",
  .info = "manage language models",
  .commands = {
    { .name = "create", .args = "MODEL", .opts = "ilvw", .main = create },
    { .name = "train", .args = "MODEL TEXTFILE...", .main = train },
    { .name = "generate", .args = "MODEL", .main = generate },
    { NULL },
  },
};

static const char *vocab = "vocab.bin";
static const char *model = "model.bin";
static size_t iterations = 10;
static size_t layers = 50;
static size_t window = 5;

static int
create (int argc, char **argv)
{
  puts ("model_create");
}

static int
train (int argc, char **argv)
{
  puts ("train");
}

static int
generate (int argc, char **argv)
{
  puts ("generate");
}

int
main (int argc, char **argv)
{
  struct corpus *c;
  struct model *m;
  struct vocab *v;

  int i;
  int j;
  int k;

  k = options_parse (argc, argv);
  options_get_str ('n', &model);
  options_get_str ('v', &vocab);
  options_get_size_t ('i', &iterations);
  options_get_size_t ('l', &layers);
  options_get_size_t ('w', &window);

  v = vocab_open (vocab);
  if (v == NULL)
    fatal ("vocab_new");

  c = corpus_new (v);
  if (c == NULL)
    fatal ("corpus_new");

  for (i = k + 1; i < argc; i++)
    if (corpus_parse (c, argv[i]) != 0)
      fatal ("corpus_parse");

  m = model_new (v, MODEL_NN);
  if (m == NULL)
    fatal ("model_new");

  for (j = 0; j < iterations; j++)
    model_train (m, c);

  if (model_save (m, model) != 0)
    fatal ("model_save");

  /*
     for (i = 0; i < 10; i++) {
     printf ("%s:\n", v->entries[i].word);
     for (j = 0; j < n->size.layer; j++)
     printf ("%f ", (double) n->syn0[i * n->size.layer + j]);
     putchar ('\n');
     }
   */

  /**
   * TODO: ...
   * if (neural_network_save (n, model) != 0)
   *   fatal ("neural_network_save");
   */

  model_free (m);
  corpus_free (c);
  vocab_free (v);
  return 0;
}
