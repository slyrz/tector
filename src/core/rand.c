/**
 * This file incorporates derivative work covered by the
 * following copyright and permission notice:
 *
 * Copyright 2013 Google Inc. All Rights Reserved.
 */
#include "rand.h"

static unsigned long long rnd = 1;

int
getrand (void)
{
  rnd = rnd * 25214903917ll + 11;
  return (rnd & 0xffffffff);
}

float
getrandf (void)
{
  return ((getrand () & 0xffff) / 65536.0) - 0.5;
}
