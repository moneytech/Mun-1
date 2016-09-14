#include <mun/codegen/il_core.h>
#include <mun/codegen/il_value.h>

void
instr_insert_after(instruction* instr, instruction* prev){
  instr->prev = prev;
  instr->next = prev->next;
  instr->next->prev = instr;
  instr->prev->next = instr;

  for(word i = instr->ops->input_count(instr) - 1; i >= 0; i--){
    il_value* use = instr->ops->input_at(instr, i);
    defn_add_input_use(use->defn, use);
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
instr_successor_count(instruction* instr){
  return 0;
}

static block_entry_instr*
instr_successor_at(instruction* instr, word index){
  return NULL;
}

void
instr_init(instruction* instr, const instruction_ops* ops){
  instr->next = NULL;
  instr->prev = NULL;
  instr->rep = kTagged;
  instr->ops = ops;
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
}

bool
defn_has_input_use(definition* defn, il_value* val){
  return ((defn->input_use_list == val) && (val->next == NULL));
}

void
instr_compile(instruction* instr, asm_buff* code){
  if(instr->ops->emit_machine_code != NULL) {
    printf("Emitting machine code for %s\n", instr->ops->name());
    instr->ops->emit_machine_code(instr, code);
  }
}