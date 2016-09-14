#ifndef MUN_IL_DEFS_H
#define MUN_IL_DEFS_H

#include "il_core.h"

HEADER_BEGIN

typedef struct{
  definition defn;

  il_value* inputs[1];
} return_instr;

return_instr* return_new(il_value* val);

typedef struct{
  definition defn;

  instance* value;
} constant_instr;

constant_instr* constant_new(instance* val);

#define to_return(i) container_of(container_of(i, definition, instr), return_instr, defn)
#define to_constant(i) container_of(container_of(i, definition, instr), constant_instr, defn)

HEADER_END

#endif