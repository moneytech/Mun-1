#include <mun/common.h>
#if defined(ARCH_IS_64)

#include <mun/codegen/il_core.h>
#include <mun/codegen/il_compile.h>
#include <mun/asm/x64.h>

void
constant_compile(instruction* instr, asm_buff* code){
  asm_movq_ri(code, TMP, ((asm_imm) to_constant(instr)->value));
}

void
return_compile(instruction* instr, asm_buff* code){
  asm_ret(code);
}

void
binary_op_compile(instruction* instr, asm_buff* code){

}

void
load_local_compile(instruction* instr, asm_buff* code){

}

void
store_local_compile(instruction* instr, asm_buff* code){

}

#endif