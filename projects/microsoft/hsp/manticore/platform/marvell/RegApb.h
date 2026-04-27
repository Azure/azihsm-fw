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
//! @brief APB_REG Registers
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


/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_0                  :16;     ///<BIT [15:0] Reserved_0
        uint32_t VSENSOR_OUT                 :16;     ///<BIT [31:16] VSENSOR_OUT
    } b;
} PadVsensorCfg4_t;

/// @brief 0x54
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PAD_SR_TOP                  :2;      ///<BIT [1:0] PAD_SR_TOP
        uint32_t PAD_SR_HSP                  :2;      ///<BIT [3:2] PAD_SR_HSP
        uint32_t PAD_SR_UART0                :2;      ///<BIT [5:4] PAD_SR_UART0
        uint32_t PAD_SR_UART1                :2;      ///<BIT [7:6] PAD_SR_UART1
        uint32_t PAD_SR_I2C0                 :2;      ///<BIT [9:8] PAD_SR_I2C0
        uint32_t PAD_SR_I2C1                 :2;      ///<BIT [11:10] PAD_SR_I2C1
        uint32_t PAD_SR_JTAG                 :2;      ///<BIT [13:12] PAD_SR_JTAG
        uint32_t PAD_SR_QSPI0                :2;      ///<BIT [15:14] PAD_SR_QSPI0
        uint32_t PAD_SR_QSPI1                :2;      ///<BIT [17:16] PAD_SR_QSPI1
        uint32_t PAD_SR_QSPI2                :2;      ///<BIT [19:18] PAD_SR_QSPI2
        uint32_t PAD_SR_SPIS0                :2;      ///<BIT [21:20] PAD_SR_SPIS0
        uint32_t PAD_SR_SPIS1                :2;      ///<BIT [23:22] PAD_SR_SPIS1
        uint32_t PAD_SR_MISC                 :2;      ///<BIT [25:24] PAD_SR_MISC
        uint32_t PAD_SR_MISC2                :2;      ///<BIT [27:26] PAD_SR_MISC2
        uint32_t PAD_SR_MISC3                :2;      ///<BIT [29:28] PAD_SR_MISC3
        uint32_t RESERVED_0                  :2;      ///<BIT [31:30] Reserved_0
    } b;
} PadSrCfg_t;

/// @brief 0x80
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PAD_PIO_ZN                  :5;      ///<BIT [4:0] PAD_PIO_ZN
        uint32_t PAD_PIO_ZP                  :5;      ///<BIT [9:5] PAD_PIO_ZP
        uint32_t RESERVED_4                  :1;      ///<BIT [10] Reserved_4
        uint32_t PAD_SIO_ZN                  :4;      ///<BIT [14:11] PAD_SIO_ZN
        uint32_t RESERVED_3                  :1;      ///<BIT [15] Reserved_3
        uint32_t PAD_SIO_ZP                  :4;      ///<BIT [19:16] PAD_SIO_ZP
        uint32_t RESERVED_2                  :1;      ///<BIT [20] Reserved_2
        uint32_t PAD_GIO_ZN                  :4;      ///<BIT [24:21] PAD_GIO_ZN
        uint32_t RESERVED_1                  :1;      ///<BIT [25] Reserved_1
        uint32_t PAD_GIO_ZP                  :4;      ///<BIT [29:26] PAD_GIO_ZP
        uint32_t RESERVED_0                  :2;      ///<BIT [31:30] Reserved_0
    } b;
} PadCfg_t;

/// @brief 0x84
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PAD_PU                      :16;     ///<BIT [15:0] PAD_PU
        uint32_t PAD_PD                      :16;     ///<BIT [31:16] PAD_PD
    } b;
} PadPuPdEnable_t;

/// @brief 0x88
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PAD_PU_EXT                  :16;     ///<BIT [15:0] PAD_PU_EXT
        uint32_t PAD_PD_EXT                  :16;     ///<BIT [31:16] PAD_PD_EXT
    } b;
} PadPuPdExtEnable_t;

/// @brief 0x8C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PAD_PU_EXT_1                :16;     ///<BIT [15:0] PAD_PU_EXT_1
        uint32_t PAD_PD_EXT_1                :16;     ///<BIT [31:16] PAD_PD_EXT_1
    } b;
} PadPuPdExtEnable1_t;

/// @brief 0x90
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PAD_PU_EXT_2                :16;     ///<BIT [15:0] PAD_PU_EXT_2
        uint32_t PAD_PD_EXT_2                :16;     ///<BIT [31:16] PAD_PD_EXT_2
    } b;
} PadPuPdExtEnable2_t;

/// @brief 0x9C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PAD_PU_EXT_3                :16;     ///<BIT [15:0] PAD_PU_EXT_3
        uint32_t PAD_PD_EXT_3                :16;     ///<BIT [31:16] PAD_PD_EXT_3
    } b;
} PadPuPdExtEnable3_t;

/// @brief 0xA0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GSRAM_RTC                   :2;      ///<BIT [1:0] GSRAM_RTC
        uint32_t GSRAM_WTC                   :2;      ///<BIT [3:2] GSRAM_WTC
        uint32_t GSRAM_SD                    :8;      ///<BIT [11:4] GSRAM_SD
        uint32_t GSRAM_SLP                   :8;      ///<BIT [19:12] GSRAM_SLP
        uint32_t GSRAM_DSLP                  :8;      ///<BIT [27:20] GSRAM_DSLP
        uint32_t DUAL_CP_SD                  :1;      ///<BIT [28] DUAL_CP_SD
        uint32_t DUAL_CP_SLP                 :1;      ///<BIT [29] DUAL_CP_SLP
        uint32_t RESERVED                    :2;      ///<BIT [31:30] RESERVED
    } b;
} SramControl1_t;

/// @brief 0xA8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPKA0_SRAM1P_RTC            :2;      ///<BIT [1:0] UPKA0_SRAM1P_RTC
        uint32_t UPKA0_SRAM1P_WTC            :2;      ///<BIT [3:2] UPKA0_SRAM1P_WTC
        uint32_t UPKA0_SRAM2P_RTC            :2;      ///<BIT [5:4] UPKA0_SRAM2P_RTC
        uint32_t UPKA0_SRAM2P_WTC            :2;      ///<BIT [7:6] UPKA0_SRAM2P_WTC
        uint32_t UPKA0_SRAM2P_KP             :3;      ///<BIT [10:8] UPKA0_SRAM2P_KP
        uint32_t RESERVED0                   :1;      ///<BIT [11] RESERVED0
        uint32_t UPKA0_SRAM1P_SD             :1;      ///<BIT [12] UPKA0_SRAM1P_SD
        uint32_t UPKA0_SRAM1P_SLP            :1;      ///<BIT [13] UPKA0_SRAM1P_SLP
        uint32_t UPKA0_SRAM2P_SD             :1;      ///<BIT [14] UPKA0_SRAM2P_SD
        uint32_t UPKA0_SRAM2P_SLP            :1;      ///<BIT [15] UPKA0_SRAM2P_SLP
        uint32_t UPKA1_SRAM1P_RTC            :2;      ///<BIT [17:16] UPKA1_SRAM1P_RTC
        uint32_t UPKA1_SRAM1P_WTC            :2;      ///<BIT [19:18] UPKA1_SRAM1P_WTC
        uint32_t UPKA1_SRAM2P_RTC            :2;      ///<BIT [21:20] UPKA1_SRAM2P_RTC
        uint32_t UPKA1_SRAM2P_WTC            :2;      ///<BIT [23:22] UPKA1_SRAM2P_WTC
        uint32_t UPKA1_SRAM2P_KP             :3;      ///<BIT [26:24] UPKA1_SRAM2P_KP
        uint32_t RESERVED1                   :1;      ///<BIT [27] RESERVED1
        uint32_t UPKA1_SRAM1P_SD             :1;      ///<BIT [28] UPKA1_SRAM1P_SD
        uint32_t UPKA1_SRAM1P_SLP            :1;      ///<BIT [29] UPKA1_SRAM1P_SLP
        uint32_t UPKA1_SRAM2P_SD             :1;      ///<BIT [30] UPKA1_SRAM2P_SD
        uint32_t UPKA1_SRAM2P_SLP            :1;      ///<BIT [31] UPKA1_SRAM2P_SLP
    } b;
} SramControl3_t;

/// @brief 0xF0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t GPIO14_15_UART_SEL          :1;      ///<BIT [0] GPIO14_15_UART_SEL
        uint32_t SPI_TPM_UART_SEL            :1;      ///<BIT [1] SPI_TPM_UART_SEL
        uint32_t SOC_MON_EN                  :1;      ///<BIT [2] SOC_MON_EN
        uint32_t PETM_DEBUG_EN               :1;      ///<BIT [3] PETM_DEBUG_EN
        uint32_t RESERVED_0                  :28;     ///<BIT [31:4] reserved_0
    } b;
} GpioSel_t;

/// @brief 0xF8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PIN_GPIO_OE                 :4;      ///<BIT [3:0] PIN_GPIO_OE
        uint32_t RESERVED_0                  :27;     ///<BIT [30:4] reserved_0
        uint32_t RSVD_31                     :1;      ///<BIT [31] rsvd_31
    } b;
} GpioOutputEnable_t;

/// @brief 0xFC
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PIN_GPIO_OUTPUT             :4;      ///<BIT [3:0] PIN_GPIO_Output
        uint32_t RESERVED_0                  :28;     ///<BIT [31:4] reserved_0
    } b;
} GpioOutput_t;

/// @brief 0x100
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PIN_GPIO_INPUT              :4;      ///<BIT [3:0] PIN_GPIO_Input
        uint32_t RESERVED_0                  :28;     ///<BIT [31:4] reserved_0
    } b;
} GpioInput_t;

/// @brief 0x104
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t STRAP0_LATCHED              :1;      ///<BIT [0] Strap0_latched
        uint32_t STRAP1_LATCHED              :1;      ///<BIT [1] Strap1_latched
        uint32_t A0_BYASSS_LATCHED           :1;      ///<BIT [2] A0_BYASSS_latched
        uint32_t RECOVERY_LATCHED            :1;      ///<BIT [3] Recovery_latched
        uint32_t FLASH_PRIORITY_LATCHED      :1;      ///<BIT [4] Flash_Priority_latched
        uint32_t RESERVED_0                  :27;     ///<BIT [31:5] reserved_0
    } b;
} SocConfig_t;

/// @brief 0x120
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t PSLVERR_MASK                :1;      ///<BIT [0] PSLVERR_MASK
        uint32_t RESERVED_0                  :15;     ///<BIT [15:1] reserved_0
        uint32_t C_REV                       :16;     ///<BIT [31:16] C_REV
    } b;
} DebugControl1_t;

/// @brief 0x200
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t UPKA_MST_PARITY_ERR_INJ     :16;     ///<BIT [15:0] upka_mst_parity_err_inj
        uint32_t UPKA_SLV_PARITY_ERR_INJ     :16;     ///<BIT [31:16] upka_slv_parity_err_inj
    } b;
} ParityErrInj0_t;

/// @brief 0x204
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AES_MST_PARITY_ERR_INJ      :1;      ///<BIT [0] aes_mst_parity_err_inj
        uint32_t AES_SLV_PARITY_ERR_INJ      :1;      ///<BIT [1] aes_slv_parity_err_inj
        uint32_t HSSHA_MST_PARITY_ERR_INJ    :1;      ///<BIT [2] hssha_mst_parity_err_inj
        uint32_t HSSHA_SLV_PARITY_ERR_INJ    :1;      ///<BIT [3] hssha_slv_parity_err_inj
        uint32_t SPI_PARITY_ERR_INJ          :1;      ///<BIT [4] spi_parity_err_inj
        uint32_t HSP_PARITY_ERR_INJ          :1;      ///<BIT [5] hsp_parity_err_inj
        uint32_t MTX_APB_PARITY_ERR_INJ      :1;      ///<BIT [6] mtx_apb_parity_err_inj
        uint32_t RNG_PARITY_ERR_INJ          :1;      ///<BIT [7] rng_parity_err_inj
        uint32_t CS_ETR_PARITY_ERR_INJ       :1;      ///<BIT [8] cs_etr_parity_err_inj
        uint32_t CS_DAP_PARITY_ERR_INJ       :1;      ///<BIT [9] cs_dap_parity_err_inj
        uint32_t DUMMY_HS_PARITY_ERR_INJ     :1;      ///<BIT [10] dummy_hs_parity_err_inj
        uint32_t RESERVED0                   :21;     ///<BIT [31:11] RESERVED0
    } b;
} ParityErrInj1_t;

/// @brief 0x208
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_0                  :1;      ///<BIT [0] Reserved_0
        uint32_t FORCE_A_W_CACHE1            :1;      ///<BIT [1] ForceAWCache1
        uint32_t RESERVED_1                  :1;      ///<BIT [2] Reserved_1
        uint32_t FORCE_A_R_CACHE1            :1;      ///<BIT [3] ForceARCache1
        uint32_t RESERVED_2                  :28;     ///<BIT [31:4] Reserved_2
    } b;
} ArwcacheControl_t;

/// @brief 0x290
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_0_1                    :2;      ///<BIT [1:0] rsvd_0_1
        uint32_t RESERVED_2                  :2;      ///<BIT [3:2] Reserved_2
        uint32_t CS_RTSEL                    :2;      ///<BIT [5:4] CS_RTSEL
        uint32_t CS_WTSEL                    :2;      ///<BIT [7:6] CS_WTSEL
        uint32_t RESERVED_1                  :8;      ///<BIT [15:8] Reserved_1
        uint32_t TPIU_CTRL                   :6;      ///<BIT [21:16] TPIU_CTRL
        uint32_t RESERVED_0                  :6;      ///<BIT [27:22] Reserved_0
        uint32_t CS_AUTH                     :4;      ///<BIT [31:28] CS_AUTH
    } b;
} CsCtrl_t;

/// @brief 0x2D0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED                    :1;      ///<BIT [0] Reserved
        uint32_t SRBIST_STATUS               :31;     ///<BIT [31:1] SRBIST_STATUS
    } b;
} SrbistStatus_t;

/// @brief 0x40C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AXIWDT_ENABLE               :1;      ///<BIT [0] AXIWDT_ENABLE
        uint32_t RESERVED_0                  :15;     ///<BIT [15:1] Reserved_0
        uint32_t AXIWDT_CNTRL                :1;      ///<BIT [16] AXIWDT_CNTRL
        uint32_t RESERVED_1                  :15;     ///<BIT [31:17] Reserved_1
    } b;
} AxiWdtCntrl_t;

/// @brief 0x410
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AXIWDT_STATUS               :16;     ///<BIT [15:0] AXIWDT_STATUS
        uint32_t RESERVED                    :16;     ///<BIT [31:16] Reserved
    } b;
} AxiWdtStatus_t;

/// @brief 0x414
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t AXIWDT_STS_CLEAR            :16;     ///<BIT [15:0] AXIWDT_STS_CLEAR
        uint32_t RESERVED                    :16;     ///<BIT [31:16] Reserved
    } b;
} AxiWdtStsClear_t;

/// @brief 0x418
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RESERVED_0                  :1;      ///<BIT [0] Reserved_0
        uint32_t AXIWDT_REG_LOCK             :1;      ///<BIT [1] AXIWDT_REG_LOCK
        uint32_t RESERVED_1                  :30;     ///<BIT [31:2] Reserved_1
    } b;
} AxiWdtLock_t;

typedef struct
{
    uint8_t rsvd0[80];                                                      // 0x0 : rsvd_0 / rsvd_0
    PadVsensorCfg4_t padVsensorCfg4;                                        // 0x50 : PAD_VSENSOR_CFG4 / 
    PadSrCfg_t padSrCfg;                                                    // 0x54 : PAD_SR_CFG / 
    uint8_t rsvd58[40];                                                     // 0x58 : rsvd_58 / rsvd_58
    PadCfg_t padCfg;                                                        // 0x80 : Pad_Configuration / 
    PadPuPdEnable_t padPuPdEnable;                                          // 0x84 : Pad_PU_PD_Enable / 
    PadPuPdExtEnable_t padPuPdExtEnable;                                    // 0x88 : Pad_PU_PD_EXT_Enable / 
    PadPuPdExtEnable1_t padPuPdExtEnable1;                                  // 0x8C : Pad_PU_PD_EXT_Enable_1 / 
    PadPuPdExtEnable2_t padPuPdExtEnable2;                                  // 0x90 : Pad_PU_PD_EXT_Enable_2 / 
    uint32_t padZpControlPadZpCtrl;                                         // 0x94 : Pad_ZP_Control / 
    uint32_t padZnControlPadZnCtrl;                                         // 0x98 : Pad_ZN_Control / 
    PadPuPdExtEnable3_t padPuPdExtEnable3;                                  // 0x9C : Pad_PU_PD_EXT_Enable_3 / 
    SramControl1_t sramControl1;                                            // 0xA0 : SRAM_Control_1 / 
    uint8_t rsvdA4[4];                                                      // 0xA4 : rsvd_a4 / rsvd_a4
    SramControl3_t sramControl3;                                            // 0xA8 : SRAM_Control_3 / 
    uint8_t rsvdAc[68];                                                     // 0xAC : rsvd_ac / rsvd_ac
    GpioSel_t gpioSel;                                                      // 0xF0 : GPIO_SEL / 
    uint32_t debugSel;                                                      // 0xF4 : DEBUG_SEL / 
    GpioOutputEnable_t gpioOutputEnable;                                    // 0xF8 : GPIO_Output_Enable / 
    GpioOutput_t gpioOutput;                                                // 0xFC : GPIO_Output / 
    GpioInput_t gpioInput;                                                  // 0x100 : GPIO_Input / 
    SocConfig_t socConfig;                                                  // 0x104 : SOC_CONFIG / 
    uint8_t rsvd108[24];                                                    // 0x108 : rsvd_108 / rsvd_108
    DebugControl1_t debugControl1;                                          // 0x120 : Debug_Control_1 / 
    uint32_t debugTest;                                                     // 0x124 : DEBUG_TEST / 
    uint32_t debugMonData;                                                  // 0x128 : DEBUG_MON_DATA / 
    uint8_t rsvd12c[212];                                                   // 0x12C : rsvd_12c / rsvd_12c
    ParityErrInj0_t parityErrInj0;                                          // 0x200 : PARITY_ERR_INJ0 / 
    ParityErrInj1_t parityErrInj1;                                          // 0x204 : PARITY_ERR_INJ1 / 
    ArwcacheControl_t arwcacheControl;                                      // 0x208 : ARWCache_Control / 
    uint8_t rsvd20c[132];                                                   // 0x20C : rsvd_20c / rsvd_20c
    CsCtrl_t csCtrl;                                                        // 0x290 : CS_CTRL / 
    uint8_t rsvd294[60];                                                    // 0x294 : rsvd_294 / rsvd_294
    SrbistStatus_t srbistStatus;                                            // 0x2D0 : SRBIST_STATUS / 
    uint8_t rsvd2d4[300];                                                   // 0x2D4 : rsvd_2d4 / rsvd_2d4
    uint32_t axiwdtBase;                                                    // 0x400 : AXI_WDT_BASE / 
    uint32_t axiwdtRdCnt;                                                   // 0x404 : AXI_WDT_RD_CNT / 
    uint32_t axiwdtWrCnt;                                                   // 0x408 : AXI_WDT_WR_CNT / 
    AxiWdtCntrl_t axiWdtCntrl;                                              // 0x40C : AXI_WDT_CNTRL / 
    AxiWdtStatus_t axiWdtStatus;                                            // 0x410 : AXI_WDT_STATUS / 
    AxiWdtStsClear_t axiWdtStsClear;                                        // 0x414 : AXI_WDT_STS_CLEAR / 
    AxiWdtLock_t axiWdtLock;                                                // 0x418 : AXI_WDT_REG_LOCK / 
} Apb_t;

COMPILE_ASSERT(offsetof(Apb_t,padVsensorCfg4)==0x50,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,padSrCfg)==0x54,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,padCfg)==0x80,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,padPuPdEnable)==0x84,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,padPuPdExtEnable)==0x88,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,padPuPdExtEnable1)==0x8C,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,padPuPdExtEnable2)==0x90,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,padZpControlPadZpCtrl)==0x94,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,padZnControlPadZnCtrl)==0x98,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,padPuPdExtEnable3)==0x9C,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,sramControl1)==0xA0,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,sramControl3)==0xA8,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,gpioSel)==0xF0,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,debugSel)==0xF4,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,gpioOutputEnable)==0xF8,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,gpioOutput)==0xFC,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,gpioInput)==0x100,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,socConfig)==0x104,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,debugControl1)==0x120,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,debugTest)==0x124,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,debugMonData)==0x128,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,parityErrInj0)==0x200,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,parityErrInj1)==0x204,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,arwcacheControl)==0x208,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,csCtrl)==0x290,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,srbistStatus)==0x2D0,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,axiwdtBase)==0x400,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,axiwdtRdCnt)==0x404,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,axiwdtWrCnt)==0x408,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,axiWdtCntrl)==0x40C,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,axiWdtStatus)==0x410,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,axiWdtStsClear)==0x414,"check register structure offset");
COMPILE_ASSERT(offsetof(Apb_t,axiWdtLock)==0x418,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Apb_t rApb; ///< 0xB0007000
