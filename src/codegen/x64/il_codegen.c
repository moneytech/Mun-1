#include <mun/common.h>
#if defined(ARCH_IS_64)

#include <mun/asm/core.h>
#include <mun/asm/x64.h>
#include <mun/codegen/intermediate_language.h>

location_summary*
constant_make_loc_summary(instruction* instr){
  constant_instr* c = to_constant(instr);
  location out;
  loc_init_c(&out, c);
  return loc_summary_make(0, out, NO_CALL);
}

void
constant_compile(instruction* instr, asm_buff* code){
  location_summary* locs = instr->locations;
  if(loc_is_register(locs->inputs[0])){
    printf("Loading '%s' into #%d\n", lua_to_string(to_constant(instr)->value), loc_get_register(locs->inputs[0]));
  }
}

location_summary*
return_make_loc_summary(instruction* instr){
  location_summary* locs = loc_summary_new(1, 0, NO_CALL);
  loc_init_r(&locs->inputs[0], RAX);
  return locs;
}

void
return_compile(instruction* instr, asm_buff* code){
  printf("Returning\n");
}

location_summary*
binary_op_make_loc_summary(instruction* instr){
  location_summary* locs = loc_summary_make(2, 0, NO_CALL);
  loc_hint_rx(&locs->inputs[0]);
  loc_hint_rx(&locs->inputs[1]);
  loc_init_s(&locs->output);
  return locs;
}

void
binary_op_compile(instruction* instr, asm_buff* code){
  location_summary* locs = instr->locations;
  location a = locs->inputs[0];
  location b = locs->inputs[1];
  location res = locs->output;

  printf("BinaryOp on #%d,#%d -> #%d\n", loc_get_register(a), loc_get_register(b), loc_get_register(res));
}

void
load_local_compile(instruction* instr, asm_buff* code){

}

void
store_local_compile(instruction* instr, asm_buff* code){

}

void
phi_compile(instruction* instr, asm_buff* code){

}

void
join_entry_compile(instruction* instr, asm_buff* code){

}

#endif