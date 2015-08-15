#include <string.h>

#include "program.h"
#include "bundle.h"
#include "log.h"
#include "vocab.h"

static void create (void);
static void train (void);
static void print (void);

struct program program = {
  .name = "vocab",
  .info = "manage vocabularies",
  .commands = {
    { .name = "create", .args = "DIR", .opts = "m", .main = create },
    { .name = "train", .args = "DIR TEXTFILE...", .main = train },
    { .name = "print", .args = "DIR", .main = print },
    {},
  },
};

static struct bundle *b;
static unsigned int min = 10;

static void
create (void)
{
  if (b->vocab)
    fatal ("vocab exists");
  b->vocab = vocab_new ();
  if (b->vocab == NULL)
    fatal ("vocab_new");
  b->vocab->min = min;
}

static void
train (void)
{
  char *arg;

  if (b->vocab == NULL)
    fatal ("vocab missing");
  while (arg = program_poparg (), arg != NULL) {
    if (vocab_parse (b->vocab, arg) != 0)
      error ("vocab_parse '%s' failed", arg);
  }
}

static void
print (void)
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

  b = bundle_open (program_poparg ());
  if (b == NULL)
    fatal ("bundle_open");
  program_run ();
  bundle_save (b);
  bundle_free (b);
  return 0;
}
