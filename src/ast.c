#include <mun/ast.h>

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
  }
}