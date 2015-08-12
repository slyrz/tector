#ifndef TECTOR_SCANNER_H
#define TECTOR_SCANNER_H

#include <stdlib.h>

/**
 * Scanner reads clean lines of ASCII text from a file descriptor.
 * Non-ASCII characters and consecutive spaces will be ignored.
 */
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
