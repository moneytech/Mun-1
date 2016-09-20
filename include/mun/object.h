#ifndef MUN_OBJECT_H
#define MUN_OBJECT_H

#include "common.h"

HEADER_BEGIN

#include "local.h"

#define FOR_EACH_TYPE(V) \
  V(String) \
  V(Number) \
  V(Function) \
  V(Boolean) \
  V(Nil) \
  V(Table) \
  V(Script)

typedef enum {
  kMOD_NONE = 1 << 0,
  kMOD_NATIVE = 1 << 1
} obj_modifier;

typedef enum {
  kIllegalTag = -1,
#define DEFINE_TAG(Type) k##Type##Tag,
  FOR_EACH_TYPE(DEFINE_TAG)
#undef DEFINE_TAG
} obj_tag;

typedef struct _object_type{
  obj_tag type_id;
} obj_type;

typedef obj_type instance;

typedef struct {
  obj_type type;

  //TODO: StringBuffer
} string;

typedef struct {
  obj_type type;

  double value;
} number;

typedef struct {
  obj_type type;

  bool value;
} boolean;

typedef struct {
  obj_type type;

  word asize;
  word size;
} table;

typedef struct {
  obj_type type;

  char* name;
  int mods;
  uword code;
  struct _ast_node* ast;

  struct {
    word num_params;
    word num_stack_locals;
    word num_copied_params;

    word first_param_index;
    word first_stack_local_index;
  } def;

  local_scope* scope;
} function;

typedef struct {
  obj_type type;

  char* location;
} lua_script;

instance* lua_script_new(char* loc);
instance* function_new(char* name, int mods);
instance* table_new(word len);
instance* boolean_new(bool value);
instance* number_new(double value);
instance* nil_new();

#define to_lua_script(inst) container_of(inst, lua_script, type)
#define to_function(inst) container_of(inst, function, type)
#define to_table(inst) container_of(inst, table, type)
#define to_boolean(inst) container_of(inst, boolean, type)
#define to_number(inst) container_of(inst, number, type)

#define lua_typeof(inst) ((inst)->type_id)
#define is_lua_script(inst) ((inst) && lua_typeof(inst) == kScriptTag)
#define is_function(inst) ((inst) && lua_typeof(inst) == kFunctionTag)
#define is_table(inst) ((inst) && lua_typeof(inst) == kTableTag)
#define is_boolean(inst) ((inst) && lua_typeof(inst) == kBooleanTag)
#define is_number(inst) ((inst) && lua_typeof(inst) == kNumberTag)

#define TYPE_GUARD(Instance, Type) \
  if(!is_##Type(Instance)){\
    fprintf(stderr, "Not of type: %s\n", #Type); \
    abort(); \
  }

char* lua_to_string(instance* inst);

void table_push(instance* table, instance* value);
void table_set_at(instance* table, word index, instance* value);
void function_allocate_variables(instance* func);

instance* table_get_at(instance* table, word index);

word table_element_offset(instance* table, word index);

MUN_INLINE bool
function_is_native(instance* func) {
  TYPE_GUARD(func, function);
  return (to_function(func)->mods & kMOD_NATIVE) == kMOD_NATIVE;
}

MUN_INLINE word
function_num_non_copied_params(instance* func) {
  TYPE_GUARD(func, function);
  function* f = to_function(func);
  return (f->def.num_copied_params == 0)
         ? f->def.num_params
         : 0;
}

HEADER_END

#endif