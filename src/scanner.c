#include "config.h"
#include "scanner.h"
#include "string.h"
#include "mem.h"

#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

static int
fetch (struct scanner *s)
{
  ssize_t r;

  for (;;) {
    if (r = read (s->fd, s->data, sizeof (s->data)), r <= 0) {
      if ((r == -1) && (errno == EINTR))
        continue;
      return -1;
    }
    break;
  }
  s->len = (size_t) r;
  s->pos = 0;
  return 0;
}

static inline bool
endofbuffer (struct scanner *s)
{
  return (s->pos >= s->len);
}

static inline bool
endoffile (struct scanner *s)
{
  return endofbuffer (s) && fetch (s);
}

static inline int
get (struct scanner *s)
{
  if (endoffile (s))
    return -1;
  return s->data[s->pos++];
}

static inline int
peek (struct scanner *s)
{
  if (endoffile (s))
    return -1;
  return s->data[s->pos];
}

struct scanner *
scanner_new (int fd)
{
  struct scanner *s;

  if (fcntl (fd, F_GETFD) != 0)
    return NULL;
  s = mem_alloc (sizeof (struct scanner), 1);
  if (s == NULL)
    return NULL;
  s->fd = fd;
  return s;
}

struct scanner *
scanner_open (const char *path)
{
  int fd;

  fd = open (path, O_RDONLY);
  if (fd < 0)
    return NULL;
  return scanner_new (fd);
}

void
scanner_free (struct scanner *s)
{
  if (s->fd >= 0)
    close (s->fd);
  mem_free (s);
}

int
scanner_rewind (struct scanner *s)
{
  s->pos = 0;
  s->len = 0;
  if (lseek (s->fd, SEEK_SET, 0) < 0)
    return -1;
  return 0;
}

int
scanner_readline (struct scanner *s, char *b, size_t l)
{
  size_t i = 0;
  int c;
  int n;

  *b = '\0';

  if (endoffile (s))
    return -1;

  while (c = get (s), c >= 0) {
    /* Discard anything that isn't ASCII. */
    if (isunicode (c)) {
      while (c = peek (s), (c > 0) && (isunicode (c)))
        get (s);
      continue;
    }
    /**
     * Ignore whitespace. If a newline is part of the processed whitespace,
     * stop.
     */
    if (isspace (c)) {
      n = (c == '\n');
      while (c = peek (s), (c > 0) && (isspace (c)))
        n |= (get (s) == '\n');
      if (i == 0)
        continue;
      /* Newline found? Stop. */
      if (n)
        break;
      /**
       * If there's a space already at the end of the buffer,
       * avoid writing another.
       */
      if (b[i - 1] == ' ')
        continue;
      c = ' ';
    }
    if (i < l)
      b[i++] = (char) c;
  }

  /* Buffer can't hold line. Ignore it and return the next line. */
  if (i >= l)
    return scanner_readline (s, b, l);

  /* Replace space at the end of buffer with null-terminator. */
  nullterm (b, i);
  return 0;
}
