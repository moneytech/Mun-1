#include <mun/object.h>

void
table_push(instance* inst, instance* value){
  TYPE_GUARD(inst, table);
  table* t = to_table(inst);
  table_set_at(inst, t->size, value);
  t->size += 1;
}

static instance**
table_data(table* t){
  const uword result = ((uword) t) + sizeof(table);
  return ((instance**) result);
}

static instance**
table_instance_addr_at_offset(table* t, word index){
  return &table_data(t)[index];
}

void
table_set_at(instance* t, word index, instance* value){
  TYPE_GUARD(t, table);
  *table_instance_addr_at_offset(to_table(t), index) = value;
}

instance*
table_get_at(instance* t, word index){
  TYPE_GUARD(t, table);
  return *table_instance_addr_at_offset(to_table(t), index);
}

static const word kOffsetOfPtr = 32;

word
table_element_offset(instance* t, word index){
  return (((word) (table_data((table*) kOffsetOfPtr) - kOffsetOfPtr)) + kWordSize);
}

char*
lua_to_string(instance* inst){
  switch(lua_typeof(inst)){
    case kFunctionTag: return "<function>";
    case kBooleanTag: return (to_boolean(inst)->value ? "true" : "false");
    case kNumberTag:{
      char* buffer = malloc(11);
      snprintf(buffer, 10, "%lf", to_number(inst)->value);
      buffer[11] = '\0';
      return buffer;
    }
    case kScriptTag: return "<script>";
    case kNilTag: return "nil";
    case kStringTag: return "<error>";
    case kTableTag: return "<table>";
    case kIllegalTag: return "<error>";
    default: return "<uh...?>";
  }
}

#define NEW_TYPE(Type, Tag, Var) \
  Type* Var = malloc(sizeof(Type)); \
  Var->type.type_id = Tag; \

#define OWNER(Var) \
  &Var->type

instance*
lua_script_new(char* loc){
  NEW_TYPE(lua_script, kScriptTag, s);
  s->location = strdup(loc);
  return OWNER(s);
}

instance*
function_new(char* name, int mods){
  NEW_TYPE(function, kFunctionTag, f);
  f->name = strdup(name);
  f->mods = mods;
  f->code = NULL;
  return OWNER(f);
}

instance*
boolean_new(bool value){
  NEW_TYPE(boolean, kBooleanTag, b);
  b->value = value;
  return OWNER(b);
}

instance*
number_new(double value){
  NEW_TYPE(number, kNumberTag, n);
  n->value = value;
  return OWNER(n);
}

instance*
table_new(word len){
  void* data = malloc(sizeof(table) + (kWordSize * len));
  table* t = ((table*) data);
  t->type.type_id = kTableTag;
  t->size = 0;
  t->asize = len;
  return &t->type;
}