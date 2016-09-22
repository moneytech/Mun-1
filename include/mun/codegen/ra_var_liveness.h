#ifndef MUN_RA_VA_LIVENESS_H
#define MUN_RA_VA_LIVENESS_H

#include <mun/bitvec.h>
#include "../common.h"

HEADER_BEGIN

#include "intermediate_language.h"
#include "ra_liveness.h"
#include "../graph/graph.h"
#include "../local.h"
#include "il_defs.h"

typedef struct{
  liveness_analysis analysis;

  word params;
  graph* flow_graph;
  object_buffer assigned_vars; // bit_vector*
} variable_analysis;

object_buffer* var_analysis_compute_assigned_vars(variable_analysis* analysis);

void var_analysis_compute_initial_sets(liveness_analysis* analysis);

MUN_INLINE void
var_analysis_init(variable_analysis* analysis, graph* g){
  liveness_init(((liveness_analysis*) analysis), graph_variable_count(g), &g->postorder);
  analysis->flow_graph = g;
  analysis->params = function_num_non_copied_params(((instance*) g->func));
  ((liveness_analysis*) analysis)->compute_initial_sets = &var_analysis_compute_initial_sets;
  buffer_init(&analysis->assigned_vars, 10);
}

MUN_INLINE bool
var_analysis_is_store_alive(variable_analysis* analysis, block_entry_instr* block, store_local_instr* store){
  if(store->is_dead) {
    return FALSE;
  }

  if(store->is_last){
    word index = local_var_bit_index(store->local, analysis->params);
    return bit_vector_contains(liveness_get_live_out(((liveness_analysis*) analysis), block), index);
  }

  return TRUE;
}

MUN_INLINE bool
var_analysis_is_last_load(variable_analysis* analysis, block_entry_instr* block, load_local_instr* load){
  word index = local_var_bit_index(load->local, analysis->params);
  return load->is_last &&
         !bit_vector_contains(liveness_get_live_in(((liveness_analysis*) analysis), block), index);
}

HEADER_END

#endif