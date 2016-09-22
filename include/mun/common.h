#ifndef MUN_COMMON_H
#define MUN_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __cplusplus
#define HEADER_BEGIN extern "C"{
#define HEADER_END };
#else
#define HEADER_BEGIN
#define HEADER_END
#endif

HEADER_BEGIN

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#if defined(_MSC_VER)
#define MUN_INLINE static __inline
#else
#define MUN_INLINE static inline
#endif

typedef int bool;

typedef intptr_t word;
typedef uintptr_t uword;

typedef uintptr_t location;

static const int kWordSize = sizeof(word);

MUN_INLINE word*
word_clone(word val){
  word* w = malloc(sizeof(word));
  memcpy(w, &val, sizeof(word));
  return w;
}

MUN_INLINE bool*
bool_clone(bool val){
  bool* b = malloc(sizeof(bool));
  memcpy(b, &val, sizeof(bool));
  return b;
}

static const word kBitsPerByte = 8;
static const word kBitsPerWord = sizeof(word) * 8;

#if defined(_M_X64) || defined(__x86_64__)
#define ARCH_IS_64 1
#elif defined(_M_IX86) || defined(__i386__)
#define ARCH_IS_32 1
#else
#error "Cannot determine CPU architecture"
#endif

#if defined(_WIN32)
#define TARGET_IS_WIN 1
#elif defined(__linux__) || defined(__FreeBSD__)
#define TARGET_IS_LINUX 1
#else
#error "Unknown Operating System"
#endif

#ifndef container_of
#define container_of(ptr_, type_, member_)({ \
  const typeof(((type_*) 0)->member_)* __mbptr = ((void*) ptr_); \
  (type_*)((char*) __mbptr - offsetof(type_, member_)); \
})
#endif

HEADER_END

#endif