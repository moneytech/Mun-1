#include <mun/iterator.h>
#include <mun/bitvec.h>
#include <mun/graph/graph.h>
#include <mun/codegen/ra_var_liveness.h>
#include <mun/codegen/intermediate_language.h>
#include <mun/buffer.h>
#include <mun/codegen/il_defs.h>
#include <mun/codegen/il_core.h>
#include <mun/object.h>

void
graph_init(graph* self, function* func, graph_entry_instr* entry){
  self->current_ssa_temp_index = 0;
  self->func = func;
  self->graph_entry = entry;
  buffer_init(&self->preorder, 10);
  buffer_init(&self->postorder, 10);
  buffer_init(&self->reverse_postorder, 10);
}

MUN_INLINE word
minimum(word a, word b){
  return a < b ? a : b;
}

static void
compress_path(word start, word current, object_buffer* /* word */ parent, object_buffer* /* word */ label){
  word next_index = (*((word*) parent->data[current]));
  if(next_index > start){
    compress_path(start, next_index, parent, label);
    (*((word*) label->data[current])) = minimum(*((word*) label->data[current]), *((word*) label->data[next_index]));
    (*((word*) parent->data[current])) = (*((word*) parent->data[next_index]));
  }
}

static void
compute_dominators(graph* g, object_buffer* /* bit_vector* */ dominance){
  word size = g->parent.size;

  object_buffer idom; // word*
  buffer_init(&idom, size);

  object_buffer semi; // word*
  buffer_init(&semi, size);

  object_buffer label; // word*
  buffer_init(&label, size);

  for(word i = 0; i < size; i++){
    buffer_add(&idom, g->parent.data[i]);
    buffer_add(&semi, word_clone(i));
    buffer_add(&label, word_clone(i));

    bit_vector* vec = malloc(sizeof(bit_vector));
    bit_vector_init(vec, size);
    buffer_add(dominance, vec);
  }

  buffer_clear(&((block_entry_instr*) g->preorder.data[0])->dominated);
  for(word block_index = size - 1; block_index >= 1; block_index--){
    block_entry_instr* b = g->preorder.data[block_index];
    buffer_clear(&b->dominated);

    for(word i = 0; i < block_predecessor_count(b); i++){
      block_entry_instr* pred = block_predecessor_at(b, i);
      word pred_index = pred->preorder_num;
      word best = pred_index;

      if(pred_index > block_index){
        compress_path(block_index, pred_index, &g->parent, &label);
        best = *((word*) label.data[pred_index]);
      }

      (*((word*) semi.data[block_index])) = minimum(*((word*) semi.data[block_index]), *((word*) semi.data[best]));
    }

    (*((word*) label.data[block_index])) = (*((word*) semi.data[block_index]));
  }

  for(word block_index = 1; block_index < size; block_index++){
    word dom_index = (*((word*) idom.data[block_index]));
    while(dom_index > (*((word*) semi.data[block_index]))){
      dom_index = (*((word*) idom.data[dom_index]));
    }
    (*((word*) idom.data[block_index])) = dom_index;
    block_add_dominated(g->preorder.data[dom_index], g->preorder.data[block_index]);
  }

  for(word block_index = 0; block_index < size; block_index++){
    block_entry_instr* b = g->preorder.data[block_index];
    word count = block_predecessor_count(b);
    if(count <= 1) continue;
    for(word i = 0; i < count; i++){
      block_entry_instr* runner = block_predecessor_at(b, i);
      while(runner != b->dominator){
        bit_vector_add(dominance->data[runner->preorder_num], block_index);
        runner = runner->dominator;
      }
    }
  }
}

static void
insert_phis(graph* g, object_buffer* assigned_vars, object_buffer* dominance, object_buffer* /* phi_instr* */ phis){
  word block_count = g->preorder.size;

  object_buffer has_already;
  buffer_init(&has_already, block_count);

  object_buffer work;
  buffer_init(&work, block_count);

  for(word block_index = 0; block_index < block_count; block_index++){
    buffer_add(&has_already, word_clone(-1));
    buffer_add(&work, word_clone(-1));
  }

  object_buffer worklist;
  buffer_init(&worklist, block_count);

  for(word var_index = 0; var_index < graph_variable_count(g); var_index++){
    for(word block_index = 0; block_index < block_count; block_index++){
      if(bit_vector_contains(buffer_at(assigned_vars, block_index), var_index)){
        *((word*) work.data[block_index]) = var_index;
        buffer_add(&worklist, buffer_at(&g->preorder, block_index));
      }
    }

    while(!buffer_is_empty(&worklist)){
      block_entry_instr* block = buffer_del_last(&worklist);

      bit_vector_foreach(buffer_at(dominance, block->preorder_num)){
        if((*((word*) buffer_at(&has_already, bit_vector_current))) < var_index){
          block_entry_instr* b = buffer_at(&g->preorder, bit_vector_current);
          phi_instr* phi = join_insert_phi(((join_entry_instr*) b), var_index, graph_variable_count(g));
          phi->alive = TRUE;
          buffer_add(phis, phi);
          *((word*) has_already.data[bit_vector_current]) = var_index;
          if((*((word*) work.data[bit_vector_current])) < var_index){
            *((word*) work.data[bit_vector_current]) = var_index;
            buffer_add(&worklist, b);
          }
        }
      }
    }
  }
}

static void graph_rename_recursive(graph* g, block_entry_instr* block, object_buffer* /* definition* */ env, object_buffer* /* phi_instr* */ live_phis, variable_analysis* var_liveness);

static void
graph_rename(graph* g, object_buffer* /* phi_instr* */ live_phis, variable_analysis* var_liveness){
  object_buffer env; // definition*
  buffer_init(&env, graph_variable_count(g));

  word count = g->func->def.num_params;
  for(word i = 0; i < count; i++){
    // insert params
  }

  constant_instr* constant_null = graph_get_constant(g, nil_new());

  for(word i = g->func->def.num_params; i < graph_variable_count(g); i++){
    env.data[i] = constant_null;
  }

  graph_rename_recursive(g, ((block_entry_instr*) g->graph_entry), &env, live_phis, var_liveness);
}

void
graph_rename_recursive(graph* g, block_entry_instr* block, object_buffer* /* definition* */ env, object_buffer* /* phi_instr* */ live_phis, variable_analysis* var_liveness){
  if(instr_is(((instruction*) block), kJoinEntryTag)){
    join_entry_instr* join = container_of(block, join_entry_instr, block);
    if(join->phis != NULL){
      for(word i = 0; i < join->phis->size; i++){
        phi_instr* phi = join->phis->data[i];
        if(phi != NULL){
          env->data[i] = phi;
          graph_alloc_ssa_index(g, ((definition*) phi));
        }
      }
    }
  }

  constant_instr* c_null = graph_get_constant(g, nil_new());

  bit_vector* live_in = liveness_get_live_in(((liveness_analysis*) var_liveness), block);
  for(word i = 0; i < graph_variable_count(g); i++){
    if(!bit_vector_contains(live_in, i)){
      env->data[i] = c_null;
    }
  }

  // Attach_environment?

  instruction* it = ((instruction*) block)->next;
  while(it != NULL){
    for(word i = instr_input_count(it) - 1; i >= 0; i--){
      il_value* v = instr_input_at(it, i);
      definition* reach = buffer_del_last(env);
      definition* input = v->defn;

      if(input != reach){
        v->defn = reach;
        input = reach;
      }

      defn_add_input_use(input, v);
    }

    for(word j = 0; j < instr_argument_count(it); j++){
      buffer_del_last(env);
    }

    if(instr_is_definition(it)){
      definition* defn = container_of(it, definition, instr);

      if(instr_is(it, kStoreLocalTag) ||
         instr_is(it, kLoadLocalTag) ||
         instr_is(it, kDropTag) ||
         instr_is(it, kConstantTag)){
        definition* result = NULL;

        if(instr_is(it, kStoreLocalTag)){
          store_local_instr* sli = to_store_local(it);

          word index = local_var_bit_index(sli->local, function_num_non_copied_params(((instance*) g->func)));
          result = sli->inputs[0]->defn;

          if(var_analysis_is_store_alive(var_liveness, block, sli)){
            env->data[index] = result;
          } else{
            env->data[index] = c_null;
          }
        } else if(instr_is(it, kLoadLocalTag)){
          load_local_instr* lli = to_load_local(it);

          word index = local_var_bit_index(lli->local, function_num_non_copied_params(((instance*) g->func)));
          result = env->data[index];

          phi_instr* phi = to_phi(((instruction*) result));
          if(instr_is(((instruction*) result), kPhiTag) && !phi->alive){
            phi->alive = TRUE;
            buffer_add(live_phis, phi);
          }

          if(var_analysis_is_last_load(var_liveness, block, lli)){
            env->data[index] = c_null;
          }
        } else if(instr_is(it, kDropTag)){
          //TODO: Drop
        } else{
          result = ((definition*) graph_get_constant(g, to_constant(it)->value));
        }

        if(defn->temp_index >= 0){
          buffer_add(env, result);
        }

        it = instr_remove_from_graph(it, g, TRUE);
      } else{
        if(defn->temp_index >= 0){
          graph_alloc_ssa_index(g, defn);
          buffer_add(env, defn);
        }
      }
    }

    for(word i = 0; i < block->dominated.size; i++){
      block_entry_instr* b = block->dominated.data[i];
      object_buffer new_env; // definition*
      buffer_init(&new_env, env->size);
      buffer_add_all(&new_env, env);
      graph_rename_recursive(g, b, &new_env, live_phis, var_liveness);
    }

    if((instr_successor_count(block->last) == 1) && instr_is(((instruction*) instr_successor_at(block->last, 0)), kJoinEntryTag)){
      join_entry_instr* successor = to_join_entry(((instruction*) instr_successor_at(block->last, 0)));
      word pred_index = join_index_of_predecessor(successor, block);
      if(successor->phis != NULL){
        for(word i = 0; i < successor->phis->size; i++){
          phi_instr* phi = successor->phis->data[i];
          if(phi != NULL){
            il_value* use = value_new(env->data[i]);
            instr_set_input_at(((instruction*) phi), pred_index, use);
          }
        }
      }
    }

    it = it->next;
  }

  for(word i = 0; i < block->dominated.size; i++){
    block_entry_instr* b = block->dominated.data[i];
    object_buffer new_env;
    buffer_init(&new_env, env->size);
    buffer_add_all(&new_env, env);

    graph_rename_recursive(g, b, &new_env, live_phis, var_liveness);
  }

  if((instr_successor_count(block->last) == 1) && instr_is(((instruction*) instr_successor_at(block->last, 0)), kJoinEntryTag)){
    join_entry_instr* join = to_join_entry(((instruction*) instr_successor_at(block->last, 0)));
    word pred_index = join_index_of_predecessor(join, block);
    if(join->phis != NULL){
      for(word i = 0; i < join->phis->size; i++){
        phi_instr* phi = join->phis->data[i];
        if(phi != NULL){
          il_value* val = value_new(env->data[i]);
          instr_set_input_at(((instruction*) phi), pred_index, val);
        }
      }
    }
  }
}

void
graph_compute_ssa(graph* self, word next_vreg){
  self->current_ssa_temp_index = next_vreg;

  object_buffer dominance;
  buffer_init(&dominance, next_vreg);
  compute_dominators(self, &dominance);

  variable_analysis var_liveness;
  var_analysis_init(&var_liveness, self);
  liveness_analyze(((liveness_analysis*) &var_liveness));

  object_buffer live_phis;
  buffer_init(&live_phis, 10); //TODO: unhardcode size

  insert_phis(self, var_analysis_compute_assigned_vars(&var_liveness), &dominance, &live_phis);
  graph_rename(self, &live_phis, &var_liveness);
}

typedef struct{
  block_entry_instr* block;
  word next_successor;
} block_traversal_state;

MUN_INLINE bool
state_has_next_successor(block_traversal_state* state){
  return state->next_successor >= 0;
}

MUN_INLINE block_entry_instr*
state_next_successor(block_traversal_state* state){
  return instr_successor_at(state->block->last, state->next_successor--);
}

void
graph_discover_blocks(graph* g){
  buffer_clear(&g->preorder);
  buffer_clear(&g->postorder);
  buffer_clear(&g->reverse_postorder);
  buffer_clear(&g->parent);

  object_buffer block_stack; // block_traversal_state
  buffer_init(&block_stack, 10);
  block_discover_blocks(((block_entry_instr*) g->graph_entry), NULL, &g->preorder, &g->parent);
  block_traversal_state init_state = {
      ((block_entry_instr*) g->graph_entry),
      (instr_successor_count(((block_entry_instr*) g->graph_entry)->last) - 1)
  };
  buffer_add(&block_stack, &init_state);

  while(!buffer_is_empty(&block_stack)){
    block_traversal_state* state = buffer_last(&block_stack);

    if(state_has_next_successor(state)){
      block_entry_instr* successor = state_next_successor(state);
      if(block_discover_blocks(successor, state->block, &g->preorder, &g->parent)){
        block_traversal_state new_state = {
            successor,
            (instr_successor_count(successor->last) - 1)
        };
        buffer_add(&block_stack, &new_state);
      }
    } else{
      buffer_del_last(&block_stack);
      state->block->postorder_num = g->postorder.size;
      buffer_add(&g->postorder, state->block);
    }
  }

  word block_count = g->postorder.size;
  for(word i = 0; i < block_count; i++){
    buffer_add(&g->reverse_postorder, g->postorder.data[block_count - i - 1]);
  }
}