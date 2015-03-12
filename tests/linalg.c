#include "../src/core/linalg.h"
#include "../src/core/mem.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define M 11
#define N 5

const float A[M * N] = {
    0.75120f, -1.60768f, -0.39993f,  0.64517f, -0.11946f,
    1.05911f,  1.07633f,  0.37630f,  1.29926f,  0.36433f,
    0.05262f, -0.03317f, -0.71406f,  0.51052f, -0.13106f,
    0.43550f, -0.95293f, -0.13295f, -0.90264f, -0.00450f,
    0.45933f, -0.79411f, -0.59319f, -0.26540f, -0.92404f,
    0.36340f,  0.83518f,  0.71027f,  0.87006f, -0.12123f,
    1.04111f, -1.45347f, -1.01267f, -1.18121f,  0.24258f,
    0.79377f, -1.46045f,  0.03335f, -1.99452f, -0.73553f,
    0.44600f, -2.07440f,  0.47107f,  1.02838f, -0.16369f,
   -0.35393f,  1.26356f, -0.91869f,  0.55792f, -0.11723f,
    0.02603f, -1.10818f,  0.85291f,  0.16045f,  0.40942f,
};

const float U[M * N] = {
   -0.28031f,  0.41322f,  0.34954f, -0.06638f,  0.03357f,
    0.27951f,  0.33712f,  0.20785f,  0.72423f,  0.10907f,
    0.02264f,  0.06977f,  0.39958f, -0.17323f, -0.01172f,
   -0.28556f, -0.09634f, -0.03057f,  0.13056f,  0.13344f,
   -0.22884f, -0.01907f,  0.34352f, -0.01519f, -0.58377f,
    0.22669f,  0.22046f, -0.08254f,  0.38917f, -0.31535f,
   -0.44717f, -0.14222f,  0.37297f,  0.24387f,  0.54053f,
   -0.51652f, -0.30451f, -0.15696f,  0.36146f, -0.39174f,
   -0.29500f,  0.65146f, -0.03221f, -0.21751f, -0.15453f,
    0.27997f, -0.15046f,  0.46675f, -0.17688f, -0.04636f,
   -0.15916f,  0.30414f, -0.40814f, -0.06017f,  0.23905f,
};

const float S[N] = {
    4.66593f,  2.97433f,  2.03381f,  1.59874f,  1.19392f,
};

const float V[N * N] = {
   -0.25092f,  0.22273f,  0.34012f,  0.87832f,  0.01758f,
    0.84448f, -0.41684f,  0.06486f,  0.32318f, -0.06667f,
    0.09420f,  0.31296f, -0.87532f,  0.29063f, -0.20623f,
    0.44906f,  0.81841f,  0.29426f, -0.19175f, -0.07210f,
    0.11560f,  0.09436f, -0.16535f,  0.05364f,  0.97340f,
};

static int
almostequal (float a, float b)
{
  return fabsf (a - b) < 0.001f;
}

static int
almostequalabs (float a, float b)
{
  return almostequal (fabsf (a), fabsf (b));
}

static void
printmatrix (const char *name, float *a, size_t m, size_t n)
{
  size_t i;
  size_t j;

  puts (name);
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++)
      printf ("%12.4f", a[i * n + j]);
    putchar ('\n');
  }
  putchar ('\n');
}

int
main (void)
{
  float *u;
  float *s;
  float *v;

  size_t i;
  size_t j;

  svd_full (A, M, N, &u, &s, &v);
  assert (u != NULL);
  assert (s != NULL);
  assert (v != NULL);

  printmatrix ("u", u, M, N);
  printmatrix ("s", s, 1, N);
  printmatrix ("v", v, N, N);

  for (j = 0; j < N; j++) {
    assert (almostequalabs (S[j], s[j]));
    for (i = 0; i < M; i++)
      assert (almostequalabs (U[i * N + j], u[i * N + j]));
    for (i = 0; i < N; i++)
      assert (almostequalabs (V[i * N + j], v[i * N + j]));
  }
  mem_free (u);
  mem_free (s);
  mem_free (v);
  return EXIT_SUCCESS;
}
