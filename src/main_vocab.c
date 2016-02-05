#include <string.h>

#include "program.h"
#include "bundle.h"
#include "log.h"
#include "vocab.h"

static void create (void);
static void train (void);
static void print (void);
static void shrink (void);
static void copy (void);

struct program program = {
  .name = "vocab",
  .info = "manage vocabularies",
  .commands = {
    { .name = "create", .args = "DIR", .opts = "m", .main = create },
    { .name = "train", .args = "DIR TEXTFILE...", .main = train },
    { .name = "print", .args = "DIR", .main = print },
    { .name = "shrink", .args = "DIR", .opts = "m", .main = shrink },
    { .name = "copy", .args = "DIR TEXTFILE...", .main = copy },
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

static void
shrink (void)
{
  if (b->vocab == NULL)
    fatal ("vocab missing");
  b->vocab->min = min;
}

static void
copy (void)
{
  char *arg;

  if (b->vocab == NULL)
    fatal ("vocab missing");
  while (arg = program_poparg (), arg != NULL) {
    if (vocab_copy (b->vocab, arg) != 0)
      error ("vocab_copy '%s' failed", arg);
  }
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
