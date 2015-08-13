#include "../src/vocab.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#define len(x) \
  (sizeof (x) / sizeof (x[0]))

struct test_vector {
  char *word;
  uint64_t count;
  uint64_t code;
  uint32_t point[5];
};

/**INDENT-OFF**/
static struct test_vector test_vectors[] = {
  { "mouse",   58, 0x0000000e, {10, 8, 5, 0, 0} },
  { "rabbit",  51, 0x0000000a, {10, 8, 5, 0, 0} },
  { "octopus", 46, 0x0000000c, {10, 8, 4, 0, 0} },
  { "dog",     45, 0x00000008, {10, 8, 4, 0, 0} },
  { "giraffe", 45, 0x0000001f, {10, 9, 7, 3, 0} },
  { "cat",     43, 0x00000017, {10, 9, 7, 3, 0} },
  { "frog",    39, 0x0000001b, {10, 9, 7, 2, 0} },
  { "turtle",  37, 0x00000013, {10, 9, 7, 2, 0} },
  { "bear",    37, 0x0000001d, {10, 9, 6, 1, 0} },
  { "baboon",  35, 0x00000015, {10, 9, 6, 1, 0} },
  { "mole",    34, 0x00000019, {10, 9, 6, 0, 0} },
  { "donkey",  30, 0x00000011, {10, 9, 6, 0, 0} },
};
/**INDENT-ON**/

int
main (void)
{
  struct vocab *v;
  size_t i;
  size_t j;

  v = vocab_new ();
  v->min = 0;
  assert (v != NULL);
  assert (vocab_parse (v, "tests/testdata/vocab.txt") == 0);
  assert (v->len == len (test_vectors));
  assert (vocab_save (v, "/tmp/vocab.bin") == 0);
  vocab_free (v);

  v = vocab_open ("/tmp/vocab.bin");
  for (i = 0; i < len (test_vectors); i++) {
    assert (strcmp (v->entries[i].word, test_vectors[i].word) == 0);
    assert (memcmp (v->entries[i].point, test_vectors[i].point, sizeof (test_vectors[i].point)) == 0);
    assert (v->entries[i].count == test_vectors[i].count);
    assert (v->entries[i].code == test_vectors[i].code);
  }
  vocab_free (v);

  return EXIT_SUCCESS;
}
