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

#define to_bvi(it) \
  container_of(it, bit_vector_iterator, iter)

static bool
bit_vector_iterator_done(iterator* iter){
  bit_vector_iterator* self = to_bvi(iter);
  return self->word_index >= self->target->data_len;
}

static void
bit_vector_iterator_advance(iterator* iter){
  bit_vector_iterator* self = to_bvi(iter);
  self->bit_index++;
  if(self->current_word == 0){
    do{
      self->word_index++;
      if(bit_vector_iterator_done(iter)) return;
      self->current_word = self->target->data[self->word_index];
    } while(self->current_word == 0);
    self->bit_index = self->word_index * kBitsPerWord;
  }

  while((self->current_word & 0xFF) == 0){
    self->current_word >>= 8;
    self->bit_index += 8;
  }

  while((self->current_word & 0x1) == 0){
    self->current_word >>= 1;
    self->bit_index++;
  }

  self->current_word >>= 1;
}

static void*
bit_vector_iterator_current_(iterator* iter){
  return &to_bvi(iter)->bit_index;
}

bit_vector_iterator*
bit_vector_iterator_new(bit_vector* vec){
  bit_vector_iterator* iter = malloc(sizeof(bit_vector_iterator));
  iter->target = vec;
  iter->bit_index = -1;
  iter->word_index = 0;
  iter->current_word = vec->data[0];
  iter->iter.done = &bit_vector_iterator_done;
  iter->iter.advance = &bit_vector_iterator_advance;
  iter->iter.current = &bit_vector_iterator_current_;
  iter->iter.advance(((iterator*) iter));
  return iter;
}