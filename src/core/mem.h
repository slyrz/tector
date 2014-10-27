#ifndef CORE_MEM_H
#define CORE_MEM_H

#include <stdlib.h>

#define freenull(p) \
  do { \
    free (p); p = NULL; \
  } while (0)

size_t sizepow2 (size_t n);
void *reallocarray (void *ptr, size_t nmemb, size_t size);
void *clearspace (void *ptr, size_t nmemb, size_t cap, size_t size);

#endif
