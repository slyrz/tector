#ifndef TECTOR_PROGRAM_H
#define TECTOR_PROGRAM_H

#include <stdlib.h>

struct command {
  const char *name;
  const char *args;
  const char *opts;
  void (*main) (void);
};

struct program {
  const char *name;
  const char *info;
  const struct command commands[];
};

#define program_init(argc,argv) \
  do { \
    int argidx = program_parseargs (argc, argv); \
    if (argidx > 0) { \
      argv += argidx; \
      argc -= argidx; \
    } \
  } while (0)

int program_parseargs (int argc, char **argv);
char* program_poparg (void);

void program_run (void);
void program_getoptstr (char c, const char **r);
void program_getoptbool (char c, int *r);
void program_getoptint (char c, int *r);
void program_getoptuint (char c, unsigned int *r);

#endif
