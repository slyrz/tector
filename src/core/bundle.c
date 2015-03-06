#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "bundle.h"
#include "log.h"
#include "mem.h"

static int
newer (const char *a, const char *b)
{
  struct stat sa;
  struct stat sb;

  if (stat (a, &sa) != 0)
    return 0;
  if (stat (b, &sb) != 0)
    return 0;
  return sa.st_mtime > sb.st_mtime;
}

static int
exists (const char *path)
{
  struct stat s;

  if (stat (path, &s) != 0)
    return 0;
  return S_ISREG (s.st_mode);
}

struct bundle *
bundle_open (const char *path)
{
  struct bundle *b;

  b = mem_alloc (1, sizeof (struct bundle));
  if (b == NULL)
    return NULL;

  b->path.bundle = strdup (path);
  if (b->path.bundle == NULL)
    goto error;
  if (asprintf (&b->path.vocab, "%s/vocab", b->path.bundle) == -1)
    goto error;
  if (asprintf (&b->path.model, "%s/model", b->path.bundle) == -1)
    goto error;

  /**
   * If a vocab and a model exist, make sure the vocab is older than the model.
   * Vocabulary changes with existing model aren't supported yet.
   */
  if (newer (b->path.vocab, b->path.model)) {
    warning ("newer");
    goto error;
  }

  if (exists (b->path.vocab)) {
    b->vocab = vocab_open (b->path.vocab);
    if (b->vocab == NULL)
      goto error;
  }
  if (exists (b->path.model)) {
    b->model = model_open (b->vocab, b->path.model);
    if (b->model == NULL)
      goto error;
  }
  return b;
error:
  if (b)
    bundle_free (b);
  return NULL;
}

void
bundle_free (struct bundle *b)
{
  if (b->vocab)
    vocab_free (b->vocab);
  if (b->model)
    model_free (b->model);
  free (b->path.bundle);
  free (b->path.vocab);
  free (b->path.model);
  mem_free (b);
}

int
bundle_save (struct bundle *b)
{
  if (b->model)
    return model_save (b->model, b->path.model);
  if (b->vocab)
    return vocab_save (b->vocab, b->path.vocab);
  return 0;
}
