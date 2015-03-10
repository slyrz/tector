#ifndef CORE_MACROS_H
#define CORE_MACROS_H

#define swap(a,b) \
  do { \
    typeof (a) t  = (a); (a) = (b); (b) = t; \
  } while (0)

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

/**
 * Returns true if v is a value between [i,j[ - the interval end is excluded so
 * that callers can pass array lengths without subtracting 1.
 */
#define inrange(v,i,j) \
  (((i) <= (v)) && ((v) < (j)))

#endif
