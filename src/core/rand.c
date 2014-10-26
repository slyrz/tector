static unsigned long long rnd = 1;

int
getrand ()
{
  rnd = rnd * 25214903917ll + 11;
  return (rnd & 0xffffffff);
}

float
getrandf ()
{
  return ((getrand () & 0xffff) / 65536.0) - 0.5;
}
