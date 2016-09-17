#include <mun/codegen/ra_use.h>

word
use_interval_intersect(use_interval* self, use_interval* other){
  if(self->start <= other->start){
    if(other->start < self->end) return other->start;
  } else if(self->start < other->end){
    return self->start;
  }

  return kIllegalPosition;
}