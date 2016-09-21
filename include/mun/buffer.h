#ifndef MUN_BUFFER_H
#define MUN_BUFFER_H

#include "common.h"

HEADER_BEGIN

typedef void* V;

typedef struct{
  word asize;
  word size;
  V* data;
} object_buffer;

void buffer_init(object_buffer* self, word size);
void buffer_resize(object_buffer* self, word nasize);

MUN_INLINE V
buffer_at(object_buffer* self, word index){
  return self->data[index];
}

MUN_INLINE V
buffer_last(object_buffer* self){
  return buffer_at(self, self->size - 1);
}

MUN_INLINE void
buffer_add(object_buffer* self, V value){
  buffer_resize(self, self->size + 1);
  self->data[self->size - 1] = value;
}

MUN_INLINE V
buffer_del_last(object_buffer* self){
  V res = buffer_at(self, self->size - 1);
  self->size--;
  return res;
}

MUN_INLINE void
buffer_clear(object_buffer* self){
  self->size = 0;
}

MUN_INLINE void
buffer_insert_at(object_buffer* self, word idx, V value){
  buffer_resize(self, self->size + 1);
  for(word i = self->size - 2; i >= idx; i--){
    self->data[i + 1] = self->data[i];
  }
  self->data[idx] = value;
}

MUN_INLINE void
buffer_add_all(object_buffer* self, object_buffer* src){
  if(self->data == NULL) buffer_init(self, 1);
  for(word i = 0; i < src->size; i++){
    printf("Adding #%li\n", i);
    buffer_add(self, src->data[i]);
  }
}

MUN_INLINE void
buffer_truncate(object_buffer* self, word size){
  self->size = size;
}

MUN_INLINE void
buffer_clone_into(object_buffer* self, object_buffer* other){
  self->asize = self->size = other->size;
  self->data = malloc(sizeof(V) * other->size);
  memcpy(self->data, other->data, sizeof(V) * other->size);
}

MUN_INLINE bool
buffer_is_empty(object_buffer* self){
  return self == NULL || self->size == 0;
}

HEADER_END

#endif