# Clear this variable, otherwise autotools adds the source directory
# as include paths, but the source directory contains header files whose names
# clash with the C standard headers.
DEFAULT_INCLUDES=

if DEBUGGING
  CFLAGS += -Wall -Wextra -Wno-unused-parameter
endif

noinst_LIBRARIES = \
  libcore.a

bin_PROGRAMS = \
  filter \
  train \
  vocab

libcore_a_SOURCES = \
  src/core/corpus.c \
  src/core/exp.c \
  src/core/filter.c \
  src/core/neural_network.c \
  src/core/ngram.c \
  src/core/rand.c \
  src/core/scanner.c \
  src/core/stem.c \
  src/core/stopwords.c \
  src/core/string.c \
  src/core/vocab.c

filter_SOURCES = src/filter.c
filter_LDADD = libcore.a

train_SOURCES = src/train.c
train_LDADD = libcore.a

vocab_SOURCES = src/vocab.c
vocab_LDADD = libcore.a
