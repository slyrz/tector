#include "serialize.h"
#include "log.h"

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
  char b[MAX_WORD_LENGTH];
  int fd;
  size_t i;
  size_t j;
  size_t l;

  fd = open (path, flags_read);
  if (fd == -1)
    return -1;
  if (check (read, fd, &n->size, sizeof (n->size)) != 0)
    goto error;
  // TODO: print warning
  if (n->size.vocab != n->v->len)
    n->size.vocab = n->v->len;
  if (neural_network_alloc (n) != 0)
    goto error;
  for (i = 0; i < n->v->len; i++) {
    if (check (read, fd, &l, sizeof (size_t)) != 0)
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
  if (check (write, fd, &n->size, sizeof (n->size)) != 0)
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
