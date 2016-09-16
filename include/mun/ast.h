#ifndef MUN_AST_H
#define MUN_AST_H

#include "common.h"
#include "object.h"
#include "local.h"

HEADER_BEGIN

#define FOR_EACH_NODE(V) \
  V(Return) \
  V(Literal) \
  V(Sequence) \
  V(BinaryOp) \
  V(LoadLocal) \
  V(StoreLocal)

#define MAX_SEQ_NODE_SIZE 0xFFFF

typedef enum{
#define DEFINE_NODE(Name) k##Name##Node,
  FOR_EACH_NODE(DEFINE_NODE)
#undef DEFINE_NODE
} ast_node_type;

static const char* ast_node_names[] = {
#define DEFINE_NAME(Name) #Name,
  FOR_EACH_NODE(DEFINE_NAME)
#undef DEFINE_NAME
};

typedef struct _ast_node{
  ast_node_type type;
  int col;
  int row;
} ast_node;

typedef struct{
  ast_node node;

  ast_node* nodes[MAX_SEQ_NODE_SIZE];
  word size;
} sequence_node;

typedef struct{
  ast_node node;

  instance* value;
} literal_node;

typedef struct{
  ast_node node;

  ast_node* value;
} return_node;

typedef struct{
  ast_node node;

  ast_node* left;
  ast_node* right;
  int operation;
} binary_op_node;

typedef struct{
  ast_node node;

  local_variable* local;
} load_local_node;

typedef struct{
  ast_node node;

  ast_node* value;
  local_variable* local;
} store_local_node;

MUN_INLINE const char*
ast_node_typeof(ast_node* node){
  return ast_node_names[node->type];
}

#define to_sequence_node(n) container_of(n, sequence_node, node)
#define to_return_node(n) container_of(n, return_node, node)
#define to_literal_node(n) container_of(n, literal_node, node)
#define to_binary_op_node(n) container_of(n, binary_op_node, node)
#define to_store_local_node(n) container_of(n, store_local_node, node)
#define to_load_local_node(n) container_of(n, load_local_node, node)

#define node_typeof(n) ((n)->type)
#define is_sequence_node(n) ((n) && node_typeof(n) == kSequenceNode)
#define is_return_node(n) ((n) && node_typeof(n) == kReturnNode)
#define is_literal_node(n) ((n) && node_typeof(n) == kLiteralNode)
#define is_binary_op_node(n) ((n) && node_typeof(n) = kBinaryOpNode)
#define is_load_local_node(n) ((n) && node_typeof(n) == kLoadLocalNode)
#define is_store_local_node(n) ((n) && node_typeof(n) == kStoreLocalNode)

#define NODE_GUARD(Node, Type) \
  if(!is_##Type(Node)){ \
    fprintf(stderr, "Not of type: %s\n", #Type); \
    abort(); \
  }

ast_node* return_node_new(ast_node* value);
ast_node* literal_node_new(instance* value);
ast_node* sequence_node_new();
ast_node* binary_op_node_new(ast_node* left, ast_node* right, int operation);
ast_node* load_local_node_new(local_variable* local);
ast_node* store_local_node_new(local_variable* local, ast_node* value);

int sequence_node_append(ast_node* seq, ast_node* child);

typedef struct _ast_node_visitor{
  void (*visit_sequence_node)(struct _ast_node_visitor* vis, ast_node* seq);
  void (*visit_binary_op_node)(struct _ast_node_visitor* vis, ast_node* op);
  void (*visit_literal_node)(struct _ast_node_visitor* vis, ast_node* lit);
  void (*visit_return_node)(struct _ast_node_visitor* vis, ast_node* ret);
  void (*visit_load_local_node)(struct _ast_node_visitor*, ast_node*);
  void (*visit_store_local_node)(struct _ast_node_visitor*, ast_node*);
} ast_node_visitor;

void visit_ast(ast_node_visitor* vis, ast_node* node);

HEADER_END

#endif