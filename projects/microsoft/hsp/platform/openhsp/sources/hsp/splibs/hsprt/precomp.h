
#pragma once

#include "hsprt.h"

#undef memcpy
#undef memmove
#undef memset

#define SMALL_BLOCK_SIZE (sizeof(uint32_t))
#define LARGE_BLOCK_SIZE \
    (SMALL_BLOCK_SIZE * 8)    // We make use of all riscv temporaries

#define LARGE_BLOCK(X) ((X) >= LARGE_BLOCK_SIZE)
#define SMALL_BLOCK(X) ((X) >= SMALL_BLOCK_SIZE)

#define ALIGN_MASK     (uintptr_t)(sizeof(uint32_t) - 1)

#define ALIGNED(D, S) \
    ((((uintptr_t)D) & ALIGN_MASK) == (((uintptr_t)S) & ALIGN_MASK))
#define UNALIGNED(X) ((uint32_t)X & (SMALL_BLOCK_SIZE - 1))
