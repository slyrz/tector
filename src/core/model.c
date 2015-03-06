#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include "mem.h"
#include "model.h"

extern struct model_interface interface_nn;

static int
model_alloc (struct model *m)
{
  if (!m->state.allocated) {
    if (m->i->alloc (m) != 0)
      return -1;
    m->state.allocated = 1;
  }
  return 0;
}

struct model *
model_new (struct vocab *v, int type)
{
  const struct model_interface *i;
  struct model *m;

  switch (type) {
    case MODEL_NN:
      i = &interface_nn;
      break;
    default:
      return NULL;
  }

  m = mem_alloc (1, i->size);
  if (m == NULL)
    return NULL;
  m->i = i;
  m->v = v;
  m->type = type;
  m->size.vocab = v->len;
  m->size.layer = 64;
  m->size.vector = 64;
  m->size.window = 5;
  if (m->i->init (m) != 0)
    goto error;
  m->state.changed = 1;
  return m;
error:
  if (m)
    model_free (m);
  return NULL;
}

struct model *
model_open (struct vocab *v, const char *path)
{
  struct file *f = NULL;
  struct model *m = NULL;

  f = file_open (path);
  if (f == NULL)
    goto error;
  m = model_new (v, f->header.type);
  if (m == NULL)
    goto error;
  m->size.layer = f->header.data[0];
  m->size.vector = f->header.data[1];
  m->size.vocab = f->header.data[2];
  m->size.window = f->header.data[3];
  if (m->size.vocab != v->len)
    goto error;
  if (model_alloc (m) != 0)
    goto error;
  if (m->i->load (m, f) != 0)
    goto error;
  file_close (f);
  m->state.changed = 0;
  return m;
error:
  if (m)
    model_free (m);
  if (f)
    file_close (f);
  return NULL;
}

int
model_save (struct model *m, const char *path)
{
  struct file *f = NULL;

  if (!m->state.changed)
    return 0;
  if (model_alloc (m) != 0)
    return -1;
  f = file_create (path);
  if (f == NULL)
    goto error;
  f->header.type = m->type;
  f->header.data[0] = m->size.layer;
  f->header.data[1] = m->size.vector;
  f->header.data[2] = m->size.vocab;
  f->header.data[3] = m->size.window;
  if (m->i->save (m, f) != 0)
    goto error;
  file_close (f);
  m->state.changed = 0;
  return 0;
error:
  if (f)
    file_close (f);
  return -1;
}

void
model_free (struct model *m)
{
  if (m->state.allocated)
    m->i->free (m);
  mem_free (m);
}

int
model_train (struct model *m, struct corpus *c)
{
  if (model_alloc (m) != 0)
    return -1;
  m->state.changed = 1;
  return m->i->train (m, c);
}
