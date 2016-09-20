#include <mun/codegen/ra_ssa_liveness.h>
#include <mun/codegen/il_value.h>
#include <mun/location.h>

#define get_ssa_liveness(p) container_of(p, ssa_liveness, analysis)

void
ssa_liveness_compute_initial_sets(liveness_analysis* analysis){
  ssa_liveness* ssa = get_ssa_liveness(analysis);

  for(word i = 0; i < analysis->postorder->size; i++){
    block_entry_instr* block = buffer_at(analysis->postorder, i);

    bit_vector* kill = liveness_get_kill_at(analysis, i);
    bit_vector* live_in = liveness_get_live_in_at(analysis, i);

    backward_instr_iter(block, it){
      instr_initialize_location_summary(it);

      location_summary* locs = it->locations;

      if(instr_is_definition(it)){
        definition* defn = container_of(it, definition, instr);
        if(defn_has_ssa_temp(defn)){
          bit_vector_add(kill, defn->ssa_temp_index);
          bit_vector_remove(live_in, defn->ssa_temp_index);
        }
      }

      for(word j = 0; j < instr_input_count(it); j++){
        il_value* input = instr_input_at(it, j);
        if(loc_is_constant(locs->inputs[j])) continue;
        bit_vector_add(live_in, input->defn->ssa_temp_index);
      }
    }
  }

  graph_entry_instr* entry = get_ssa_liveness(analysis)->entry;
  for(word i = 0; i < entry->initial_definitions.size; i++){
    definition* defn = buffer_at(&entry->initial_definitions, i);
    bit_vector_add(buffer_at(&analysis->kill, entry->block.postorder_num), defn->ssa_temp_index);
    bit_vector_add(buffer_at(&analysis->live_in, entry->block.postorder_num), defn->ssa_temp_index);
  }
}