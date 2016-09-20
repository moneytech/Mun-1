#ifndef MUN_STACK_FRAME_H
#define MUN_STACK_FRAME_H

#include "../common.h"

#if defined(ARCH_IS_64)
#include "frame/x64.h"
#else
#error "Unknown architecture"
#endif

#endif