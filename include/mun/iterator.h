#ifndef MUN_ITERATOR_H
#define MUN_ITERATOR_H

#include "common.h"

HEADER_BEGIN

typedef struct _iterator{
  bool (*done)(struct _iterator*);
  void (*advance)(struct _iterator*);
  void* (*current)(struct _iterator*);
  void (*remove)(struct _iterator*);
} iterator;

#define foreach(iter) \
  for(iterator* it = ((iterator*) iter); \
      !it->done(it); \
      it->advance(it))

#define it_current \
  it->current(it)

#define it_remove \
  it->remove(it)

HEADER_END

#endif