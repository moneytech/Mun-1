#ifndef MUN_ENVIRONMENT_H
#define MUN_ENVIRONMENT_H

#include "../common.h"

HEADER_BEGIN

#include "../buffer.h"
#include "../object.h"

typedef struct _instruction instruction;

typedef struct _environment{
  object_buffer values; // il_value*
  location* locs;
  word param_count;
  function* func;
  struct _environment* outer;
} environment;

environment* env_deep_copy(environment* env, word length);

void env_deep_copy_to(environment* self, instruction* instr);

HEADER_END

#endif