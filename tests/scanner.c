#include "../src/scanner.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

const char *lines[] = {
  "this is sentence one",
  "this is sentence two",
  "this is sentence three",
  "this is sentence four",
  "this is sentence five",
};

static void
test (const char *path)
{
  struct scanner *s;
  char b[1024];
  int i;

  s = scanner_open (path);
  assert (s != NULL);
  i = 0;
  while (scanner_readline (s, b, sizeof (b)) == 0)
    assert ((i < 5) && (strcmp (lines[i++], b) == 0));
  assert (i == 5);
  scanner_rewind (s);
  i = 0;
  while (scanner_readline (s, b, sizeof (b)) == 0)
    assert ((i < 5) && (strcmp (lines[i++], b) == 0));
  assert (i == 5);
  scanner_free (s);
}

int
main (void)
{
  test ("tests/testdata/scanner_clean.txt");
  test ("tests/testdata/scanner_dirty.txt");
  test ("tests/testdata/scanner_large.txt");
  return EXIT_SUCCESS;
}
