#ifndef MUN_RA_ALLOCATOR_H
#define MUN_RA_ALLOCATOR_H

#include "../common.h"

HEADER_BEGIN

#include "../buffer.h"
#include "../graph/graph.h"
#include "ra_range.h"

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

  live_range* cpu_regs[kNumberOfCpuRegisters];
  bool blocked_cpu_regs[kNumberOfCpuRegisters];

  live_range* fpu_regs[kNumberOfFpuRegisters];
  bool blocked_fpu_regs[kNumberOfFpuRegisters];
} graph_allocator;

void graph_alloc_init(graph_allocator* alloc, graph* flow_graph);
void graph_alloc_regs(graph_allocator* alloc);

HEADER_END

#endif