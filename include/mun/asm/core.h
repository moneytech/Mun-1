#ifndef MUN_CORE_H
#define MUN_CORE_H

#include "../common.h"

HEADER_BEGIN

typedef struct _asm_buff{
  uword contents;
  uword cursor;
  uword limit;
} asm_buff;

#define EMIT(Type) \
  MUN_INLINE void \
  asm_buff_emit_##Type(asm_buff* self, Type value){ \
    *((Type*) self->cursor) = value; \
    self->cursor += sizeof(Type); \
  }
EMIT(uint8_t);
EMIT(int32_t);
EMIT(int64_t);
#undef EMIT

#define IO(Type) \
  MUN_INLINE Type \
  asm_buff_load_##Type(asm_buff* self, word pos){ \
    return ((Type) (self->contents + pos)); \
  } \
  MUN_INLINE void \
  asm_buff_store_##Type(asm_buff* self, word pos, Type value){ \
    *((Type*) (self->contents + pos)) = value; \
  }
IO(int32_t);
#undef IO

MUN_INLINE word
asm_buff_size(asm_buff* self){
  return self->cursor - self->contents;
}

void asm_buff_init(asm_buff* self);

#if defined(ARCH_IS_64)
#include "x64.h"
#include "frame/x64.h"
#else
#error "Unknown CPU Architecture"
#endif

HEADER_END

#endif