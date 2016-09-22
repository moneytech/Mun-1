#include <mun/codegen/ra_var_liveness.h>
#include <mun/buffer.h>
#include <mun/codegen/il_defs.h>

object_buffer*
var_analysis_compute_assigned_vars(variable_analysis* analysis){
  buffer_clear(&analysis->assigned_vars);

  word block_count = analysis->flow_graph->preorder.size;
  for(word i = 0; i < block_count; i++){
    block_entry_instr* block = analysis->flow_graph->preorder.data[i];

    bit_vector* kill = liveness_get_kill(((liveness_analysis*) analysis), block);
    bit_vector_intersect(kill, liveness_get_live_out(((liveness_analysis*) analysis), block));
    buffer_add(&analysis->assigned_vars, kill);
  }

  return &analysis->assigned_vars;
}

void
var_analysis_compute_initial_sets(liveness_analysis* liveness){
  variable_analysis* analysis = container_of(liveness, variable_analysis, analysis);

  bit_vector* last_loads = malloc(sizeof(bit_vector));
  bit_vector_init(last_loads, liveness->var_count);

  word block_count = liveness->postorder->size;
  for(word i = 0; i < block_count; i++){
    block_entry_instr* block = liveness->postorder->data[i];
    bit_vector* kill = liveness->kill.data[i];
    bit_vector* live_in = liveness->live_in.data[i];
    bit_vector_clear(last_loads);

    backward_instr_iter(block, it){
      load_local_instr* load = to_load_local(it);
      if(instr_is(it, kLoadLocalTag)){
        word index = local_var_bit_index(load->local, analysis->params);
        if(index >= liveness->live_in.size) continue;
        buffer_add(&liveness->live_in, word_clone(index));
        if(!bit_vector_contains(last_loads, index)){
          bit_vector_add(last_loads, index);
          load->is_last = TRUE;
        }
        continue;
      }

      store_local_instr* store = to_store_local(it);
      if(instr_is(it, kStoreLocalTag)){
        word index = local_var_bit_index(store->local, analysis->params);
        if(index >= liveness->live_in.size) continue;
        if(bit_vector_contains(kill, index)){
          if(!bit_vector_contains(live_in, index)){
            store->is_dead = TRUE;
          }
        } else{
          if(!bit_vector_contains(live_in, index)){
            store->is_last = TRUE;
          }
          bit_vector_add(kill, index);
        }
        bit_vector_remove(live_in, index);
        continue;
      }
    }
  }
}