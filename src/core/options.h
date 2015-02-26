#ifndef CORE_OPTIONS_H
#define CORE_OPTIONS_H

#include <stdlib.h>

struct command {
  const char *name;
  const char *args;
  const char *opts;
  int (*main) (int argc, char **argv);
};

struct program {
  const char *name;
  const char *info;
  const struct command commands[];
};

int options_parse (int argc, char **argv);
void options_get_str (char c, const char **r);
void options_get_bool (char c, int *r);
void options_get_int (char c, int *r);
void options_get_size_t (char c, size_t * r);

#endif
