#include "config.h"
#include "filter.h"
#include "stopwords.h"
#include "stem.h"
#include "string.h"

#include <string.h>

/**
 * Filter 1:
 *  - convert upper- to lowercase
 *  - remove all non-alphabetic characters
 *  - split hyphenated words
 *  - discard the filtered word if it doesn't contain at least two characters,
 *    or if more than two characters were filtered or if it consists of
 *    a single character only (e.g. "aaaaa").
 */
static inline int
filter01 (char *restrict w, char **n)
{
  unsigned int v = 0;
  char *d = w;
  int c = 0;

  while (*w) {
    *d = *w;
    if (isupper (*d))
      *d += ('a' - 'A');
    if (islower (*d)) {
      v |= 1 << ((*d - 'a') & 0x1f);
      d++;
      c++;
    }
    if (*w == '-')
      break;
    w++;
  }

  if (*w)
    *n = w + 1;

  *d = '\0';

  /**
   * Discard if the remaining word has a length <= 1 or
   * more then 2 chars were filtered.
   */
  return (c <= 1) || ((w - d) > 2) || ((v & (v - 1)) == 0);
}

/**
 * Filter 2:
 *  - remove stopwords.
 */
static inline int
filter02 (char *restrict w, char **n)
{
  return isstopword (w);
}

/**
 * Filter 3:
 *  - stem words.
 */
static inline int
filter03 (char *restrict w, char **n)
{
  return (stem (w) <= 1);
}

#define perform(f,d,b) \
  do { \
    if (f (d,b) != 0) { \
      *d = '\0'; \
      goto abort; \
    } \
  } while (0)

/**
 * Performs various filters on a word. This uses memmove and
 * thus can work in-place. The filtered word will never exceed the unfiltered
 * word, so no buffer overflows.
 */
static int
filterword (char *dst, char *src, int l)
{
  char *br = NULL;
  int k = 0;

  if (*src == '\0')
    return 0;

  if (dst != src)
    memmove (dst, src, l);
  dst[l] = '\0';

  perform (filter01, dst, &br);
  perform (filter02, dst, &br);
  perform (filter03, dst, &br);
  k = strlen (dst);
abort:
  if (br) {
    if (k)
      dst[k++] = ' ';
    k += filterword (dst + k, br, l - (br - dst));
  }
  return k;
}

char *
filter (char *src)
{
  char *inp = src;
  char *out = src;
  char *w;
  int d;

  while (w = strtok_r (inp, " ", &inp), w) {
    d = filterword (out, w, strlen (w));
    if (d) {
      out += d;
      if (*inp)
        *out++ = ' ';
    }
  }
  nullterm (src, out - src);
  return src;
}
