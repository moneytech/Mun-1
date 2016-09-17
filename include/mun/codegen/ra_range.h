#ifndef MUN_RA_RANGE_H
#define MUN_RA_RANGE_H

#include "../common.h"

HEADER_BEGIN

#include "../location.h"
#include "ra_use.h"

typedef struct _live_range live_range;

typedef struct{
  use_interval* first_pending_use;
  use_position* first_register_use;
  use_position* first_beneficial_use;
  use_position* first_hinted_use;
} alloc_finger;

MUN_INLINE void
alloc_finger_init(alloc_finger* self){
  self->first_pending_use = NULL;
  self->first_register_use = NULL;
  self->first_beneficial_use = NULL;
  self->first_hinted_use = NULL;
}

MUN_INLINE void
alloc_finger_update(alloc_finger* self, word pos){
  if((self->first_register_use != NULL) && (self->first_register_use->pos >= pos)){
    self->first_register_use = NULL;
  }

  if((self->first_beneficial_use != NULL) && (self->first_beneficial_use->pos < pos)){
    self->first_beneficial_use = NULL;
  }
}

inline void alloc_finger_initialize(alloc_finger* self, live_range* range);

location alloc_finger_first_hint(alloc_finger* self);

use_position* alloc_finger_first_interfering(alloc_finger* self, word after);
use_position* alloc_finger_first_register(alloc_finger* self, word after);
use_position* alloc_finger_first_beneficial(alloc_finger* self, word after);

bool alloc_finger_advance(alloc_finger* self, word start);

typedef struct _live_range{
  word vreg;
  representation rep;
  location assigned;
  location spill;
  use_position* uses;
  use_interval* first_use_interval;
  use_interval* last_use_interval;
  live_range* next_sibling;
  alloc_finger finger;
} live_range;

MUN_INLINE void
live_range_init(live_range* self, word vreg, representation rep){
  self->vreg = vreg;
  self->rep = rep;
  self->assigned = kInvalidLocation;
  self->spill = kInvalidLocation;
  self->uses = NULL;
  self->first_use_interval = NULL;
  self->last_use_interval = NULL;
  self->next_sibling = NULL;
  alloc_finger_init(&self->finger);
}

MUN_INLINE word
live_range_start(live_range* range){
  return range->first_use_interval->start;
}

MUN_INLINE word
live_range_end(live_range* range){
  return range->last_use_interval->end;
}

void live_range_define(live_range* range, word pos);
inline void live_range_add_hinted(live_range* range, word pos, location* slot, location* hint);
void live_range_add_interval(live_range* range, word start, word end);

live_range* live_range_split(live_range* range, word pos);

use_position* live_range_add_use(live_range* range, word pos, location* slot);

bool live_range_contains(live_range* range, word pos);

MUN_INLINE bool
live_range_can_cover(live_range* range, word pos){
  return ((live_range_start(range) <= pos) && (pos < live_range_end(range)));
}

HEADER_END

#endif