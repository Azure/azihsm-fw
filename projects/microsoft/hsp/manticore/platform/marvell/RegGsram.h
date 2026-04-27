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
//! @brief GSRAM Registers
//!
//=============================================================================

// Generated with Dullahan v2.4.3.

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "SharedStruct.h"
#include "SysTypes.h"

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------


/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BLOCK_MEM_ECC_ENB           :8;      ///<BIT [7:0] Block_Mem_ECC_Enb
        uint32_t RSVD                        :24;     ///<BIT [31:8] RSVD_0
    } b;
} Memeccenb_t;

/// @brief 0x44
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MEMECCSET                   :8;      ///<BIT [7:0] MEMECCSET
        uint32_t RSVD                        :24;     ///<BIT [31:8] RSVD_0
    } b;
} Memeccset_t;

/// @brief 0x48
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BLOCK_MEMORY_ECC_ENB_CLR    :8;      ///<BIT [7:0] Block_Memory_ECC_Enb_Clr
        uint32_t RSVD                        :24;     ///<BIT [31:8] RSVD_0
    } b;
} Memeccclr_t;

/// @brief 0x4C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t NO_BLOCK_MEMORY             :3;      ///<BIT [2:0] No_Block_Memory
        uint32_t RSVD_2                      :5;      ///<BIT [7:3] RSVD_2
        uint32_t BLOCK_MEMORY_BASE_INDEX     :3;      ///<BIT [10:8] Block_Memory_Base_Index
        uint32_t RSVD_1                      :5;      ///<BIT [15:11] RSVD_1
        uint32_t BLOCK_MEMORY_INIT_START     :1;      ///<BIT [16] Block_Memory_Init_Start
        uint32_t RSVD                        :15;     ///<BIT [31:17] RSVD_0
    } b;
} Meminit_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BLOCK_MEM_SINGLE_BIT_ERROR  :1;      ///<BIT [0] Block_Mem_Single_Bit_Error
        uint32_t BLOCK_MEM_DOUBLE_BIT_ERROR  :1;      ///<BIT [1] Block_Mem_Double_Bit_Error
        uint32_t BLOCK_MEM_INIT_DONE         :1;      ///<BIT [2] Block_Mem_Init_Done
        uint32_t RSVD_3_6                    :4;      ///<BIT [6:3] rsvd_3_6
        uint32_t RSVD                        :25;     ///<BIT [31:7] RSVD_0
    } b;
} Memintstt_t;

/// @brief 0x54
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BLOCK_MEM_SINGLE_BIT_ERROR_INT_ENB :1;      ///<BIT [0] Block_Mem_Single_Bit_Error_Int_Enb
        uint32_t BLOCK_MEM_DOUBLE_BIT_ERROR_INT_ENB :1;      ///<BIT [1] Block_Mem_Double_Bit_Error_Int_Enb
        uint32_t BLOCK_MEM_INIT_DONE_INT_ENB :1;      ///<BIT [2] Block_Mem_Init_Done_Int_Enb
        uint32_t RSVD_3_6                    :4;      ///<BIT [6:3] rsvd_3_6
        uint32_t RSVD                        :25;     ///<BIT [31:7] RSVD_0
    } b;
} Memintenb_t;

/// @brief 0x58
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BLOCK_MEM_SINGLE_BIT_ERROR_INT_ENB_SET :1;      ///<BIT [0] Block_Mem_Single_Bit_Error_Int_Enb_Set
        uint32_t BLOCK_MEM_DOUBLE_BIT_ERROR_INT_ENB_SET :1;      ///<BIT [1] Block_Mem_Double_Bit_Error_Int_Enb_Set
        uint32_t BLOCK_MEM_INIT_DONE_INT_ENB_SET :1;      ///<BIT [2] Block_Mem_Init_Done_Int_Enb_Set
        uint32_t RSVD_3_6                    :4;      ///<BIT [6:3] rsvd_3_6
        uint32_t RSVD                        :25;     ///<BIT [31:7] RSVD_0
    } b;
} Memintset_t;

/// @brief 0x5C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BLOCK_MEM_SINGLE_BIT_ERROR_INT_ENB_CLR :1;      ///<BIT [0] Block_Mem_Single_Bit_Error_Int_Enb_Clr
        uint32_t BLOCK_MEM_DOUBLE_BIT_ERROR_INT_ENB_CLR :1;      ///<BIT [1] Block_Mem_Double_Bit_Error_Int_Enb_Clr
        uint32_t BLOCK_MEM_INIT_DONE_INT_ENB_CLR :1;      ///<BIT [2] Block_Mem_Init_Done_Int_Enb_Clr
        uint32_t RSVD_3_6                    :4;      ///<BIT [6:3] rsvd_3_6
        uint32_t RSVD                        :25;     ///<BIT [31:7] RSVD_0
    } b;
} Memintclr_t;

/// @brief 0x70
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      :3;      ///<BIT [2:0] RSVD_1
        uint32_t ERR_ADDR                    :18;     ///<BIT [20:3] Err_Addr
        uint32_t RSVD                        :11;     ///<BIT [31:21] RSVD_0
    } b;
} Errinjadr_t;

/// @brief 0x74
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t INJ_TRIG                    :1;      ///<BIT [0] Inj_Trig
        uint32_t RSVD                        :30;     ///<BIT [30:1] RSVD_0
        uint32_t INJ_DONE                    :1;      ///<BIT [31] Inj_Done
    } b;
} Errinjcts_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SINGLE_BIT_ERROR_CHANNEL    :1;      ///<BIT [0] Single_Bit_Error_Channel
        uint32_t RSVD_2                      :4;      ///<BIT [4:1] RSVD_2
        uint32_t SINGLE_BIT_ERROR_ADDRESS    :16;     ///<BIT [20:5] Single_Bit_Error_Address
        uint32_t RSVD_1                      :3;      ///<BIT [23:21] RSVD_1
        uint32_t SINGLE_BIT_ERROR_SEGMENT    :4;      ///<BIT [27:24] Single_Bit_Error_Segment
        uint32_t RSVD                        :4;      ///<BIT [31:28] RSVD_0
    } b;
} Errslog0_t;

/// @brief 0x84
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SINGLE_BIT_ERROR_SYNDROME_0 :7;      ///<BIT [6:0] Single_Bit_Error_Syndrome_0
        uint32_t RSVD_3                      :1;      ///<BIT [7] RSVD_3
        uint32_t SINGLE_BIT_ERROR_SYNDROME_1 :7;      ///<BIT [14:8] Single_Bit_Error_Syndrome_1
        uint32_t RSVD_2                      :1;      ///<BIT [15] RSVD_2
        uint32_t SINGLE_BIT_ERROR_SYNDROME_2 :7;      ///<BIT [22:16] Single_Bit_Error_Syndrome_2
        uint32_t RSVD_1                      :1;      ///<BIT [23] RSVD_1
        uint32_t SINGLE_BIT_ERROR_SYNDROME_3 :7;      ///<BIT [30:24] Single_Bit_Error_Syndrome_3
        uint32_t RSVD                        :1;      ///<BIT [31] RSVD_0
    } b;
} Errslog1_t;

/// @brief 0x90
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SINGLE_BIT_ERROR_COUNTER    :16;     ///<BIT [15:0] Single_Bit_Error_Counter
        uint32_t SINGLE_BIT_ERROR_HWID       :6;      ///<BIT [21:16] Single_Bit_Error_HWID
        uint32_t RSVD                        :9;      ///<BIT [30:22] RSVD_0
        uint32_t SINGLE_BIT_ERROR_LOG_VALID  :1;      ///<BIT [31] Single_Bit_Error_Log_Valid
    } b;
} Errssts_t;

/// @brief 0x94
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SINGLE_BIT_ERROR_THRESHOLD  :16;     ///<BIT [15:0] Single_Bit_Error_Threshold
        uint32_t RSVD                        :16;     ///<BIT [31:16] RSVD_0
    } b;
} Errsthr_t;

/// @brief 0xA0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DOUBLE_BIT_ERROR_CHANNEL    :1;      ///<BIT [0] Double_Bit_Error_Channel
        uint32_t RSVD_2                      :4;      ///<BIT [4:1] RSVD_2
        uint32_t DOUBLE_BIT_ERROR_ADDRESS    :16;     ///<BIT [20:5] Double_Bit_Error_Address
        uint32_t RSVD_1                      :3;      ///<BIT [23:21] RSVD_1
        uint32_t DOUBLE_BIT_ERROR_SEGMENT    :4;      ///<BIT [27:24] Double_Bit_Error_Segment
        uint32_t RSVD                        :4;      ///<BIT [31:28] RSVD_0
    } b;
} Errdlog0_t;

/// @brief 0xB0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DOUBLE_BIT_ERROR_COUNTER    :16;     ///<BIT [15:0] Double_Bit_Error_Counter
        uint32_t DOUBLE_BIT_ERROR_HWID       :6;      ///<BIT [21:16] Double_Bit_Error_HWID
        uint32_t RSVD                        :9;      ///<BIT [30:22] RSVD_0
        uint32_t DOUBLE_BIT_ERROR_LOG_VALID  :1;      ///<BIT [31] Double_Bit_Error_Log_Valid
    } b;
} Errdsts_t;

/// @brief 0xB4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t DOUBLE_BIT_ERROR_THRESHOLD  :16;     ///<BIT [15:0] Double_Bit_Error_Threshold
        uint32_t RSVD                        :16;     ///<BIT [31:16] RSVD_0
    } b;
} Errdthr_t;

/// @brief 0xC0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BLOCK_MEMORY_ARB_MODE       :4;      ///<BIT [3:0] Block_Memory_Arb_Mode
        uint32_t RSVD                        :28;     ///<BIT [31:4] RSVD_0
    } b;
} Memarbctl0_t;

/// @brief 0xD0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PARITY_ERROR_CHECK_ENABLE   :1;      ///<BIT [0] Parity_Error_Check_Enable
        uint32_t PARITY_ERROR_GENERATE_ENABLE :1;      ///<BIT [1] Parity_Error_Generate_Enable
        uint32_t PARITY_ERROR_READ_RESPONSE_SLV_ERR_DIS :1;      ///<BIT [2] Parity_Error_Read_Response_SLV_ERR_dis
        uint32_t PARITY_ERROR_WRITE_RESPONSE_SLV_ERR_DIS :1;      ///<BIT [3] Parity_Error_Write_Response_SLV_ERR_dis
        uint32_t RSVD_1                      :12;     ///<BIT [15:4] RSVD_1
        uint32_t PARITY_ERROR_READ_INJ_TRIG  :1;      ///<BIT [16] Parity_Error_Read_Inj_Trig
        uint32_t PARITY_ERROR_WRITE_INJ_TRIG :1;      ///<BIT [17] Parity_Error_Write_Inj_Trig
        uint32_t RSVD                        :13;     ///<BIT [30:18] RSVD_0
        uint32_t PARITY_MODE                 :1;      ///<BIT [31] Parity_Mode
    } b;
} Parctrl_t;

/// @brief 0xD4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PARITY_ERROR_ADDRESS        :22;     ///<BIT [21:0] Parity_Error_Address
        uint32_t RSVD                        :6;      ///<BIT [27:22] RSVD_0
        uint32_t PARITY_ERROR_COUNTER        :4;      ///<BIT [31:28] Parity_Error_Counter
    } b;
} Parerr0_t;

/// @brief 0x100
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t BLOCK_MEMORY_MAPPING_MODE   :3;      ///<BIT [2:0] Block_Memory_Mapping_Mode
        uint32_t RESERVED_0                  :29;     ///<BIT [31:3] Reserved_0
    } b;
} Memmap0_t;

/// @brief 0x200
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t MASTER_0_READ_WEIGHT        :16;     ///<BIT [15:0] Master_0_Read_Weight
        uint32_t MASTER_0_WRITE_WEIGHT       :16;     ///<BIT [31:16] Master_0_Write_Weight
    } b;
} Mst0wei_t;

typedef struct
{
    uint8_t rsvd0[64];                                                      // 0x0 : rsvd_0 / rsvd_0
    Memeccenb_t memeccenb;                                                  // 0x40 : MEMECCENB / 
    Memeccset_t memeccset;                                                  // 0x44 : MEMECCSET / 
    Memeccclr_t memeccclr;                                                  // 0x48 : MEMECCCLR / 
    Meminit_t meminit;                                                      // 0x4C : MEMINIT / 
    Memintstt_t memintstt;                                                  // 0x50 : MEMINTSTT / 
    Memintenb_t memintenb;                                                  // 0x54 : MEMINTENB / 
    Memintset_t memintset;                                                  // 0x58 : MEMINTSET / 
    Memintclr_t memintclr;                                                  // 0x5C : MEMINTCLR / 
    uint32_t errinjbloErrBitLow;                                            // 0x60 : ERRINJBLO / 
    uint32_t errinjbhiErrBitHigh;                                           // 0x64 : ERRINJBHI / 
    uint8_t rsvd68[8];                                                      // 0x68 : rsvd_68 / rsvd_68
    Errinjadr_t errinjadr;                                                  // 0x70 : ERRINJADR / 
    Errinjcts_t errinjcts;                                                  // 0x74 : ERRINJCTS / 
    uint8_t rsvd78[8];                                                      // 0x78 : rsvd_78 / rsvd_78
    Errslog0_t errslog0;                                                    // 0x80 : ERRSLOG0 / 
    Errslog1_t errslog1;                                                    // 0x84 : ERRSLOG1 / 
    uint8_t rsvd88[8];                                                      // 0x88 : rsvd_88 / rsvd_88
    Errssts_t errssts;                                                      // 0x90 : ERRSSTS / 
    Errsthr_t errsthr;                                                      // 0x94 : ERRSTHR / 
    uint8_t rsvd98[8];                                                      // 0x98 : rsvd_98 / rsvd_98
    Errdlog0_t errdlog0;                                                    // 0xA0 : ERRDLOG0 / 
    uint8_t rsvdA4[12];                                                     // 0xA4 : rsvd_a4 / rsvd_a4
    Errdsts_t errdsts;                                                      // 0xB0 : ERRDSTS / 
    Errdthr_t errdthr;                                                      // 0xB4 : ERRDTHR / 
    uint8_t rsvdB8[8];                                                      // 0xB8 : rsvd_b8 / rsvd_b8
    Memarbctl0_t memarbctl0;                                                // 0xC0 : MEMARBCTL0 / 
    uint8_t rsvdC4[12];                                                     // 0xC4 : rsvd_c4 / rsvd_c4
    Parctrl_t parctrl;                                                      // 0xD0 : PARCTRL / 
    Parerr0_t parerr0;                                                      // 0xD4 : PARERR0 / 
    uint8_t rsvdD8[40];                                                     // 0xD8 : rsvd_d8 / rsvd_d8
    Memmap0_t memmap0;                                                      // 0x100 : MEMMAP0 / 
    uint8_t rsvd104[252];                                                   // 0x104 : rsvd_104 / rsvd_104
    Mst0wei_t mst0wei;                                                      // 0x200 : MST0WEI / 
} Gsram_t;

COMPILE_ASSERT(offsetof(Gsram_t,memeccenb)==0x40,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,memeccset)==0x44,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,memeccclr)==0x48,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,meminit)==0x4C,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,memintstt)==0x50,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,memintenb)==0x54,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,memintset)==0x58,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,memintclr)==0x5C,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errinjbloErrBitLow)==0x60,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errinjbhiErrBitHigh)==0x64,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errinjadr)==0x70,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errinjcts)==0x74,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errslog0)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errslog1)==0x84,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errssts)==0x90,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errsthr)==0x94,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errdlog0)==0xA0,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errdsts)==0xB0,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,errdthr)==0xB4,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,memarbctl0)==0xC0,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,parctrl)==0xD0,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,parerr0)==0xD4,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,memmap0)==0x100,"check register structure offset");
COMPILE_ASSERT(offsetof(Gsram_t,mst0wei)==0x200,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Gsram_t rGsram; ///< 0xB000C000
