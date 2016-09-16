#include <mun/codegen/il_entries.h>

static char*
graph_name(){
  return "GraphEntry";
}

graph_entry_instr*
graph_entry_new(function* func, target_entry_instr* target){
  graph_entry_instr* g = malloc(sizeof(graph_entry_instr));
  static const instruction_ops ops = {
      NULL, // set_input_at
      NULL, // emit_machine_code
      NULL, // argument_at
      NULL, // argument_count
      NULL, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      NULL, // input_at
      &graph_name, // name
  };
  instr_init(((instruction*) g), &ops);
  defn_init(((definition*) g));
  g->func = func;
  g->normal_entry = target;
  g->entry_count = 0;
  g->spill_slot_count = 0;
  return g;
}

static char*
target_name(){
  return "TargetEntry";
}

target_entry_instr*
target_entry_new(){
  target_entry_instr* t = malloc(sizeof(target_entry_instr));
  static const instruction_ops ops = {
      NULL, // set_input_at
      NULL, // emit_machine_code
      NULL, // argument_at
      NULL, // argument_count
      NULL, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      NULL, // input_at
      &target_name, // name
  };
  instr_init(((instruction*) t), &ops);
  defn_init(((definition*) t));
  return t;
}