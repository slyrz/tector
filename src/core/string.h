#ifndef CORE_STRING_H
#define CORE_STRING_H

#include <stdlib.h>

/**
 * Testing ASCII character classes. Not using <ctypes.h> because we really
 * want to work with ASCII only.
 */
#define isupper(x) 		(((x) >= 'A') && ((x) <= 'Z'))
#define islower(x) 		(((x) >= 'a') && ((x) <= 'z'))
#define isspace(x) 		(((x) <= ' ') || ((x) == '\127'))
#define isunicode(x) 	((x) & 0x80)

void nullterm (char *s, size_t l);

#endif
