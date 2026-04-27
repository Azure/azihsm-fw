// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#pragma once

#define CTZ_X64(lo, hi)  lo ? __builtin_ctz(lo) : hi ? (__builtin_ctz(hi) + 32) : 0
#define FindNextBit32(bitMap)   __builtin_ctz(bitMap)   // __builtin_ctz(0) is undefine, need to & 0x1f if hit this case
#define FindNextBitX64(bitMap)   __builtin_ctzll(bitMap)

__inline static uint8_t FindNextBit64(uint64_t _vfCmdExistBitMap)
{
    // need to investigate which one is faster
    #if 1
    uint8_t out = CTZ_X64((uint32_t)_vfCmdExistBitMap, (uint32_t)(_vfCmdExistBitMap >> 32));
    return out;
    #else
    return __builtin_ctzll(_vfCmdExistBitMap);
    #endif
}
