#include "../src/core/filter.h"

#include <string.h>
#include <stdlib.h>
#include <err.h>

#define test(inp,out) \
  do { \
    char buf[1024]; \
    strcpy (buf, inp); \
    if (strcmp (filter (buf), out) != 0) \
      err (EXIT_FAILURE, "expected '%s', got '%s'", out, buf); \
  } while (0);

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
  test ("testing tests", "test test");
  test ("test test   \0test", "test test");
  test ("test test t#es!t", "test test test");
  test ("test test t#e_s!t", "test test");
  test ("i test a am of test we they were t#e_s!t", "test test");
  test ("testing testing testing", "test test test");
  test ("riverrun, past Eve and Adam's, from swerve of shore to bend "
        "of bai, brings us by a commodius vicus of recirculation back to "
        "Howth Castle and Environs.",
        "riverrun past ev adam swerv shore bend bai bring us commodiu vicu "
        "recircul back howth castl environ");
  test ("Sir Tristram, violer d'amores, fr'over the short sea, had "
        "passencore rearrived from North Armorica on this side the scraggy "
        "isthmus of Europe Minor to wielderfight his penisolate war:",
        "sir tristram violer damor frover short sea passencor rearriv "
        "north armorica side scraggi isthmu europ minor wielderfight "
        "penisol war");
  return EXIT_SUCCESS;
}
