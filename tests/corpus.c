#include "../src/core/vocab.h"
#include "../src/core/corpus.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

const size_t words[] = {
  4, 2, 2, 0, 1,
  2, 0, 0,
  2, 2, 0,
  5, 3, 1, 3, 2, 3,
  3, 3, 0, 1,
  3, 3, 3, 2,
  2, 2, 1,
  2, 3, 0,
  2, 2, 1,
  4, 1, 0, 2, 2,
};

int
main (void)
{
  struct corpus *c;
  struct vocab *v;

  size_t i;
  size_t j;
  size_t k;

  v = vocab_new ();
  assert (v != NULL);
  assert (vocab_add (v, "cat") == 0);
  assert (vocab_add (v, "dog") == 0);
  assert (vocab_add (v, "frog") == 0);
  assert (vocab_add (v, "mouse") == 0);

  c = corpus_new (v);
  assert (c != NULL);
  assert (corpus_parse (c, "tests/testdata/corpus.txt") == 0);

  assert (memcmp (c->words.ptr, words, c->words.len * sizeof (size_t)) == 0);
  for (i = j = 0; i < c->sentences.len; i++) {
    assert (c->sentences.ptr[i]->len == c->words.ptr[j++]);
    for (k = 0; k < c->sentences.ptr[i]->len; k++)
      assert (c->sentences.ptr[i]->words[k] == c->words.ptr[j++]);
  }
  corpus_free (c);
  return EXIT_SUCCESS;
}
