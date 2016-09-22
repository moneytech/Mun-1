#include <mun/buffer.h>
#include <mun/codegen/intermediate_language.h>
#include <mun/graph/graph_visitor.h>
#include <mun/codegen/il_defs.h>

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

static void
return_accept(instruction* instr, graph_visitor* vis){
  vis->visit_return(vis, instr);
}

return_instr*
return_new(il_value* val){
  return_instr* ret = malloc(sizeof(return_instr));
  static instruction_ops ops = {
      &return_set_input_at, // set_input_at
      &return_compile, // emit_machine_code
      &return_make_loc_summary, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      &return_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &return_input_at, // input_at
      &return_name, // name
      &return_accept, // accept
  };
  instr_init(&ret->defn.instr, kReturnTag, &ops);
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

static void
constant_accept(instruction* instr, graph_visitor* vis){
  vis->visit_constant(vis, instr);
}

constant_instr*
constant_new(instance* val){
  constant_instr* c = malloc(sizeof(constant_instr));
  static instruction_ops ops = {
      &constant_set_input_at, // set_input_at
      &constant_compile, // emit_machine_code
      &constant_make_loc_summary, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      &constant_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &constant_input_at, // input_at
      &constant_name, // name
      &constant_accept, // accept
  };
  instr_init(&c->defn.instr, kConstantTag, &ops);
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

static representation
binary_op_get_input_representation(instruction* instr, word index){
  return kUnboxedNumber;
}

static representation
binary_op_get_representation(instruction* instr){
  return kUnboxedNumber;
}

static void
binary_op_accept(instruction* instr, graph_visitor* vis){
  vis->visit_binary_op(vis, instr);
}

binary_op_instr*
binary_op_new(int oper, il_value* left, il_value* right){
  binary_op_instr* bop = malloc(sizeof(binary_op_instr));
  static instruction_ops ops ={
      &binary_op_set_input_at, // set_input_at
      &binary_op_compile, // emit_machine_code
      &binary_op_make_loc_summary, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      &binary_op_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &binary_op_input_at, // input_at
      &binary_op_name, // name
      &binary_op_accept, // accept
  };
  instr_init(((instruction*) bop), kBinaryOpTag, &ops);
  defn_init(((definition*) bop));
  bop->operation = oper;
  ((instruction*) bop)->get_representation = &binary_op_get_representation;
  ((instruction*) bop)->get_input_representation = &binary_op_get_input_representation;
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

static void
load_local_accept(instruction* instr, graph_visitor* vis){
  vis->visit_load_local(vis, instr);
}

load_local_instr*
load_local_new(local_variable* local){
  load_local_instr* lli = malloc(sizeof(load_local_instr));
  static instruction_ops ops = {
      &load_local_set_input_at, // set_input_at
      &load_local_compile, // emit_machine_code
      &load_local_make_loc_summary, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      &load_local_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &load_local_input_at, // input_at
      &load_local_name, // name
      &load_local_accept, // accept
  };
  instr_init(((instruction*) lli), kLoadLocalTag, &ops);
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

static void
store_local_accept(instruction* instr, graph_visitor* vis){
  vis->visit_store_local(vis, instr);
}

store_local_instr*
store_local_new(local_variable* local, il_value* val){
  store_local_instr* ssi = malloc(sizeof(store_local_instr));
  static instruction_ops ops = {
      &store_local_set_input_at, // set_input_at
      &store_local_compile, // emit_machine_code
      NULL, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      &store_local_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &store_local_input_at, // input_at
      &store_local_name, // name
      &store_local_accept, // accept
  };
  instr_init(((instruction*) ssi), kStoreLocalTag, &ops);
  defn_init(((definition*) ssi));
  ssi->local = local;
  ssi->is_dead = FALSE;
  ssi->is_last = FALSE;
  instr_set_input_at(((instruction*) ssi), 0, val);
  return ssi;
}

static char*
phi_name(){
  return "Phi";
}

static word
phi_input_count(instruction* instr){
  return to_phi(instr)->inputs.size;
}

static il_value*
phi_input_at(instruction* instr, word index){
  return buffer_at(&to_phi(instr)->inputs, index);
}

static void
phi_set_input_at(instruction* instr, word index, il_value* val){
  to_phi(instr)->inputs.data[index] = val;
}

static representation
phi_get_representation(instruction* instr){
  return to_phi(instr)->rep;
}

phi_instr*
phi_new(join_entry_instr* join, word num_inputs){
  phi_instr* phi = malloc(sizeof(phi_instr));
  static instruction_ops ops = {
      &phi_set_input_at, // set_input_at
      &phi_compile, // emit_machine_code
      NULL,
      NULL,
      NULL,
      &phi_input_count, // input_count
      NULL, // successor_count
      NULL, // success_at
      NULL, // get_block
      &phi_input_at, // input_at
      &phi_name, // name
  };
  instr_init(((instruction*) phi), kPhiTag, &ops);
  defn_init(((definition*) phi));
  phi->alive = FALSE;
  phi->reaching = NULL;
  phi->block = join;
  ((instruction*) phi)->get_representation = &phi_get_representation;
  buffer_init(&phi->inputs, num_inputs);
  for(word i = 0; i < num_inputs; i++){
    buffer_add(&phi->inputs, NULL);
  }
  return phi;
}

static char*
box_name(){
  return "BoxInstr";
}

static word
box_input_count(instruction* instr){
  return 1;
}

static il_value*
box_input_at(instruction* instr, word index){
  return to_box(instr)->input;
}

static void
box_set_input_at(instruction* instr, word index, il_value* value){
  to_box(instr)->input = value;
}

static representation
box_get_input_representation(instruction* instr, word index){
  return to_box(instr)->from;
}

box_instr*
box_new(representation from, il_value* value){
  box_instr* box = malloc(sizeof(box_instr));
  static instruction_ops ops = {
      &box_set_input_at, // set_input_at
      &box_compile, // emit_machine_code
      &box_make_loc_summary, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      &box_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &box_input_at, // input_at
      &box_name, // name
      NULL, // accept
  };
  instr_init(((instruction*) box), kBoxTag, &ops);
  defn_init(((definition*) box));
  box->input = value;
  box->from = from;
  ((instruction*) box)->get_input_representation = &box_get_input_representation;
  instr_set_input_at(((instruction*) box), 0, value);
  return box;
}

static char*
unbox_name(){
  return "UnboxInstr";
}

static word
unbox_input_count(instruction* instr){
  return 1;
}

static il_value*
unbox_input_at(instruction* instr, word index){
  return to_unbox(instr)->input;
}

static void
unbox_set_input_at(instruction* instr, word index, il_value* value){
  to_unbox(instr)->input = value;
}


static representation
unbox_get_representation(instruction* instr){
  return to_unbox(instr)->to;
}

unbox_instr*
unbox_new(representation to, il_value* value){
  unbox_instr* unbox = malloc(sizeof(unbox_instr));
  static instruction_ops ops = {
      &unbox_set_input_at, // set_input_at
      &unbox_compile, // emit_machine_code
      &unbox_make_loc_summary, // make_location_summary
      NULL, // argument_at
      NULL, // argument_count
      &unbox_input_count, // input_count
      NULL, // successor_count
      NULL, // successor_at
      NULL, // get_block
      &unbox_input_at, // input_at
      &unbox_name, // name
      NULL, // accept
  };
  instr_init(((instruction*) unbox), kUnboxTag, &ops);
  defn_init(((definition*) unbox));
  unbox->to = to;
  unbox->input = value;
  ((instruction*) unbox)->get_representation = &unbox_get_representation;
  instr_set_input_at(((instruction*) unbox), 0, value);
  return unbox;
}

#define get_phit(it) container_of(it, phi_iterator, iter)

static void
phi_iterator_advance(iterator* it){
  get_phit(it)->index++;
}

static bool
phi_iterator_done(iterator* it){
  phi_iterator* phit = get_phit(it);
  return ((phit->phis == NULL) || (phit->index >= phit->phis->size));
}

static void*
phi_iterator_current(iterator* it){
  return get_phit(it)->phis->data[get_phit(it)->index];
}

phi_iterator*
phi_iterator_new(join_entry_instr* join){
  phi_iterator* it = malloc(sizeof(phi_iterator));
  it->index = 0;
  it->phis = join->phis;
  it->iter.advance = &phi_iterator_advance;
  it->iter.current = &phi_iterator_current;
  it->iter.done = &phi_iterator_done;
  return it;
}