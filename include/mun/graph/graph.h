#ifndef MUN_GRAPH_H
#define MUN_GRAPH_H

#include <mun/object.h>
#include "../common.h"

HEADER_BEGIN

#include "../buffer.h"
#include "../codegen/intermediate_language.h"

typedef struct _graph{
  graph_entry_instr* graph_entry;

  word current_ssa_temp_index;

  function* func;

  object_buffer parent; // word
  object_buffer preorder; // block_entry_instr*
  object_buffer postorder; // block_entry_instr*
  object_buffer reverse_postorder; // block_entry_instr*
} graph;

void graph_init(graph* g, function* func, graph_entry_instr* entry);
void graph_compute_ssa(graph* g, word next_vreg);
void graph_discover_blocks(graph* g);

MUN_INLINE word
graph_alloc_ssa_temp(graph* g){
  return g->current_ssa_temp_index++;
}

MUN_INLINE void
graph_alloc_ssa_index(graph* g, definition* defn){
  defn->ssa_temp_index = graph_alloc_ssa_temp(g);
  graph_alloc_ssa_temp(g);
}

MUN_INLINE void
graph_add_to_initial_definitions(graph* g, definition* defn){
  defn->instr.prev = ((instruction*) g->graph_entry);
  buffer_add(&g->graph_entry->initial_definitions, defn);
}

MUN_INLINE constant_instr*
graph_get_constant(graph* g, instance* val){
  constant_instr* c = constant_new(val);
  c->defn.ssa_temp_index = graph_alloc_ssa_temp(g);
  graph_add_to_initial_definitions(g, ((definition*) c));
  return c;
}

MUN_INLINE word
graph_variable_count(graph* g){
  return g->func->def.num_copied_params +
         function_num_non_copied_params(((instance*) g->func)) +
         g->func->def.num_stack_locals;
}

HEADER_END

#endif