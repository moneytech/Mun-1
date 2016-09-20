#ifndef MUN_INSTRUCTIONS_H
#define MUN_INSTRUCTIONS_H

#define FOR_EACH_INSTRUCTION(V) \
  V(constant) \
  V(return) \
  V(store_local) \
  V(load_local) \
  V(binary_op) \
  V(graph_entry) \
  V(target_entry) \
  V(join_entry) \
  V(phi) \
  V(parallel_move) \
  V(goto) \
  V(block_entry)

#endif