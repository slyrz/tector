# Clear this variable, otherwise autotools adds the source directory
# as include paths, but the source directory contains header files whose names
# clash with the C standard headers.
DEFAULT_INCLUDES=

noinst_LIBRARIES = \
  libcore.a

bin_PROGRAMS = \
  corpus \
  filter \
  train \
  vocab

libcore_a_SOURCES = \
  src/core/corpus.c \
  src/core/exp.c \
  src/core/filter.c \
  src/core/hash.c \
  src/core/mem.c \
  src/core/neural_network.c \
  src/core/ngram.c \
  src/core/options.c \
  src/core/scanner.c \
  src/core/serialize.c \
  src/core/stem.c \
  src/core/stopwords.c \
  src/core/string.c \
  src/core/vocab.c

LDADD = libcore.a
corpus_SOURCES = src/corpus.c
filter_SOURCES = src/filter.c
train_SOURCES = src/train.c
vocab_SOURCES = src/vocab.c

check_PROGRAMS = \
  tests/corpus \
  tests/filter \
  tests/load \
  tests/neural_network \
  tests/options \
  tests/scanner \
  tests/vocab

TESTS = $(check_PROGRAMS)
