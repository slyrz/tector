#include "options.h"
#include "string.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

extern struct command command;

#define makeoption(c,n,a) \
  [(c) - 'a'] = {n, a, NULL, c}

static struct option options[32] = {
  makeoption ('c', "corpus", required_argument),
  makeoption ('h', "help", no_argument),
  makeoption ('i', "iterations", required_argument),
  makeoption ('l', "layers", required_argument),
  makeoption ('m', "mincount", required_argument),
  makeoption ('n', "neuralnetwork", required_argument),
  makeoption ('r', "rebuild", no_argument),
  makeoption ('v', "vocab", required_argument),
  makeoption ('w', "window", required_argument),
};

/**
 * This array stores the optargs of the parsed options. Active options without
 * argument will have the empty string as their value. If the option wasn't
 * present, the value will be NULL.
 */
static const char *values[32];

/**
 * The index of options is the alphabetical index of the option's
 * short flag. This function returns 31 for stuff that isn't
 * an alphabetical character, so arrays need 32 instead of 26
 * elements and the element at index 31 stores the trash.
 */
static int
idx (int c)
{
  return ((ordalpha (c) - 1) & 31);
}

static const char *
val (int c)
{
  return values[idx (c)];
}

static void
print_option (const struct option *opt)
{
  const char *arg[] = {
    [no_argument] = "",
    [optional_argument] = "[ARG]",
    [required_argument] = "<ARG>",
  };

  fprintf (stderr, "   -%c, --%s %s\n", opt->val, opt->name, arg[opt->has_arg]);
}

static void
print_usage_and_exit (struct option *longopts)
{
  fprintf (stderr, "%s [OPTION...] %s\n\nOption:\n", command.name, command.args);
  while (longopts->name)
    print_option (longopts++);
  exit (0);
}

static void
append (char *shortopts, struct option *longopts, const struct option *src)
{
  while (longopts->name)
    longopts++;
  memcpy (longopts, src, sizeof (struct option));

  while (*shortopts)
    shortopts++;
  *shortopts++ = (char) src->val;
  switch (src->has_arg) {
    case optional_argument:
      *shortopts++ = ':';
      *shortopts++ = ':';
      break;
    case required_argument:
      *shortopts++ = ':';
      break;
  }
  *shortopts = '\0';
}

int
options_parse (int argc, char **argv)
{
  const char *optstring = command.opts;

  char shortopts[64];
  struct option longopts[32];

  unsigned int b = 0;
  int i = 0;
  int c = 0;

  memset (shortopts, 0, sizeof (shortopts));
  memset (longopts, 0, sizeof (longopts));

  /**
   * Bucket sort options. Avoids adding the same option multiple times.
   * Then clear the -help flag because we want to add it at the end. Also
   * clear the trash bit.
   */
  while (*optstring) {
    b |= 1u << idx (*optstring);
    optstring++;
  }
  b &= ~(1u << idx ('h'));
  b &= ~(1u << 31);

  while (b) {
    if (b & 1)
      append (shortopts, longopts, options + i);
    b >>= 1;
    i++;
  }
  append (shortopts, longopts, options + idx ('h'));

  i = 0;
  for (;;) {
    c = getopt_long (argc, argv, shortopts, longopts, &i);
    if ((c < 0) || (c == 'h') || (c == '?'))
      break;
    /**
     * If the option has no argument, set the value to an empty string,
     * so that the non-NULL value tells us that the option was present.
     */
    values[idx (c)] = optarg ? optarg : "";
  }
  values[31] = NULL;

  if (c > 0)
    print_usage_and_exit (longopts);

  return optind;
}

void
options_get_str (char c, const char **r)
{
  if (val (c) == NULL)
    return;
  *r = val (c);
}

void
options_get_bool (char c, int *r)
{
  *r = (val (c) != NULL);
}

#define parseint(s,o,t,m) \
  do { \
    char *e; \
    unsigned long v; \
    errno = 0; \
    v = strtoul (s, &e, 10); \
    if ((errno == 0) && (*e == '\0') && (v <= (m))) \
      (o) = (t) v; \
  } while (0)

void
options_get_int (char c, int *r)
{
  if (val (c) == NULL)
    return;
  parseint (val (c), *r, int, INT_MAX);
}

void
options_get_size_t (char c, size_t *r)
{
  if (val (c) == NULL)
    return;
  parseint (val (c), *r, size_t, SIZE_MAX);
}
