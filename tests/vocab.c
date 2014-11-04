#include "../src/core/vocab.h"
#include "../src/core/serialize.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define len(x) \
  (sizeof (x) / sizeof (x[0]))

const char *words[] = {
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

  memset (c, 0, n * sizeof (size_t));

  v = vocab_new ();
  assert (v != NULL);
  assert (v->len == 0);

  for (i = 0; i < 10000; i++) {
    j = lrand48 () % n;
    assert (vocab_add (v, words[j]) == 0);
    c[j]++;
  }
  assert (v->len == n);
  assert (vocab_shrink (v) == 0);
  assert (v->len == n);

  assert (vocab_add (v, "spider") == 0);
  assert (vocab_add (v, "skunk") == 0);
  assert (vocab_add (v, "wallaby") == 0);
  assert (v->len == n + 3);
  assert (vocab_shrink (v) == 0);
  assert (v->len == n);

  for (i = 0; i < n; i++) {
    assert (vocab_get_index (v, words[i], &j) == 0);
    assert (v->pool[j].count == c[i]);
  }
  assert (vocab_save (v, "vocab.bin") == 0);
  vocab_free (v);

  v = vocab_new ();
  assert (v != NULL);
  assert (v->len == 0);
  assert (vocab_load (v, "vocab.bin") == 0);
  assert (v->len == n);
  for (i = 0; i < n; i++) {
    assert (vocab_get_index (v, words[i], &j) == 0);
    assert (v->pool[j].count == c[i]);
  }
  vocab_free (v);
  return EXIT_SUCCESS;
}
