#ifndef MUN_RA_ALLOCATOR_H
#define MUN_RA_ALLOCATOR_H

#include "../common.h"

HEADER_BEGIN

#include "../asm/core.h"
#include "../buffer.h"
#include "../graph/graph.h"
#include "ra_range.h"
#include "ra_ssa_liveness.h"

typedef struct{
  graph* flow_graph;

  object_buffer phis; // phi_instr*
} reaching_defs;

MUN_INLINE void
reaching_defs_init(reaching_defs* defs, graph* g){
  defs->flow_graph = g;
  buffer_init(&defs->phis, 10);
}

MUN_INLINE bit_vector*
reaching_defs_get(reaching_defs* defs, phi_instr* phi){
  if(phi->reaching == NULL){

  }
}

typedef struct{
  word num_of_regs;
  word spill_count;
  word vreg_count;

  location_kind kind;

  graph* flow_graph;

  object_buffer value_reps; // representation
  object_buffer live_ranges; // live_range*
  object_buffer unallocated_cpu; // live_range*
  object_buffer unallocated_fpu; // live_range*
  object_buffer spilled; // live_range*
  object_buffer instructions; // instruction*
  object_buffer registers; // object_buffer[live_range*]*
  object_buffer blocked_registers; // bool
  object_buffer unallocated; // live_range*
  object_buffer postorder; // block_entry_instr*
  object_buffer block_order; // block_entry_instr*
  object_buffer info; // block_info;

  live_range* cpu_regs[kNumberOfCpuRegisters];
  bool blocked_cpu_regs[kNumberOfCpuRegisters];

  live_range* fpu_regs[kNumberOfFpuRegisters];
  bool blocked_fpu_regs[kNumberOfFpuRegisters];

  ssa_liveness liveness;

  reaching_defs reaching;
} graph_allocator;

void graph_alloc_init(graph_allocator* alloc, graph* flow_graph);
void graph_alloc_regs(graph_allocator* alloc);

typedef struct _block_info{
  struct _block_info* loop;
  bool is_loop: 1;
  block_entry_instr* entry;
  block_entry_instr* last;
  bit_vector* backedge;
} block_info;

MUN_INLINE void
block_info_init(block_info* self, block_entry_instr* entry){
  self->entry = entry;
  self->is_loop = FALSE;
  self->loop = NULL;
  self->backedge = NULL;
}

MUN_INLINE block_info*
block_info_loop_header(block_info* self){
  if(self->is_loop){
    return self;
  } else if(self->loop != NULL){
    return self->loop;
  } else{
    return NULL;
  }
}

HEADER_END

#endif