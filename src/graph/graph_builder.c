#include <mun/graph/graph_builder.h>
#include <mun/codegen/il_defs.h>
#include <mun/codegen/il_value.h>
#include <mun/ast.h>
#include <mun/object.h>

static instruction*
append_fragment(block_entry_instr* entry, effect_visitor vis){
  if(evis_is_empty(&vis)) return ((instruction*) entry);
  instr_link_to(((instruction*) entry), vis.entry);
  return vis.exit;
}

il_value*
evis_bind(effect_visitor* self, definition* defn){
  if(evis_is_empty(self)){
    self->entry = ((instruction*) defn);
  } else{
    instr_link_to(self->exit, ((instruction*) defn));
  }
  self->exit = ((instruction*) defn);
  return value_new(defn);
}

void
evis_do(effect_visitor* self, definition* defn){
  if(evis_is_empty(self)){
    self->entry = ((instruction*) defn);
  } else{
    instr_link_to(self->exit, ((instruction*) defn));
  }
  self->exit = ((instruction*) defn);
}

void
evis_add_instruction(effect_visitor* self, instruction* instr){
  if(evis_is_empty(self)){
    self->entry = self->exit = instr;
  } else{
    instr_link_to(self->exit, instr);
    self->exit = instr;
  }
}

MUN_INLINE void
close_frag(effect_visitor* vis){
  vis->exit = NULL;
}

void
evis_add_return_exit(effect_visitor* self, il_value* val){
  return_instr* ret = return_new(val);
  evis_add_instruction(self, ((instruction*) ret));
  close_frag(self);
}

void
evis_append(effect_visitor* self, effect_visitor other){
  if(evis_is_empty(&other)) return;
  if(evis_is_empty(self)){
    self->entry = other.entry;
  } else{
    instr_link_to(self->exit, other.entry);
  }
  self->exit = other.exit;
}

#define get_evis(v) container_of(v, effect_visitor, visitor)

static void
evis_visit_literal(ast_node_visitor* vis, ast_node* node){
  definition* defn = ((definition*) constant_new(to_constant(node)->value));
  evis_return_definition(get_evis(vis), defn);
}

static void
evis_visit_sequence(ast_node_visitor* vis, ast_node* node){
  sequence_node* seq = to_sequence_node(node);

  word i = 0;
  while(evis_is_open(get_evis(vis)) && (i < seq->size)){
    effect_visitor for_effect;
    evis_init(&for_effect);

    visit_ast(((ast_node_visitor*) &for_effect), seq->nodes[i++]);
    evis_append(get_evis(vis), for_effect);
    if(!evis_is_open(get_evis(vis))){
      break;
    }
  }
}

static void
evis_visit_return(ast_node_visitor* vis, ast_node* node){
  value_visitor for_value;
  vvis_init(&for_value);
  visit_ast(((ast_node_visitor*) &for_value), to_return_node(node)->value);
  evis_append(get_evis(vis), for_value.effect);
  evis_add_return_exit(get_evis(vis), for_value.value);
}

#define VISITOR self->visitor

void
evis_init(effect_visitor* self){
  self->owner = NULL;
  self->entry = NULL;
  self->exit = NULL;
  VISITOR.visit_literal_node = &evis_visit_literal;
  VISITOR.visit_sequence_node = &evis_visit_sequence;
  VISITOR.visit_return_node = &evis_visit_return;
}

#undef VISITOR
#define VISITOR self->effect.visitor

#define get_vvis(v) container_of(get_evis(v), value_visitor, effect)

static void
vvis_visit_literal(ast_node_visitor* vis, ast_node* node){
  definition* defn = ((definition*) constant_new(to_constant(node)->value));
  vvis_return_definition(get_vvis(vis), defn);
}

void
vvis_init(value_visitor* self){
  self->value = NULL;
  evis_init(((effect_visitor*) self));
  VISITOR.visit_literal_node = &vvis_visit_literal;
}

void
graph_builder_init(graph_builder* self, function* func){
  self->func = func;
  self->ast = func->ast;
}

graph*
graph_build(graph_builder* builder){
  target_entry_instr* normal = target_entry_new();
  builder->entry = graph_entry_new(builder->func, normal);
  effect_visitor for_effect;
  evis_init(&for_effect);
  visit_ast(((ast_node_visitor*) &for_effect), builder->ast);
  append_fragment(((block_entry_instr*) normal), for_effect);
  graph* g = malloc(sizeof(graph));
  graph_init(g, builder->func, builder->entry);
  return g;
}