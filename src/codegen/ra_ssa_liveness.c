#include <mun/codegen/ra_ssa_liveness.h>
#include <mun/codegen/il_value.h>
#include <mun/location.h>
#include <mun/buffer.h>
#include <mun/codegen/il_core.h>
#include <mun/codegen/il_entries.h>

#define get_ssa_liveness(p) container_of(p, ssa_liveness, analysis)

void
ssa_liveness_compute_initial_sets(liveness_analysis* analysis){
  ssa_liveness* ssa = get_ssa_liveness(analysis);

  word block_count = analysis->postorder->size;
  for(word i = 0; i < block_count; i++){
    block_entry_instr* block = analysis->postorder->data[i];

    bit_vector* kill = analysis->kill.data[i];
    bit_vector* live_in = analysis->live_in.data[i];

    backward_instr_iter(block, it){
      location_summary* locs = it->locations = it->ops->make_location_summary(it);

      if(it->is_def){
        definition* defn = container_of(it, definition, instr);
        if(defn->ssa_temp_index >= 0){
          bit_vector_add(kill, defn->ssa_temp_index);
          bit_vector_remove(live_in, defn->ssa_temp_index);
        }

        for(word j = 0; j < instr_input_count(it); j++){
          il_value* input = instr_input_at(it, j);
          if(loc_is_constant(locs->inputs[j])) continue;
          bit_vector_add(live_in, input->defn->ssa_temp_index);
        }
      }

      if(instr_is(((instruction*) block), kJoinEntryTag)){
        join_entry_instr* join = to_join_entry(((instruction*) block));
        phis_foreach(join){
          phi_instr* phi = it_current;
          bit_vector_add(kill, ((definition*) phi)->ssa_temp_index);
          bit_vector_remove(live_in, ((definition*) phi)->ssa_temp_index);

          for(word k = 0; k < instr_input_count(((instruction*) phi)); k++){
            il_value* val = instr_input_at(((instruction*) phi), k);
            if(instr_is(((instruction*) val->defn), kConstantTag)) continue;

            block_entry_instr* pred = block_predecessor_at(block, k);
            word use = val->defn->ssa_temp_index;
            if(!bit_vector_contains(analysis->kill.data[pred->preorder_num], use)){
              bit_vector_add(analysis->live_in.data[pred->preorder_num], use);
            }
          }
        }
      }
    }
  }

  for(word i = 0; i < ssa->entry->initial_definitions.size; i++){
    definition* defn = ssa->entry->initial_definitions.data[i];
    word vreg = defn->ssa_temp_index;

    bit_vector_add(analysis->kill.data[((block_entry_instr*) ssa->entry)->postorder_num], vreg);
    bit_vector_remove(analysis->live_in.data[((block_entry_instr*) ssa->entry)->postorder_num], vreg);
  }
}