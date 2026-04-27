// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

/***********************************************************************************
 *
 * @file SysTypes.h
 *
 * System primitive data type, macro and constant definitions
 *
 ***********************************************************************************/
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************************
 *                                                                                  *
 * Dependencies                                                                     *
 *                                                                                  *
 ***********************************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "Attr.h"

/***********************************************************************************
 *                                                                                  *
 * Constant Definitions                                                             *
 *                                                                                  *
 ***********************************************************************************/

/***********************************************************************************
 *                                                                                  *
 * Macros Definitions                                                               *
 *                                                                                  *
 ***********************************************************************************/
/// @brief  Null pointer constant
#ifndef NULL
#define NULL                    ((void*)0)              ///< NULL pointer constant
#endif

/// @brief  Generic macro TRUE and FALSE for bool type
#undef TRUE
#define TRUE                    (1)                     ///< TRUE, numeric value is 1
#undef FALSE
#define FALSE                   (0)                     ///< FALSE, numeric value is 0

/// @brief  Generic macro ENABLE and DISABLE for bool type
#undef ENABLE
#define ENABLE                  (1)                     ///< ENABLE, numeric value is 1
#undef DISABLE
#define DISABLE                 (0)                     ///< DISABLE, numeric value is 0

/// @brief  Generic macro ON and OFF for bool type
#define ON                      (1)                     ///< ON, numeric value is 1
#define OFF                     (0)                     ///< OFF, numeric value is 0

/// @brief  Maximum values of unsigned integer types
#define U8_MAX                  (0xFF)                  ///< Max value for uint8_t  data type
#define U16_MAX                 (0xFFFF)                ///< Max value for uint16_t data type
#define U32_MAX                 (0xFFFFFFFF)            ///< Max value for uint32_t data type
#define U64_MAX                 (0xFFFFFFFFFFFFFFFFLL)  ///< Max value for uint64_t data type

/// @brief  Generic data units
#define KBYTE                   (1UL << 10)             ///< Kilo-bytes 1024
#define MBYTE                   (1UL << 20)             ///< Mega-bytes 1048576
#define GBYTE                   (1UL << 30)             ///< Giga-bytes 1073741824

/// @brief  Generic clock units
#define KHZ                     (1000)                  ///< Kilo-hertz
#define MHZ                     (1000000)               ///< Mega-hertz
#define GHZ                     (1000000000)            ///< Giga-hertz

/// @brief  Gets MIN and MAX
#define MAX(x, y)               ((x) > (y) ? (x) : (y))   ///< Maximum of two value
#define MIN(x, y)               ((x) < (y) ? (x) : (y))   ///< Minimum of two value

/// @brief  Sets unsigned integer all bits
#define U8_ALL_BIT_SET          (0xFF)                  ///< All bits are set.
#define U16_ALL_BIT_SET         (0xFFFF)                ///< All bits are set.
#define U32_ALL_BIT_SET         (0xFFFFFFFF)            ///< All bits are set.
#define U64_ALL_BIT_SET         (0xFFFFFFFFFFFFFFFFLL)  ///< All bits are set.

/// @brief  Bit manipulation
#define B_0                     0x00000001              ///< Bit value definition
#define B_1                     0x00000002
#define B_2                     0x00000004
#define B_3                     0x00000008
#define B_4                     0x00000010
#define B_5                     0x00000020
#define B_6                     0x00000040
#define B_7                     0x00000080
#define B_8                     0x00000100
#define B_9                     0x00000200
#define B_10                    0x00000400
#define B_11                    0x00000800
#define B_12                    0x00001000
#define B_13                    0x00002000
#define B_14                    0x00004000
#define B_15                    0x00008000
#define B_16                    0x00010000
#define B_17                    0x00020000
#define B_18                    0x00040000
#define B_19                    0x00080000
#define B_20                    0x00100000
#define B_21                    0x00200000
#define B_22                    0x00400000
#define B_23                    0x00800000
#define B_24                    0x01000000
#define B_25                    0x02000000
#define B_26                    0x04000000
#define B_27                    0x08000000
#define B_28                    0x10000000
#define B_29                    0x20000000
#define B_30                    0x40000000
#define B_31                    0x80000000

#define BIT(pos)                (1U << ((uint32_t)(pos)))                       ///< Convert bit position to value
#define BITS(H, L)              ((BIT((H)+1) - 1) & ~(BIT((L)) - 1))            ///< Convert bit range to value
#define BITS_U32(H, L)          ((U32_MAX >> (31 - (H))) & (U32_MAX << (L)))    ///< Convert bit range to value for U32 (fast?)
#define BIT_ULL(pos)            (1ULL << (pos))                                 ///< Convert bit position to value for U64

#define MASK(bit)               ((1U << ((uint32_t)(bit))) - 1)                 ///< Convert a bit mask value

/// @brief  Convert to Base-36 macro
#define BASE36_0                "0"
#define BASE36_1                "1"
#define BASE36_2                "2"
#define BASE36_3                "3"
#define BASE36_4                "4"
#define BASE36_5                "5"
#define BASE36_6                "6"
#define BASE36_7                "7"
#define BASE36_8                "8"
#define BASE36_9                "9"
#define BASE36_10               "A"
#define BASE36_11               "B"
#define BASE36_12               "C"
#define BASE36_13               "D"
#define BASE36_14               "E"
#define BASE36_15               "F"
#define BASE36_16               "G"
#define BASE36_17               "H"
#define BASE36_18               "I"
#define BASE36_19               "J"
#define BASE36_20               "K"
#define BASE36_21               "L"
#define BASE36_22               "M"
#define BASE36_23               "N"
#define BASE36_24               "O"
#define BASE36_25               "P"
#define BASE36_26               "Q"
#define BASE36_27               "R"
#define BASE36_28               "S"
#define BASE36_29               "T"
#define BASE36_30               "U"
#define BASE36_31               "V"
#define BASE36_32               "W"
#define BASE36_33               "X"
#define BASE36_34               "Y"
#define BASE36_35               "Z"

#define _BASE36_ENCODE(N)       BASE36_ ## N
#define BASE36_ENCODE(N)        _BASE36_ENCODE(N)

/**
 * The REINTERPRET_CAST macro changes one data type into another.
 * It should be used to cast between incompatible pointer types.
 *
 *  @param[in]  Type    Pointer type
 *  @param[in]  Exp     Expression to be casted.
 */
#define REINTERPRET_CAST(Type, Exp)         (Type)(void*)(Exp)

/// @brief  Macro realted memory access
#define MEM_U8(addr)                        (*(REINTERPRET_CAST(uint8_t*, addr)))           ///< Memory I/O for uint8_t
#define MEM_U16(addr)                       (*(REINTERPRET_CAST(uint16_t*, addr)))          ///< Memory I/O for uint16_t
#define MEM_U32(addr)                       (*(REINTERPRET_CAST(uint32_t*, addr)))          ///< Memory I/O for uint32_t
#define MEM_U64(addr)                       (*(REINTERPRET_CAST(uint64_t*, addr)))          ///< Memory I/O for uint64_t

#define MEM_S8(addr)                        (*(REINTERPRET_CAST(int8_t*,  addr)))           ///< Memory I/O for int8_t
#define MEM_S16(addr)                       (*(REINTERPRET_CAST(int16_t*, addr)))           ///< Memory I/O for int16_t
#define MEM_S32(addr)                       (*(REINTERPRET_CAST(int32_t*, addr)))           ///< Memory I/O for int32_t
#define MEM_S64(addr)                       (*(REINTERPRET_CAST(int64_t*, addr)))           ///< Memory I/O for int64_t

/// @brief  Macro realted register access
#define REG_U8(addr)                        (*(REINTERPRET_CAST(volatile uint8_t*, addr)))  ///< Register I/O for uint8_t
#define REG_U16(addr)                       (*(REINTERPRET_CAST(volatile uint16_t*, addr))) ///< Register I/O for uint16_t
#define REG_U32(addr)                       (*(REINTERPRET_CAST(volatile uint32_t*, addr))) ///< Register I/O for uint32_t
#define REG_U64(addr)                       (*(REINTERPRET_CAST(volatile uint64_t*, addr))) ///< Register I/O for uint64_t

#define REG_S8(addr)                        (*(REINTERPRET_CAST(volatile int8_t*, addr)))   ///< Register I/O for int8_t
#define REG_S16(addr)                       (*(REINTERPRET_CAST(volatile int16_t*, addr)))  ///< Register I/O for int16_t
#define REG_S32(addr)                       (*(REINTERPRET_CAST(volatile int32_t*, addr)))  ///< Register I/O for int32_t
#define REG_S64(addr)                       (*(REINTERPRET_CAST(volatile int64_t*, addr)))  ///< Register I/O for int64_t

/// @brief  Bit field write with mask
#define REG_U8_FW(addr, v, mask)            (REG_U8(addr) = (uint8_t)((REG_U8(addr) & (~(mask))) | (v)))
#define REG_U16_FW(addr, v, mask)           (REG_U16(addr) = (uint16_t)((REG_U16(addr) & (~(mask))) | (v)))
#define REG_U32_FW(addr, v, mask)           (REG_U32(addr) = ((REG_U32(addr) & (~(mask))) | (v)))

/// @brief  Bit field write with bit positions, ADDR[bp1:bp0] = value
#define REG_U8_BW(addr, bp1, bp0, value)    REG_U8_FW((addr), ((value) << (bp0)), (((0xFFU) >> (7 - (bp1))) & ((0xFFU) << (bp0))))
#define REG_U16_BW(addr, bp1, bp0, value)   REG_U16_FW((addr), ((value) << (bp0)), (((0xFFFFU) > (15 - (bp1))) & ((0xFFFFU) << (bp0))))
#define REG_U32_BW(addr, bp1, bp0, value)   REG_U32_FW((addr), ((value) << (bp0)), (((0xFFFFFFFFU) >> (31 - (bp1))) & ((0xFFFFFFFFU) << (bp0))))

/// @brief Make integer
#define MAKE_U16(HIGH, LOW)                 (((uint16_t)(HIGH) << 8) | (LOW))               ///< Make uint16_t with two uint8_t HIGH and LOW
#define MAKE_U32(HIGH, LOW)                 (((uint32_t)(HIGH) << 16) | (LOW))              ///< Make uint32_t with two uint16_t HIGH and LOW
#define MAKE_U64(HIGH, LOW)                 (((uint64_t)(HIGH) << 32) | (LOW))              ///< Make uint64_t with two uint32_t HIGH and LOW
#define MAKE_U32_4(hi, m1, m0, lo)          MAKE_U32(MAKE_U16(hi, m1), MAKE_U16(m0, lo))    ///< Make uint32_t from 4 byte values

/// @brief Returns the bits of a integer
#define GET_B03_B00(X)                      ((uint8_t)((X) & 0xF))
#define GET_B07_B04(X)                      ((uint8_t)((X >> 4) & 0xF))
#define GET_B11_B08(X)                      ((uint8_t)((X >> 8) & 0xF))
#define GET_B15_B12(X)                      ((uint8_t)((X >> 12) & 0xF))
#define GET_B19_B16(X)                      ((uint8_t)((X >> 16) & 0xF))
#define GET_B23_B20(X)                      ((uint8_t)((X >> 20) & 0xF))
#define GET_B27_B24(X)                      ((uint8_t)((X >> 24) & 0xF))
#define GET_B31_B28(X)                      ((uint8_t)((X >> 28) & 0xF))

#define GET_B07_00(X)                       ((uint8_t)(X))
#define GET_B15_08(X)                       ((uint8_t)((X) >> 8))
#define GET_B23_16(X)                       ((uint8_t)((X) >> 16))
#define GET_B31_24(X)                       ((uint8_t)((X) >> 24))

#define GET_B15_00(X)                       ((uint16_t)(X))
#define GET_B23_08(X)                       ((uint16_t)((X) >> 8))
#define GET_B31_16(X)                       ((uint16_t)((X) >> 16))

#define GET_B31_00(X)                       ((uint32_t)(X))
#define GET_B63_32(X)                       ((uint32_t)((X) >> 32))

#define LOW8(X)                             GET_B07_00(X)
#define LOW16(X)                            GET_B15_00(X)
#define LOW32(X)                            GET_B31_00(X)
#define HIGH8(X)                            GET_B15_08(X)
#define HIGH16(X)                           GET_B31_16(X)
#define HIGH32(X)                           GET_B63_32(X)

#define GET_BIT(X, BP)                      (((X) >> (BP)) & 1)
#define GET_BITS(X, H, L)                   (((X) >> (L)) & (BIT((H - L) + 1) - 1))
#define GET_BITS_RANGE(X, H, L)             ((X)&BITS(H, L))

#define BIT_SET(var, bits)                  ((var) |= (bits))                     ///< Bits set
#define BIT_CLR(var, bits)                  ((var) &= (~(bits)))                  ///< Bits clear
#define BIT_XOR(var, bits)                  ((var) ^= (bits))                     ///< Bits XOR
#define BIT_FW(var, v, mask)                  ((var) = ((var) & (~(mask))) | (v))       ///< Bits field write

/// @brief  Return the value in var mask and shifted as indicated
#define BIT_FIELD_RD(var, mask, shift)      (((var) & (mask)) >> (shift))
/// @brief  Return the value in var with the bits in v shifted and masked to be combined into return value
#define BIT_FIELD_WR(var, v, mask, shift)   (((var) & ~(mask)) | (((v) << (shift)) & (mask)))

/// @brief  Set or clear one bit in bitmap array type uint8_t, uint16_t, uint32_t (LSB is bit position 0)
#define BA_SET_U8_L2M(BUFP, BITP)           ((BUFP)[(BITP) / 8] |= ((uint8_t)1 << ((BITP) % 8)))
#define BA_CLR_U8_L2M(BUFP, BITP)           ((BUFP)[(BITP) / 8] &= (~((uint8_t)1 << ((BITP) % 8))))
#define BA_SET_U16_L2M(BUFP, BITP)          ((BUFP)[(BITP) / 16] |= ((uint16_t)1 << ((BITP) % 16)))
#define BA_CLR_U16_L2M(BUFP, BITP)          ((BUFP)[(BITP) / 16] &= (~((uint16_t)1 << ((BITP) % 16))))
#define BA_SET_U32_L2M(BUFP, BITP)          ((BUFP)[(BITP) / 32] |= ((uint32_t)1 << ((BITP) % 32)))
#define BA_CLR_U32_L2M(BUFP, BITP)          ((BUFP)[(BITP) / 32] &= (~((uint32_t)1 << ((BITP) % 32))))
#define BA_WR1_U32_L2M(BUFP, BITP)          ((BUFP)[(BITP) / 32] = ((uint32_t)1 << ((BITP) % 32)))

/// @brief  Test one bit in bitmap array type uint8_t, uint16_t, uint32_t (LSB is bit position 0)
#define BA_TESTB_U8_L2M(BUFP, BITP)         ((BUFP)[(BITP) / 8] & ((uint8_t)1 << ((BITP) % 8)))
#define BA_TESTB_U16_L2M(BUFP, BITP)        ((BUFP)[(BITP) / 16] & ((uint16_t)1 << ((BITP) % 16)))
#define BA_TESTB_U32_L2M(BUFP, BITP)        ((BUFP)[(BITP) / 32] & ((uint32_t)1 << ((BITP) % 32)))

/// @brief  Set or clear one bit in bitmap array type uint8_t, uint16_t, uint32_t (MSB is bit position 0)
#define BA_SET_U8_M2L(BUFP, BITP)           ((BUFP)[(BITP) / 8] |= (0x80 >> (BITP) % 8))
#define BA_CLR_U8_M2L(BUFP, BITP)           ((BUFP)[(BITP) / 8] &= (~(0x80 >> (BITP) % 8)))
#define BA_SET_U16_M2L(BUFP, BITP)          ((BUFP)[(BITP) / 16] |= (0x8000 >> (BITP) % 16))
#define BA_CLR_U16_M2L(BUFP, BITP)          ((BUFP)[(BITP) / 16] &= (~(0x8000 >> (BITP) % 16)))
#define BA_SET_U32_M2L(BUFP, BITP)          ((BUFP)[(BITP) / 32] |= (0x80000000 >> (BITP) % 32))
#define BA_CLR_U32_M2L(BUFP, BITP)          ((BUFP)[(BITP) / 32] &= (~(0x80000000 >> (BITP) % 32)))
#define BA_WR1_U32_M2L(BUFP, BITP)          ((BUFP)[(BITP) / 32] = (0x80000000 >> (BITP) % 32))

/// @brief  Test bit one bit in bitmap array type uint8_t, uint16_t, uint32_t (MSB is bit position 0)
#define BA_TESTB_U8_M2L(BUFP, BITP)         ((BUFP)[(BITP) / 8] & (0x80 >> (BITP) % 8))
#define BA_TESTB_U16_M2L(BUFP, BITP)        ((BUFP)[(BITP) / 16] & (0x8000 >> (BITP) % 16))
#define BA_TESTB_U32_M2L(BUFP, BITP)        ((BUFP)[(BITP) / 32] & (0x80000000 >> (BITP) % 32))

/// @brief  Return bitmap array size of uint32_t type
#define BITMAP_ARRAY_SIZE_U32(bitCount)     (((bitCount) - 1) / 32 + 1)

/// @brief  MACRO related aligned/unaligned
#define PACKED_U16(addr)                    (((PackedU16_t*)addr)->word)            ///< Unaligned 16bit data I/O
#define PACKED_U32(addr)                    (((PackedU32_t*)addr)->dword)           ///< unaligned 32bit data I/O

#define TEST_U16_ALIGNED(X)                 (((uint32_t)(X) & 1) == 0)              ///< Test uint16_t(word) aligned
#define TEST_U32_ALIGNED(X)                 (((uint32_t)(X) & 3) == 0)              ///< Test uint32_t(dword) aligned

#define TEST_ALIGNED(X, A)                  (((uint32_t)(X)&(A - 1)) == 0)          ///< Test A size aligned (A shall be power of 2)
#define TEST_IS_POWER_OF2(X)                (((uint32_t)(X)&((X)-1)) == 0)          ///< Test X is power of 2

#define ALIGN_FLOOR(X, A)                   ((uint32_t)(X)&(~(A - 1)))              ///< Largest aligned value that is not greater than X
#define ALIGN_FLOOR_U64(X, A)               ((uint64_t)(X)&(~(A - 1)))              ///< Largest aligned value that is not greater than X

#define ALIGN_CEIL(X, A)                    ALIGN_FLOOR((X) + (A - 1), A)             ///< Smallest aligned value that is not less than x
#define ALIGN_CEIL_U64(X, A)                ALIGN_FLOOR_U64((X) + (A - 1), A)         ///< Smallest aligned value that is not less than x

#define CONVERT_UNIT_FLOOR(X, U)            ((X) / (U))                             ///< unit conversion, largest value that is not greater than X
#define CONVERT_UNIT_CEIL(X, U)             (((X) + (U)-1) / (U))                   ///< unit conversion, smallest value that is not less than X
#define CONVERT_UNIT_CEIL_U64(X, U)         (((uint64_t)(X) + (U)-1) / (U))         ///< unit conversion, smallest value that is not less than X

/// @brief  MACRO related CLZ (Count Leading Zeros) and FFS (Find First Set)
#define __FFS(W, X)                         ((W)-__clz((X)&(-(X))))                             ///< Find First Set using CLZ (Count Leading Zeros), (1 to N, 0 is invalid)
#define FFS(X)                              (__FFS(32, (X)) - 1)                                ///< Find First Set for uint32_t from LSB to MSB (0 to N-1, 0xFFFF is invalid)
#define FFS_RANGE(X, H, L)                  (FFS(GET_BITS_RANGE((X), (H), (L))))                  ///< Find First Set in bits range from LSB to MSB (0 to N-1, 0xFFFF is invalid)
#define FFS_RANGE_RVS(X, H, L)              (31 - __clz((X)&GET_BITS_RANGE((X), (H), (L))))     ///< Find First Set in bits range from MSB to LSB (N-1 to 0, 0xFFFF is invalid)

/// @brief  MACRO related structure size and offset
#define SIZEOF_MEMBER(s, m)                 sizeof(((s*)0)->m)                                  ///< size of the member in structure
#define STR_PTR_FROM_MEMBER(addr, s, m)     (REINTERPRET_CAST(s*, ((uint8_t*)(addr) - offsetof(s, m))))
#define ARRAY_INDEX_FROM_PTR(A, P)          (((uint32_t)(P)-(uint32_t)(A)) / sizeof(A[0]))      ///< compute array index from pointer

/// @brief  Compile assert check
#if defined (__cplusplus)
#define COMPILE_ASSERT(exp, str)            static_assert((exp), str)
#else
#define COMPILE_ASSERT(exp, str)            extern char __ct_[(exp) ? 1 : -1]
#endif

#define ASSERT_STRUCT_SIZE_MULTIPLE(type, size)                         COMPILE_ASSERT(sizeof(type) % size == 0, "Struct has incorrect size")
#define ASSERT_OFFSET_ALIGNMENT_OF_MEMBER(type, element, alignment)     COMPILE_ASSERT(offsetof(type, element) % alignment == 0, "Element within struct is not properly aligned")

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
/***********************************************************************************
 *                                                                                  *
 * Public Data Type Definitions                                                     *
 *                                                                                  *
 ***********************************************************************************/
///< @brief Constant string pointer
typedef const char* Cstr_t;

/// @brief  Packed structure to access unaligned word(16bit) data
typedef ATTR_UNALIGNED union
{
    uint16_t word;

    ATTR_UNALIGNED struct
    {
        uint8_t low;
        uint8_t high;
    } byte;
} PackedU16_t;

/// @brief  Packed structure to access unaligned dword(32bit) data
typedef ATTR_UNALIGNED union
{
    uint32_t dword;

    ATTR_UNALIGNED struct
    {
        uint16_t low;
        uint16_t high;
    } word;

    ATTR_UNALIGNED struct
    {
        uint8_t b0;
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
    } byte;
} PackedU32_t;

/// @brief  union data structure for aligned word(16bit) data
typedef union
{
    uint16_t word;

    struct
    {
        uint8_t low;
        uint8_t high;
    } byte;
} UnionU16_t;

/// @brief  union data structure for aligned dword(32bit) data
typedef union
{
    uint32_t dword;

    struct
    {
        uint16_t low;
        uint16_t high;
    } word;

    struct
    {
        uint8_t b0;
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
    } byte;
} UnionU32_t;

#ifdef __cplusplus
}
#endif
