#include "config.h"
#include "stem.h"
#include "string.h"

#include <string.h>

struct stem {
  size_t k;
  size_t j;
  char *b;
};

static inline int
cons (struct stem *restrict s, size_t i)
{
  switch (s->b[i]) {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
      return 0;
    case 'y':
      return (i == 0) || ((i > 0) && !cons (s, i - 1));
    default:
      return 1;
  }
}

static inline size_t
m (struct stem *restrict s)
{
  size_t n = 0;
  size_t i = 0;
  for (;;) {
    if (i > s->j)
      return n;
    if (!cons (s, i))
      break;
    i++;
  }
  i++;
  for (;;) {
    for (;;) {
      if (i > s->j)
        return n;
      if (cons (s, i))
        break;
      i++;
    }
    i++;
    n++;
    for (;;) {
      if (i > s->j)
        return n;
      if (!cons (s, i))
        break;
      i++;
    }
    i++;
  }
}

static inline int
vowelinstem (struct stem *restrict s)
{
  size_t i;
  for (i = 0; i <= s->j; i++)
    if (!cons (s, i))
      return 1;
  return 0;
}

static inline int
doublec (struct stem *restrict s, size_t j)
{
  return (j > 0) && (cons (s, j)) && (s->b[j] == s->b[j - 1]);
}

static inline int
cvc (struct stem *restrict s, size_t i)
{
  int ch;
  if ((i >= 2) && cons (s, i) && !cons (s, i - 1) && cons (s, i - 2)) {
    ch = s->b[i];
    return (ch != 'w') & (ch != 'x') & (s->b[i] != 'y');
  }
  return 0;
}

static inline int
ends (struct stem *restrict s, const char *w)
{
  size_t l = (size_t) w[0];
  if ((l > s->k + 1) || (w[l] != s->b[s->k]))
    return 0;
  if (memcmp (s->b + s->k - l + 1, w + 1, l))
    return 0;
  if (l < s->k)
    s->j = s->k - l;
  else
    s->j = 0;
  return 1;
}

static inline void
setto (struct stem *restrict s, const char *w)
{
  size_t l = (size_t) w[0];
  memmove (s->b + s->j + 1, w + 1, l);
  s->k = s->j + l;
}

static inline void
r (struct stem *restrict s, const char *w)
{
  if (m (s))
    setto (s, w);
}

static inline void
step1ab (struct stem *restrict s)
{
  if (s->b[s->k] == 's') {
    if (ends (s, "\04sses")) {
      s->k -= (s->k > 0);
      s->k -= (s->k > 0);
    }
    else if (ends (s, "\03ies"))
      setto (s, "\01i");
    else if (s->b[s->k - 1] != 's')
      s->k -= (s->k > 0);
  }
  if (ends (s, "\03eed")) {
    if (m (s) > 0)
      s->k -= (s->k > 0);
  }
  else if ((ends (s, "\02ed") || ends (s, "\03ing")) && vowelinstem (s)) {
    s->k = s->j;
    if (ends (s, "\02at"))
      setto (s, "\03ate");
    else if (ends (s, "\02bl"))
      setto (s, "\03ble");
    else if (ends (s, "\02iz"))
      setto (s, "\03ize");
    else if (doublec (s, s->k)) {
      s->k -= (s->k > 0);
      {
        int ch = s->b[s->k];
        if (ch == 'l' || ch == 's' || ch == 'z')
          s->k++;
      }
    }
    else if (m (s) == 1 && cvc (s, s->k))
      setto (s, "\01e");
  }
}

static inline void
step1c (struct stem *restrict s)
{
  if (ends (s, "\01y") && vowelinstem (s))
    s->b[s->k] = 'i';
}

#define match2(x,y,z) \
  if (ends (x, y)) { r (x, z); break; }

static inline void
step2 (struct stem *restrict s)
{
  switch (s->b[s->k - 1]) {
    case 'a':
      match2 (s, "\07ational", "\03ate");
      match2 (s, "\06tional", "\04tion");
      break;
    case 'c':
      match2 (s, "\04enci", "\04ence");
      match2 (s, "\04anci", "\04ance");
      break;
    case 'e':
      match2 (s, "\04izer", "\03ize");
      break;
    case 'l':
      match2 (s, "\03bli", "\03ble");
      match2 (s, "\04alli", "\02al");
      match2 (s, "\05entli", "\03ent");
      match2 (s, "\03eli", "\01e");
      match2 (s, "\05ousli", "\03ous");
      break;
    case 'o':
      match2 (s, "\07ization", "\03ize");
      match2 (s, "\05ation", "\03ate");
      match2 (s, "\04ator", "\03ate");
      break;
    case 's':
      match2 (s, "\05alism", "\02al");
      match2 (s, "\07iveness", "\03ive");
      match2 (s, "\07fulness", "\03ful");
      match2 (s, "\07ousness", "\03ous");
      break;
    case 't':
      match2 (s, "\05aliti", "\02al");
      match2 (s, "\05iviti", "\03ive");
      match2 (s, "\06biliti", "\03ble");
      break;
    case 'g':
      match2 (s, "\04logi", "\03log");
  }
}

#define match3(x,y,z) \
  if (ends (x, y)) { r (x, z); break; }

static inline void
step3 (struct stem *restrict s)
{
  switch (s->b[s->k]) {
    case 'e':
      match3 (s, "\05icate", "\02ic");
      match3 (s, "\05ative", "\00");
      match3 (s, "\05alize", "\02al");
      break;
    case 'i':
      match3 (s, "\05iciti", "\02ic");
      break;
    case 'l':
      match3 (s, "\04ical", "\02ic");
      match3 (s, "\03ful", "\00");
      break;
    case 's':
      match3 (s, "\04ness", "\00");
      break;
  }
}

#define match4(x) \
  if (x) break

#define atjeither(s,x,y) \
  ((s->j > 0) && (s->b[s->j] == x || s->b[s->j] == y))

static inline void
step4 (struct stem *restrict s)
{
  switch (s->b[s->k - 1]) {
    case 'a':
      match4 (ends (s, "\02al"));
      return;
    case 'c':
      match4 (ends (s, "\04ance"));
      match4 (ends (s, "\04ence"));
      return;
    case 'e':
      match4 (ends (s, "\02er"));
      return;
    case 'i':
      match4 (ends (s, "\02ic"));
      return;
    case 'l':
      match4 (ends (s, "\04able"));
      match4 (ends (s, "\04ible"));
      return;
    case 'n':
      match4 (ends (s, "\03ant"));
      match4 (ends (s, "\05ement"));
      match4 (ends (s, "\04ment"));
      match4 (ends (s, "\03ent"));
      return;
    case 'o':
      match4 (ends (s, "\03ion") && atjeither (s, 's', 't'));
      match4 (ends (s, "\02ou"));
      return;
    case 's':
      match4 (ends (s, "\03ism"));
      return;
    case 't':
      match4 (ends (s, "\03ate"));
      match4 (ends (s, "\03iti"));
      return;
    case 'u':
      match4 (ends (s, "\03ous"));
      return;
    case 'v':
      match4 (ends (s, "\03ive"));
      return;
    case 'z':
      match4 (ends (s, "\03ize"));
      return;
    default:
      return;
  }
  if (m (s) > 1)
    s->k = s->j;
}

static inline void
step5 (struct stem *restrict s)
{
  s->j = s->k;
  if (s->b[s->k] == 'e') {
    size_t a = m (s);
    if (a > 1 || (a == 1 && !cvc (s, s->k - 1)))
      s->k -= (s->k > 0);
  }
  if (s->b[s->k] == 'l' && doublec (s, s->k) && m (s) > 1)
    s->k -= (s->k > 0);
}

size_t
stem (char *w)
{
  struct stem s = (struct stem) {
    0, 0, w
  };

  s.k = strlen (w);
  if (s.k <= 2)
    return s.k;

  s.k--;
  step1ab (&s);
  if (s.k)
    step1c (&s);
  if (s.k)
    step2 (&s);
  if (s.k)
    step3 (&s);
  if (s.k)
    step4 (&s);
  if (s.k)
    step5 (&s);
  s.k++;
  nullterm (s.b, s.k);
  return s.k;
}
