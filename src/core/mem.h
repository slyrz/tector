#ifndef CORE_MEM_H
#define CORE_MEM_H

#include <stdlib.h>
#include <stdint.h>

#define mem_freenull(p) \
  do { \
    mem_free (p); p = NULL; \
  } while (0)

size_t sizepow2 (size_t n);

void *mem_alloc (size_t nmemb, size_t size);
void *mem_align (size_t nmemb, size_t size, size_t alignment);
void *mem_realloc (void *ptr, size_t nmemb, size_t size);
void mem_free (void *ptr);
void mem_clear (void *ptr, size_t nmemb, size_t size);

#endif
