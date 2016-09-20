#ifndef MUN_IL_ENTRIES_H
#define MUN_IL_ENTRIES_H

#if !defined(MUN_INTERMEDIATE_LANGUAGE_H)
#error "Please #include <mun/codegen/intermediage_language.h> directly"
#endif

#include "../common.h"

HEADER_BEGIN

#include "intermediate_language.h"
#include "../buffer.h"

typedef struct _block_entry_instr{
  instruction instr;

  word preorder_num;
  word postorder_num;
  word start_pos;
  word end_pos;
  word offset;

  instruction* last;

  object_buffer dominated; // block_entry_instr*

  struct _block_entry_instr* dominator;

  word (*predecessor_count)(block_entry_instr*);

  void (*add_predecessor)(block_entry_instr*, block_entry_instr*);
  void (*clear_predecessors)(block_entry_instr*);

  block_entry_instr* (*predecessor_at)(block_entry_instr*, word);

  parallel_move_instr* parallel_move;
} block_entry_instr;

MUN_INLINE word
block_predecessor_count(block_entry_instr* block){
  return block->predecessor_count(block);
}

MUN_INLINE block_entry_instr*
block_predecessor_at(block_entry_instr* block, word index){
  return block->predecessor_at(block, index);
}

MUN_INLINE void
block_add_dominated(block_entry_instr* self, block_entry_instr* block){
  block->dominator = self;
  buffer_add(&self->dominated, block);
}

MUN_INLINE parallel_move_instr*
block_get_parallel_move(block_entry_instr* block){
  return (block->parallel_move == NULL ? (block->parallel_move = parallel_move_new()) : block->parallel_move);
}

bool block_discover_blocks(block_entry_instr* self, block_entry_instr* predecessor, object_buffer* /* block_entry_instr* */ preorder, object_buffer* /* word */ parent);

typedef struct _target_entry_instr target_entry_instr;

typedef struct{
  block_entry_instr block;

  target_entry_instr* normal_entry;

  function* func;

  word entry_count;
  word spill_slot_count;
  word fixed_slot_count;

  object_buffer initial_definitions;
} graph_entry_instr;

graph_entry_instr* graph_entry_new(function* func, target_entry_instr* target);

struct _target_entry_instr{
  block_entry_instr block;

  block_entry_instr* predecessor;
};

target_entry_instr* target_entry_new();

typedef struct _phi_instr phi_instr;

typedef struct _join_entry_instr{
  block_entry_instr block;

  block_entry_instr* predecessors[2];
  object_buffer* phis;
} join_entry_instr;

join_entry_instr* join_entry_new();

phi_instr* join_insert_phi(join_entry_instr* join, word var_index, word var_count);

MUN_INLINE word
join_index_of_predecessor(join_entry_instr* join, block_entry_instr* pred){
  if(join->predecessors[0] == pred) return 0;
  if(join->predecessors[1] == pred) return 1;
  return -1;
}

#define to_graph_entry(i) container_of(container_of(i, block_entry_instr, instr), graph_entry_instr, block)
#define to_join_entry(i) container_of(container_of(i, block_entry_instr, instr), join_entry_instr, block)

#define backward_instr_iter(Block, Name) \
  for(instruction* Name = block->last; \
      Name != ((instruction*) Block); \
      Name = Name->prev)

#define forward_instr_iter(Block, Name) \
  for(instruction* Name = ((instruction*) Block)->next; \
      Name != NULL; \
      Name = ((instruction*) Name)->next)

HEADER_END

#endif