#include <mun/graph/graph.h>

void
graph_init(graph* self, function* func, graph_entry_instr* entry){
  self->current_ssa_temp_index = 0;
  self->func = func;
  self->graph_entry = entry;
}