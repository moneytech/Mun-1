#ifndef MUN_CONSTANT_PROPAGATION_H
#define MUN_CONSTANT_PROPAGATION_H

#include "../../common.h"

HEADER_BEGIN

#include "../../bitvec.h"
#include "../../buffer.h"
#include "../graph_visitor.h"

typedef struct{
  object_buffer defs; // definition*
  bit_vector contains;
} definition_worklist;

MUN_INLINE bool
def_worklist_contains(definition_worklist* defs, definition* def){
  return (def->ssa_temp_index >= 0) &&
         bit_vector_contains(&defs->contains, def->ssa_temp_index);
}

MUN_INLINE bool
def_worklist_is_empty(definition_worklist* defs){
  return buffer_is_empty(&defs->defs);
}

MUN_INLINE void
def_worklist_init(definition_worklist* defs, graph* g, word init){
  buffer_init(&defs->defs, init);
  bit_vector_init(&defs->contains, g->current_ssa_temp_index);
}

MUN_INLINE void
def_worklist_add(definition_worklist* defs, definition* defn){
  if(!def_worklist_contains(defs, defn)){
    buffer_add(&defs->defs, defn);
    bit_vector_add(&defs->contains, defn->ssa_temp_index);
  }
}

MUN_INLINE definition*
def_worklist_del_last(definition_worklist* defs){
  definition* defn = buffer_del_last(&defs->defs);
  bit_vector_remove(&defs->contains, defn->ssa_temp_index);
  return defn;
}

typedef struct{
  graph_visitor vis;
  graph* flow_graph;
  object_buffer block_worklist; // block_entry_instr*
  definition_worklist worklist;
  bit_vector reachable;
} constant_propagator;

void cp_init(constant_propagator* cp, graph* g);
void cp_analyze(constant_propagator* cp);
void cp_transform(constant_propagator* cp);

#define get_cp(gvis) container_of(gvis, constant_propagator, vis)

HEADER_END

#endif