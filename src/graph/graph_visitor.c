#include <mun/codegen/intermediate_language.h>
#include <mun/graph/graph_visitor.h>

void
visit_blocks(graph_visitor* vis){
  for(word i = 0; i < vis->block_order->size; i++){
    block_entry_instr* block = vis->block_order->data[i];
    ((instruction*) block)->ops->accept(((instruction*) block), vis);
    forward_instr_iter(block, it){
      it->ops->accept(it, vis);
    }
  }
}