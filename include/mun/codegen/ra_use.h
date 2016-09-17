#ifndef MUN_RA_USE_H
#define MUN_RA_USE_H

#include "../common.h"

HEADER_BEGIN

#include "../location.h"

static const word kIllegalPosition = -1;

typedef struct _use_position{
  word pos;
  location* slot;
  location* hint;
  struct _use_position* next;
} use_position;

MUN_INLINE void
use_pos_init(use_position* self, word pos, use_position* next, location* loc){
  self->pos = pos;
  self->next = next;
  self->slot = loc;
  self->hint = NULL;
}

MUN_INLINE bool
use_pos_has_hint(use_position* pos){
  return (pos->hint != NULL) && !loc_is_unallocated(*pos->hint);
}

typedef struct _use_interval{
  word start;
  word end;
  struct _use_interval* next;
} use_interval;

MUN_INLINE void
use_interval_init(use_interval* self, word start, word end, use_interval* next){
  self->start = start;
  self->end = end;
  self->next = next;
}

MUN_INLINE bool
use_interval_contains(use_interval* interval, word pos){
  return ((interval->start <= pos) && (pos < interval->end));
}

word use_interval_intersect(use_interval* self, use_interval* other);

HEADER_END

#endif