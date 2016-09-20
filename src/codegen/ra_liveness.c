#include <mun/codegen/ra_liveness.h>

static bool
liveness_update_live_out(liveness_analysis* analysis, block_entry_instr* block){
  bit_vector* live_out = liveness_get_live_out(analysis, block);
  bool changed = FALSE;

  instruction* last = block->last;
  for(word i = 0; i < instr_successor_count(last); i++){
    block_entry_instr* successor = instr_successor_at(last, i);
    if(bit_vector_add_all(live_out, liveness_get_live_in(analysis, successor))){
      changed = TRUE;
    }
  }

  return changed;
}

static bool
liveness_update_live_in(liveness_analysis* analysis, block_entry_instr* block){
  bit_vector* live_out = liveness_get_live_out(analysis, block);
  bit_vector* kill = liveness_get_kill(analysis, block);
  bit_vector* live_in = liveness_get_live_in(analysis, block);
  return bit_vector_kill_and_add(live_in, kill, live_out);
}

static void
liveness_compute_live_sets(liveness_analysis* analysis){
  word block_count = analysis->postorder->size;

  bool changed;
  do{
    changed = FALSE;

    for(word i = 0; i < block_count; i++){
      block_entry_instr* block = buffer_at(analysis->postorder, i);
      if(liveness_update_live_out(analysis, block) && liveness_update_live_in(analysis, block)){
        changed = TRUE;
      }
    }
  } while(changed);
}

MUN_INLINE bit_vector*
bit_vec_new(word size){
  bit_vector* vec = malloc(sizeof(bit_vector));
  bit_vector_init(vec, size);
  return vec;
}

void
liveness_analyze(liveness_analysis* analysis){
  for(word i = 0; i < analysis->postorder->size; i++){
    buffer_add(&analysis->live_out, bit_vec_new(analysis->var_count));
    buffer_add(&analysis->kill, bit_vec_new(analysis->var_count));
    buffer_add(&analysis->live_in, bit_vec_new(analysis->var_count));
  }

  analysis->compute_initial_sets(analysis);
  liveness_compute_live_sets(analysis);
}