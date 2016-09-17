#ifndef MUN_ARM_H
#define MUN_ARM_H

#include "core.h"

HEADER_BEGIN

typedef enum{
  H = 1 << 5, // halfword
  L = 1 << 20, // load or store
  S = 1 << 20, // set condition code (or leave unchanged)
  W = 1 << 21, // writeback base register (or leave unchanged)
  A = 1 << 21, // accumulate in multiply instruction (or not)
  B = 1 << 22, // unsigned byte (or word)
  D = 1 << 22, // high/lo bit of start of d/s register range
  N = 1 << 22, // long (or short)
  U = 1 << 23, // positive (or negative) offset/index
  P = 1 << 24, // offset/pre-indexed addressing (or post-indexed addressing)
  I = 1 << 25, // immediate shifter operand (or not)

  B0 = 1,
  B1 = 1 << 1,
  B2 = 1 << 2,
  B3 = 1 << 3,
  B4 = 1 << 4,
  B5 = 1 << 5,
  B6 = 1 << 6,
  B7 = 1 << 7,
  B8 = 1 << 8,
  B9 = 1 << 9,
  B10 = 1 << 10,
  B11 = 1 << 11,
  B12 = 1 << 12,
  B13 = 1 << 13,
  B14 = 1 << 14,
  B15 = 1 << 15,
  B16 = 1 << 16,
  B17 = 1 << 17,
  B18 = 1 << 18,
  B19 = 1 << 19,
  B20 = 1 << 20,
  B21 = 1 << 21,
  B22 = 1 << 22,
  B23 = 1 << 23,
  B24 = 1 << 24,
  B25 = 1 << 25,
  B26 = 1 << 26,
  B27 = 1 << 27,
} asm_encoding;

typedef struct{
  word pos;
} asm_label;

word asm_label_pos(asm_label* self);

MUN_INLINE void
asm_label_bind_to(asm_label* self, word pos){
  self->pos = -pos - kWordSize;
}

MUN_INLINE void
asm_label_link_to(asm_label* self, word pos){
  self->pos = pos + kWordSize;
}

MUN_INLINE bool
asm_label_is_bound(asm_label* self){
  return self->pos < 0;
}

MUN_INLINE bool
asm_label_is_unused(asm_label* self){
  return self->pos == 0;
}

MUN_INLINE bool
asm_label_is_linked(asm_label* self){
  return self->pos > 0;
}

typedef struct{
  uint32_t type;
  uint32_t encoding;
} asm_operand;

void asm_operand_new_i(asm_operand* self, uint32_t imm);
void asm_operand_new_ii(asm_operand* self, uint32_t rotate, uint32_t imm);
void asm_operand_new_r(asm_operand* self, asm_register rm);
void asm_operand_new_si(asm_operand* self, asm_register rm, shift s, uint32_t imm);
void asm_operand_new_sr(asm_operand* self, asm_register rm, shift s, asm_register rs);

bool asm_can_hold(uint32_t imm, asm_operand* oper);

typedef enum{
  kByte,
  kUnsignedByte,
  kHalfword,
  kUnsignedHalfword,
  kWord,
  kUnsignedWord,
  kWordPair,
  kSWord,
  kDWord,
  kRegList,
} asm_operand_size;

typedef enum{
  DA = (0|0|0) << 21, // dec after
  IA = (0|4|0) << 21, // inc after
  DB = (8|0|0) << 21, // dec before
  IB = (8|4|0) << 21, // inc before
  
  DA_W = (0|0|1) << 21, // dec after with writeback
  IA_W = (0|4|1) << 21, // inc after with writeback
  DB_W = (8|0|1) << 21, // dec before with writeback
  IB_W = (0|4|1) << 21, // inc before with writeback
} asm_block_address_mode;

typedef enum{
  kImmediate,
  kIndexRegister,
  kScaledIndexRegister,
} asm_offset_kind;

typedef enum{
  kModeMask = (8|4|1) << 21,
  // bit encoding P U W
  kOffset = (8|4|0) << 21, // offset (w/o writeback)
  kPreIndex = (8|4|1) << 21, // pre-indexed addressing with writeback
  kPostIndex = (0|4|0) << 21, // post-indexed addressing with writeback
  kNegOffset = (8|0|0) << 21, // negative offset (w/o writeback)
  kNegPreIndex = (8|0|1) << 21, // negative pre-indexed with writeback
  kNegPostIndex = (0|0|0) << 21, // negative post-indexed with writebakc
} asm_address_mode;

typedef struct{
  uint32_t encoding;
  asm_offset_kind kind;
} asm_address;

void asm_addr_init_a(asm_address* self, asm_register rn, int32_t offset, asm_address_mode am);
void asm_addr_init_r(asm_address* self, asm_register rn, asm_register r, asm_address_mode am);
void asm_addr_init_sr(asm_address* self, asm_register rn, asm_register rm, shift s, uint32_t imm, asm_address_mode mode);
void asm_addr_init_rs(asm_address* self, asm_register rn, asm_register rm, shift s, asm_regiter r, asm_address_mode mode);

asm_operand_size operand_size_for(word cid);

bool operand_size_can_load_offset(asm_operand_size size, int32_t offset, int32_t* offset_mask);
bool operand_size_can_store_offset(asm_operand_size size, int32_t offset, int32_t* offset_mask);
bool operand_size_can_imm_offset(asm_operand_size size, int32_t cid, int64_t offset);

MUN_INLINE asm_address_mode
asm_addr_mode(asm_address* self){
  return ((asm_address_node) self->encoding & kModeMask);
}

MUN_INLINE bool
asm_addr_has_writeback(asm_address* self){
  asm_address_mode mode = asm_addr_mode(self);
  return (mode == kPreIndex) ||
         (mode == kPostIndex) ||
         (mode == kNegPreIndex) ||
         (mode == kNegPostIndex);
}

// add instructions
void asm_add(asm_buff* self, asm_register rd, asm_register rn, asm_operand* o, condition cond);
void asm_adds(asm_buff* self, asm_register rd, asm_register rn, asm_operand* o, condition cond);

// mov instructions
void asm_mov(asm_buff* self, asm_register rd, asm_operand* o, condition cond);
void asm_movs(asm_buff* self, asm_register rd, asm_operand* o, condition cond); 

// div instructions
void asm_sdiv(asm_buff* self, asm_register rd, asm_register rn, asm_register rm, condition cond);
void asm_udiv(asm_buff* self, asm_register rd, asm_register rn, asm_register rm, condition cond);

// load/store instructions
void asm_ldr(asm_buff* self, asm_register rd, asm_address* ad, condition cond);
void asm_str(asm_buff* self, asm_register rd, asm_address* ad, condition cond);
void asm_ldrb(asm_buff* self, asm_register rd, asm_address* ad, condition cond);
void asm_strb(asm_buff* self, asm_register rd, asm_address* ad, condition cond);
void asm_ldrh(asm_buff* self, asm_register rd, asm_address* ad, condition);
void asm_strh(asm_buff* self, asm_register rd, asm_address* ad, condition cond);
void asm_ldrsb(asm_buff* self, asm_register rd, asm_address* ad, condition cond);
void asm_ldrsh(asm_buff* self, asm_register rd, asm_address* ad, condition cond);
void asm_ldrd(asm_buff* self, asm_register rd, asm_register rd2, asm_register rn, in32_t offset, condition cond);
void asm_strd(asm_buff* self, asm_register rd, asm_register rd2, asm_register rn, int32_t offset, condition cond);
void asm_ldm(asm_buff* self, asm_block_address_mode am, asm_register base, int32_t regs, condition cond);
void asm_stm(asm_buff* self, asm_block_address_mode am, asm_register base, int32_t regs, condition cond);
void asm_ldrex(asm_buff* self, asm_register rd, asm_register rn, condition cond);
void asm_strex(asm_buff* self, asm_register rd, asm_register rt, condition cond);

MUN_INLINE int32_t 
bkpt_encode(uint16_t imm){
  return (AL << kConditionShift) | B24 | B21 |
            ((imm >> 4) << 8) | B6 | B5 | B4 | (imm & 0xF);
}

HEADER_END

#endif
