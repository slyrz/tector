#include <string.h>

#include "core/program.h"
#include "core/bundle.h"
#include "core/log.h"
#include "core/vocab.h"

static void create (int, char **);
static void train (int, char **);
static void print (int, char **);

struct program program = {
  .name = "vocab",
  .info = "manage vocabularies",
  .commands = {
    { .name = "create", .args = "DIR", .opts = "m", .main = create },
    { .name = "train", .args = "DIR TEXTFILE...", .main = train },
    { .name = "print", .args = "DIR", .main = print },
    { NULL },
  },
};

static struct bundle *b;
static unsigned int min = 10;

static void
create (int argc, char **argv)
{
  if (b->vocab)
    fatal ("vocab exists");
  b->vocab = vocab_new ();
  if (b->vocab == NULL)
    fatal ("vocab_new");
  b->vocab->min = min;
}

static void
train (int argc, char **argv)
{
  int i;

  if (b->vocab == NULL)
    fatal ("vocab missing");
  for (i = 1; i < argc; i++) {
    if (vocab_parse (b->vocab, argv[i]) != 0)
      error ("vocab_parse '%s' failed", argv[i]);
  }
}

static void
print (int argc, char **argv)
{
  size_t i;

  if (b->vocab == NULL)
    fatal ("vocab missing");
  for (i = 0; i < b->vocab->len; i++)
    printf ("%8u %s\n", b->vocab->entries[i].count, b->vocab->entries[i].word);
}

int
main (int argc, char **argv)
{
  program_init (argc, argv);
  program_getoptuint ('m', &min);

  b = bundle_open (argv[0]);
  if (b == NULL)
    fatal ("bundle_open");
  program_run ();
  bundle_save (b);
  bundle_free (b);
  return 0;
}
