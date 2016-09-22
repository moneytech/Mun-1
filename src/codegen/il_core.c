#include <mun/codegen/intermediate_language.h>
#include <mun/bitfield.h>
#include <mun/graph/graph.h>

void
instr_insert_after(instruction* instr, instruction* prev){
  instr->prev = prev;
  instr->next = prev->next;
  instr->next->prev = instr;
  instr->prev->next = instr;

  for(word i = instr_input_count(instr) - 1; i >= 0; i--){
    il_value* input = instr_input_at(instr, i);
    defn_add_input_use(input->defn, input);
  }
}

instruction*
instr_append(instruction* instr, instruction* tail){
  instr_link_to(instr, tail);
  for(word i = tail->ops->input_count(tail) - 1; i >= 0; i--){
    il_value* input = tail->ops->input_at(tail, i);
    defn_add_input_use(input->defn, input);
  }
  return tail;
}

void
instr_set_input_at(instruction* instr, word index, il_value* val){
  val->instr = instr;
  val->index = index;
  instr->ops->set_input_at(instr, index, val);
}

il_value*
instr_input_at(instruction* instr, word index){
  if(instr->ops->input_at != NULL) return instr->ops->input_at(instr, index);
  return NULL;
}

static word
instr_successor_count_(instruction* instr){
  return 0;
}

static representation
instr_get_input_representation(instruction* instr, word index){
  return kTagged;
}

static representation
instr_get_representation(instruction* instr){
  return kTagged;
}

void
instr_init(instruction* instr, instruction_tag tag, instruction_ops* ops){
  instr->next = NULL;
  instr->prev = NULL;
  instr->ops = ops;
  instr->is_def = FALSE;
  instr->tag = tag;
  instr->get_input_representation = &instr_get_input_representation;
  instr->get_representation = &instr_get_representation;
  if(ops->successor_count == NULL) ops->successor_count = &instr_successor_count_;
}

void
defn_replace_uses_with(definition* defn, definition* other){
  il_value* current = NULL;
  il_value* next = defn->input_use_list;

  if(next != NULL){
    while(next != NULL){
      current = next;
      current->defn = other;
      next = current->next;
    }

    next = other->input_use_list;
    current->next = next;
    if(next != NULL) next->prev = current;
    other->input_use_list = defn->input_use_list;
    defn->input_use_list = NULL;
  }
}

void
defn_add_input_use(definition* defn, il_value* val){
  value_add_to_list(val, &defn->input_use_list);
}

void
defn_init(definition* defn){
  defn->input_use_list = NULL;
  defn->constant_value = NULL;
  defn->ssa_temp_index = -1;
  defn->temp_index = -1;
  ((instruction*) defn)->is_def = TRUE;
}

bool
defn_has_input_use(definition* defn, il_value* val){
  return ((defn->input_use_list == val) && (val->next == NULL));
}

void
instr_compile(instruction* instr, asm_buff* code){
  if(instr->ops->emit_machine_code != NULL) {
    instr->ops->emit_machine_code(instr, code);
  }
}

static char*
parallel_move_name(){
  return "ParallelMove";
}

parallel_move_instr*
parallel_move_new(){
  parallel_move_instr* move = malloc(sizeof(parallel_move_instr));
  static instruction_ops ops = {
      NULL, // set_input_at
      NULL, // emit_machine_code
      NULL, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      NULL, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      NULL, // input_at
      &parallel_move_name, // name
  };
  instr_init(((instruction*) move), kParallelMoveTag, &ops);
  buffer_init(&move->moves, 4);
  return move;
}

static char*
goto_name(){
  return "GotoInstr";
}

static word
goto_successor_count(instruction* instr){
  return 1;
}

static block_entry_instr*
goto_successor_at(instruction* instr, word index){
  return ((block_entry_instr*) to_goto(instr)->successor);
}

goto_instr*
goto_new(join_entry_instr* join){
  goto_instr* go = malloc(sizeof(goto_instr));
  static instruction_ops ops = {
      NULL, // set_input_at
      NULL, // emit_machine_code
      NULL, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      NULL, // input_count
      &goto_successor_count, // successor_count
      &goto_successor_at, // successor_at
      NULL, // get_block
      NULL, // input_at
      &goto_name, // name
  };
  instr_init(((instruction*) go), kGotoTag, &ops);
  go->block = NULL;
  go->successor = join;
  go->parallel_move = NULL;
  return go;
}

static void
unuse_all_inputs(instruction* self){
  for(word i = instr_input_count(self) - 1; i >= 0; i--){
    value_remove_from_list(instr_input_at(self, i));
  }
}

instruction*
instr_remove_from_graph(instruction* self, graph* g, bool ret_prev){
  instruction* prev = self->prev;
  instruction* next = self->next;

  instr_link_to(prev, next);
  unuse_all_inputs(self);

  self->prev = NULL;
  self->next = NULL;

  return (ret_prev ? prev : next);
}