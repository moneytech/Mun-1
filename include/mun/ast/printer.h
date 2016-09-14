#ifndef MUN_PRINTER_H
#define MUN_PRINTER_H

#include "../common.h"
#include "../ast.h"

typedef struct{
  ast_node_visitor visitor;

  int indent;
} ast_printer;

ast_node_visitor* ast_printer_new();

#endif