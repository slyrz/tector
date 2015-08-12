#include "../src/file.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define TEST_PATH   "/tmp/testfile"
#define TEST_TEXT   "Hello World"
#define TEST_TYPE   1234

int
main (void)
{
  struct file *f;
  char buf[1024];
  size_t i;

  f = file_create (TEST_PATH);
  assert (f != NULL);
  f->header.type = TEST_TYPE;
  for (i = 0; i < 5; i++)
    f->header.data[i] = i;
  file_writestr (f, TEST_TEXT);
  file_close (f);

  f = file_open (TEST_PATH);
  assert (f != NULL);
  assert (f->header.type == TEST_TYPE);
  for (i = 0; i < 5; i++)
    assert (f->header.data[i] == i);
  assert (file_readstr (f, buf, sizeof (buf)) == 0);
  assert (strcmp (buf, TEST_TEXT) == 0);
  file_close (f);

  for (i = 0; i < 100; i++)
    assert (file_open ("/dev/urandom") == NULL);

  return EXIT_SUCCESS;
}
