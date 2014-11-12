#include "../src/core/vocab.h"
#include "../src/core/corpus.h"
#include "../src/core/serialize.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

const size_t words[] = {
  1, 0,
  1, 3,
  1, 2,
  1, 1,
  4, 2, 2, 0, 1,
  2, 0, 0,
  1, 3,
  2, 2, 0,
  5, 3, 1, 3, 2, 3,
  1, 2,
  3, 3, 0, 1,
  3, 3, 3, 2,
  2, 2, 1,
  2, 3, 0,
  2, 2, 1,
  1, 0,
  1, 1,
  4, 1, 0, 2, 2,
  1, 1,
  1, 2,
};

static void
verify (struct corpus *c)
{
  size_t i;
  size_t j;
  size_t k;

  assert (memcmp (c->words.ptr, words, c->words.len * sizeof (size_t)) == 0);
  for (i = j = 0; i < c->sentences.len; i++) {
    assert (c->sentences.ptr[i]->len == c->words.ptr[j++]);
    for (k = 0; k < c->sentences.ptr[i]->len; k++)
      assert (c->sentences.ptr[i]->words[k] == c->words.ptr[j++]);
  }
}

int
main (void)
{
  struct corpus *c;
  struct vocab *v;
  int i;

  v = vocab_new ();
  assert (v != NULL);
  assert (vocab_add (v, "cat") == 0);
  assert (vocab_add (v, "dog") == 0);
  assert (vocab_add (v, "frog") == 0);
  assert (vocab_add (v, "mous") == 0);

  c = corpus_new (v);
  assert (c != NULL);
  assert (corpus_parse (c, "tests/testdata/corpus.txt") == 0);
  verify (c);
  assert (corpus_save (c, "/tmp/corpus.bin") == 0);
  corpus_free (c);

  c = corpus_new (v);
  assert (c != NULL);
  assert (corpus_load (c, "/tmp/corpus.bin") == 0);
  verify (c);
  corpus_free (c);

  assert (vocab_add (v, "spider") == 0);

  c = corpus_new (v);
  assert (c != NULL);
  assert (corpus_load (c, "/tmp/corpus.bin") != 0);
  corpus_free (c);

  c = corpus_new (v);
  for (i = 0; i < 500; i++)
    assert (corpus_parse (c, "tests/testdata/corpus.txt") == 0);
  corpus_free (c);

  vocab_free (v);
  return EXIT_SUCCESS;
}
