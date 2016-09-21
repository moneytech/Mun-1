#include <mun/graph/graph_builder.h>
#include <mun/codegen/il_core.h>
#include <mun/ast.h>

static instruction*
append_fragment(block_entry_instr* entry, effect_visitor vis){
  if(evis_is_empty(&vis)) return ((instruction*) entry);
  instr_link_to(((instruction*) entry), vis.entry);
  return vis.exit;
}

il_value*
evis_bind(effect_visitor* self, definition* defn){
  graph_builder_dealloc_temps(self->owner, instr_input_count(((instruction*) defn)));
  defn->temp_index = graph_builder_alloc_temp(self->owner);
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
  graph_builder_dealloc_temps(self->owner, instr_input_count(((instruction*) defn)));
  if(evis_is_empty(self)){
    self->entry = ((instruction*) defn);
  } else{
    instr_link_to(self->exit, ((instruction*) defn));
  }
  self->exit = ((instruction*) defn);
}

void
evis_add_instruction(effect_visitor* self, instruction* instr){
  printf("Adding Instruction: %s, %s\n", instr->ops->name(), evis_is_empty(self) ? "true" : "false");
  graph_builder_dealloc_temps(self->owner, instr_input_count(instr));
  if(evis_is_empty(self)){
    self->entry = self->exit = instr;
  } else{
    printf("Exit: %s\n", self->exit->ops->name());
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

static void
evis_return_value(ast_node_visitor* vis, il_value* val){

}

static void
evis_return_definition(ast_node_visitor* vis, definition* defn){
  printf("EVIS ReturnDefinition\n");
  if(!instr_is(((instruction*) defn), kConstantTag)){
    evis_do(get_evis(vis), defn);
  }
}

static void
evis_visit_literal(ast_node_visitor* vis, ast_node* node){
  definition* defn = ((definition*) constant_new(to_literal_node(node)->value));
  get_evis(vis)->return_definition(vis, defn);
}

static void
evis_visit_sequence(ast_node_visitor* vis, ast_node* node){
  sequence_node* seq = to_sequence_node(node);

  word i = 0;
  while(evis_is_open(get_evis(vis)) && (i < seq->size)){
    effect_visitor evis;
    evis_init(&evis, get_evis(vis)->owner);
    visit_ast(((ast_node_visitor*) &evis), seq->nodes[i++]);
    evis_append(get_evis(vis), evis);
    if(!evis_is_open(get_evis(vis))){
      break;
    }
  }
}

static void
evis_visit_return(ast_node_visitor* vis, ast_node* node){
  value_visitor for_value;
  vvis_init(&for_value, get_evis(vis)->owner);

  visit_ast(((ast_node_visitor*) &for_value), to_return_node(node)->value);

  evis_append(get_evis(vis), for_value.effect);
  evis_add_return_exit(get_evis(vis), for_value.value);
}

static void
evis_visit_binary_op(ast_node_visitor* vis, ast_node* node){
  binary_op_node* bopn = to_binary_op_node(node);

  value_visitor for_left_value;
  vvis_init(&for_left_value, get_evis(vis)->owner);
  visit_ast(((ast_node_visitor*) &for_left_value), bopn->left);
  evis_append(get_evis(vis), for_left_value.effect);

  value_visitor for_right_value;
  vvis_init(&for_right_value, get_evis(vis)->owner);
  visit_ast(((ast_node_visitor*) &for_right_value), bopn->right);
  evis_append(get_evis(vis), for_right_value.effect);

  vis_return_definition(vis, ((definition*) binary_op_new(bopn->operation, for_left_value.value, for_right_value.value)));
}

static void
evis_visit_store_local(ast_node_visitor* vis, ast_node* node){
  value_visitor for_value;
  vvis_init(&for_value, get_evis(vis)->owner);
  store_local_node* sln = to_store_local_node(node);
  visit_ast(((ast_node_visitor*) &for_value), sln->value);
  evis_append(get_evis(vis), for_value.effect);
  definition* store = ((definition*) store_local_new(sln->local, for_value.value));
  vis_return_definition(vis, store);
}

#define VISITOR self->visitor

void
evis_init(effect_visitor* self, graph_builder* owner){
  self->owner = owner;
  self->entry = NULL;
  self->exit = NULL;
  self->return_value = &evis_return_value;
  self->return_definition = &evis_return_definition;
  VISITOR.visit_literal_node = &evis_visit_literal;
  VISITOR.visit_sequence_node = &evis_visit_sequence;
  VISITOR.visit_return_node = &evis_visit_return;
  VISITOR.visit_binary_op_node = &evis_visit_binary_op;
  VISITOR.visit_store_local_node = &evis_visit_store_local;
}

#undef VISITOR
#define VISITOR self->effect.visitor

static void
vvis_return_value(ast_node_visitor* vis, il_value* val){
  get_vvis(vis)->value = val;
}

static void
vvis_return_definition(ast_node_visitor* vis, definition* defn){
  get_vvis(vis)->value = evis_bind(get_evis(vis), defn);
}

static void
vvis_visit_load_local(ast_node_visitor* vis, ast_node* node){
  definition* load = NULL;
  load_local_node* lln = to_load_local_node(node);
  if(local_var_is_constant(lln->local)){
    load = ((definition*) constant_new(lln->local->value));
  } else{
    load = ((definition*) load_local_new(lln->local));
  }
  get_vvis(vis)->effect.return_definition(vis, load);
}

void
vvis_init(value_visitor* self, graph_builder* owner){
  evis_init(((effect_visitor*) self), owner);
  self->value = NULL;
  self->effect.return_value = &vvis_return_value;
  self->effect.return_definition = &vvis_return_definition;
  VISITOR.visit_load_local_node = &vvis_visit_load_local;
}

void
graph_builder_init(graph_builder* self, function* func){
  self->func = func;
  self->ast = func->ast;
  self->temp_count = 0;
}

graph*
graph_build(graph_builder* builder){
  target_entry_instr* normal = target_entry_new();
  builder->entry = graph_entry_new(builder->func, normal);
  effect_visitor for_effect;
  evis_init(&for_effect, builder);
  visit_ast(((ast_node_visitor*) &for_effect), builder->ast);
  append_fragment(((block_entry_instr*) normal), for_effect);

  graph* g = malloc(sizeof(graph));
  graph_init(g, builder->func, builder->entry);
  return g;
}