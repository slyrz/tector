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
  src/bundle.c \
  src/corpus.c \
  src/exp.c \
  src/file.c \
  src/filter.c \
  src/hash.c \
  src/linalg.c \
  src/mem.c \
  src/model.c \
  src/model_nn.c \
  src/model_svd.c \
  src/model_glove.c \
  src/program.c \
  src/scanner.c \
  src/stem.c \
  src/stopwords.c \
  src/string.c \
  src/vocab.c

LDADD = libcore.a
filter_SOURCES = src/main_filter.c
model_SOURCES = src/main_model.c
vocab_SOURCES = src/main_vocab.c

check_PROGRAMS = \
  tests/corpus \
  tests/file \
  tests/filter \
  tests/linalg \
  tests/scanner \
  tests/vocab

TESTS = $(check_PROGRAMS)
