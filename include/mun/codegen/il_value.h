#ifndef MUN_IL_VALUE_H
#define MUN_IL_VALUE_H

#include "../common.h"

HEADER_BEGIN

typedef struct _definition definition;
typedef struct _instruction instruction;

typedef struct _il_value{
  definition* defn;
  instruction* instr;
  struct _il_value* next;
  struct _il_value* prev;
  word index;
} il_value;

il_value* value_new(definition* defn);

void value_remove_from_list(il_value* val);
void value_add_input_use(definition* defn, il_value* val);

MUN_INLINE void
value_bind_to(il_value* val, definition* defn){
  value_remove_from_list(val);
  val->defn = defn;
  value_add_input_use(defn, val);
}

MUN_INLINE void
value_add_to_list(il_value* val, il_value** list){
  il_value* next = *list;
  *list = val;
  val->next = next;
  val->prev = NULL;
  if(next != NULL) next->prev = val;
}

HEADER_END

#endif