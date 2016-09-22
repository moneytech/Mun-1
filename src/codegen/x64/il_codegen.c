#include <mun/common.h>
#if defined(ARCH_IS_64)

#include <mun/asm/core.h>
#include <mun/codegen/intermediate_language.h>

location_summary*
constant_make_loc_summary(instruction* instr){
  location out;
  loc_init_c(&out, to_constant(instr));
  return loc_summary_make(0, out, NO_CALL);
}

void
constant_compile(instruction* instr, asm_buff* code){
  location_summary* locs = instr->locations;
  if(loc_is_register(locs->output)){
    asm_movq_ri(code, loc_get_register(locs->output), ((asm_imm) to_constant(instr)->value));
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
  asm_ret(code);
}

location_summary*
binary_op_make_loc_summary(instruction* instr){
  location_summary* locs = loc_summary_new(2, 0, NO_CALL);
  loc_hint_rx(&locs->inputs[0]);
  loc_hint_rx(&locs->inputs[1]);
  loc_init_s(&locs->output);
  return locs;
}

void
binary_op_compile(instruction* instr, asm_buff* code){
  asm_fpu_register left = loc_get_fpu_register(instr->locations->inputs[0]);
  asm_fpu_register right = loc_get_fpu_register(instr->locations->inputs[1]);
  asm_fpu_register result = loc_get_fpu_register(instr->locations->output);

  switch(to_binary_op(instr)->operation){
    case 0x0: asm_addsd_rr(code, left, right); break;
    case 0x1: asm_subsd_rr(code, left, right); break;
    case 0x2: asm_mulsd_rr(code, left, right); break;
    case 0x3: asm_divsd_rr(code, left, right); break;
    default:{
      fprintf(stderr, "Unreachable\n");
      abort();
    }
  }
}

location_summary*
load_local_make_loc_summary(instruction* instr){
  load_local_instr* lli = to_load_local(instr);
  word stack_index = lli->local->index < 0 ?
                     kFirstLocalSlotFromFp - lli->local->index :
                     kParamEndSlotFromFp - lli->local->index;
  location stack_slot;
  loc_init_z(&stack_slot, stack_index);
  return loc_summary_make(0, stack_slot, NO_CALL);
}

void
load_local_compile(instruction* instr, asm_buff* code){
  // Fallthrough
}

location_summary*
store_local_make_loc_summary(instruction* instr){
  location same;
  loc_init_s(&same);
  return loc_summary_make(1, same, NO_CALL);
}

void
store_local_compile(instruction* instr, asm_buff* code){
  store_local_instr* sli = to_store_local(instr);
  asm_address slot;
  asm_addr_init(&slot, RBP, sli->local->index * kWordSize);
  asm_movq_ar(code, &slot, loc_get_register(instr->locations->inputs[0]));
}

void
phi_compile(instruction* instr, asm_buff* code){

}

void
join_entry_compile(instruction* instr, asm_buff* code){

}

location_summary*
box_make_loc_summary(instruction* instr){
  location_summary* locs = loc_summary_new(1, 0, NO_CALL);
  loc_hint_rx(&locs->inputs[0]);
  loc_hint_rr(&locs->output);
  return locs;
}

void
box_compile(instruction* instr, asm_buff* code){
  asm_register out = loc_get_register(instr->locations->output);
  asm_fpu_register in = loc_get_fpu_register(instr->locations->inputs[0]);

  asm_address num_val_off_addr;
  asm_addr_init(&num_val_off_addr, out, offsetof(number, value));

  asm_movq_ri(code, out, ((asm_imm) number_new(0.0)));
  asm_movsd_ar(code, &num_val_off_addr, in);
}

location_summary*
unbox_make_loc_summary(instruction* instr){
  location_summary* locs = loc_summary_new(1, 0, NO_CALL);
  loc_hint_rr(&locs->inputs[0]);
  loc_hint_rx(&locs->output);
  return locs;
}

void
unbox_compile(instruction* instr, asm_buff* code){
  asm_register in = loc_get_register(instr->locations->inputs[0]);
  asm_fpu_register out = loc_get_fpu_register(instr->locations->output);
  asm_address num_val_off_addr;
  asm_addr_init(&num_val_off_addr, in, offsetof(number, value));
  asm_movsd_ra(code, out, &num_val_off_addr);
}

#endif