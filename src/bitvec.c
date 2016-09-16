#include <mun/bitvec.h>

void
bit_vector_init(bit_vector* self, word len){
  self->length = len;
  self->data_len = sizefor(len);
  self->data = malloc(sizeof(uword) * self->data_len);
  bit_vector_clear(self);
}

void
bit_vector_clear(bit_vector* vec){
  for(word i = 0; i < vec->data_len; i++){
    vec->data[i] = 0;
  }
}

void
bit_vector_set_all(bit_vector* vec){
  for(word i = 0; i < vec->data_len; i++){
    vec->data[i] = ((uword) -1);
  }
}

bool
bit_vector_add_all(bit_vector* vec, bit_vector* from){
  bool changed = FALSE;
  for(word i = 0; i < vec->data_len; i++){
    const uword before = vec->data[i];
    const uword after = vec->data[i] | from->data[i];
    if(before != after){
      changed = TRUE;
      vec->data[i] = after;
    }
  }
  return changed;
}

bool
bit_vector_kill_and_add(bit_vector* vec, bit_vector* kill, bit_vector* gen){
  bool changed = FALSE;
  for(word i = 0; i < vec->data_len; i++){
    const uword before = vec->data[i];
    const uword after = vec->data[i] | (gen->data[i] & ~kill->data[i]);
    if(before != after){
      changed = TRUE;
      vec->data[i] = after;
    }
  }
  return changed;
}