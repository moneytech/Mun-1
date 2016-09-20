#include <mun/buffer.h>
#include <mun/codegen/intermediate_language.h>

static char*
graph_name() {
  return "GraphEntry";
}

static word
graph_predecessor_count(block_entry_instr* block) {
  return 0;
}

static block_entry_instr*
graph_predecessor_at(block_entry_instr* block, word index) {
  return NULL;
}

static void
graph_add_predecessor(block_entry_instr* self, block_entry_instr* pred){
  // Fallthrough
}

static void
graph_clear_predecessors(block_entry_instr* self){
  // Fallthrough
}

static word
graph_successor_count(instruction* instr){
  return 1;
}

static block_entry_instr*
graph_successor_at(instruction* instr, word index){
  if(index == 0) return ((block_entry_instr*) to_graph_entry(instr)->normal_entry);
  return NULL;
}

graph_entry_instr*
graph_entry_new(function* func, target_entry_instr* target) {
  graph_entry_instr* g = malloc(sizeof(graph_entry_instr));
  static instruction_ops ops = {
      NULL, // set_input_at
      NULL, // emit_machine_code
      NULL, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      NULL, // input_count
      &graph_successor_count, // successor_count
      &graph_successor_at, // successor_at
      NULL, // get_block
      NULL, // input_at
      &graph_name, // name
  };
  instr_init(((instruction*) g), kGraphEntryTag, &ops);
  defn_init(((definition*) g));
  g->block.clear_predecessors = &graph_clear_predecessors;
  g->block.add_predecessor = &graph_add_predecessor;
  g->block.predecessor_count = &graph_predecessor_count;
  g->block.predecessor_at = &graph_predecessor_at;
  g->func = func;
  g->normal_entry = target;
  g->entry_count = 0;
  g->spill_slot_count = 0;
  buffer_init(&g->initial_definitions, 10);
  return g;
}

static char*
target_name() {
  return "TargetEntry";
}

#define block_to_target(b) \
  container_of(b, target_entry_instr, block)

static word
target_predecessor_count(block_entry_instr* block) {
  return (block_to_target(block)->predecessor == NULL ? 0 : 1);
}

static block_entry_instr*
target_predecessor_at(block_entry_instr* block, word index) {
  return block_to_target(block)->predecessor;
}

static void
target_add_predecessor(block_entry_instr* self, block_entry_instr* pred){
  block_to_target(self)->predecessor = pred;
}

static void
target_clear_predecessors(block_entry_instr* self){
  block_to_target(self)->predecessor = NULL;
}

target_entry_instr*
target_entry_new() {
  target_entry_instr* t = malloc(sizeof(target_entry_instr));
  static instruction_ops ops = {
      NULL, // set_input_at
      NULL, // emit_machine_code
      NULL, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      NULL, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      NULL, // input_at
      &target_name, // name
  };
  instr_init(((instruction*) t), kTargetEntryTag, &ops);
  defn_init(((definition*) t));
  t->block.clear_predecessors = &target_clear_predecessors;
  t->block.add_predecessor = &target_add_predecessor;
  t->block.predecessor_at = &target_predecessor_at;
  t->block.predecessor_count = &target_predecessor_count;
  return t;
}

static char*
join_name() {
  return "JoinEntry";
}

static word
join_predecessor_count(block_entry_instr* block) {
  join_entry_instr* join = container_of(block, join_entry_instr, block);
  word count = 0;
  if (join->predecessors[0] != NULL) count++;
  if (join->predecessors[1] != NULL) count++;
  return count;
}

static block_entry_instr*
join_predecessor_at(block_entry_instr* block, word index) {
  return container_of(block, join_entry_instr, block)->predecessors[index];
}

static void
join_clear_predecessors(block_entry_instr* block){
  join_entry_instr* join = container_of(block, join_entry_instr, block);
  join->predecessors[0x0] = NULL;
  join->predecessors[0x1] = NULL;
}

#define block_to_join(b) \
  container_of(b, join_entry_instr, block)

static void
join_add_predecessor(block_entry_instr* self, block_entry_instr* pred){
  join_entry_instr* join = block_to_join(self);
  word index = 0;
  while((index < 2) && (join->predecessors[index] != pred)){
    index++;
  }
  join->predecessors[index] = pred;
}

join_entry_instr*
join_entry_new() {
  join_entry_instr* join = malloc(sizeof(join_entry_instr));
  static instruction_ops ops = {
      NULL, // set_input_at
      &join_entry_compile, // emit_machine_code
      NULL, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      NULL, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      NULL, // input_at
      &join_name,
  };
  instr_init(((instruction*) join), kJoinEntryTag, &ops);
  defn_init(((definition*) join));
  join->block.clear_predecessors = &join_clear_predecessors;
  join->block.add_predecessor = &join_add_predecessor;
  join->block.predecessor_count = &join_predecessor_count;
  join->block.predecessor_at = &join_predecessor_at;
  join->predecessors[0] = NULL;
  join->predecessors[1] = NULL;
  join->phis = NULL;
  return join;
}

phi_instr*
join_insert_phi(join_entry_instr* join, word var_index, word var_count) {
  if (join->phis == NULL) {
    join->phis = malloc(sizeof(object_buffer));
    buffer_init(join->phis, var_count);

    for (word i = 0; i < var_count; i++) {
      buffer_add(join->phis, NULL);
    }
  }

  return (join->phis->data[var_index] = phi_new(join, block_predecessor_count(((block_entry_instr*) join))));
}

MUN_INLINE bool
is_marked(block_entry_instr* block, object_buffer* preorder) {
  word index = block->preorder_num;
  return (index >= 0) &&
         (index < preorder->size) &&
         (preorder->data[index] == block);
}

bool
block_discover_blocks(block_entry_instr* self, block_entry_instr* predecessor, object_buffer* preorder,
                      object_buffer* parent) {
  if(is_marked(self, preorder)){
    self->add_predecessor(self, predecessor);
    return FALSE;
  }

  self->clear_predecessors(self);
  if(predecessor != NULL) self->add_predecessor(self, predecessor);

  word parent_num = (predecessor == NULL) ?
                    -1 :
                    predecessor->preorder_num;

  buffer_add(parent, word_clone(parent_num));
  self->preorder_num = parent->size;
  buffer_add(preorder, self);

  instruction* last = ((instruction*) self);
  forward_instr_iter(self, it){
    last = it;
  }

  self->last = last;
  return TRUE;
}