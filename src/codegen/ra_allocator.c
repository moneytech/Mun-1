#include <mun/codegen/ra_allocator.h>
#include <mun/buffer.h>
#include <mun/graph/graph.h>
#include <mun/codegen/il_core.h>
#include <mun/codegen/il_value.h>
#include <mun/codegen/il_entries.h>
#include <assert.h>

void
graph_alloc_init(graph_allocator* self, graph* g){
  self->flow_graph = g;
  buffer_clone_into(&self->postorder, &g->postorder);
  ssa_liveness_init(&self->liveness, g);
  buffer_init(&self->instructions, 10);
  buffer_init(&self->info, 10);
  buffer_init(&self->unallocated_cpu, 10);
  buffer_init(&self->unallocated_fpu, 10);
  buffer_init(&self->unallocated, 10);
  buffer_init(&self->registers, 10);
  self->vreg_count = g->current_ssa_temp_index;
  printf("VRegs: %li\n", self->vreg_count);
  buffer_init(&self->live_ranges, self->vreg_count);
  buffer_clone_into(&self->block_order, &g->reverse_postorder);
  self->num_of_regs = 0;
  buffer_init(&self->blocked_registers, 10);
  reaching_defs_init(&self->reaching, g);

  for(word i = 0; i < self->vreg_count; i++){
    printf("Adding NULL: %li\n", i);
    buffer_add(&self->live_ranges, NULL);
  }

  for(word i = 0; i < kNumberOfCpuRegisters; i++){
    if((kAvailableCpuRegistersList & (1 << i)) == 0){
      self->blocked_cpu_regs[i] = TRUE;
    } else{
      self->blocked_cpu_regs[i] = FALSE;
    }
  }

  self->blocked_fpu_regs[FPUTMP] = TRUE;
}

static void number_instructions(graph_allocator* self);
static void build_live_ranges(graph_allocator* self);
static void prep_allocation(graph_allocator* self, location_kind reg_kind, word num_regs, object_buffer* /* live_range* */ unallocated, live_range** blocked_ranges, bool* blocked_regs);
static void alloc_unallocated_ranges(graph_allocator* self);
static void resolve_control_flow(graph_allocator* self);

void
graph_alloc_regs(graph_allocator* self){
  liveness_analyze(((liveness_analysis*) &self->liveness));
  number_instructions(self);
  build_live_ranges(self);

  prep_allocation(self, kRegister, kNumberOfCpuRegisters, &self->unallocated_cpu, self->cpu_regs, self->blocked_fpu_regs);
  alloc_unallocated_ranges(self);

  self->spill_count = self->spilled.size;
  buffer_clear(&self->spilled);

  prep_allocation(self, kFpuRegister, kNumberOfFpuRegisters, &self->unallocated_fpu, self->fpu_regs, self->blocked_fpu_regs);
  alloc_unallocated_ranges(self);

  resolve_control_flow(self);
}

void
number_instructions(graph_allocator* self){
  word pos = 0;

  word block_count = self->postorder.size;
  for(word i = block_count - 1; i >= 0; i--){
    block_entry_instr* block = self->postorder.data[i];

    block_info* info = malloc(sizeof(block_info));
    block_info_init(info, block);

    buffer_add(&self->instructions, block);
    buffer_add(&self->info, info);

    block->start_pos = pos;
    ((instruction*) block)->lifetime_pos = pos;
    pos += 2;

    forward_instr_iter(block, it){
      if(!instr_is(it, kParallelMoveTag)){
        buffer_add(&self->instructions, it);
        buffer_add(&self->instructions, info);
        it->lifetime_pos = pos;
        printf("%s => %li\n", it->ops->name(), pos);
        pos += 2;
      }
    }

    block->end_pos = pos;
  }

  for(word i = block_count - 1; i >= 0; i--){
    block_entry_instr* block = self->postorder.data[i];

    if(instr_is(((instruction*) block), kJoinEntryTag)){
      word move_count = 0;

      join_entry_instr* join = to_join_entry(((instruction*) block));
      phis_foreach(join){
        move_count += 1;
      }

      for(word k = 0; k < block_predecessor_count(block); k++){
        instruction* last = block_predecessor_at(block, k)->last;
        parallel_move_instr* move = goto_get_parallel_move(to_goto(last));

        for(word j = 0; j < move_count; j++){
          parallel_move_add_move(move, kInvalidLocation, kInvalid);
        }
      }
    }
  }
}

MUN_INLINE block_info*
block_info_at(graph_allocator* self, word pos){
  return self->info.data[pos / 2];
}

MUN_INLINE live_range*
get_live_range(graph_allocator* self, word vreg){
  printf("Getting Live Range: %li\n", vreg);
  if(self->live_ranges.data[vreg] == NULL){
    printf("\tCreated #%li\n", vreg);
    live_range* range = malloc(sizeof(live_range));
    live_range_init(range, vreg, kTagged);

    self->live_ranges.data[vreg] = range;
  }

  return self->live_ranges.data[vreg];
}

static instruction*
connect_outgoing_phi_moves(graph_allocator* self, block_entry_instr* block, bit_vector* interference){
  instruction* last = block->last;
  if(!instr_is(last, kGotoTag)) return last;

  goto_instr* go = to_goto(last);
  if(!goto_has_parallel_move(go)) return ((instruction*) go)->prev;

  parallel_move_instr* move = go->parallel_move;

  word pos = ((instruction*) go)->lifetime_pos;

  join_entry_instr* join = go->successor;

  word pred_index = join_index_of_predecessor(join, block);

  word move_index = 0;

  phis_foreach(join){
    il_value* val = instr_input_at(((instruction*) phis_current), pred_index);
    move_operands* m = move->moves.data[move_index++];
    constant_instr* c = to_constant(((instruction*) val->defn));
    if(instr_is(((instruction*) val->defn), kConstantTag)){
      loc_init_c(&m->src, c);
      continue;
    }

    word vreg = val->defn->ssa_temp_index;

    live_range* range = get_live_range(self, vreg);
    if(interference != NULL) bit_vector_add(interference, vreg);
    live_range_add_interval(range, block->start_pos, pos);
    live_range_add_hinted(range, pos, &m->src, &get_live_range(self, phis_current->defn.ssa_temp_index)->assigned);
    loc_hint_rr(&m->src);
  }

  return ((instruction*) go)->prev;
}

MUN_INLINE word
to_instruction_start(word pos){
  return (pos & ~1);
}

static live_range*
split_between(graph_allocator* self, live_range* range, word from, word to){
  word split;

  block_info* info = block_info_at(self, to);
  if(from < ((instruction*) info->entry)->lifetime_pos){
    block_info* loop_header = info->loop;
    while((loop_header != NULL) && (from < ((instruction*) loop_header->entry)->lifetime_pos)){
      info = loop_header;
      loop_header = loop_header->loop;
    }

    split = ((instruction*) info->entry)->lifetime_pos;
  } else{
    split = to_instruction_start(to) - 1;
  }

  return live_range_split(range, split);
}

MUN_INLINE bool
should_be_allocated_before(live_range* a, live_range* b){
  return live_range_start(a) <= live_range_start(b);
}

static void
add_to_sorted_list(object_buffer* /* live_range* */ ranges, live_range* range){
  alloc_finger_initialize(&range->finger, range);
  if(buffer_is_empty(ranges)){
    buffer_add(ranges, range);
    return;
  }

  for(word i = ranges->size - 1; i >= 0; i--){
    if(should_be_allocated_before(range, ((live_range*) ranges->data[i]))){
      buffer_insert_at(ranges, i + 1, range);
      return;
    }
  }

  buffer_insert_at(ranges, 0, range);
}

static void
complete_range(graph_allocator* self, live_range* range, location_kind kind){
  switch(kind){
    case kRegister:{
      add_to_sorted_list(&self->unallocated_cpu, range);
      break;
    }
    case kFpuRegister:{
      add_to_sorted_list(&self->unallocated_fpu, range);
      break;
    }
    default:{
      fprintf(stderr, "Unhandled register kind: %d\n", kind);
      abort();
    }
  }
}

static void
convert_all_uses(live_range* range){
  if(range->vreg == kNoRegister) return;

  location loc = range->assigned;
  for(use_position* use = range->uses; use != NULL; use = use->next){
    *use->slot = loc;
  }
}

static void
process_initial_definition(graph_allocator* self, definition* defn, live_range* range, block_entry_instr* block){
  printf("Processing Initial Definition: '%s\n", ((instruction*) defn)->ops->name());

  if(instr_is(((instruction*) defn), kParameterTag)){

  } else{
    constant_instr* c = to_constant((instruction*) defn);
    loc_init_c(&range->assigned, c);
    loc_init_c(&range->spill, c);
  }

  alloc_finger_initialize(&range->finger, range);

  use_position* use = alloc_finger_first_beneficial(&range->finger, block->start_pos);
  if(use != NULL){
    live_range* tail = split_between(self, range, block->start_pos, use->pos);
    complete_range(self, tail, kRegister);
  }

  convert_all_uses(range);
}

static bool
has_only_unconstrained_uses(live_range* range){
  use_position* use = range->uses;
  while(use != NULL){
    location any;
    loc_init_a(&any);

    if((*use->slot) != any){
      return FALSE;
    }

    use = use->next;
  }

  return TRUE;
}

static instruction*
instruction_at(graph_allocator* self, word pos){
  return self->instructions.data[pos / 2];
}

MUN_INLINE bool
is_instruction_start(word pos){
  return (pos & 1) == 0;
}

static parallel_move_instr*
create_move_before(instruction* instr, word pos){
  instruction* prev = instr->prev;
  parallel_move_instr* move = to_parallel_move(instr);
  if((!instr_is(instr, kParallelMoveTag)) || (instr->lifetime_pos != pos)){
    move = parallel_move_new();
    instr_link_to(prev, ((instruction*) move));
    instr_link_to(((instruction*) move), instr);
    ((instruction*) move)->lifetime_pos = pos;
  }
  return move;
}

static parallel_move_instr*
create_move_after(instruction* instr, word pos){
  instruction* next = instr->next;
  if(instr_is(next, kParallelMoveTag) && next->lifetime_pos == pos){
    return to_parallel_move(next);
  }
  return create_move_before(next, pos);
}

static move_operands*
add_move_at(graph_allocator* self, word pos, location to, location from){
  if(pos < 2){
    block_entry_instr* block = container_of(instruction_at(self, 2), block_entry_instr, instr);
    return parallel_move_add_move(block_get_parallel_move(block), to, from);
  }

  instruction* instr = instruction_at(self, pos);
  parallel_move_instr* move = NULL;

  if(is_instruction_start(pos)){
    move = create_move_before(instr, pos);
  } else{
    move = create_move_after(instr, pos);
  }

  return parallel_move_add_move(move, to, from);
}

static void
block_register_location(graph_allocator* self, location loc, word from, word to, bool* blocked_registers, live_range** blocked_ranges){
  if(blocked_registers[loc_get_register(loc)]){
    return;
  }

  if(blocked_ranges[loc_get_register(loc)] == NULL){
    live_range* range = malloc(sizeof(live_range));
    live_range_init(range, kNoRegister, kTagged);
    blocked_ranges[loc_get_register(loc)] = range;
    range->assigned = loc;
  }

  live_range_add_interval(blocked_ranges[loc_get_register(loc)], from, to);
}

static void
block_location(graph_allocator* self, location loc, word from, word to){
  if(loc_is_register(loc)){
    printf("Blocking (CPU) %li -> %li #%d\n", from, to, loc_get_register(loc));
    block_register_location(self, loc, from, to, self->blocked_cpu_regs, self->cpu_regs);
  } else if(loc_is_fpu_register(loc)){
    printf("Blocking (FPU) %li -> %li #%d\n", from, to, loc_get_fpu_register(loc));
    block_register_location(self, loc, from, to, self->blocked_fpu_regs, self->fpu_regs);
  } else{
    fprintf(stderr, "Unreachable\n");
    abort();
  }
}

static void
process_one_input(graph_allocator* self, block_entry_instr* block, word pos, location* in_ref, il_value* input, word vreg){
  live_range* range = get_live_range(self, vreg);
  printf("Processing Input: @%li\n", pos);
  if(loc_get_kind(*in_ref) == kRegister || loc_get_kind(*in_ref) == kFpuRegister){
    location any;
    loc_init_a(&any);

    move_operands* move = add_move_at(self, pos - 1, *in_ref, any);
    block_location(self, *in_ref, pos - 1, pos + 1);

    live_range_add_interval(range, block->start_pos, pos - 1);
    live_range_add_hinted(range, pos - 1, &move->src, in_ref);
  } else if(loc_is_unallocated(*in_ref)){
    live_range_add_interval(range, block->start_pos, pos + 1);
    live_range_add_use(range, pos + 1, in_ref);
  } else{
    assert(loc_is_constant(*in_ref));
  }
}

static void
process_one_output(graph_allocator* self, block_entry_instr* block, word pos, location* out, instruction* def, word vreg, bool same, location* in_ref, instruction* input, word input_vreg, bit_vector* interference){
  printf("Processing output: @%li\n", pos);

  live_range* range = vreg >= 0 ?
                      get_live_range(self, vreg) :
                      malloc(sizeof(live_range));
  if(vreg >= 0) live_range_init(range, -2, kTagged);

  if(loc_is_register(*out) || loc_is_fpu_register(*out)){
    block_location(self, *out, pos, pos + 1);
    if(range->vreg == -2) return;
    use_position* use = range->uses;
    if(use == NULL) return;
    if(use->pos == (pos + 1)){
      *(use->slot) = *out;
      range->uses = range->uses->next;
    }

    live_range_define(range, pos + 1);
    if(live_range_start(range) == live_range_end(range)) return;

    location any;
    loc_init_a(&any);
    move_operands* move = add_move_at(self, pos + 1, any, *out);
    live_range_add_hinted(range, pos + 1, &move->dest, out);
  } else if(same){
    *out = *in_ref;

    location rr;
    loc_hint_rr(&rr);

    location any;
    loc_init_a(&any);
    move_operands* move = add_move_at(self, pos, rr, any);
    live_range* input_range = get_live_range(self, input_vreg);
    live_range_add_interval(input_range, block->start_pos, pos);
    live_range_add_use(input_range, pos, &move->src);

    live_range_define(range, pos);
    live_range_add_hinted(range, pos, out, &move->src);
    live_range_add_use(range, pos, &move->dest);
    live_range_add_use(range, pos, in_ref);

    if((interference != NULL) &&
       range->vreg >= 0 &&
       bit_vector_contains(interference, range->vreg)){
      bit_vector_add(interference, container_of(input, definition, instr)->ssa_temp_index);
    }
  } else{
    live_range_define(range, pos);
    live_range_add_use(range, pos, out);
  }

  complete_range(self, range, kRegister);
}

static void
process_one_instruction(graph_allocator* self, block_entry_instr* block, instruction* instr, bit_vector* interference){
  location_summary* locs = instr->locations;

  if(instr_is_definition(instr) &&
     instr_is(instr, kConstantTag)){

    definition* defn = container_of(instr, definition, instr);
    live_range* range = (defn->ssa_temp_index != -1) ?
                        get_live_range(self, defn->ssa_temp_index) :
                        NULL;

    if((range == NULL) || (range->uses == NULL)){
      locs->output = kInvalidLocation;
      return;
    }

    if(has_only_unconstrained_uses(range)){
      constant_instr* c = to_constant(instr);
      loc_init_c(&range->assigned, c);
      loc_init_c(&range->spill, c);
      alloc_finger_initialize(&range->finger, range);
      convert_all_uses(range);
      locs->output = kInvalidLocation;
      return;
    }
  }

  word pos = instr->lifetime_pos;

  if(loc_is_unallocated(locs->output) &&
        (loc_get_policy(locs->output) == kSameAsFirstInput)){
    if(loc_get_kind(locs->inputs[0]) == kRegister ||
       loc_get_kind(locs->inputs[0])){
      locs->output = locs->inputs[0];
    }
  }

  bool same_as_first = loc_is_unallocated(locs->output) &&
                       loc_get_policy(locs->output) == kSameAsFirstInput;

  printf("Processing %li Inputs For @%li\n", (locs->inputs_len - (same_as_first ? 1 : 0)), pos);

  for(word j = same_as_first ? 1 : 0; j < locs->inputs_len; j++){
    il_value* input = instr_input_at(instr, j);
    location* in_ref = &locs->inputs[j];
    process_one_input(self, block, pos, in_ref, input, input->defn->ssa_temp_index);
  }

  for(word j = 0; j < locs->temps_len; j++){
    location temp = locs->temps[j];

    if(loc_get_kind(temp) == kRegister || loc_get_kind(temp) == kFpuRegister){
      block_location(self, temp, pos, pos + 1);
    } else if(loc_is_unallocated(temp)){
      live_range* range = malloc(sizeof(live_range));
      live_range_init(range, -2, kTagged);
      live_range_add_interval(range, pos, pos + 1);
      live_range_add_use(range, pos, &locs->temps[j]);
      complete_range(self, range, kRegister);
    } else{
      fprintf(stderr, "Unreachable\n");
      abort();
    }
  }

  if(locs->contains_call){
    for(word reg = 0; reg < kNumberOfCpuRegisters; reg++){
      location reg_loc;
      loc_init_r(&reg_loc, ((asm_register) reg));
      block_location(self, reg_loc, pos, pos + 1);
    }
  }

  if(!instr_is_definition(instr) || loc_is_invalid(locs->output)){
    return;
  }

  location* out_slot = &locs->output;

  definition* defn = container_of(instr, definition, instr);
  if(same_as_first){
    location* in_ref = &locs->inputs[0];
    definition* input = instr_input_at(instr, 0)->defn;
    process_one_output(self, block, pos, out_slot, ((instruction*) defn), defn->ssa_temp_index, TRUE, in_ref, ((instruction*) input), input->ssa_temp_index, interference);
  } else{
    process_one_output(self, block, pos, out_slot, ((instruction*) defn), defn->ssa_temp_index, FALSE, NULL, NULL, -1, interference);
  }
}

static void
connect_incoming_phi_moves(graph_allocator* self, join_entry_instr* join){
  word pos = ((block_entry_instr*) join)->start_pos;

  word move_idx = 0;

  phis_foreach(join){
    word vreg = ((definition*) phis_current)->ssa_temp_index;

    live_range* range = get_live_range(self, vreg);
    live_range_define(range, pos);

    for(word pred_idx = 0; pred_idx < instr_input_count(((instruction*) phis_current)); pred_idx++){
      block_entry_instr* pred = block_predecessor_at(((block_entry_instr*) join), pred_idx);
      goto_instr* go = to_goto(pred->last);
      move_operands* move = go->parallel_move->moves.data[move_idx];
      loc_hint_rr(&move->dest);
      live_range_add_use(range, pos, &move->dest);
    }

    complete_range(self, range, kRegister); //TODO: Fixme
    move_idx += 1;
  }
}

void
build_live_ranges(graph_allocator* self){
  word block_count = self->postorder.size;

  bit_vector* interference = NULL;

  for(word i = 0; i < block_count - 1; i++){
    block_entry_instr* block = self->postorder.data[i];
    block_info* info = block_info_at(self, block->start_pos);

    bit_vector_foreach(liveness_get_live_out_at(((liveness_analysis*) &self->liveness), i)){
      live_range* range = get_live_range(self, bit_vector_current);
      live_range_add_interval(range, block->start_pos, block->end_pos);
    }

    block_info* loop_header = info->loop;
    if((loop_header != NULL) && (loop_header->last == block)){
      interference = malloc(sizeof(bit_vector));
      bit_vector_init(interference, self->flow_graph->current_ssa_temp_index);
      bit_vector_add_all(interference, liveness_get_live_in(((liveness_analysis*) &self->liveness), loop_header->entry));
    }

    instruction* current = connect_outgoing_phi_moves(self, block, interference);
    while(current != ((instruction*) block)){
      printf("Current: '%s'\n", current->ops->name());
      if(!instr_is(current, kParallelMoveTag)){
        process_one_instruction(self, block, current, interference);
      }
      current = current->prev;
    }

    if(instr_is(((instruction*) block), kJoinEntryTag)){
      connect_incoming_phi_moves(self, to_join_entry(((instruction*) block)));
    }
  }

  graph_entry_instr* gentry = self->flow_graph->graph_entry;
  for(word i = 0; i < gentry->initial_definitions.size; i++){
    definition* defn = gentry->initial_definitions.data[i];
    live_range* range = get_live_range(self, defn->ssa_temp_index);
    live_range_add_interval(range, ((block_entry_instr*) gentry)->start_pos, ((block_entry_instr*) gentry)->end_pos);
    live_range_define(range, ((block_entry_instr*) gentry)->start_pos);
    process_initial_definition(self, defn, range, ((block_entry_instr*) gentry));
  }
}

static void
prep_allocation(graph_allocator* self, location_kind kind, word num_regs, object_buffer* unallocated, live_range** blocked_ranges, bool* blocked_registers){
  self->kind = kind;
  self->num_of_regs = num_regs;

  buffer_clear(&self->blocked_registers);
  buffer_clear(&self->registers);

  for(word i = 0; i < num_regs; i++){
    buffer_add(&self->blocked_registers, bool_clone(FALSE));

    object_buffer* register_ranges = malloc(sizeof(object_buffer));
    buffer_init(register_ranges, 10);

    buffer_add(&self->registers, register_ranges);
  }

  buffer_add_all(&self->unallocated, unallocated);

  for(word reg = 0; reg < self->num_of_regs; reg++){
    self->blocked_registers.data[reg] = bool_clone(blocked_registers[reg]);
    live_range* range = blocked_ranges[reg];
    if(range != NULL){
      alloc_finger_initialize(&range->finger, range);
      buffer_add(&self->registers, range);
    }
  }
}

static void
remove_evicted(graph_allocator* self, word reg, word first_evicted){
  word to = first_evicted;
  word from = first_evicted + 1;
  while(from < ((object_buffer*) self->registers.data[reg])->size){
    live_range* allocated = ((object_buffer*) self->registers.data[reg])->data[from++];
    if(allocated != NULL){
      ((object_buffer*) self->registers.data[reg])->data[to++] = allocated;
    }
  }
}

static void
advance_active_intervals(graph_allocator* self, word start){
  for(word reg = 0; reg < self->num_of_regs; reg++){
    if(buffer_is_empty(self->registers.data[reg])) continue;

    word first_evicted = -1;
    for(word i = ((object_buffer*) self->registers.data[reg])->size - 1; i >= 0; i--){
      live_range* range = ((object_buffer*) self->registers.data[reg])->data[i];
      if(alloc_finger_advance(&range->finger, start)){
        convert_all_uses(range);
        ((object_buffer*) self->registers.data[reg])->data[i] = NULL;
        first_evicted = i;
      }
    }

    if(first_evicted != -1) remove_evicted(self, reg, first_evicted);
  }
}

static void
alloc_unallocated_ranges(graph_allocator* self){
  while(!buffer_is_empty(&self->unallocated)){
    live_range* range = buffer_del_last(&self->unallocated);

    word start = live_range_start(range);

    advance_active_intervals(self, start);
  }
}

MUN_INLINE bool
target_location_is_spill_slot(live_range* range, location target){
  return loc_is_constant(target);
}

static void
connect_split_siblings(graph_allocator* self, live_range* parent, block_entry_instr* source, block_entry_instr* target){
  if(parent->next_sibling == NULL){
    return;
  }

  word source_pos = source->end_pos - 1;
  word target_pos = target->start_pos;

  location target_loc;
  loc_init(&target_loc);
  location source_loc;
  loc_init(&source_loc);

  live_range* range = parent;
  while((range != NULL) && (loc_is_invalid(source_loc) || loc_is_invalid(target_loc))){
    if(live_range_can_cover(range, source_pos)){
      source_loc = range->assigned;
    }

    if(live_range_can_cover(range, target_pos)){
      target_loc = range->assigned;
    }

    range = range->next_sibling;
  }

  if(source_loc == target_loc) return;

  if(target_location_is_spill_slot(parent, target_loc)){
    return;
  }

  instruction* last = source->last;
  if((instr_successor_count(last) == 1) && !instr_is(((instruction*) source), kGraphEntryTag)){
    parallel_move_add_move(goto_get_parallel_move(to_goto(last)), target_loc, source_loc);
  } else{
    parallel_move_add_move(block_get_parallel_move(target), target_loc, source_loc);
  }
}

MUN_INLINE bool
is_block(graph_allocator* self, word pos){
  return is_instruction_start(pos) &&
      (instr_is(instruction_at(self, pos), kGraphEntryTag) ||
       instr_is(instruction_at(self, pos), kTargetEntryTag) ||
       instr_is(instruction_at(self, pos), kJoinEntryTag));
}

static void
resolve_control_flow(graph_allocator* self){
  for(word vreg = 0; vreg < self->live_ranges.size; vreg++){
    live_range* range = self->live_ranges.data[vreg];
    if(range == NULL) continue;

    while(range->next_sibling != NULL){
      live_range* sibling = range->next_sibling;
      if((live_range_end(range) == live_range_start(sibling)) &&
          range->assigned != sibling->assigned &&
          !is_block(self, live_range_end(range))){

        add_move_at(self, live_range_start(sibling), sibling->assigned, range->assigned);
      }

      range = sibling;
    }

    for(word i = 1; i < self->block_order.size; i++){
      block_entry_instr* block = self->block_order.data[i];
      bit_vector* live = liveness_get_live_in(((liveness_analysis*) &self->liveness), block);

      bit_vector_foreach(live){
        live_range* block_range = get_live_range(self, bit_vector_current);
        for(word j = 0; j < block_predecessor_count(block); j++){
          connect_split_siblings(self, block_range, block_predecessor_at(block, j), block);
        }
      }
    }

    for(word i = 0; i < self->spilled.size; i++){
      live_range* spilled_range = self->spilled.data[i];
      if(loc_is_constant(spilled_range->assigned)){

      } else{
        add_move_at(self, live_range_start(range) + 1, range->spill, range->assigned);
      }
    }
  }
}