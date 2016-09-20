#include <mun/ast.h>
#include <mun/local.h>

#define NEW_NODE(Type, ID, Var) \
  Type* Var = malloc(sizeof(Type)); \
  Var->node.type = ID;

#define OWNER(Var) \
  &Var->node

ast_node*
return_node_new(ast_node* value){
  NEW_NODE(return_node, kReturnNode, r);
  r->value = value;
  return OWNER(r);
}

ast_node*
literal_node_new(instance* value){
  NEW_NODE(literal_node, kLiteralNode, l);
  l->value = value;
  return OWNER(l);
}

ast_node*
sequence_node_new(){
  NEW_NODE(sequence_node, kSequenceNode, s);
  s->size = 0;
  return OWNER(s);
}

ast_node*
binary_op_node_new(ast_node* left, ast_node* right, int operation){
  NEW_NODE(binary_op_node, kBinaryOpNode, bop);
  bop->left = left;
  bop->right = right;
  bop->operation = operation;
  return OWNER(bop);
}

ast_node*
load_local_node_new(local_variable* local){
  NEW_NODE(load_local_node, kLoadLocalNode, ll);
  ll->local = local;
  return OWNER(ll);
}

ast_node*
store_local_node_new(local_variable* local, ast_node* value){
  NEW_NODE(store_local_node, kStoreLocalNode, sl);
  sl->local = local;
  sl->value = value;
  return OWNER(sl);
}

int
sequence_node_append(ast_node* n, ast_node* child){
  NODE_GUARD(n, sequence_node);
  sequence_node* seq = to_sequence_node(n);
  if(seq->size + 1 >= MAX_SEQ_NODE_SIZE) return 0;
  seq->nodes[seq->size++] = child;
  return 1;
}

void
visit_ast(ast_node_visitor* visitor, ast_node* node){
  switch(node->type){
    case kLiteralNode: visitor->visit_literal_node(visitor, node); break;
    case kReturnNode: visitor->visit_return_node(visitor, node); break;
    case kSequenceNode: visitor->visit_sequence_node(visitor, node); break;
    case kBinaryOpNode: visitor->visit_binary_op_node(visitor, node); break;
    case kLoadLocalNode: visitor->visit_load_local_node(visitor, node); break;
    case kStoreLocalNode: visitor->visit_store_local_node(visitor, node); break;
  }
}