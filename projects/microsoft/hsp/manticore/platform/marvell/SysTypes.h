//-----------------------------------------------------------------------------
//
// Copyright (c) 2022 Marvell. All rights reserved.
// The following file is subject to the limited use license agreement by and
// between Marvell and you, your employer or other entity on behalf of whom
// you act. In the absence of such license agreement the following file is
// subject to Marvell's standard Limited Use License Agreement.
//
//-----------------------------------------------------------------------------

//=============================================================================
//!
//! @brief <b> System primitive data type, macro and constant definitions </b>
//!
//! DO NOT USE ANY CONDITIONAL COMPILE OPTION !!
//!
//=============================================================================

#pragma once

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------
#define NONE        (0)                     ///< NONE, numeric value is 0

#define FALSE       (0)                     ///< FALSE, numeric value is 0 
#define TRUE        (1)                     ///< TRUE,  numeric value is 1

#define OFF         (0)                     ///< OFF, numeric value is 0 
#define ON          (1)                     ///<  ON, numeric value is 1 

#define DISABLE     (0)                     ///< Disable
#define ENABLE      (1)                     ///< Enable

#define B_0         0x00000001              ///< Bit value definition
#define B_1         0x00000002
#define B_2         0x00000004
#define B_3         0x00000008
#define B_4         0x00000010
#define B_5         0x00000020
#define B_6         0x00000040
#define B_7         0x00000080
#define B_8         0x00000100
#define B_9         0x00000200
#define B_10        0x00000400
#define B_11        0x00000800
#define B_12        0x00001000
#define B_13        0x00002000
#define B_14        0x00004000
#define B_15        0x00008000
#define B_16        0x00010000
#define B_17        0x00020000
#define B_18        0x00040000
#define B_19        0x00080000
#define B_20        0x00100000
#define B_21        0x00200000
#define B_22        0x00400000
#define B_23        0x00800000
#define B_24        0x01000000
#define B_25        0x02000000
#define B_26        0x04000000
#define B_27        0x08000000
#define B_28        0x10000000
#define B_29        0x20000000
#define B_30        0x40000000
#define B_31        0x80000000

#define U8_MAX      (0xFF)                  ///< max value for U8  data type
#define U16_MAX     (0xFFFF)                ///< max value for U16 data type
#define U32_MAX     (0xFFFFFFFF)            ///< max value for U32 data type
#define U64_MAX     (0xFFFFFFFFFFFFFFFFLL)  ///< max value for U64 data type

#define KBYTE       (1<<10)                 ///< Kilo-bytes 1024
#define MBYTE       (1<<20)                 ///< mega-bytes 1048576
#define GBYTE       (1<<30)                 ///< giga-bytes 1073741824

#define KHZ         (1000)                  ///< kilo-hertz
#define MHZ         (1000000)               ///< mega-hertz
#define GHZ         (1000000000)            ///< giga-hertz

#ifndef NULL
#define NULL        ((void *)0)             ///< NULL pointer 
#endif

#define U8_ALL_BIT_SET     (0xFF)                  ///< all bits are set.
#define U16_ALL_BIT_SET    (0xFFFF)                ///< all bits are set.
#define U32_ALL_BIT_SET    (0xFFFFFFFF)            ///< all bits are set.
#define U64_ALL_BIT_SET    (0xFFFFFFFFFFFFFFFFLL)  ///< all bits are set.

//-----------------------------------------------------------------------------
//  Macros Definitions
//-----------------------------------------------------------------------------
#define MEM_U8(addr)            ( *( U8* ) (addr) )     ///< memory I/O for U8             
#define MEM_S8(addr)            ( *( S8* ) (addr) )     ///< memory I/O for S8        
#define MEM_U16(addr)           ( *( U16* ) (addr) )    ///< memory I/O for U16
#define MEM_S16(addr)           ( *( S16* ) (addr) )    ///< memory I/O for S16
#define MEM_U32(addr)           ( *( U32* ) (addr) )    ///< memory I/O for U32
#define MEM_S32(addr)           ( *( S32* ) (addr) )    ///< memory I/O for S32
#define MEM_U64(addr)           ( *( U64* ) (addr) )    ///< memory I/O for U64
#define MEM_S64(addr)           ( *( S64* ) (addr) )    ///< memory I/O for S64

#define REG_U8(addr)            ( *( volatile U8* ) (addr) )    ///< memory mapped register I/O for U8
#define REG_S8(addr)            ( *( volatile S8* ) (addr) )    ///< memory mapped register I/O for S8
#define REG_U16(addr)           ( *( volatile U16* ) (addr) )   ///< memory mapped register I/O for U16
#define REG_S16(addr)           ( *( volatile S16* ) (addr) )   ///< memory mapped register I/O for S16
#define REG_U32(addr)           ( *( volatile U32* ) (addr) )   ///< memory mapped register I/O for U32
#define REG_S32(addr)           ( *( volatile S32* ) (addr) )   ///< memory mapped register I/O for S32
#define REG_U64(addr)           ( *( volatile U64* ) (addr) )   ///< memory mapped register I/O for U64
#define REG_S64(addr)           ( *( volatile S64* ) (addr) )   ///< memory mapped register I/O for S64

#define REG_U8_FW(addr,v,mask)  (REG_U8(addr) = ((REG_U8(addr)&(~(mask)))|v))   ///< bit field write
#define REG_U16_FW(addr,v,mask) (REG_U16(addr) = ((REG_U16(addr)&(~(mask)))|v)) ///< bit field write
#define REG_U32_FW(addr,v,mask) (REG_U32(addr) = ((REG_U32(addr)&(~(mask)))|v)) ///< bit field write

#define PACKED_U16(addr)        (((tPACKED_U16 *)addr)->word)   ///< Unaligned 16bit data I/O
#define PACKED_U32(addr)        (((tPACKED_U32 *)addr)->dword)  ///< unaligned 32bit data I/O

#define TO_U8(data)             ((U8)((data) & U8_MAX))         ///< convert to U8 type
#define TO_U16(data)            ((U16)((data) & U16_MAX))       ///< convert to U16 type
#define TO_U32(data)            ((U32)((data) & U32_MAX))       ///< convert to U32 type
#define TO_U64(data)            ((U64)(data))                   ///< convert to U64 type

#define SIZEOF_MEMBER(s,m)      sizeof(((s *)0)->m)             ///< size of the member in structure

#define STR_PTR_FROM_MEMBER(addr,s,m) ((s *)((U8 *)(addr) - offsetof(s,m))) 

#define MAKE_U16(HIGH,LOW)      (((U16)(HIGH)<<8)|(LOW))        ///< Make U16 with two U8  HIGH and LOW
#define MAKE_U32(HIGH,LOW)      (((U32)(HIGH)<<16)|(LOW))       ///< Make U32 with two U16 HIGH and LOW
#define MAKE_U64(HIGH,LOW)      (((U64)(HIGH)<<32)|(LOW))       ///< Make U64 with two U32 HIGH and LOW

#define CHAR4_TO_U32(A,B,C,D)   ((A)|((B)<<8)|((C)<<16)|((D)<<24))

#define TEST_U16_ALIGNED(X)     (((U32)(X)&1)==0)               ///< Test U16(word) aligned
#define TEST_U32_ALIGNED(X)     (((U32)(X)&3)==0)               ///< Test U32(dword) aligned

#define ALIGN_FLOOR(X,A)        ((U32)(X)&(~(A-1)))             ///< the largest aligned value not greater than X
#define ALIGN_CEIL(X,A)         ALIGN_FLOOR((X)+(A-1),A)        ///< the smallest aligned value not less than x

#define BIT(pos)                (1 << (pos))                    ///< Convert bit position to value

#define BIT_SET(var, bits)      (var |= (bits))                 ///< Bits set
#define BIT_CLR(var, bits)      (var &= (~(bits)))              ///< Bits clear
#define BIT_XOR(var, bits)      (var ^= (bits))                 ///< Bits XOR
#define BIT_FW(var,v,mask)      (var = (var&(~(mask)))|v)       ///< Bits field write

#define MAX(x,y)                ((x)>(y) ? (x) : (y))           ///< maximum value of two
#define MIN(x,y)                ((x)<(y) ? (x) : (y))           ///< minimum value of two 

#define LOG2(X)                 (31-__clz(X))                   ///< Log 2

#define ARRAY_INDEX_FROM_PTR(A, P) (((U32)(P)-(U32)(A))/sizeof(A[0]))  ///< compute array index from pointer

#define COMPILE_ASSERT(exp,str) extern char __ct_[(exp) ? 1 : -1] ///< compile time assert

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------
typedef unsigned int        BOOL;   ///< TRUE, FALSE or ENABLE, DISABLE or ON, OFF

typedef unsigned char       U8;     ///< unsigned  8 bit integer
typedef unsigned short      U16;    ///< unsigned 16 bit integer
typedef unsigned int        U32;    ///< unsigned 32 bit integer
/*
typedef unsigned char       uint8_t;     ///< unsigned  8 bit integer
typedef unsigned short      uint16_t;    ///< unsigned 16 bit integer
typedef unsigned int        uint32_t;    ///< unsigned 32 bit integer
*/
typedef unsigned long long  U64;    ///< unsigned 64 bit integer, 8 byte data 

typedef signed char         S8;     ///< signed  8 bit integer 
typedef signed short        S16;    ///< signed 16 bit integer 
typedef signed int          S32;    ///< signed 32 bit integer 
typedef signed long long    S64;    ///< signed 64 bit integer 

typedef float               F32;    ///< 32 bit floating point number
typedef double              F64;    ///< 64 bit floating point number

typedef void*               PVOID;  ///< void pointer

typedef const char*         CSTR;   ///< constant string pointer

/// @brief packed structure to access unaligned word(16bit) data
typedef union __attribute__ ((packed))
{
    U16 word;
    struct __attribute__ ((packed))
    {
        U8 low;
        U8 high;
    } byte;
} tPACKED_U16;

/// @brief packed structure to access unaligned dword(32bit) data
typedef union __attribute__ ((packed))
{
    U32 dword;
    struct __attribute__ ((packed))
    {
        U16 low;
        U16 high;
    } word;
    struct __attribute__ ((packed))
    {
        U8 b0;
        U8 b1;
        U8 b2;
        U8 b3;
    } byte;
} tPACKED_U32;

/// @brief union data structure for aligned word(16bit) data
typedef union
{
    U16 word;
    struct
    {
        U8 low;
        U8 high;
    } byte;
} tUNION_U16;

/// @brief union data structure for aligned dword(32bit) data
typedef union
{
    U32 dword;
    struct
    {
        U16 low;
        U16 high;
    } word;
    struct
    {
        U8 b0;
        U8 b1;
        U8 b2;
        U8 b3;
    } byte;
} tUNION_U32;

/// @brief Doubly Linked List
typedef struct 
{ 
    PVOID prev;     ///< prevoius pointer (tail pointer)
    PVOID next;     ///< next pointer (head pointer)
} tDLINK_ENTRY;

/// @brief Singly Linked List
typedef struct
{
    PVOID next;     ///< next pointer
} tSLINK_ENTRY;

/// @brief List head and tail 
typedef struct
{ 
    PVOID tail;     ///< tail pointer
    PVOID head;     ///< head pointer
} tLINK_HEAD_TAIL;
