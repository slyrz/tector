#include "../src/string.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define test(inp,out) \
  do { \
    char buf[1024]; \
    strcpy (buf, inp); \
    assert (strcmp (filter (buf), out) == 0); \
  } while (0)

int
main (void)
{
  test ("", "");
  test ("test", "test");
  test ("  test", "test");
  test ("  test  ", "test");
  test ("Test", "test");
  test ("TEST", "test");
  test ("test test", "test test");
  test ("-test-", "test");
  test ("test-test", "test test");
  test ("testing tests", "test test");
  test ("test test   \0test", "test test");
  test ("test test t#es!t", "test test test");
  test ("test test t#e_s!t", "test test");
  test ("i test a am of test we they were t#e_s!t", "test test");
  test ("testing testing testing", "test test test");
  test ("http://www.example.org/", "");
  test ("www.example.org", "wwwexampleorg");
  return EXIT_SUCCESS;
}
