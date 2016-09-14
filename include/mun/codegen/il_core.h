#ifndef MUN_IL_CORE_H
#define MUN_IL_CORE_H

#include "../common.h"
#include "../object.h"

HEADER_BEGIN

typedef enum{
  kUnboxedNumber,
  kBoxedNumber,
  kTagged,
  kNone
} representation;

typedef struct _asm_buff asm_buff;

typedef struct _instruction instruction;
typedef struct _definition definition;
typedef struct _il_value il_value;
typedef struct _block_entry_instr block_entry_instr;

typedef struct{
  void (*set_input_at)(instruction*, word, il_value*);
  void (*emit_machine_code)(instruction*, asm_buff*);

  definition* (*argument_at)(instruction*, word);

  word (*argument_count)(instruction*);
  word (*input_count)(instruction*);
  word (*successor_count)(instruction*);

  block_entry_instr* (*successor_at)(instruction*, word);
  block_entry_instr* (*get_block)(instruction*);

  il_value* (*input_at)(instruction*, word);

  char* (*name)();
} instruction_ops;

struct _instruction{
  instruction* next;
  instruction* prev;

  representation rep;

  const instruction_ops* ops;
};

void instr_set_input_at(instruction* instr, word index, il_value* val);
void instr_insert_after(instruction* instr, instruction* prev);
void instr_init(instruction* instr, const instruction_ops* ops);
void instr_compile(instruction* instr, asm_buff* code);

MUN_INLINE void
instr_link_to(instruction* instr, instruction* next){
  instr->next = next;
  next->prev = instr;
}

MUN_INLINE void
instr_insert_before(instruction* instr, instruction* next){
  instr_insert_after(next->prev, instr);
}

il_value* instr_input_at(instruction* instr, word index);

instruction* instr_append(instruction* instr, instruction* tail);

struct _definition{
  instruction instr;

  word temp_index;
  word ssa_temp_index;

  il_value* input_use_list;

  instance* constant_value;
};

void defn_replace_uses_with(definition* defn, definition* other);
void defn_add_input_use(definition* defn, il_value* val);
void defn_init(definition* defn);

bool defn_has_input_use(definition* defn, il_value* use);

MUN_INLINE bool
defn_has_uses(definition* defn){
  return (defn->input_use_list != NULL);
}

HEADER_END

#endif