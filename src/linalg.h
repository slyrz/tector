#ifndef TECTOR_LINALG_H
#define TECTOR_LINALG_H

#include <stdlib.h>

void svd_full (const float *a, size_t m, size_t n, float **ou, float **os, float **ov);
void svd_topk (const float *a, size_t m, size_t n, size_t k, float **ou);

#endif
