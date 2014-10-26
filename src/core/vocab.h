#ifndef CORE_VOCAB_H
#define CORE_VOCAB_H

#include <stdlib.h>
#include <stdint.h>

#define MAX_CODE_LENGTH 40
#define MAX_WORD_LENGTH 80

#if MAX_CODE_LENGTH > 64
#error "code length exceeds 64 bits"
#endif

struct vocab_entry {
  uint32_t hash;
  uint32_t count;
  uint64_t code;
  uint32_t point[MAX_CODE_LENGTH];
  char data[MAX_WORD_LENGTH];
};

struct vocab {
  size_t cap;
  size_t len;
  struct vocab_entry **table;
  struct vocab_entry *pool;
};

struct vocab *vocab_new (void);
struct vocab *vocab_new_from_path (const char *path);
void vocab_free (struct vocab *t);

void vocab_add (struct vocab *t, const char *w);
struct vocab_entry *vocab_get (struct vocab *t, const char *w);
int vocab_get_index (struct vocab *t, const char *w, size_t * i);
void vocab_print (const struct vocab *t);
void vocab_shrink (struct vocab *t);
void vocab_grow (struct vocab *t, size_t cap);
void vocab_encode (struct vocab *t);
void vocab_save (struct vocab *t, const char *path);
void vocab_load (struct vocab *t, const char *path);

#endif
