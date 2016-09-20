#ifndef MUN_RA_LIVENESS_H
#define MUN_RA_LIVENESS_H

#include <mun/buffer.h>
#include "../common.h"

HEADER_BEGIN

#include "../bitvec.h"
#include "../buffer.h"
#include "intermediate_language.h"

typedef struct _liveness_analysis{
  object_buffer live_out; // bit_vector*
  object_buffer live_in; // bit_vector*
  object_buffer kill; // bit_vector*

  object_buffer* postorder; // block_entry_instr*

  word var_count;

  void (*compute_initial_sets)(struct _liveness_analysis*);
} liveness_analysis;

MUN_INLINE bit_vector*
liveness_get_live_in_at(liveness_analysis* analysis, word postorder){
  return buffer_at(&analysis->live_in, postorder);
}

MUN_INLINE bit_vector*
liveness_get_live_out_at(liveness_analysis* analysis, word postorder){
  return buffer_at(&analysis->live_out, postorder);
}

MUN_INLINE bit_vector*
liveness_get_kill_at(liveness_analysis* analysis, word postorder){
  return buffer_at(&analysis->kill, postorder);
}

MUN_INLINE bit_vector*
liveness_get_live_in(liveness_analysis* analysis, block_entry_instr* block){
  return liveness_get_live_in_at(analysis, block->postorder_num);
}

MUN_INLINE bit_vector*
liveness_get_live_out(liveness_analysis* analysis, block_entry_instr* block){
  return liveness_get_live_out_at(analysis, block->postorder_num);
}

MUN_INLINE bit_vector*
liveness_get_kill(liveness_analysis* analysis, block_entry_instr* block){
  return liveness_get_kill_at(analysis, block->postorder_num);
}

void liveness_analyze(liveness_analysis* analysis);

MUN_INLINE void
liveness_init(liveness_analysis* analysis, word var_count, object_buffer* postorder){
  analysis->var_count = var_count;
  analysis->postorder = postorder;
  buffer_init(&analysis->live_in, postorder->size);
  buffer_init(&analysis->live_out, postorder->size);
  buffer_init(&analysis->kill, postorder->size);
}

HEADER_END

#endif