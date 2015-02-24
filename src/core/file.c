#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include "hash.h"
#include "mem.h"
#include "file.h"

const int mode_read = O_RDONLY;
const int mode_write = O_WRONLY | O_CREAT | O_TRUNC;

static uint32_t
header_checksum (struct file *f)
{
  return hashptr ((void *) &f->header + sizeof (f->header.checksum),
                  sizeof (f->header) - sizeof (f->header.checksum));
}

static int
header_read (struct file *f)
{
  lseek (f->fd, SEEK_SET, 0);
  return file_read (f, &f->header, sizeof (f->header));
}

static int
header_write (struct file *f)
{
  lseek (f->fd, SEEK_SET, 0);
  return file_write (f, &f->header, sizeof (f->header));
}

struct file *
file_new (const char *path, int mode)
{
  struct file *f;

  f = mem_alloc (1, sizeof (struct file));
  if (f == NULL)
    return NULL;

  f->fd = open (path, mode, 0666);
  if (f->fd == -1)
    goto error;

  f->mode = mode;
  if (f->mode == mode_read) {
    if (header_read (f) != 0)
      goto error;
    if (f->header.checksum != header_checksum (f))
      goto error;
  }
  else {
    if (header_write (f) != 0)
      goto error;
  }
  return f;
error:
  if ((f != NULL) && (f->fd >= 0))
    close (f->fd);
  mem_free (f);
  return NULL;
}

struct file *
file_open (const char *path)
{
  return file_new (path, mode_read);
}

struct file *
file_create (const char *path)
{
  return file_new (path, mode_write);
}

void
file_close (struct file *f)
{
  if (f->mode == mode_write) {
    f->header.checksum = header_checksum (f);
    header_write (f);
  }
  close (f->fd);
  mem_free (f);
}

int
file_read (struct file *f, void *buf, size_t size)
{
  return -(read (f->fd, buf, size) != size);
}

int
file_write (struct file *f, const void *buf, size_t size)
{
  return -(write (f->fd, buf, size) != size);
}

int
file_readstr (struct file *f, char *buf, size_t size)
{
  uint64_t len;

  if (file_read (f, &len, sizeof (uint64_t)) != 0)
    return -1;
  if (len >= size)
    return -1;
  if (file_read (f, buf, len) != 0)
    return -1;
  buf[len] = '\0';
  return 0;
}

int
file_writestr (struct file *f, const char *buf)
{
  uint64_t len;

  len = (uint64_t) strlen (buf);
  if (file_write (f, &len, sizeof (uint64_t)) != 0)
    return -1;
  if (file_write (f, buf, len) != 0)
    return -1;
  return 0;
}
