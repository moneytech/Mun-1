#include <mun/asm/arm.h>

word
asm_label_pos(asm_label* self){
  return (asm_label_is_bound(self) ? -self->pos - kWordSize : self->pos - kWordSize);
}

MUN_INLINE void
emit(asm_buff* self, int32_t value){
  asm_buff_emit_int32_t(self, value);
}

static void
emit_type_01(asm_buff* self, condition cond, int type, asm_opcode code, int set, asm_register rn, asm_register rd, asm_operand* oper){
  int32_t enc = cond << kConditionShift |
                type << kTypeShift |
                code << kOpcodeShift |
                set << kSShift |
                rn << kRNShift |
                rd << kRDShift |
                oper->encoding;
  emit(self, enc);
}

static void
emit_type_5(asm_buff* self, condition cond, int32_t off, bool link){
  int32_t enc = cond << kConditionShift |
                5 << kTypeShift |
                (link ? 1 : 0) << kLinkShift;
  emit(branch_offset_encode(offset, enc));
}

static void
emit_memop(asm_buff* self, condition cond, bool load, bool byte, asm_register rd, asm_address* ad){
  int32_t enc = cond << kConditionShift |
                B26 | (
