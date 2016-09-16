#include <mun/asm/core.h>
#include <mun/codegen/il_core.h>
#include <mun/codegen/il_value.h>
#include <mun/codegen/il_defs.h>
#include <mun/asm/x64.h>
#include <mun/buffer.h>
#include <mun/ast.h>
#include <mun/graph/graph_builder.h>
#include <mun/graph/graph.h>
#include <mun/object.h>
#include <mun/codegen/il_entries.h>

int
main(int argc, char** argv){
  ast_node* seq = sequence_node_new();

  ast_node* test = literal_node_new(boolean_new(TRUE));
  local_variable* local_1 = local_var_new("test");
  sequence_node_append(seq, store_local_node_new(local_1, test));

  ast_node* ret = return_node_new(load_local_node_new(local_1));
  sequence_node_append(seq, ret);

  function* thirty = to_function(function_new("thirty", kMOD_NONE));
  thirty->ast = seq;

  graph_builder builder;
  graph_builder_init(&builder, thirty);

  graph* g = graph_build(&builder);

  instruction* instr = ((instruction*) g->graph_entry->normal_entry);
  while(instr != NULL){
    printf("%s\n", instr->ops->name());
    instr = instr->next;
  }
  return 0;
}