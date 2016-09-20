#ifndef MUN_GRAPH_BUILDER_H
#define MUN_GRAPH_BUILDER_H

#include "../common.h"

HEADER_BEGIN

#include "../ast.h"
#include "graph.h"

typedef struct{
  ast_node* ast;

  function* func;

  graph_entry_instr* entry;

  word temp_count;
} graph_builder;

void graph_builder_init(graph_builder* builder, function* func);

graph* graph_build(graph_builder* builder);

MUN_INLINE word
graph_builder_alloc_temp(graph_builder* self){
  return self->temp_count++;
}

MUN_INLINE void
graph_builder_dealloc_temps(graph_builder* self, word count){
  self->temp_count -= count;
}

typedef struct{
  ast_node_visitor visitor;

  void (*return_definition)(ast_node_visitor*, definition*);
  void (*return_value)(ast_node_visitor*, il_value*);

  graph_builder* owner;
  instruction* exit;
  instruction* entry;
} effect_visitor;

MUN_INLINE bool
evis_is_empty(effect_visitor* vis){
  return vis->entry == NULL;
}

MUN_INLINE bool
evis_is_open(effect_visitor* vis){
  return evis_is_empty(vis) || vis->exit != NULL;
}

void evis_init(effect_visitor* vis, graph_builder* owner);
void evis_append(effect_visitor* vis, effect_visitor other);
void evis_do(effect_visitor* vis, definition* defn);
void evis_add_return_exit(effect_visitor* vis, il_value* val);
void evis_add_instruction(effect_visitor* vis, instruction* instr);

il_value* evis_bind(effect_visitor* vis, definition* defn);

typedef struct{
  effect_visitor effect;

  il_value* value;
} value_visitor;

void vvis_init(value_visitor* vis, graph_builder* owner);

#define get_evis(v) container_of(v, effect_visitor, visitor)
#define get_vvis(v) container_of(get_evis(v), value_visitor, effect)

MUN_INLINE void
vis_return_value(ast_node_visitor* vis, il_value* val){
  get_evis(vis)->return_value(vis, val);
}

MUN_INLINE void
vis_return_definition(ast_node_visitor* vis, definition* defn){
  get_evis(vis)->return_definition(vis, defn);
}

HEADER_END

#endif