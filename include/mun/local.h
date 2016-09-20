#ifndef MUN_LOCAL_H
#define MUN_LOCAL_H

#include "common.h"

HEADER_BEGIN

#include "buffer.h"
#include "asm/core.h"

typedef struct _object_type instance;
typedef struct _local_scope local_scope;

typedef struct _local_variable{
  char* name;
  local_scope* owner;
  int index;
  instance* value;
} local_variable;

local_variable* local_var_new(char* name);

MUN_INLINE bool
local_var_is_constant(local_variable* var){
  return var->value != NULL;
}

MUN_INLINE bool
local_var_has_index(local_variable* var){
  return var->index != -1;
}

#if defined(ARCH_IS_64)
static const int kParamEndSlotFromFp = 1;
static const int kFirstLocalSlotFromFp = -3;
#else
#error "Unsupported CPU Architecture"
#endif

MUN_INLINE int
local_var_bit_index(local_variable* var, word param_count){
  return (int) ((var->index > 0) ?
           param_count - (var->index - kParamEndSlotFromFp) :
           param_count - (var->index - kFirstLocalSlotFromFp));
}

typedef struct _local_scope{
  local_scope* parent;
  local_scope* child;
  local_scope* sibling;

  object_buffer variables;
} local_scope;

local_scope* local_scope_new(local_scope* parent);

bool local_scope_add(local_scope* scope, local_variable* var);

local_variable* local_scope_local_lookup(local_scope* scope, char* name);
local_variable* local_scope_lookup(local_scope* scope, char* name);

int local_scope_alloc_vars(local_scope* scope, int first_param_index, int num_params, int first_frame_index);

MUN_INLINE local_variable*
local_scope_var_at(local_scope* scope, word index){
  return buffer_at(&scope->variables, index);
}

MUN_INLINE bool
local_scope_insert_param(local_scope* scope, word index, local_variable* var){
  if(local_scope_local_lookup(scope, var->name) != NULL){
    return FALSE;
  }
  buffer_insert_at(&scope->variables, index, var);
  var->owner = scope;
  return TRUE;
}

MUN_INLINE word
local_scope_var_count(local_scope* scope){
  return scope->variables.size;
}

HEADER_END

#endif