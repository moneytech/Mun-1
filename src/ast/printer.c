#include <mun/ast/printer.h>

#define get_printer(vis) container_of(vis, ast_printer, visitor)

static void
correctify(ast_printer* printer){
  for(int i = 0; i < printer->indent; i++) printf(" ");
}

static void
visit_sequence(ast_node_visitor* vis, ast_node* node){
  ast_printer* printer = get_printer(vis);
  sequence_node* seq = to_sequence_node(node);
  correctify(printer);
  printer->indent++;
  printf("{\n");
  for(int i = 0; i < seq->size; i++){
    correctify(printer);
    visit_ast(vis, seq->nodes[i]);
    printf("\n");
  }
  printer->indent--;
  correctify(printer);
  printf("}\n");
}

MUN_INLINE char*
binary_op_to_string(int operation){
  switch(operation){
    case 0x0: return "+";
    case 0x1: return "-";
    case 0x2: return "*";
    case 0x3: return "/";
    default: return "?";
  }
}

static void
visit_binary_op(ast_node_visitor* vis, ast_node* node){
  binary_op_node* bop = to_binary_op_node(node);
  visit_ast(vis, bop->left);
  printf(" %s ", binary_op_to_string(bop->operation));
  visit_ast(vis, bop->right);
}

static void
visit_return(ast_node_visitor* vis, ast_node* node){
  printf("return ");
  visit_ast(vis, to_return_node(node)->value);
}

static void
visit_literal(ast_node_visitor* vis, ast_node* node){
  printf("%s", lua_to_string(to_literal_node(node)->value));
}

ast_node_visitor*
ast_printer_new(){
  ast_printer* printer = malloc(sizeof(ast_printer));
  printer->indent = 0;
  printer->visitor.visit_binary_op_node = &visit_binary_op;
  printer->visitor.visit_sequence_node = &visit_sequence;
  printer->visitor.visit_literal_node = &visit_literal;
  printer->visitor.visit_return_node = &visit_return;
  return &printer->visitor;
}