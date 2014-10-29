#include "config.h"
#include "scanner.h"
#include "string.h"

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

static inline int
endofbuffer (struct scanner *s)
{
  return (s->pos >= s->len);
}

static inline int
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
scanner_new (const char *path)
{
  struct scanner *s;

  s = calloc (sizeof (struct scanner), 1);
  if (s == NULL)
    goto error;
  s->fd = open (path, O_RDONLY);
  if (s->fd < 0)
    goto error;
  return s;
error:
  if (s)
    scanner_free (s);
  return NULL;
}

void
scanner_free (struct scanner *s)
{
  if (s->fd >= 0)
    close (s->fd);
  free (s);
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

  while (c = get (s), c > 0) {
    if (isunicode (c)) {
      while (c = peek (s), (c > 0) && (isunicode (c)))
        get (s);
      continue;
    }
    if (isspace (c)) {
      n = (c == '\n');
      while (c = peek (s), (c > 0) && (isspace (c)))
        n |= (get (s) == '\n');
      if (i == 0)
        continue;
      if (n)
        break;
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

  /* Remove space at the end of buffer with null. */
  nullterm (b, i);
  return 0;
}
