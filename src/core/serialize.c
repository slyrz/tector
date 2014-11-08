#include "serialize.h"
#include "log.h"

#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


const int flags_read = O_RDONLY;
const int flags_write = O_CREAT | O_TRUNC | O_WRONLY;

const uint32_t magic = 0x793981e5;

/**
 * Wrapper around read/write, returns 0 on success and -1 on failure.
 */
#define check(func,fd,ptr,size) \
  (-(func (fd, ptr, size) != (ssize_t) (size)))

struct header {
  uint64_t checksum;
  uint64_t size[4];
};

static inline uint64_t
checksum (uint64_t s[4])
{
  uint64_t h = 0x14c4f52f026b86c8ull;
  h ^= s[0];
  h *= 1099511628211ull;
  h ^= s[1];
  h *= 1099511628211ull;
  h ^= s[2];
  h *= 1099511628211ull;
  h ^= s[3];
  h *= 1099511628211ull;
  return h;
}

static int
header_vread (int fd, size_t n, va_list ap)
{
  struct header h;
  size_t i;

  memset (&h, 0, sizeof (struct header));
  if (check (read, fd, &h, sizeof (struct header)) != 0)
    return -1;
  if (h.checksum != checksum (h.size))
    return -1;
  for (i = 0; i < n; i++)
    *(va_arg (ap, size_t *)) = (size_t) h.size[i];
  return 0;
}

static int
header_vwrite (int fd, size_t n, va_list ap)
{
  struct header h;
  size_t i;

  memset (&h, 0, sizeof (struct header));
  for (i = 0; i < n; i++)
    h.size[i] = (uint64_t) va_arg (ap, size_t);
  h.checksum= checksum (h.size);
  if (check (write, fd, &h, sizeof (struct header)) != 0)
    return -1;
  return 0;
}

static int
header_read (int fd, size_t n, ...)
{
  va_list ap;
  int r;

  va_start (ap, n);
  r = header_vread (fd, n, ap);
  va_end (ap);
  return r;
}

static int
header_write (int fd, size_t n, ...)
{
  va_list ap;
  int r;

  va_start (ap, n);
  r = header_vwrite (fd, n, ap);
  va_end (ap);
  return r;
}


int
vocab_load (struct vocab *v, const char *path)
{
  int fd;

  fd = open (path, flags_read);
  if (fd == -1)
    return -1;
  if (header_read (fd, 1, &v->len) != 0)
    goto error;
  if (vocab_alloc (v) != 0)
    goto error;
  if (check (read, fd, v->pool, v->len * sizeof (struct vocab_entry)) != 0)
    goto error;
  if (vocab_build (v) != 0)
    goto error;
  close (fd);
  return 0;
error:
  if (fd >= 0)
    close (fd);
  return -1;
}

int
vocab_save (struct vocab *v, const char *path)
{
  int fd;

  fd = open (path, flags_write, 0666);
  if (fd == -1)
    return -1;
  if (header_write (fd, 1, v->len) != 0)
    goto error;
  if (check (write, fd, v->pool, v->len * sizeof (struct vocab_entry)) != 0)
    goto error;
  close (fd);
  return 0;
error:
  if (fd >= 0)
    close (fd);
  return -1;
}

int
corpus_load (struct corpus *c, const char *path)
{
  int fd;

  fd = open (path, flags_read);
  if (fd == -1)
    return -1;
  if (header_read (fd, 2, &c->words.len, &c->sentences.len) != 0)
    goto error;
  if (corpus_alloc (c) != 0)
    goto error;
  if (check (read, fd, c->words.ptr, c->words.len * sizeof (size_t)) != 0)
    goto error;
  if (corpus_build (c) != 0)
    goto error;
  close (fd);
  return 0;
error:
  if (fd >= 0)
    close (fd);
  return -1;
}

int
corpus_save (struct corpus *c, const char *path)
{
  int fd;

  fd = open (path, flags_write, 0666);
  if (fd == -1)
    return -1;
  if (header_write (fd, 2, c->words.len, c->sentences.len) != 0)
    goto error;
  if (check (write, fd, c->words.ptr, c->words.len * sizeof (size_t)) != 0)
    goto error;
  close (fd);
  return 0;
error:
  if (fd >= 0)
    close (fd);
  return -1;
}

int
neural_network_load (struct neural_network *n, const char *path)
{
  char b[MAX_WORD_LENGTH];
  int fd;
  size_t i;
  size_t j;
  size_t l;

  fd = open (path, flags_read);
  if (fd == -1)
    return -1;
  if (header_read (fd, 3, &n->size.vocab, &n->size.layer, &n->size.window) != 0)
    goto error;
  if (neural_network_alloc (n) != 0)
    goto error;
  for (i = 0; i < n->v->len; i++) {
    if (check (read, fd, &l, sizeof (size_t)) != 0)
      goto error;
    if (l >= MAX_WORD_LENGTH)
      goto error;
    if (check (read, fd, b, l) != 0)
      goto error;
    b[l] = '\0';
    if (vocab_get_index (n->v, b, &j) == 0) {
      if (check (read, fd, n->syn0 + j * n->size.layer, n->size.layer * sizeof (float)) != 0)
        goto error;
    }
    else if (lseek (fd, (off_t) (n->size.layer * sizeof (float)), SEEK_CUR) < 0)
      goto error;
  }
  close (fd);
  return 0;
error:
  if (fd >= 0)
    close (fd);
  return -1;
}

int
neural_network_save (struct neural_network *n, const char *path)
{
  int fd;
  size_t i;
  size_t l;

  fd = open (path, flags_write, 0666);
  if (fd == -1)
    return -1;
  if (header_write (fd, 3, n->size.vocab, n->size.layer, n->size.window) != 0)
    goto error;
  for (i = 0; i < n->v->len; i++) {
    l = strlen (n->v->pool[i].data);
    if (check (write, fd, &l, sizeof (size_t)) != 0)
      goto error;
    if (check (write, fd, n->v->pool[i].data, l) != 0)
      goto error;
    if (check (write, fd, n->syn0 + i * n->size.layer, n->size.layer * sizeof (float)) != 0)
      goto error;
  }
  close (fd);
  return 0;
error:
  if (fd >= 0)
    close (fd);
  return -1;
}
