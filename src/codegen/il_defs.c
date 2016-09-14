#include <mun/codegen/il_defs.h>
#include <mun/codegen/il_compile.h>

static word
return_input_count(instruction* instr){
  return 1;
}

static il_value*
return_input_at(instruction* instr, word index){
  return_instr* ret = to_return(instr);
  return ret->inputs[index];
}

static void
return_set_input_at(instruction* instr, word index, il_value* val){
  to_return(instr)->inputs[index] = val;
}

static char*
return_name(){
  return "Return";
}

return_instr*
return_new(il_value* val){
  return_instr* ret = malloc(sizeof(return_instr));
  static const instruction_ops ops = {
      &return_set_input_at, // set_input_at
      &return_compile, // emit_machine_code
      NULL, // argument_at
      NULL, // argument_count
      &return_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &return_input_at, // input_at
      &return_name, // name
  };
  instr_init(&ret->defn.instr, &ops);
  defn_init(&ret->defn);
  instr_set_input_at(&ret->defn.instr, 0, val);
  return ret;
}

static word
constant_input_count(instruction* instr){
  return 0;
}

static il_value*
constant_input_at(instruction* instr, word index){
  return NULL;
}

static void
constant_set_input_at(instruction* instr, word index, il_value* val){
  //Fallthrough
}

static char*
constant_name(){
  return "Constant";
}

constant_instr*
constant_new(instance* val){
  constant_instr* c = malloc(sizeof(constant_instr));
  static const instruction_ops ops = {
      &constant_set_input_at, // set_input_at
      &constant_compile, // emit_machine_code
      NULL, // argument_at
      NULL, // argument_count
      &constant_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &constant_input_at, // input_at
      &constant_name // name
  };
  instr_init(&c->defn.instr, &ops);
  defn_init(&c->defn);
  c->value = val;
  return c;
}