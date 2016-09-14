#ifndef MUN_IL_X64_H
#define MUN_IL_X64_H

#include "../common.h"
#include "il_defs.h"

HEADER_BEGIN

void constant_compile(instruction* instr, asm_buff* code);
void return_compile(instruction* instr, asm_buff* code);

HEADER_END

#endif