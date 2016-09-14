#ifndef MUN_GRAPH_BUILDER_H
#define MUN_GRAPH_BUILDER_H

#include "../ast.h"
#include "graph.h"

HEADER_BEGIN

typedef struct{
  ast_node* ast;

  function* func;

  graph_entry_instr* entry;
} graph_builder;

void graph_builder_init(graph_builder* builder, function* func);

graph* graph_build(graph_builder* builder);

typedef struct{
  ast_node_visitor visitor;

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

void evis_init(effect_visitor* vis);
void evis_append(effect_visitor* vis, effect_visitor other);
void evis_do(effect_visitor* vis, definition* defn);
void evis_add_return_exit(effect_visitor* vis, il_value* val);
void evis_add_instruction(effect_visitor* vis, instruction* instr);

MUN_INLINE void
evis_return_definition(effect_visitor* vis, definition* defn){
  if(!is_constant(((instruction*) defn))){
    evis_do(vis, defn);
  }
}

il_value* evis_bind(effect_visitor* vis, definition* defn);

typedef struct{
  effect_visitor effect;

  il_value* value;
} value_visitor;

void vvis_init(value_visitor* vis);

MUN_INLINE void
vvis_return_value(value_visitor* vis, il_value* val){
  vis->value = val;
}

MUN_INLINE void
vvis_return_definition(value_visitor* vis, definition* defn){
  vvis_return_value(vis, evis_bind(((effect_visitor*) vis), defn));
}

HEADER_END

#endif