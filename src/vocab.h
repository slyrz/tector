#ifndef TECTOR_VOCAB_H
#define TECTOR_VOCAB_H

#include <stdlib.h>
#include <stdint.h>

#define MAX_CODE_LENGTH 40
#define MAX_WORD_LENGTH 80

/**
 * The codes are stored as 64 bit integers and 1 bit, the MSB, is used to
 * indicate the code length, thus 63 bit remain to store the code.
 */
#if MAX_CODE_LENGTH > 63
#error "code length exceeds 64 bit"
#endif

#if MAX_WORD_LENGTH > 255
#error "word length requires new serialize functions"
#endif

struct vocab_entry {
  uint32_t hash;
  uint32_t count;
  uint64_t code;
  int32_t point[MAX_CODE_LENGTH];
  char word[MAX_WORD_LENGTH];
};

struct vocab {
  size_t min;
  size_t cap;
  size_t len;
  struct vocab_entry **table;
  struct vocab_entry *entries;
};

struct vocab *vocab_new (void);
struct vocab *vocab_open (const char *path);
void vocab_free (struct vocab *v);
int vocab_save (struct vocab *v, const char *path);
int vocab_alloc (struct vocab *v);
int vocab_build (struct vocab *v);
int vocab_parse (struct vocab *v, const char *path);
int vocab_copy (struct vocab *v, const char *path);
int vocab_add (struct vocab *v, const char *w);
int vocab_add_count (struct vocab *v, const char *w, uint32_t count);
int vocab_find (struct vocab *v, const char *w, size_t *p);
int vocab_shrink (struct vocab *v);
int vocab_encode (struct vocab *v);
uint32_t vocab_id (struct vocab *v);

#endif
