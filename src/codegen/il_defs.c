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

static char*
binary_op_name(){
  return "BinaryOp";
}

static word
binary_op_input_count(instruction* instr){
  return 2;
}

static il_value*
binary_op_input_at(instruction* instr, word index) {
  return to_binary_op(instr)->inputs[index];
}

static void
binary_op_set_input_at(instruction* instr, word index, il_value* val){
  to_binary_op(instr)->inputs[index] = val;
}

binary_op_instr*
binary_op_new(int oper, il_value* left, il_value* right){
  binary_op_instr* bop = malloc(sizeof(binary_op_instr));
  static const instruction_ops ops ={
      &binary_op_set_input_at, // set_input_at
      &binary_op_compile, // emit_machine_code
      NULL, // argument_at
      NULL, // argument_count
      &binary_op_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &binary_op_input_at, // input_at
      &binary_op_name, // name
  };
  instr_init(((instruction*) bop), &ops);
  defn_init(((definition*) bop));
  bop->operation = oper;
  instr_set_input_at(((instruction*) bop), 0, left);
  instr_set_input_at(((instruction*) bop), 1, right);
  return bop;
}

static char*
load_local_name(){
  return "LoadLocal";
}

static word
load_local_input_count(instruction* instr){
  return 0;
}

static il_value*
load_local_input_at(instruction* instr, word index){
  return NULL;
}

static void
load_local_set_input_at(instruction* instr, word index, il_value* val){
  //Fallthrough
}

load_local_instr*
load_local_new(local_variable* local){
  load_local_instr* lli = malloc(sizeof(load_local_instr));
  static const instruction_ops ops = {
      &load_local_set_input_at, // set_input_at
      &load_local_compile, // emit_machine_code
      NULL, // argument_at
      NULL, // argument_count
      &load_local_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &load_local_input_at, // input_at
      &load_local_name, // name
  };
  instr_init(((instruction*) lli), &ops);
  defn_init(((definition*) lli));
  lli->local = local;
  lli->is_last = FALSE;
  return lli;
}

static char*
store_local_name(){
  return "StoreLocal";
}

static word
store_local_input_count(instruction* instr){
  return 1;
}

static il_value*
store_local_input_at(instruction* instr, word index){
  return to_store_local(instr)->inputs[index];
}

static void
store_local_set_input_at(instruction* instr, word index, il_value* val){
  to_store_local(instr)->inputs[index] = val;
}

store_local_instr*
store_local_new(local_variable* local, il_value* val){
  store_local_instr* ssi = malloc(sizeof(store_local_instr));
  static const instruction_ops ops = {
      &store_local_set_input_at, // set_input_at
      &store_local_compile, // emit_machine_code
      NULL, // argument_at
      NULL, // argument_count
      &store_local_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &store_local_input_at, // input_at
      &store_local_name, // name
  };
  instr_init(((instruction*) ssi), &ops);
  defn_init(((definition*) ssi));
  ssi->local = local;
  ssi->is_dead = FALSE;
  ssi->is_last = FALSE;
  instr_set_input_at(((instruction*) ssi), 0, val);
  return ssi;
}