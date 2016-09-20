#ifndef MUN_IL_CORE_H
#define MUN_IL_CORE_H

#if !defined(MUN_INTERMEDIATE_LANGUAGE_H)
#error "Please #include <mun/codegen/intermediate_language.h> directly"
#endif

#include "../common.h"

HEADER_BEGIN

#include "../object.h"
#include "../buffer.h"
#include "../location.h"

typedef enum{
  kConstantTag = 1 << 0,
  kReturnTag = 1 << 1,
  kStoreLocalTag = 1 << 2,
  kLoadLocalTag = 1 << 3,
  kBinaryOpTag = 1 << 4,
  kGraphEntryTag = 1 << 5,
  kTargetEntryTag = 1 << 6,
  kPhiTag = 1 << 7,
  kJoinEntryTag = 1 << 8,
  kDropTag = 1 << 9,
  kParallelMoveTag = 1 << 10,
  kGotoTag = 1 << 11,
  kParameterTag = 1 << 12,
} instruction_tag;

typedef enum{
  kUnboxedNumber,
  kBoxedNumber,
  kTagged,
  kNone
} representation;

typedef struct _graph graph;
typedef struct _asm_buff asm_buff;
typedef struct _location_summary location_summary;
typedef struct _il_value il_value;
typedef struct _block_entry_instr block_entry_instr;
typedef struct _join_entry_instr join_entry_instr;

typedef struct{
  void (*set_input_at)(instruction*, word, il_value*);
  void (*emit_machine_code)(instruction*, asm_buff*);

  location_summary* (*make_location_summary)(instruction* instr);

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

  uword flags;
  word lifetime_pos;

  const instruction_ops* ops;

  location_summary* locations;
};

static const uword kInvalidFlags = 0x0;

bool instr_is_definition(instruction* instr);
bool instr_is(instruction* instr, instruction_tag tag);

void instr_set_input_at(instruction* instr, word index, il_value* val);
void instr_insert_after(instruction* instr, instruction* prev);
void instr_init(instruction* instr, instruction_tag tag, instruction_ops* ops);
void instr_compile(instruction* instr, asm_buff* code);

instruction* instr_remove_from_graph(instruction* self, graph* g, bool ret_prev);

MUN_INLINE void
instr_link_to(instruction* instr, instruction* next){
  instr->next = next;
  next->prev = instr;
}

MUN_INLINE word
instr_successor_count(instruction* instr){
  return instr->ops->successor_count(instr);
}

MUN_INLINE word
instr_input_count(instruction* instr){
  return instr->ops->input_count(instr);
}

MUN_INLINE word
instr_argument_count(instruction* instr){
  return instr->ops->argument_count != NULL ?
         instr->ops->argument_count(instr) :
         0;
}

MUN_INLINE block_entry_instr*
instr_successor_at(instruction* instr, word index){
  return instr->ops->successor_at(instr, index);
}

MUN_INLINE void
instr_insert_before(instruction* instr, instruction* next){
  instr_insert_after(next->prev, instr);
}

MUN_INLINE void
instr_initialize_location_summary(instruction* instr){
  printf("Initializing Location Summary For: '%s'\n", instr->ops->name());
  instr->locations = instr->ops->make_location_summary(instr);
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

MUN_INLINE bool
defn_has_ssa_temp(definition* defn){
  return defn->ssa_temp_index >= 0;
}

typedef struct{
  location dest;
  location src;
} move_operands;

MUN_INLINE location
move_operands_mark_pending(move_operands* self){
  return (self->dest = kInvalidLocation);
}

MUN_INLINE void
move_operands_clear_pending(move_operands* self, location dest){
  self->dest = dest;
}

MUN_INLINE void
move_operands_eliminate(move_operands* self){
  self->src = self->dest = kInvalidLocation;
}

MUN_INLINE bool
move_operands_is_pending(move_operands* self){
  return loc_is_invalid(self->dest) && !loc_is_invalid(self->src);
}

MUN_INLINE bool
move_operands_is_eliminated(move_operands* self){
  return loc_is_invalid(self->src);
}

MUN_INLINE bool
move_operands_blocks(move_operands* self, location loc){
  return !move_operands_is_eliminated(self) && (self->src != loc);
}

MUN_INLINE bool
move_operands_is_redundant(move_operands* self){
  return move_operands_is_eliminated(self) ||
         loc_is_invalid(self->dest) ||
         self->src == self->dest;
}

typedef struct{
  instruction instr;

  object_buffer moves; // move_operands*
} parallel_move_instr;

parallel_move_instr* parallel_move_new();

MUN_INLINE move_operands*
parallel_move_add_move(parallel_move_instr* self, location dest, location src){
  move_operands* move = malloc(sizeof(move_operands));
  move->src = src;
  move->dest = dest;
  buffer_add(&self->moves, move);
  return move;
}

MUN_INLINE bool
parallel_move_is_redundant(parallel_move_instr* self){
  for(word i = 0; i < self->moves.size; i++){
    if(!move_operands_is_redundant(self->moves.data[i])){
      return FALSE;
    }
  }

  return TRUE;
}

typedef struct{
  instruction instr;

  block_entry_instr* block;
  join_entry_instr* successor;
  parallel_move_instr* parallel_move;
} goto_instr;

goto_instr* goto_new(join_entry_instr* join);

MUN_INLINE bool
goto_has_parallel_move(goto_instr* go){
  return go->parallel_move != NULL;
}

MUN_INLINE bool
goto_has_non_redundant_moves(goto_instr* go){
  return goto_has_parallel_move(go) &&
         !parallel_move_is_redundant(go->parallel_move);
}

MUN_INLINE parallel_move_instr*
goto_get_parallel_move(goto_instr* go){
  return (go->parallel_move != NULL ? go->parallel_move : (go->parallel_move = parallel_move_new()));
}

#define to_goto(i) container_of(i, goto_instr, instr)
#define to_parallel_move(i) container_of(i, parallel_move_instr, instr)

HEADER_END

#endif