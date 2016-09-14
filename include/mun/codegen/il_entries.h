#ifndef MUN_IL_ENTRIES_H
#define MUN_IL_ENTRIES_H

#include "../common.h"
#include "il_core.h"

HEADER_BEGIN

typedef struct _block_entry_instr{
  instruction instr;

  word preorder_num;
  word postorder_num;
  word start_pos;
  word end_pos;
  word offset;

  instruction* last;
} block_entry_instr;

typedef struct _target_entry_instr target_entry_instr;

typedef struct{
  block_entry_instr block;

  target_entry_instr* normal_entry;

  function* func;

  word entry_count;
  word spill_slot_count;
  word fixed_slot_count;
} graph_entry_instr;

graph_entry_instr* graph_entry_new(function* func, target_entry_instr* target);

struct _target_entry_instr{
  block_entry_instr block;
};

#define to_graph_entry(i) container_of(container_of(i, block_entry_instr, instr), graph_entry_intr, block)

HEADER_END

#endif