#include <mun/codegen/intermediate_language.h>

il_value*
value_new(definition* defn){
  il_value* val = malloc(sizeof(il_value));
  val->instr = NULL;
  val->defn = defn;
  val->next = NULL;
  val->prev = NULL;
  val->instr = 0;
  return val;
}

void
value_remove_from_list(il_value* val){
  definition* defn = val->defn;
  il_value* next = val->next;
  if(val == defn->input_use_list) {
    defn->input_use_list = next;
    if (next != NULL) next->prev = NULL;
  }else{
    il_value* prev = val->prev;
    prev->next = next;
    if(next != NULL) next->prev = prev;
  }

  val->next = NULL;
  val->prev = NULL;
}

void
value_add_input_use(definition* defn, il_value* val){
  value_add_to_list(val, &defn->input_use_list);
}