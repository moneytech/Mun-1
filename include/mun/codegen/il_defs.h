#ifndef MUN_IL_DEFS_H
#define MUN_IL_DEFS_H

#if !defined(MUN_INTERMEDIATE_LANGUAGE_H)
#error "Please #include <mun/codegen/intermediate_language.h> directly"
#endif

#include "../common.h"

HEADER_BEGIN

#include "intermediate_language.h"
#include "../bitvec.h"

typedef struct _local_variable local_variable;

typedef struct{
  definition defn;

  il_value* inputs[1];
} return_instr;

return_instr* return_new(il_value* val);

typedef struct _constant_instr{
  definition defn;

  instance* value;
} constant_instr;

constant_instr* constant_new(instance* val);

typedef struct{
  definition defn;

  int operation;
  il_value* inputs[2];
} binary_op_instr;

binary_op_instr* binary_op_new(int oper, il_value* left, il_value* right);

typedef struct{
  definition defn;

  local_variable* local;
  bool is_last;
} load_local_instr;

load_local_instr* load_local_new(local_variable* local);

typedef struct{
  definition defn;

  local_variable* local;
  bool is_dead;
  bool is_last;
  il_value* inputs[1];
} store_local_instr;

store_local_instr* store_local_new(local_variable* local, il_value* val);

typedef struct _phi_instr{
  definition defn;

  bool alive;
  join_entry_instr* block;
  object_buffer inputs; // il_value*
  bit_vector* reaching;
} phi_instr;

phi_instr* phi_new(join_entry_instr* join, word num_inputs);

#define to_return(i) container_of(container_of(i, definition, instr), return_instr, defn)
#define to_constant(i) container_of(container_of(i, definition, instr), constant_instr, defn)
#define to_binary_op(i) container_of(container_of(i, definition, instr), binary_op_instr, defn)
#define to_load_local(i) container_of(container_of(i, definition, instr), load_local_instr, defn)
#define to_store_local(i) container_of(container_of(i, definition, instr), store_local_instr, defn)
#define to_phi(i) container_of(container_of(i, definition, instr), phi_instr, defn)

typedef struct{
  iterator iter;

  word index;
  object_buffer* phis; // phi_instr*
} phi_iterator;

phi_iterator* phi_iterator_new(join_entry_instr* join);

#define phis_foreach(join) foreach(phi_iterator_new(join))

#define phis_current \
  ((phi_instr*) it_current)

HEADER_END

#endif