#include "options.h"
#include "string.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

extern struct program program;

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

static int
uninitialized (const struct command *c)
{
  char *p = (char *) c;
  int i;

  for (i = 0; i < sizeof (struct command); i++)
    if (p[i] != 0)
      return 0;
  return 1;
}

static void
print_option (FILE * stream, const struct option *o)
{
  const char *arg[] = {
    [no_argument] = "",
    [optional_argument] = "[ARG]",
    [required_argument] = "<ARG>",
  };

  fprintf (stream, "   -%c, --%s %s\n", o->val, o->name, arg[o->has_arg]);
}

static void
print_command (FILE * stream, const struct command *c)
{
  fprintf (stream, "\t%s", program.name);
  if (c->name)
    fprintf (stream, " %s", c->name);
  if (c->opts)
    fprintf (stream, " [-%s]", c->opts);
  if (c->args)
    fprintf (stream, " %s", c->args);
  fputc ('\n', stream);
}

static void
print_usage_and_exit (FILE * stream, const struct command *c, struct option *o)
{
  int i;

  fprintf (stream, "%s", program.name);
  if (program.info)
    fprintf (stream, " - %s\n", program.info);
  else
    fputc ('\n', stream);
  fprintf (stream, "\nUsage:\n");

  if (c != NULL) {
    print_command (stream, c);
  }
  else {
    i = 0;
    for (;;) {
      if (uninitialized (program.commands + i))
        break;
      print_command (stream, program.commands + i);
      i++;
    }
  }
  if (o) {
    fprintf (stream, "\nOptions:\n");
    while (o->name)
      print_option (stream, o++);
  }
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

static int
parse (const struct command *c, int argc, char **argv)
{
  const char *optstring;

  char shortopts[64];
  struct option longopts[32];

  unsigned int b = 0;
  int i = 0;
  int v = 0;

  memset (shortopts, 0, sizeof (shortopts));
  memset (longopts, 0, sizeof (longopts));

  if (c->opts)
    optstring = c->opts;
  else
    optstring = "";

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

  /**
   * This trickery lets getopt start parsing flags after the command name,
   * otherwise getopt would stops parsing immediately.
   */
  optind = 1;
  if (c->name != NULL)
    optind++;

  i = 0;
  for (;;) {
    v = getopt_long (argc, argv, shortopts, longopts, &i);
    if ((v < 0) || (v == 'h') || (v == '?'))
      break;
    /**
     * If the option has no argument, set the value to an empty string,
     * so that the non-NULL value tells us that the option was present.
     */
    values[idx (v)] = optarg ? optarg : "";
  }
  values[31] = NULL;
  if (v > 0)
    print_usage_and_exit (stdout, c, longopts);
  return optind;
}

int
options_parse (int argc, char **argv)
{
  const struct command *c;
  int i;

  /**
   * Find the command whose name matches the first argument or the first
   * command with unspecified name.
   */
  c = program.commands;
  while (c->name) {
    if ((argv[1]) && (strcmp (c->name, argv[1]) == 0))
      break;
    c++;
  }
  if (uninitialized (c))
    goto error;

  // Always parse options, so that every command can handle -h.
  i = parse (c, argc, argv);
  if (c->main)
    exit (c->main (argc - i, argv + i));
  return i;
error:
  print_usage_and_exit (stderr, NULL, NULL);
  return -1;
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
options_get_size_t (char c, size_t * r)
{
  if (val (c) == NULL)
    return;
  parseint (val (c), *r, size_t, SIZE_MAX);
}
