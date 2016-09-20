#include <mun/bitfield.h>
#include <mun/location.h>
#include <mun/object.h>
#include <mun/codegen/ra_ssa_liveness.h>
#include <mun/graph/graph.h>
#include <mun/ast.h>
#include <mun/graph/graph_builder.h>
#include <mun/buffer.h>
#include <mun/codegen/ra_allocator.h>
#include <mun/codegen/intermediate_language.h>
#include <mun/codegen/il_entries.h>
#include <mun/codegen/il_core.h>
#include <mun/graph/visitors/constant_propagation.h>

int
main(int argc, char** argv){
  ast_node* code = sequence_node_new();
  sequence_node_append(code, return_node_new(literal_node_new(number_new(3.141592654))));

  instance* pi = function_new("pi", kMOD_NONE);
  to_function(pi)->ast = code;

  function_allocate_variables(pi);

  graph_builder builder;
  graph_builder_init(&builder, to_function(pi));

  graph* g = graph_build(&builder);
  graph_discover_blocks(g);
  graph_compute_ssa(g, 0);

  forward_instr_iter(((block_entry_instr*) g->graph_entry->normal_entry), it){
    if(instr_is_definition(it)){
      printf("$%li\n", container_of(it, definition, instr)->ssa_temp_index);
      for(int i = 0; i < instr_input_count(it); i++){
        printf("\t-> $%li\n", instr_input_at(it, i)->defn->ssa_temp_index);
      }
    }
  }

  printf("Initial Definitions:\n");
  for(word i = 0; i < g->graph_entry->initial_definitions.size; i++){
    printf("\t\t--> %s\n", ((instruction*) g->graph_entry->initial_definitions.data[i])->ops->name());
    printf("\t\t #%li\n", ((definition*) g->graph_entry->initial_definitions.data[i])->ssa_temp_index);
    printf("\t\t <%s>\n", lua_to_string(((constant_instr*) g->graph_entry->initial_definitions.data[i])->value));
  }

  graph_allocator galloc;
  graph_alloc_init(&galloc, g);
  graph_alloc_regs(&galloc);

  instruction* instr = ((instruction*) g->graph_entry->normal_entry);
  while((instr != NULL)){
    printf("Emitting Machine Code For: '%s'\n", instr->ops->name());
    instr->ops->emit_machine_code(instr, NULL);
    instr = instr->next;
  }
  return 0;
}