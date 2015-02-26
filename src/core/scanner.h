#ifndef CORE_SCANNER_H
#define CORE_SCANNER_H

#include <stdlib.h>

struct scanner {
  int fd;
  size_t len;
  size_t pos;
  unsigned char data[8192];
};

struct scanner *scanner_new (int fd);
struct scanner *scanner_open (const char *path);
void scanner_free (struct scanner *s);

int scanner_rewind (struct scanner *s);
int scanner_readline (struct scanner *s, char *buf, size_t l);

#endif
