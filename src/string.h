#ifndef TECTOR_STRING_H
#define TECTOR_STRING_H

#include <stdlib.h>

int isupper (int c);
int islower (int c);
int isspace (int c);
int isalpha (int c);
int isunicode (int c);

char lowercase (int c);
char uppercase (int c);

int ordalpha (int c);

char *nullterm (char *s, size_t l);
char *formatsize (char *s, size_t v);

#endif
