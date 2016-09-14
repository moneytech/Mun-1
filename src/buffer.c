#include <mun/buffer.h>

void
buffer_init(object_buffer* self, word size){
  self->size = 0;
  self->asize = size;
  self->data = malloc(sizeof(V) * size);
}

void
buffer_resize(object_buffer* self, word nasize){
  if(nasize > self->asize){
    V* ndata = realloc(self->data, sizeof(V) * nasize);
    self->data = ndata;
    self->asize = nasize;
  }
  self->size = nasize;
}