#include <string.h>
#include <stdint.h>
#include <math.h>

#include "linalg.h"
#include "macros.h"
#include "mem.h"

static float
pythag (float a, float b)
{
  float x = fabs (a);
  float y = fabs (b);

  if (x < y)
    swap (x, y);
  if (x != 0.0)
    return x * sqrt (1.0 + (y * y) / (x * x));
  return 0.0;
}

/**
 * Golub-Reinsch SVD.
 */
void
svd_full (const float *a, size_t m, size_t n, float **ou, float **os, float **ov)
{
  float eps = 1.e-15;
  float tol = 1.e-64 / eps;

  int itmax = 50;
  int iteration;

  int h, i, j, k, l;

  float *p = NULL;
  float *q = NULL;
  float *u = NULL;
  float *v = NULL;

  float d, e, f, g, s, x, y, z;

  l = 0;
  g = 0.0;
  x = 0.0;

  if (m < n)
    goto error;
  p = mem_alloc (n, sizeof (float));
  q = mem_alloc (n, sizeof (float));
  u = mem_alloc (m * n, sizeof (float));
  v = mem_alloc (n * n, sizeof (float));
  if (p == NULL || q == NULL || u == NULL || v == NULL)
    goto error;
  memcpy (u, a, m * n * sizeof (float));

  for (i = 0; i < n; i++) {
    p[i] = g;
    s = 0.0;
    l = i + 1;
    for (j = i; j < m; j++)
      s += u[j * n + i] * u[j * n + i];
    if (s <= tol)
      g = 0.0;
    else {
      f = u[i * n + i];
      if (f < 0.0)
        g = sqrt (s);
      else
        g = -sqrt (s);
      d = f * g - s;
      u[i * n + i] = f - g;
      for (j = l; j < n; j++) {
        s = 0.0;
        for (k = i; k < m; k++)
          s += u[k * n + i] * u[k * n + j];
        f = s / d;
        for (k = i; k < m; k++) {
          u[k * n + j] += f * u[k * n + i];
        }
      }
    }
    q[i] = g;
    s = 0.0;
    for (j = l; j < n; j++)
      s += u[i * n + j] * u[i * n + j];
    if (s <= tol)
      g = 0.0;
    else {
      f = u[i * n + i + 1];
      if (f < 0.0)
        g = sqrt (s);
      else
        g = -sqrt (s);
      d = f * g - s;
      u[i * n + i + 1] = f - g;
      for (j = l; j < n; j++)
        p[j] = u[i * n + j] / d;
      for (j = l; j < m; j++) {
        s = 0.0;
        for (k = l; k < n; k++)
          s += u[j * n + k] * u[i * n + k];
        for (k = l; k < n; k++)
          u[j * n + k] += s * p[k];
      }
    }
    y = fabs (q[i]) + fabs (p[i]);
    if (y > x)
      x = y;
  }

  for (i = n - 1; i > -1; i--) {
    if (g != 0.0) {
      d = g * u[i * n + i + 1];
      for (j = l; j < n; j++)
        v[j * n + i] = u[i * n + j] / d;
      for (j = l; j < n; j++) {
        s = 0.0;
        for (k = l; k < n; k++)
          s += u[i * n + k] * v[k * n + j];
        for (k = l; k < n; k++)
          v[k * n + j] += s * v[k * n + i];
      }
    }
    for (j = l; j < n; j++) {
      v[i * n + j] = 0.0;
      v[j * n + i] = 0.0;
    }
    v[i * n + i] = 1.0;
    g = p[i];
    l = i;
  }

  for (i = n - 1; i > -1; i--) {
    l = i + 1;
    g = q[i];
    for (j = l; j < n; j++)
      u[i * n + j] = 0.0;
    if (g != 0.0) {
      d = u[i * n + i] * g;
      for (j = l; j < n; j++) {
        s = 0.0;
        for (k = l; k < m; k++)
          s += u[k * n + i] * u[k * n + j];
        f = s / d;
        for (k = i; k < m; k++)
          u[k * n + j] += f * u[k * n + i];
      }
      for (j = i; j < m; j++)
        u[j * n + i] /= g;
    }
    else
      for (j = i; j < m; j++)
        u[j * n + i] = 0.0;
    u[i * n + i] += 1.0;
  }

  eps *= x;
  for (k = n - 1; k > -1; k--) {
    for (iteration = 0; iteration < itmax; iteration++) {
      int conv;
      for (l = k; l > -1; l--) {
        conv = (fabs (p[l]) <= eps);
        if ((conv) || (fabs (q[l - 1]) <= eps))
          break;
      }
      if (!conv) {
        e = 0.0;
        s = 1.0;
        h = l - 1;
        for (i = l; i < k + 1; i++) {
          f = s * p[i];
          p[i] = e * p[i];
          if (fabs (f) <= eps)
            break;
          g = q[i];
          d = pythag (f, g);
          q[i] = d;
          e = g / d;
          s = -f / d;
          for (j = 0; j < m; j++) {
            y = u[j * n + h];
            z = u[j * n + i];
            u[j * n + h] = y * e + z * s;
            u[j * n + i] = -y * s + z * e;
          }
        }
      }
      z = q[k];
      if (l == k) {
        if (z < 0.0) {
          q[k] = -z;
          for (j = 0; j < n; j++)
            v[j * n + k] = -v[j * n + k];
        }
        break;
      }
      if (iteration >= itmax - 1)
        break;
      x = q[l];
      y = q[k - 1];
      g = p[k - 1];
      d = p[k];
      f = ((y - z) * (y + z) + (g - d) * (g + d)) / (2.0 * d * y);
      g = pythag (f, 1.0);
      if (f < 0)
        f = ((x - z) * (x + z) + d * (y / (f - g) - d)) / x;
      else
        f = ((x - z) * (x + z) + d * (y / (f + g) - d)) / x;
      e = 1.0;
      s = 1.0;
      for (i = l + 1; i < k + 1; i++) {
        g = p[i];
        y = q[i];
        d = s * g;
        g = e * g;
        z = pythag (f, d);
        p[i - 1] = z;
        e = f / z;
        s = d / z;
        f = x * e + g * s;
        g = -x * s + g * e;
        d = y * s;
        y = y * e;
        for (j = 0; j < n; j++) {
          x = v[j * n + i - 1];
          z = v[j * n + i];
          v[j * n + i - 1] = x * e + z * s;
          v[j * n + i] = -x * s + z * e;
        }
        z = pythag (f, d);
        q[i - 1] = z;
        e = f / z;
        s = d / z;
        f = e * g + s * y;
        x = -s * g + e * y;
        for (j = 0; j < m; j++) {
          y = u[j * n + i - 1];
          z = u[j * n + i];
          u[j * n + i - 1] = y * e + z * s;
          u[j * n + i] = -y * s + z * e;
        }
      }
      p[l] = 0.0;
      p[k] = f;
      q[k] = x;
    }
  }
  goto done;
error:
  mem_freenull (q);
  mem_freenull (u);
  mem_freenull (v);
done:
  mem_free (p);
  *ou = u;
  *os = q;
  *ov = v;
}

/**
 * Performs in-place matrix transposition of a[m,n].
 */
static void
transpose (float *a, size_t m, size_t n)
{
  size_t i, j, k;
  float t;

  for (k = 0; k < m * n; k++) {
    j = k;
    i = 0;
    do {
      i++;
      j = (j % m) * n + j / m;
    } while (j > k);
    if (j < k || i == 1)
      continue;
    t = a[k];
    j = k;
    do {
      i = (j % m) * n + j / m;
      if (i == k)
        a[j] = t;
      else
        a[j] = a[i];
      j = i;
    } while (j > k);
  }
}

/**
 * Gram-Schmidt QR decomposition. Writes the Q matrix into q.
 * R isn't needed elsewhere, so it's only partially stored in memory (a single
 * row instead of the whole matrix) and not returned.
 */
static void
qr (const float *a, size_t m, size_t n, float *q)
{
  float s, r[n];
  size_t i;
  size_t j;
  size_t k;

  memcpy (q, a, m * n * sizeof (float));
  for (k = 0; k < n; k++) {
    memset (r, 0, k * sizeof (float));
    for (j = 0; j < m; j++)
      for (i = 0; i < k; i++)
        r[i] += q[j * n + i] * q[j * n + k];
    for (j = 0; j < m; j++)
      for (i = 0; i < k; i++)
        q[j * n + k] -= q[j * n + i] * r[i];
    s = 0.0;
    for (j = 0; j < m; j++)
      s += q[j * n + k] * q[j * n + k];
    r[k] = sqrt (s);
    for (j = 0; j < m; j++)
      q[j * n + k] /= r[k];
  }
}

/**
 * Draws a normal distributed variable using the ratio-of-uniforms methd.
 */
static inline float
rnorm (void)
{
  const float sig = 1.0;
  const float mu = 0.0;
  float u, v;
  float x, y;

  for (;;) {
    u = drand48 ();
    v = drand48 ();
    v = (v - 0.5) * 1.71552776992141;
    x = u * u * log (u) * -4.0;
    y = v * v;
    if (x >= y)
      break;
  }
  return mu + sig * (v / u);
}

/**
 * Calculates
 *   d = a * b
 * with matrices of shape a[m,n] -> b[n,o] -> d[m,o].
 */
static void
dotnn (const float *a, const float *b, size_t m, size_t n, size_t o, float *d)
{
  size_t i, j, k;

  for (i = 0; i < m; i++) {
    for (j = 0; j < o; j++) {
      d[i * o + j] = 0.0;
      for (k = 0; k < n; k++)
        d[i * o + j] += a[i * n + k] * b[k * o + j];
    }
  }
}

/**
 * Calculates
 *   d = a' * b
 * with matrices of shape a[m,n] -> b[m,o] -> d[n,o].
 */
static void
dottn (const float *a, const float *b, size_t m, size_t n, size_t o, float *d)
{
  size_t i, j, k;

  for (i = 0; i < n * o; i++)
    d[i] = 0.0;

  for (k = 0; k < m; k++)
    for (i = 0; i < n; i++)
      for (j = 0; j < o; j++)
        d[i * o + j] += a[k * n + i] * b[k * o + j];
}

static int
cmp (const void *a, const void *b)
{
  const float x = **((const float **) a);
  const float y = **((const float **) b);
  return (x < y) - (x > y);
}

/**
 * Trim writes the top k columns with largest weights in the weight vector
 * w[n] from matrix a[m,n] into matrix d[m,k].
 */
static void
trim (const float *a, const float *w, size_t m, size_t n, size_t k, float *d)
{
  float const *p[n];
  size_t i;
  size_t j;

  for (i = 0; i < n; i++)
    p[i] = w + i;
  qsort (p, n, sizeof (float *), cmp);

  for (i = 0; i < m; i++)
    for (j = 0; j < k; j++)
      d[i * k + j] = a[i * m + (p[j] - w)];
}

/**
 * Randomized SVD based on
 *
 * Halko, Nathan, Per-Gunnar Martinsson, and Joel A. Tropp.
 * "Finding structure with randomness: Probabilistic algorithms for
 * constructing approximate matrix decompositions."
 * SIAM review 53.2 (2011): 217-288.
 */
void
svd_topk (const float *a, size_t m, size_t n, size_t k, float **ou)
{
  const size_t l = 2 * k;       // top2k approximation
  const size_t p = 2;           // power iterations
  size_t i;

  float *b = NULL;
  float *q = NULL;
  float *s = NULL;
  float *u = NULL;
  float *v = NULL;
  float *y = NULL;

  if (l >= min (m, n))
    goto error;

  b = mem_alloc (l * n, sizeof (float));
  q = mem_alloc (l * max (m, n), sizeof (float));
  y = mem_alloc (l * max (m, n), sizeof (float));
  if (b == NULL || q == NULL || y == NULL)
    goto error;

  for (i = 0; i < n * l; i++)
    b[i] = rnorm ();

  dotnn (a, b, m, n, l, y);
  qr (y, m, l, q);
  for (i = 0; i < p; i++) {
    dottn (a, q, m, n, l, y);
    qr (y, n, l, q);
    dotnn (a, q, m, n, l, y);
    qr (y, m, l, q);
  }
  dottn (q, a, m, l, n, b);
  transpose (b, l, n);
  svd_full (b, n, l, &u, &s, &v);
  trim (v, s, l, l, k, b);
  dotnn (q, b, m, l, k, y);
  goto done;
error:
  mem_freenull (y);
done:
  *ou = y;
  mem_free (u);
  mem_free (s);
  mem_free (v);
  mem_free (b);
  mem_free (q);
}
