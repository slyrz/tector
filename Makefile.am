# Clear this variable, otherwise autotools adds the source directory
# as include paths, but the source directory contains header files whose names
# clash with the C standard headers.
DEFAULT_INCLUDES=

noinst_LIBRARIES = \
  libcore.a

bin_PROGRAMS = \
  filter \
  model \
  vocab

libcore_a_SOURCES = \
  src/core/corpus.c \
  src/core/exp.c \
  src/core/file.c \
  src/core/filter.c \
  src/core/hash.c \
  src/core/mem.c \
  src/core/model.c \
  src/core/model_nn.c \
  src/core/ngram.c \
  src/core/program.c \
  src/core/scanner.c \
  src/core/stem.c \
  src/core/stopwords.c \
  src/core/string.c \
  src/core/vocab.c

LDADD = libcore.a
filter_SOURCES = src/filter.c
model_SOURCES = src/model.c
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
