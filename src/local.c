#include <mun/local.h>

bool
local_scope_add(local_scope* self, local_variable* local){
  if(local_scope_local_lookup(self, local->name) != NULL){
    return FALSE;
  }
  buffer_add(&self->variables, local);
  if(local->owner == NULL){
    local->owner = self;
  }
  return TRUE;
}

int
local_scope_alloc_vars(local_scope* self, int first_param_index, int num_params, int first_frame_index){
  int pos = 0;
  int frame_index = first_param_index;

  while(pos < num_params){
    local_variable* param = local_scope_var_at(self, pos);
    pos++;
    param->index = frame_index--;
  }

  frame_index = first_frame_index;
  while(pos < local_scope_var_count(self)){
    local_variable* local = local_scope_var_at(self, pos);
    pos++;
    if(local->owner == self){
      local->index = frame_index--;
    }
  }

  int min_frame_index = frame_index;
  local_scope* child = self->child;
  while(child != NULL){
    int child_frame_index = local_scope_alloc_vars(child, 0, 0, frame_index);
    if(child_frame_index < min_frame_index){
      min_frame_index = child_frame_index;
    }
    child = child->sibling;
  }

  return min_frame_index;
}

local_variable*
local_scope_local_lookup(local_scope* self, char* name){
  for(word i = 0; i < local_scope_var_count(self); i++){
    local_variable* local = local_scope_var_at(self, i);
    if(strcmp(local->name, name) == 0){
      return local;
    }
  }

  return NULL;
}

local_variable*
local_scope_lookup(local_scope* self, char* name){
  local_scope* current_scope = self;
  while(current_scope != NULL){
    local_variable* local = local_scope_local_lookup(current_scope, name);
    if(local != NULL){
      return local;
    }
    current_scope = current_scope->parent;
  }
  return NULL;
}

local_scope*
local_scope_new(local_scope* parent){
  local_scope* self = malloc(sizeof(local_scope));
  self->parent = parent;
  self->child = NULL;
  self->sibling = NULL;
  buffer_init(&self->variables, 10);
  if(parent != NULL){
    self->sibling = parent->child;
    parent->child = self;
  }
  return self;
}

local_variable*
local_var_new(char* name){
  local_variable* local = malloc(sizeof(local_variable));
  local->name = strdup(name);
  local->index = -1;
  local->owner = NULL;
  local->value = NULL;
  return local;
}