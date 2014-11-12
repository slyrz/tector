#include "../src/core/vocab.h"
#include "../src/core/serialize.h"
#include "../src/core/filter.h"

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

static char*
randstr (void)
{
  static char b[64 + 1];

  long int r = lrand48 ();
  int i;

  for (i = 0; i < sizeof (b) - 1; i++) {
    if (r == 0)
      r = lrand48 ();
    b[i] = 'a' + (r % 26); r >>= 1;
  }
  b[i] = '\0';
  return b;
}

int
main (void)
{
  const size_t n = len (words);
  const size_t x = 100000;
  size_t c[n];
  size_t i;
  size_t j;
  struct vocab *v;

  memset (c, 0, n * sizeof (size_t));

  for (i = 0; i < n; i++)
    filter (words[i]);

  v = vocab_new ();
  assert (v != NULL);
  assert (v->len == 0);
  assert (vocab_parse (v, "tests/testdata/vocab.txt") == 0);
  assert (v->len == n);

  for (i = 0; i < n; i++)
    v->entries[i].count = 0;

  for (i = 0; i < 10000; i++) {
    j = (size_t) lrand48 () % n;
    assert (vocab_add (v, words[j]) == 0);
    c[j]++;
  }
  assert (v->len == n);
  assert (vocab_shrink (v, 2) == 0);
  assert (v->len == n);

  for (i = 0; i < x; i++)
    vocab_add (v, randstr ());

  assert (v->len == x + n);
  assert (vocab_shrink (v, 2) == 0);
  assert (v->len == n);
  assert (vocab_encode (v) == 0);

  for (i = 0; i < n; i++) {
    assert (vocab_find (v, words[i], &j) == 0);
    assert (v->entries[j].count == c[i]);
  }
  assert (vocab_save (v, "/tmp/vocab.bin") == 0);
  vocab_free (v);

  v = vocab_new ();
  assert (v != NULL);
  assert (v->len == 0);
  assert (vocab_load (v, "/tmp/vocab.bin") == 0);
  assert (v->len == n);
  for (i = 0; i < n; i++) {
    assert (vocab_find (v, words[i], &j) == 0);
    assert (v->entries[j].count == c[i]);
  }
  vocab_free (v);
  return EXIT_SUCCESS;
}
