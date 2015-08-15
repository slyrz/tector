#include "config.h"
#include "stem.h"
#include "stopwords.h"
#include "string.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int
isupper (int c)
{
  return ((unsigned int) c - 'A') < 26;
}

int
islower (int c)
{
  return ((unsigned int) c - 'a') < 26;
}

int
isspace (int c)
{
  return (c <= ' ') || (c == 127);
}

int
isalpha (int c)
{
  return islower (c | 0x20);
}

int
isunicode (int c)
{
  return c & 0x80;
}

char
lowercase (int c)
{
  if (isupper (c))
    return (char) (c | 0x20);
  return (char) c;
}

char
uppercase (int c)
{
  if (islower (c))
    return (char) (c & 0x5f);
  return (char) c;
}

int
ordalpha (int c)
{
  c |= 0x20;
  if (islower (c))
    return 1 + (c - 'a');
  return 0;
}

char *
nullterm (char *s, size_t l)
{
  if ((l > 0) && (s[l - 1] == ' '))
    l--;
  s[l] = '\0';
  return s;
}

static inline int
log1024 (unsigned long v)
{
  int e = 0;
  e += (v >= 1024);
  v /= 1024;
  e += (v >= 1024);
  v /= 1024;
  e += (v >= 1024);
  v /= 1024;
  e += (v >= 1024);
  return e;
}

char *
formatsize (char *s, size_t v)
{
  const int w = log1024 (v);
  const size_t x = v >> (w * 10);
  sprintf (s, "%zu %sB", x, &"\0\0K\0M\0G\0T\0"[w << 1]);
  return s;
}

/**
 * Cleans and filters a word. This function can work in-place.
 * The filtered word will never exceed the unfiltered word, so no
 * buffer overflows.
 */
static size_t
filterword (char *dst, const char *restrict src)
{
  bool split = false;
  int i = 0;
  int j = 0;
  int mask = 0;

  /**
   * First write the characters from src to dst. Filter everything that isn't
   * part of the Latin alphabet [a-z]. Also keep track of what letters were read
   * in the bitmask mask.
   */
  while (src[i] != '\0') {
    /* Split words at dashes. */
    if (src[i] == '-') {
      split = true;
      break;
    }
    if (isalpha (src[i])) {
      dst[j] = lowercase (src[i]);
      mask |= 1 << ordalpha (dst[j]);
      j++;
    }
    i++;
  }
  dst[j] = '\0';

  /**
   * Ignore the word if it has less than 2 characters.
   */
  if (j < 2)
    j = 0;

  /**
   * Ignore the word if it contained more than 2 non-alphabetic
   * characters.
   */
  if ((i - j) > 2)
    j = 0;

  /**
   * Ignore the word if it consists of a single letter only (e.g. "aaaaa"),
   * this means only 1 bit in mask is set.
   */
  if ((mask & (mask - 1)) == 0)
    j = 0;

  /**
   * Ignore the word if it is a stopword.
   */
  if (j > 0) {
    if (isstopword (dst))
      j = 0;
  }

  /**
   * Stem the word. Ignore it if the stemmed word has less than 2 characters.
   */
  if (j > 0) {
    if (j = stem (dst), j < 2)
      j = 0;
  }

  /**
   * If processing stopped at a dash, split the word and call this function
   * on the remaining text.
   */
  if (split) {
    if (j > 0)
      dst[j++] = ' ';
    j += filterword (dst + j, src + i + 1);
  }
  return j;
}

char *
filter (char *restrict src)
{
  char *inp = src;
  char *out = src;
  char *word;
  size_t n;

  while (word = strtok_r (inp, " ", &inp), word) {
    n = filterword (out, word);
    if (n > 0) {
      out += n;
      if (*inp)
        *out++ = ' ';
    }
  }
  nullterm (src, (size_t) (out - src));
  return src;
}
