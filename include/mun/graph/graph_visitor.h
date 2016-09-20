#ifndef MUN_GRAPH_VISITOR_H
#define MUN_GRAPH_VISITOR_H

#include "../common.h"

HEADER_BEGIN

#include "../codegen/instructions.h"
#include "../buffer.h"
#include "graph.h"

typedef struct _graph_visitor{
  object_buffer* block_order;

#define DEFINE_VISIT_FUNC(Name) \
  void (*visit_##Name)(struct _graph_visitor*,instruction*);
  FOR_EACH_INSTRUCTION(DEFINE_VISIT_FUNC)
#undef DEFINE_VISIT_FUNC
} graph_visitor;

void visit_blocks(graph_visitor* vis);

HEADER_END

#endif