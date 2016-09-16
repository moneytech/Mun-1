#ifndef MUN_GRAPH_H
#define MUN_GRAPH_H

#include "../common.h"

HEADER_BEGIN

#include "../buffer.h"
#include "../codegen/il_entries.h"

typedef struct {
  graph_entry_instr* graph_entry;

  word current_ssa_temp_index;

  function* func;

  object_buffer preorder; // block_entry_instr*
  object_buffer postorder; // block_entry_instr*
  object_buffer reverse_postorder; // block_entry_instr*
} graph;

void graph_init(graph* g, function* func, graph_entry_instr* entry);

HEADER_END

#endif