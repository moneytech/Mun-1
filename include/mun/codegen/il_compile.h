#ifndef MUN_IL_X64_H
#define MUN_IL_X64_H

#if !defined(MUN_INTERMEDIATE_LANGUAGE_H)
#error "Please #include <mun/codegen/intermediate_language.h> directly"
#endif

#include "../common.h"

HEADER_BEGIN

location_summary* constant_make_loc_summary(instruction* instr);
void constant_compile(instruction* instr, asm_buff* code);

location_summary* return_make_loc_summary(instruction* instr);
void return_compile(instruction* instr, asm_buff* code);

location_summary* binary_op_make_loc_summary(instruction* instr);
void binary_op_compile(instruction* instr, asm_buff* code);

location_summary* load_local_make_loc_summary(instruction* instr);
void load_local_compile(instruction* instr, asm_buff* code);

location_summary* store_local_make_loc_summary(instruction* instr);
void store_local_compile(instruction* instr, asm_buff* code);

location_summary* phi_make_loc_summary(instruction* instr);
void phi_compile(instruction* instr, asm_buff* code);

location_summary* join_entry_make_loc_summary(instruction* instr);
void join_entry_compile(instruction* instr, asm_buff* code);

HEADER_END

#endif