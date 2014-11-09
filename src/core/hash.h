#ifndef CORE_HASH_H
#define CORE_HASH_H

#include "config.h"
#include <stdlib.h>
#include <stdint.h>

uint32_t hashstr (const char *restrict ptr);
uint32_t hashptr (const void *restrict ptr, size_t n);

#endif
