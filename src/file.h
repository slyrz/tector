#ifndef TECTOR_FILE_H
#define TECTOR_FILE_H

#include <stdint.h>
#include <unistd.h>

/**
 * File provides convient functions for storing and reading complex
 * data structures. It's used to serialize the vocab and the model.
 */
struct file {
  int fd;
  int mode;
  struct {
    uint32_t checksum;
    uint32_t type;
    uint64_t data[31];
  } header;
};

struct file *file_open (const char *);
struct file *file_create (const char *);
void file_close (struct file *);
int file_read (struct file *, void *, size_t);
int file_write (struct file *, const void *, size_t);
int file_readstr (struct file *, char *, size_t);
int file_writestr (struct file *, const char *);
int file_skip (struct file *, off_t);

#endif
