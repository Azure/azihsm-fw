// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ArmIntrinsics.h"
#include "MemorySection.h"
#include "SysTypes.h"
//#include "FeaturesGen.h"

#ifdef __cplusplus
extern "C"
{
#endif

//=============================================================================
//
//! @file
//! @brief  Header file for Memory I/O
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  constant definitions:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------

#define M7_MEM_SET(dest, ch, n)            __builtin_memset(dest, ch, n)//memset(dest, ch, n)
#define MEM_CLR(dest, n)                   M7_MEM_SET(dest, 0, n)
#define M7_MEM_COPY(dest, src, n)          memcpy(dest, src, n)
#define MEM_MOVE(dest, src, n)             memmove(dest, src, n)
#define MEM_CMP(src1, src2, n)             memcmp(src1, src2, n)

#define RANDOM(R)                       (((uint32_t)rand()) % (R))        ///< get random number beteen 0 to R-1
#define RAND()                          (rand())                    ///< get random value
#define SRAND(S)                        (srand(S))          ///< set random seed value

#define writeb(data, addr) (*((volatile uint8_t*)(addr)) = (uint8_t)data)
#define writew(data, addr) (*((volatile uint16_t*)(addr)) = (uint16_t)data)
#define writel(data, addr) (*((volatile uint32_t*)(addr)) = (uint32_t)data)
#define writeq(data, addr) (*((volatile uint64_t*)(addr)) = (uint64_t)data)

#define readb(addr) (*((volatile uint8_t*)(addr)))
#define readw(addr) (*((volatile uint16_t*)(addr)))
#define readl(addr) (*((volatile uint32_t*)(addr)))
#define readq(addr) (*((volatile uint64_t*)(addr)))

/**
 *  Swaps the endianness of the argument
 *
 *  @param    Arg       [in ] The argument that needs to be swapped
 *  @return   Nothing
 *
 */
#define SwapEndianness(Arg)                  \
    if (sizeof(Arg) == 4)                    \
    {                                        \
        Arg = __builtin_bswap32(Arg);        \
    }                                        \
    else if (sizeof(Arg) == 2)               \
    {                                        \
        Arg = (((Arg) << 8) | ((Arg) >> 8)); \
    }

/**
 *  returns a 32 bit value with endianness swapped
 *
 *  @param[in] value        Value
 *
 *  @returns  swapped endianness
 **/
__inline static uint32_t SwapEndianness32Bit(uint32_t value)
{
    return __builtin_bswap32(value);
}

/**
 *  returns a 16 bit value with endianess swapped
 *
 *  @param[in] value        Value
 *
 *  @returns  swapped endianness
 **/
__inline static uint16_t SwapEndianness16Bit(uint16_t value)
{
    return ((uint16_t)((value) << 8) | ((value) >> 8));
}

/**
 *   copies one ddword between memories
 *
 *    @param[out] pDest   destination buffer pointer
 *    @param[in]  pSrc    source buffer pointer
 **/
inline void MemCopy1U64(uint64_t* pDest, uint64_t* pSrc)
{
    *pDest = *pSrc;
}

//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------

typedef enum _MemFillPatternType_t
{
    cMemDataMode            = 0,
    cMemByteMode,
    cMemWordMode,
    cMemDwordMode,
} MemFillPatternType_t;

#pragma pack(push)
#pragma pack(1)
typedef union MemFillModes_t
{
    uint16_t all;
    struct
    {
        MemFillPatternType_t    pattern       : 2;      ///< use specified fill pattern
        uint16_t                incremental   : 1;      ///< use incremental fill pattern
        uint16_t                random        : 1;      ///< use random fill pattern
        uint16_t                tag           : 1;      ///< add the specified tag at begin, middle and end of the fill area
        uint16_t                unused        : 11;     ///< unused
    } b;
} MemFillModes_t;
#pragma pack(pop)
COMPILE_ASSERT(sizeof(MemFillModes_t) == sizeof(uint16_t), "Invalid MemFillModes_t size");

//-----------------------------------------------------------------------------
//  Exported variable reference
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Exported function reference
//-----------------------------------------------------------------------------

uint32_t  MemReadU8(uint32_t address);
uint32_t  MemReadU16(uint32_t address);
uint32_t  MemReadU32(uint32_t address);
void MemWriteU8(uint32_t address, uint32_t data);
void MemWriteU16(uint32_t address, uint32_t data);
void MemWriteU32(uint32_t address, uint32_t data);
void* MemFillU32(void* pDest, uint32_t data, uint32_t byteSize) ATTR_NAKED;
void* MemFillU32Inc(void* pDest, uint32_t data, uint32_t byteSize, uint32_t delta) ATTR_NAKED;
uint32_t MemSumU8(const uint8_t* pBuffer, uint32_t byteSize);
void MemSwapU16(uint16_t* pDestData, const uint16_t* pSrcData, uint32_t wordSize);
void MemSwapCopy(void* pDest, const void* pSrc, uint32_t byteSize);
void MemCopyU8(uint8_t* pDest, const uint8_t* pSrc, uint32_t byteSize);
void MemCopyU16(uint16_t* pDest, const uint16_t* pSrc, uint32_t byteSize);
void MemCopyU32(uint32_t* pDest, const uint32_t* pSrc, uint32_t byteSize);
void MemCopyU64(uint64_t* pDest, const uint64_t* pSrc, uint32_t byteSize);
void* MemCopy3U32(uint32_t* pDest, uint32_t* pSrc) ATTR_NAKED;
void* MemCopy4U32(uint32_t* pDest, uint32_t* pSrc) ATTR_NAKED;
void* MemCopy5U32(uint32_t* pDest, uint32_t* pSrc) ATTR_NAKED;
void* MemCopy6U32(uint32_t* pDest, uint32_t* pSrc) ATTR_NAKED;
void* MemCopy7U32(uint32_t* pDest, uint32_t* pSrc) ATTR_NAKED;
void* MemCopy8U32(uint32_t* pDest, uint32_t* pSrc) ATTR_NAKED;
void* MemCopy9U32(uint32_t* pDest, uint32_t* pSrc) ATTR_NAKED;
void* MemCopy10U32(uint32_t* pDest, uint32_t* pSrc) ATTR_NAKED;
void* MemCopy12U32(uint32_t* pDest, uint32_t* pSrc) ATTR_NAKED;
void* MemCopy16U32(uint32_t* pDest, uint32_t* pSrc) ATTR_NAKED;

/**
 *  Compares two memory areas
 *
 *  @param[in]  pArea1               first area to compare
 *  @param[in]  pArea2               second area to compare
 *  @param[in]  byteSize             byte size of buffer (shall be multiple of 4)
 *  @param[in]  dumpError            true: print error info when compare error, false: do not print error info
 *  @param[in]  exitAfterFirstError  true: exit after first miscompare, false: continue to check after first miscompare
 *  @return     true if areas are equal, false otherwise
 *
 */
bool MemCompareU32InSize(uint32_t* pArea1, uint32_t* pArea2, uint32_t byteSize, bool dumpError, bool exitAfterFirstError);

/**
 *  Compares a memory area with a pattern.
 *
 *  @param[in] pArea      area to compare
 *  @param[in] pattern    pattern to compare with
 *  @param[in] byteSize   byte size of buffer
 *  @return    true if areas matches pattern, false otherwise
 */
bool MemCompareU8WithPattern(const uint8_t* pArea, uint8_t pattern, uint32_t byteSize);

/**
 *  Fills a memory area with a pattern.
 *
 *  @param pArea        memory area to be filled
 *  @param byteCount    area size in number of bytes
 *  @param modes        fill pattern mode(s)
 *  @param pattern      pattern value
 *  @param tag          tag value
 */
void MemFillPattern(void* pArea, uint32_t byteCount, MemFillModes_t mode, uint32_t pattern, uint32_t tag);

/**
 *  Fills a memory area with a random pattern.
 *
 *  @param pArea        memory area to be filled
 */
void MemFillRandomData(void* pArea, uint32_t byteCount);

#ifdef __cplusplus
}
#endif
