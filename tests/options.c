#include "../src/core/options.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#define len(x) \
  (sizeof (x) / sizeof (x[0]))

const struct command command = {
  .name = "test",
  .args = "",
  .opts = "cv",
};

static char *argv[] = {
  "a.out", "-v", "232092", "a", "b", "c",  NULL,
};

union val {
  const char *s;
  int b;
  int i;
  size_t z;
};

int
main (void)
{
  int k;
  union val v;

  k = options_parse (len(argv)-1, argv);

  options_get_bool ('v', &v.b);
  assert (v.b == 1);
  options_get_size_t ('v', &v.z);
  assert (v.z == 232092);
  options_get_int ('v', &v.i);
  assert (v.i == 232092);
  options_get_str ('v', &v.s);
  assert (strcmp (v.s, "232092") == 0);

  v.b = 1;
  options_get_bool ('c', &v.b);
  assert (v.b == 0);
  v.z = 1234;
  options_get_size_t ('c', &v.z);
  assert (v.z == 1234);
  v.i = 1234;
  options_get_int ('c', &v.i);
  assert (v.i == 1234);
  v.s = "abc";
  options_get_str ('c', &v.s);
  assert (strcmp (v.s, "abc") == 0);

  return EXIT_SUCCESS;
}
