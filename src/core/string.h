#ifndef CORE_STRING_H
#define CORE_STRING_H

#include <stdlib.h>

int isupper (int c);
int islower (int c);
int isspace (int c);
int isalpha (int c);
int isunicode (int c);

char lowercase (int c);
char uppercase (int c);

int ordalpha (int c);

void nullterm (char *s, size_t l);
void formatsize (char *s, size_t v);

#endif
