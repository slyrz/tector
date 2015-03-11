#include "../src/core/vocab.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#define len(x) \
  (sizeof (x) / sizeof (x[0]))

static char words[][32] = {
  "baboon",
  "bear",
  "cat",
  "dog",
  "donkey",
  "frog",
  "giraffe",
  "mole",
  "mouse",
  "octopus",
  "rabbit",
  "turtle",
};

int
main (void)
{
  const size_t n = len (words);
  size_t c[n];
  size_t i;
  size_t j;
  struct vocab *v;

  v = vocab_new ();
  v->min = 0;
  assert (v != NULL);
  assert (vocab_parse (v, "tests/testdata/vocab.txt") == 0);
  assert (v->len == len (words));
  for (i = 0; i < len (words); i++) {
    assert (vocab_find (v, words[i], &j) == 0);
    c[i] = v->entries[j].count;
  }
  assert (vocab_save (v, "/tmp/vocab.bin") == 0);
  vocab_free (v);

  v = vocab_open ("/tmp/vocab.bin");
  assert (v != NULL);
  assert (v->len == len (words));
  for (i = 0; i < len (words); i++) {
    assert (vocab_find (v, words[i], &j) == 0);
    assert (c[i] == v->entries[j].count);
  }
  vocab_free (v);
  return EXIT_SUCCESS;
}
