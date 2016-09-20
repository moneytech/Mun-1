#ifndef MUN_RA_SSA_LIVENESS_H
#define MUN_RA_SSA_LIVENESS_H

#include <mun/graph/graph.h>
#include <mun/buffer.h>
#include "../common.h"

HEADER_BEGIN

#include "ra_liveness.h"
#include "../graph/graph.h"

typedef struct{
  liveness_analysis analysis;

  graph_entry_instr* entry;
} ssa_liveness;

void ssa_liveness_compute_initial_sets(liveness_analysis* analysis);

MUN_INLINE void
ssa_liveness_init(ssa_liveness* liveness, graph* g){
  liveness_init(((liveness_analysis*) liveness), g->current_ssa_temp_index, &g->postorder);
  liveness->entry = g->graph_entry;
  liveness->analysis.compute_initial_sets = &ssa_liveness_compute_initial_sets;
}

HEADER_END

#endif