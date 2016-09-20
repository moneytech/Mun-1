#include <mun/codegen/ra_range.h>

location
alloc_finger_first_hint(alloc_finger* self){
  use_position* use = self->first_hinted_use;
  while(use != NULL){
    if(use_pos_has_hint(use)) return *use->hint;
    use = use->next;
  }

  return kInvalidLocation;
}

bool
alloc_finger_advance(alloc_finger* self, word start){
  use_interval* a = self->first_pending_use;
  while(a != NULL && a->end <= start) a = a->next;
  return (self->first_pending_use = a) == NULL;
}

MUN_INLINE use_position*
first_use_after(use_position* use, word after){
  while((use != NULL) && (use->pos < after)) use = use->next;
  return use;
}

use_position*
alloc_finger_first_register(alloc_finger* self, word after){
  for(use_position* use = first_use_after(self->first_register_use, after);
      use != NULL;
      use = use->next){
    location* loc = use->slot;
    location_policy loc_p = loc_get_policy(*loc);
    if(loc_is_unallocated(*loc) && ((loc_p == kRequiresRegister) || (loc_p == kRequiresFpuRegister))){
      return (self->first_register_use = use);
    }
  }
  return NULL;
}

use_position*
alloc_finger_first_beneficial(alloc_finger* self, word after){
  for(use_position* use = first_use_after(self->first_beneficial_use, after);
      use != NULL;
      use = use->next){
    location* loc = use->slot;
    if(loc_is_unallocated(*loc) && loc_is_register_beneficial(*loc)){
      return (self->first_beneficial_use = use);
    }
  }
  return NULL;
}

MUN_INLINE bool
is_instruction_end(word pos){
  return (pos & 1) == 1;
}

use_position*
alloc_finger_first_interfering(alloc_finger* self, word after){
  if(is_instruction_end(after)) after += 1;
  return alloc_finger_first_register(self, after);
}

void
alloc_finger_initialize(alloc_finger* self, live_range* range){
  self->first_pending_use = range->first_use_interval;
  self->first_register_use = self->first_beneficial_use = self->first_hinted_use = range->uses;
}

void
live_range_define(live_range* self, word pos){
  if(self->first_use_interval == NULL){
    self->first_use_interval = self->last_use_interval = malloc(sizeof(use_interval));
    use_interval_init(self->first_use_interval, pos, pos + 1, NULL);
  } else{
    self->first_use_interval->start = pos;
  }
}

use_position*
live_range_add_use(live_range* self, word pos, location* slot){
  if(self->uses != NULL){
    if((self->uses->pos == pos) &&
        (self->uses->slot == slot)){
      return self->uses;
    } else if(self->uses->pos < pos){
      use_position* insert_after = self->uses;
      while((insert_after->next != NULL) &&
          (insert_after->next->pos < pos)){
        insert_after = insert_after->next;
      }

      use_position* insert_before = insert_after->next;
      while((insert_before != NULL) && (insert_before->pos == pos)){
        if(insert_before->slot == slot){
          return insert_before;
        }
      }

      use_position* next = malloc(sizeof(use_position));
      use_pos_init(next, pos, insert_after->next, slot);
      return insert_after->next = next;
    }
  }

  self->uses = malloc(sizeof(use_position));
  use_pos_init(self->uses, pos, self->uses, slot);
  return self->uses;
}

void
live_range_add_hinted(live_range* self, word pos, location* slot, location* hint){
  live_range_add_use(self, pos, slot)->hint = hint;
}

void
live_range_add_interval(live_range* self, word start, word end){
  if(self->first_use_interval != NULL){
    if(start > self->first_use_interval->start){
      return;
    } else if(start == self->first_use_interval->start){
      if(end <= self->first_use_interval->end) return;
      self->first_use_interval->end = end;
      return;
    } else if(end == self->first_use_interval->start){
      self->first_use_interval->start = start;
      return;
    }
  }

  use_interval* new = malloc(sizeof(use_interval));
  use_interval_init(new, start, end, self->first_use_interval);
  self->first_use_interval = new;
  if(self->last_use_interval == NULL) self->last_use_interval = new;
}

static use_position*
split_lists(use_position** head, word split_pos, bool split_at_start){
  use_position* last_before_split = NULL;
  use_position* pos = *head;

  if(split_at_start){
    while((pos != NULL) && (pos->pos < split_pos)){
      last_before_split = pos;
      pos = pos->next;
    }
  } else{
    while((pos != NULL) && (pos->pos <= split_pos)){
      last_before_split = pos;
      pos = pos->next;
    }
  }

  if(last_before_split == NULL){
    *head = NULL;
  } else{
    last_before_split->next = NULL;
  }

  return pos;
}

live_range*
live_range_split(live_range* self, word pos){
  if(live_range_start(self) == pos) return self;

  use_interval* interval = self->finger.first_pending_use;
  if(interval == NULL){
    alloc_finger_initialize(&self->finger, self);
    interval = self->finger.first_pending_use;
  }

  if(pos <= interval->start) interval = self->first_use_interval;

  use_interval* last_before = NULL;
  while(interval->end <= pos){
    last_before = interval;
    interval = interval->next;
  }

  bool split_at_start = (interval->start == pos);

  use_interval* first_after = interval;
  if(!split_at_start && use_interval_contains(interval, pos)){
    first_after = malloc(sizeof(use_interval));
    use_interval_init(first_after, pos, interval->end, interval->next);
    interval->end = pos;
    interval->next = first_after;
    last_before = interval;
  }

  use_position* first_after_split = split_lists(&self->uses, pos, split_at_start);
  use_interval* last_use_interval = (last_before == self->last_use_interval) ?
                                    first_after :
                                    self->last_use_interval;

  live_range* new_sibling = malloc(sizeof(live_range));
  new_sibling->rep = self->rep;
  new_sibling->vreg = self->vreg;
  new_sibling->uses = first_after_split;
  new_sibling->first_use_interval = first_after;
  new_sibling->last_use_interval = last_use_interval;
  new_sibling->next_sibling = self->next_sibling;
  new_sibling->assigned = kInvalidLocation;
  alloc_finger_init(&new_sibling->finger);

  self->next_sibling = new_sibling;
  self->last_use_interval = last_before;
  self->last_use_interval = NULL;

  if(first_after_split != NULL){
    alloc_finger_update(&self->finger, first_after_split->pos);
  }
  return self->next_sibling;
}

bool
live_range_contains(live_range* self, word pos){
  if(!live_range_can_cover(self, pos)) return FALSE;

  for(use_interval* interval = self->first_use_interval;
      interval != NULL;
      interval = interval->next){
    if(use_interval_contains(interval, pos)){
      return TRUE;
    }
  }

  return FALSE;
}