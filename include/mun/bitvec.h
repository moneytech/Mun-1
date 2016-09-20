#ifndef MUN_BITVEC_H
#define MUN_BITVEC_H

#include "common.h"
#include "iterator.h"

HEADER_BEGIN

typedef struct{
  word length;
  word data_len;
  uword* data;
} bit_vector;

void bit_vector_init(bit_vector* vec, word len);
void bit_vector_clear(bit_vector* vec);
void bit_vector_set_all(bit_vector* vec);

bool bit_vector_add_all(bit_vector* vec, bit_vector* from);
bool bit_vector_kill_and_add(bit_vector* vec, bit_vector* kill, bit_vector* gen);

MUN_INLINE void
bit_vector_remove(bit_vector* vec, word i){
  vec->data[i / kBitsPerWord] &= ~(1 << (i % kBitsPerWord));
}

MUN_INLINE void
bit_vector_add(bit_vector* vec, word i){
  vec->data[i / kBitsPerWord] |= (1 << (i % kBitsPerWord));
}

MUN_INLINE bool
bit_vector_contains(bit_vector* vec, word i){
  uword block = vec->data[i / kBitsPerWord];
  return (block & (1 << (i % kBitsPerWord))) != 0;
}

MUN_INLINE void
bit_vector_intersect(bit_vector* vec, bit_vector* other){
  for(word i = 0; i < vec->data_len; i++){
    vec->data[i] = vec->data[i] & other->data[i];
  }
}

MUN_INLINE word
sizefor(word len){
  return 1+ ((len - 1) / kBitsPerWord);
}

typedef struct{
  iterator iter;

  bit_vector* target;
  word bit_index;
  word word_index;
  uword current_word;
} bit_vector_iterator;

bit_vector_iterator* bit_vector_iterator_new(bit_vector* vec);

#define bit_vector_foreach(vec) \
  foreach(bit_vector_iterator_new(vec))

#define bit_vector_current \
  (*((word*) it_current))

HEADER_END

#endif