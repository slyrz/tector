#include "config.h"
#include "scanner.h"
#include "string.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define eof(s) \
  ((s)->pos >= (s)->len)

#define get(s) \
  ((eof (s)) ? -1 : (s)->data[(s)->pos++])

#define peek(s) \
  ((eof (s)) ? -1 : (s)->data[(s)->pos])

static int
getfilesize (const char *path)
{
  struct stat sbuf;
  if (stat (path, &sbuf) != 0)
    return 0;
  return sbuf.st_size;
}

struct scanner *
scanner_new (const char *path)
{
  struct scanner *s = NULL;

  s = calloc (sizeof (struct scanner), 1);
  if (s == NULL)
    goto error;

  s->fd = open (path, O_RDONLY);
  if (s->fd < 0)
    goto error;

  s->len = getfilesize (path);
  s->pos = 0;

  s->data = mmap (0, s->len, PROT_READ, MAP_SHARED, s->fd, 0);
  if (s->data == MAP_FAILED)
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
  if (s->data)
    munmap (s->data, s->len);
  close (s->fd);
  free (s);
}

void
scanner_rewind (struct scanner *s)
{
  s->pos = 0;
}

int
scanner_readline (struct scanner *s, char *b, size_t l)
{
  size_t i = 0;
  int c;
  int n;

  *b = '\0';

  if (eof (s))
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
      b[i++] = c;
  }

  /* Buffer can't hold line. */
  if (i >= l)
    i = 0;

  /* Remove space at the end of buffer with null. */
  nullterm (b, i);
  return 0;
}
