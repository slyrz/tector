#include "serialize.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

const int flags_read = O_RDONLY;
const int flags_write = O_CREAT | O_TRUNC | O_WRONLY;

/**
 * Wrapper around read/write, returns 0 on success and -1 on failure.
 */
#define check(func,fd,ptr,size) \
  (-(func (fd, ptr, size) != (ssize_t) (size)))

int
vocab_load (struct vocab *v, const char *path)
{
  int fd;

  fd = open (path, flags_read);
  if (fd == -1)
    return -1;
  if (check (read, fd, &v->len, sizeof (size_t)) != 0)
    goto error;
  if (vocab_grow (v, v->len) != 0)
    goto error;
  if (check (read, fd, v->pool, v->len * sizeof (struct vocab_entry)) != 0)
    goto error;
  close (fd);
  vocab_rebuild (v);
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
  if (check (write, fd, &v->len, sizeof (size_t)) != 0)
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
  if (check (read, fd, &c->words.len, sizeof (size_t)) != 0)
    goto error;
  if (check (read, fd, &c->sentences.len, sizeof (size_t)) != 0)
    goto error;
  if (corpus_grow (c, c->words.len, c->sentences.len) != 0)
    goto error;
  if (check (read, fd, c->words.ptr, c->words.len * sizeof (size_t)) != 0)
    goto error;
  close (fd);
  corpus_rebuild (c);
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
  if (check (write, fd, &c->words.len, sizeof (size_t)) != 0)
    goto error;
  if (check (write, fd, &c->sentences.len, sizeof (size_t)) != 0)
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
  int fd;

  fd = open (path, flags_read);
  if (fd == -1)
    return -1;
  if (check (read, fd, &n->size, sizeof (n->size)) != 0)
    goto error;
  if (neural_network_alloc (n) != 0)
    goto error;
  if (check (read, fd, n->syn0, n->size.vocab * n->size.layer * sizeof (float)) != 0)
    goto error;
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

  fd = open (path, flags_write, 0666);
  if (fd == -1)
    return -1;
  if (check (write, fd, &n->size, sizeof (n->size)) != 0)
    goto error;
  if (check (write, fd, n->syn0, n->size.vocab * n->size.layer * sizeof (float)) != 0)
    goto error;
  close (fd);
  return 0;
error:
  if (fd >= 0)
    close (fd);
  return -1;
}
