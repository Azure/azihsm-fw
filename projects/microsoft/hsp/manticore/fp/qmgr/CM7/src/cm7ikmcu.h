/**************************************************************************//**
 * @file     cm7ikmcu.h
 * @brief    CMSIS-Core(M) Device Header File for Device <Device>
 *
 * @version  V1.0.0
 * @date     18. July 2023
 ******************************************************************************/
/*
 * Copyright (c) 2009-2023 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CM7IKMCU_H
#define CM7IKMCU_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                Processor and Core Peripherals                              */
/******************************************************************************/


/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */
typedef enum IRQn
{
/* ================     Cortex-M7 Core Exception Numbers     ================ */
    NonMaskableInt_IRQn        = -14,  /*  2 Non Maskable Interrupt */
    HardFault_IRQn             = -13,  /*  3 Cortex-M7 Hard Fault Interrupt */
    SVCall_IRQn                =  -5,  /* 11 Cortex-M7 SV Call Interrupt */
    PendSV_IRQn                 =  -2,  /* 14 Cortex-M7 Pend SV Interrupt */
    SysTick_IRQn               =  -1,  /* 15 Cortex-M7 System Tick Interrupt */

/* ================     Device-specific Interrupt Numbers    ================ */
    GPIO_IRQn                  = 0,
    CM7IKMCU_IRQ01_IRQn        = 1,
    CM7IKMCU_IRQ02_IRQn        = 2,
    CM7IKMCU_IRQ03_IRQn        = 3,
    CM7IKMCU_IRQ04_IRQn        = 4,
    CM7IKMCU_IRQ05_IRQn        = 5,
    CM7IKMCU_IRQ06_IRQn        = 6,
    CM7IKMCU_IRQ07_IRQn        = 7,
    CM7IKMCU_IRQ08_IRQn        = 8,
    CM7IKMCU_IRQ09_IRQn        = 9,
    CM7IKMCU_IRQ10_IRQn        = 10,
    CM7IKMCU_IRQ11_IRQn        = 11,
    CM7IKMCU_IRQ12_IRQn        = 12,
    CM7IKMCU_IRQ13_IRQn        = 13,
    CM7IKMCU_IRQ14_IRQn        = 14,
    CM7IKMCU_IRQ15_IRQn        = 15,
    CM7IKMCU_IRQ16_IRQn        = 16,
    CM7IKMCU_IRQ17_IRQn        = 17,
    CM7IKMCU_IRQ18_IRQn        = 18,
    CM7IKMCU_IRQ19_IRQn        = 19,
    CM7IKMCU_IRQ20_IRQn        = 20,
    CM7IKMCU_IRQ21_IRQn        = 21,
    CM7IKMCU_IRQ22_IRQn        = 22,
    CM7IKMCU_IRQ23_IRQn        = 23,
    CM7IKMCU_IRQ24_IRQn        = 24,
    CM7IKMCU_IRQ25_IRQn        = 25,
    CM7IKMCU_IRQ26_IRQn        = 26,
    CM7IKMCU_IRQ27_IRQn        = 27,
    CM7IKMCU_IRQ28_IRQn        = 28,
    CM7IKMCU_IRQ29_IRQn        = 29,
    CM7IKMCU_IRQ30_IRQn        = 30,
    CM7IKMCU_IRQ31_IRQn        = 31,
    CM7IKMCU_IRQ32_IRQn        = 32,
    CM7IKMCU_IRQ33_IRQn        = 33,
    CM7IKMCU_IRQ34_IRQn        = 34,
    CM7IKMCU_IRQ35_IRQn        = 35,
    CM7IKMCU_IRQ36_IRQn        = 36,
    CM7IKMCU_IRQ37_IRQn        = 37,
    CM7IKMCU_IRQ38_IRQn        = 38,
    CM7IKMCU_IRQ39_IRQn        = 39,
    CM7IKMCU_IRQ40_IRQn        = 40,
    CM7IKMCU_IRQ41_IRQn        = 41,
    CM7IKMCU_IRQ42_IRQn        = 42,
    CM7IKMCU_IRQ43_IRQn        = 43,
    CM7IKMCU_IRQ44_IRQn        = 44,
    CM7IKMCU_IRQ45_IRQn        = 45,
    CM7IKMCU_IRQ46_IRQn        = 46,
    CM7IKMCU_IRQ47_IRQn        = 47,
    CM7IKMCU_IRQ88_IRQn        = 88,
    CM7IKMCU_IRQ89_IRQn        = 89,
    CM7IKMCU_IRQ128_IRQn       = 128,
    CM7IKMCU_IRQ129_IRQn       = 129,
    CM7IKMCU_IRQ130_IRQn       = 130,
    CM7IKMCU_IRQ131_IRQn       = 131,
    CM7IKMCU_IRQ132_IRQn       = 132,
    CM7IKMCU_IRQ133_IRQn       = 133
} IRQn_Type;


/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */


/* --------  Configuration of Core Peripherals  ----------------------------------- */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */
#define __NVIC_PRIO_BITS          2U        /* Number of Bits used for Priority Levels */
#define __VTOR_PRESENT            1U        /* Set to 1 if VTOR is present */
#define __MPU_PRESENT             1U        /* Set to 1 if MPU is present */
#define __FPU_PRESENT             1U        /* Set to 1 if FPU is present */
#define __FPU_DP                  0U        /* Set to 1 if FPU is double precision FPU (default is single precision FPU) */
#define __DSP_PRESENT             0U        /* Set to 1 if DSP extension are present */
#define __SAUREGION_PRESENT       0U        /* Set to 1 if SAU regions are present */
#define __PMU_PRESENT             0U        /* Set to 1 if PMU is present */
#define __PMU_NUM_EVENTCNT        0U        /* Set number of PMU Event Counters */
#define __ICACHE_PRESENT          1U        /* Set to 1 if I-Cache is present */
#define __DCACHE_PRESENT          1U        /* Set to 1 if D-Cache is present */
#define __DTCM_PRESENT            0U        /* Set to 1 if DTCM is present */

#include "core_cm7.h"                       /* Processor and core peripherals */
#include "system_cm7ikmcu.h"                  /* System Header (SystemCoreClock, etc.) */

/**
 * Initialize the system clock
 */
extern void SystemInit(void);

/**
 * Enable Caches
 */
extern void cache_enable(void);

/**
 * Invalidate Caches
 */
extern void cache_invalidate(void);


/******************************************************************************/
/*                Device Specific Peripheral registers structures             */
/******************************************************************************/

/* --------  Cortex-M7 ETM (Embedded Trace Macrocell)  --------------------------- */
typedef struct
{
    uint32_t RESERVED0;
    __IO uint32_t TRCPRGCTLR;
    __IO uint32_t TRCPROCSELR;
    __I  uint32_t TRCSTATR;
    __IO uint32_t TRCCONFIGR;
    uint32_t RESERVED1;
    __IO uint32_t TRCAUXCTLR;
    uint32_t RESERVED2;
    __IO uint32_t TRCEVENTCTL0R;
    __IO uint32_t TRCEVENTCTL1R;
    uint32_t RESERVED3;
    __IO uint32_t TRCSTALLCTLR;
    __IO uint32_t TRCTSCTLR;
    __IO uint32_t TRCSYNCPR;
    __IO uint32_t TRCCCCTLR;
    __IO uint32_t TRCBBCTLR;
    __IO uint32_t TRCTRACEIDR;
    uint32_t RESERVED4[15];
    __IO uint32_t TRCVICTLR;
    __IO uint32_t TRCVIIECTLR;
    __IO uint32_t TRCVISSCTLR;
    __IO uint32_t TRCVIPCSSCTLR;
    uint32_t RESERVED5[4];
    __IO uint32_t TRCVDCTLR;
    __IO uint32_t TRCVDSACCTLR;
    __IO uint32_t TRCVDARCCTLR;
    uint32_t RESERVED6[21];
    __IO uint32_t TRCSEQEVR0;
    __IO uint32_t TRCSEQEVR1;
    __IO uint32_t TRCSEQEVR2;
    uint32_t RESERVED7[3];
    __IO uint32_t TRCSEQRSTEVR;
    __IO uint32_t TRCSEQSTR;
    __IO uint32_t TRCEXTINSELR;
    uint32_t RESERVED8[7];
    __IO uint32_t TRCCNTRLDVR0;
    __IO uint32_t TRCCNTRLDVR1;
    uint32_t RESERVED9[2];
    __IO uint32_t TRCCNTCTLR0;
    __IO uint32_t TRCCNTCTLR1;
    uint32_t RESERVED10[2];
    __IO uint32_t TRCCNTVR0;
    __IO uint32_t TRCCNTVR1;
    uint32_t RESERVED11[6];
    __I  uint32_t TRCIDR8;
    __I  uint32_t TRCIDR9;
    __I  uint32_t TRCIDR10;
    __I  uint32_t TRCIDR11;
    __I  uint32_t TRCIDR12;
    __I  uint32_t TRCIDR13;
    uint32_t RESERVED12[18];
    __I  uint32_t TRCIDR0;
    __I  uint32_t TRCIDR1;
    __I  uint32_t TRCIDR2;
    __I  uint32_t TRCIDR3;
    __I  uint32_t TRCIDR4;
    __I  uint32_t TRCIDR5;
    __I  uint32_t TRCIDR6;
    __I  uint32_t TRCIDR7;
    uint32_t RESERVED13[2];
    __IO uint32_t TRCRSCTLR2;
    __IO uint32_t TRCRSCTLR3;
    __IO uint32_t TRCRSCTLR4;
    __IO uint32_t TRCRSCTLR5;
    __IO uint32_t TRCRSCTLR6;
    __IO uint32_t TRCRSCTLR7;
    __IO uint32_t TRCRSCTLR8;
    __IO uint32_t TRCRSCTLR9;
    __IO uint32_t TRCRSCTLR10;
    __IO uint32_t TRCRSCTLR11;
    __IO uint32_t TRCRSCTLR12;
    __IO uint32_t TRCRSCTLR13;
    __IO uint32_t TRCRSCTLR14;
    __IO uint32_t TRCRSCTLR15;
    uint32_t RESERVED14[16];
    __IO uint32_t TRCSSCCR0;
    uint32_t RESERVED15[7];
    __IO uint32_t TRCSSCSR0;
    uint32_t RESERVED16[7];
    __IO uint32_t TRCSSPCICR0;
    uint32_t RESERVED17[15];
    __O  uint32_t TRCOSLAR;
    __I  uint32_t TRCOSLSR;
    uint32_t RESERVED18[2];
    __IO uint32_t TRCPDCR;
    __I  uint32_t TRCPDSR;
    uint32_t RESERVED19[58];
    __IO uint32_t TRCACVR0;
    uint32_t RESERVED20;
    __IO uint32_t TRCACVR1;
    uint32_t RESERVED21;
    __IO uint32_t TRCACVR2;
    uint32_t RESERVED22;
    __IO uint32_t TRCACVR3;
    uint32_t RESERVED23;
    __IO uint32_t TRCACVR4;
    uint32_t RESERVED24;
    __IO uint32_t TRCACVR5;
    uint32_t RESERVED25;
    __IO uint32_t TRCACVR6;
    uint32_t RESERVED26;
    __IO uint32_t TRCACVR7;
    uint32_t RESERVED27;
    __IO uint32_t TRCACVR8;
    uint32_t RESERVED28;
    __IO uint32_t TRCACVR9;
    uint32_t RESERVED29;
    __IO uint32_t TRCACVR10;
    uint32_t RESERVED30;
    __IO uint32_t TRCACVR11;
    uint32_t RESERVED31;
    __IO uint32_t TRCACVR12;
    uint32_t RESERVED32;
    __IO uint32_t TRCACVR13;
    uint32_t RESERVED33;
    __IO uint32_t TRCACVR14;
    uint32_t RESERVED34;
    __IO uint32_t TRCACVR15;
    uint32_t RESERVED35;
    __IO uint32_t TRCACATR0;
    uint32_t RESERVED36;
    __IO uint32_t TRCACATR1;
    uint32_t RESERVED37;
    __IO uint32_t TRCACATR2;
    uint32_t RESERVED38;
    __IO uint32_t TRCACATR3;
    uint32_t RESERVED39;
    __IO uint32_t TRCACATR4;
    uint32_t RESERVED40;
    __IO uint32_t TRCACATR5;
    uint32_t RESERVED41;
    __IO uint32_t TRCACATR6;
    uint32_t RESERVED42;
    __IO uint32_t TRCACATR7;
    uint32_t RESERVED43;
    __IO uint32_t TRCACATR8;
    uint32_t RESERVED44;
    __IO uint32_t TRCACATR9;
    uint32_t RESERVED45;
    __IO uint32_t TRCACATR10;
    uint32_t RESERVED46;
    __IO uint32_t TRCACATR11;
    uint32_t RESERVED47;
    __IO uint32_t TRCACATR12;
    uint32_t RESERVED48;
    __IO uint32_t TRCACATR13;
    uint32_t RESERVED49;
    __IO uint32_t TRCACATR14;
    uint32_t RESERVED50;
    __IO uint32_t TRCACATR15;
    uint32_t RESERVED51;
    __IO uint32_t TRCDVCVR0;
    uint32_t RESERVED52[3];
    __IO uint32_t TRCDVCVR1;
    uint32_t RESERVED53[27];
    __IO uint32_t TRCDVCMR0;
    uint32_t RESERVED54[3];
    __IO uint32_t TRCDVCMR1;
    uint32_t RESERVED55[603];
    __IO uint32_t TRCITCTRL;
    uint32_t RESERVED56[39];
    __IO uint32_t TRCCLAIMSET;
    __IO uint32_t TRCCLAIMCLR;
    __I  uint32_t TRCDEVAFF0;
    __I  uint32_t TRCDEVAFF1;
    __O  uint32_t TRCLAR;
    __I  uint32_t TRCLSR;
    __I  uint32_t TRCAUTHSTATUS;
    __I  uint32_t TRCDEVARCH;
    uint32_t RESERVED57[2];
    __I  uint32_t TRCDEVID;
    __I  uint32_t TRCDEVTYPE;
    __I  uint32_t TRCPIDR4;
    __I  uint32_t TRCPIDR5;
    __I  uint32_t TRCPIDR6;
    __I  uint32_t TRCPIDR7;
    __I  uint32_t TRCPIDR0;
    __I  uint32_t TRCPIDR1;
    __I  uint32_t TRCPIDR2;
    __I  uint32_t TRCPIDR3;
    __I  uint32_t TRCCIDR0;
    __I  uint32_t TRCCIDR1;
    __I  uint32_t TRCCIDR2;
    __I  uint32_t TRCCIDR3;
} ETM_Type;

#define ETM_TRCPRGCTLR_EN_Pos            0
#define ETM_TRCPRGCTLR_EN_Msk            (1UL << ETM_TRCPRGCTLR_EN_Pos)
#define ETM_TRCPROCSELR_PROCSEL_Pos      0
#define ETM_TRCPROCSELR_PROCSEL_Msk      (0x3UL << ETM_TRCPROCSELR_PROCSEL_Pos)
#define ETM_TRCSTATR_IDLE_Pos            0
#define ETM_TRCSTATR_IDLE_Msk            (1UL << ETM_TRCSTATR_IDLE_Pos)
#define ETM_TRCSTATR_PMSTABLE_Pos        0
#define ETM_TRCSTATR_PMSTABLE_Msk        (1UL << ETM_TRCSTATR_PMSTABLE_Pos)
#define ETM_TRCCONFIGR_INSTP0_Pos        1
#define ETM_TRCCONFIGR_INSTP0_Msk        (0x3UL << ETM_TRCCONFIGR_INSTP0_Pos)
#define ETM_TRCCONFIGR_BB_Pos            3
#define ETM_TRCCONFIGR_BB_Msk            (1UL << ETM_TRCCONFIGR_BB_Pos)
#define ETM_TRCCONFIGR_CCI_Pos           4
#define ETM_TRCCONFIGR_CCI_Msk           (1UL << ETM_TRCCONFIGR_CCI_Pos)
#define ETM_TRCCONFIGR_COND_Pos          8
#define ETM_TRCCONFIGR_COND_Msk          (0x7UL << ETM_TRCCONFIGR_COND_Pos)
#define ETM_TRCCONFIGR_TS_Pos            11
#define ETM_TRCCONFIGR_TS_Msk            (1UL << ETM_TRCCONFIGR_TS_Pos)
#define ETM_TRCCONFIGR_RS_Pos            12
#define ETM_TRCCONFIGR_RS_Msk            (1UL << ETM_TRCCONFIGR_RS_Pos)
#define ETM_TRCCONFIGR_DA_Pos            16
#define ETM_TRCCONFIGR_DA_Msk            (1UL << ETM_TRCCONFIGR_DA_Pos)
#define ETM_TRCCONFIGR_DV_Pos            17
#define ETM_TRCCONFIGR_DV_Msk            (1UL << ETM_TRCCONFIGR_DV_Pos)
#define ETM_TRCSYNCPR_PERIOD_Pos         0
#define ETM_TRCSYNCPR_PERIOD_Msk         (0x1FUL << ETM_TRCSYNCPR_PERIOD_Pos)
#define ETM_TRCTRACEIDR_TRACEID_Pos      0
#define ETM_TRCTRACEIDR_TRACEID_Msk      (0x7FUL << ETM_TRCTRACEIDR_TRACEID_Pos)
#define ETM_TRCVICTLR_EVENT_Pos          0
#define ETM_TRCVICTLR_EVENT_Msk          (0xFFUL << ETM_TRCVICTLR_EVENT_Pos)
#define ETM_TRCVICTLR_SSSTATUS_Pos       9
#define ETM_TRCVICTLR_SSSTATUS_Msk       (1UL << ETM_TRCVICTLR_SSSTATUS_Pos)
#define ETM_TRCVICTLR_TRCRESET_Pos       10
#define ETM_TRCVICTLR_TRCRESET_Msk       (1UL << ETM_TRCVICTLR_TRCRESET_Pos)
#define ETM_TRCVICTLR_TRCERR_Pos         11
#define ETM_TRCVICTLR_TRCERR_Msk         (1UL << ETM_TRCVICTLR_TRCERR_Pos)
#define ETM_TRCVICTLR_EXLEVEL_S_Pos      16
#define ETM_TRCVICTLR_EXLEVEL_S_Msk      (0xFUL << ETM_TRCVICTLR_EXLEVEL_S_Pos)
#define ETM_TRCVICTLR_EXLEVEL_NS_Pos     20
#define ETM_TRCVICTLR_EXLEVEL_NS_Msk     (0xFUL << ETM_TRCVICTLR_EXLEVEL_NS_Pos)
#define ETM_TRCPIDR4_DES_2_Pos           0
#define ETM_TRCPIDR4_DES_2_Msk           (0xFUL << ETM_TRCPIDR4_DES_2_Pos)
#define ETM_TRCPIDR4_SIZE_Pos            4
#define ETM_TRCPIDR4_SIZE_Msk            (0xFUL << ETM_TRCPIDR4_SIZE_Pos)
#define ETM_TRCPIDR0_PART_0_Pos          0
#define ETM_TRCPIDR0_PART_0_Msk          (0xFFUL << ETM_TRCPIDR4_PART_0_Pos)
#define ETM_TRCPIDR1_PART_1_Pos          0
#define ETM_TRCPIDR1_PART_1_Msk          (0xFUL << ETM_TRCPIDR1_PART_1_Pos)
#define ETM_TRCPIDR1_DES_0_Pos           4
#define ETM_TRCPIDR1_DES_0_Msk           (0xFUL << ETM_TRCPIDR1_DES_0_Pos)
#define ETM_TRCPIDR2_DES_1_Pos           0
#define ETM_TRCPIDR2_DES_1_Msk           (0x7UL << ETM_TRCPIDR2_DES_1_Pos)
#define ETM_TRCPIDR2_REVISION_Pos        4
#define ETM_TRCPIDR2_REVISION_Msk        (0xFUL << ETM_TRCPIDR2_REVISION_Pos)
#define ETM_TRCPIDR3_CMOD_Pos            0
#define ETM_TRCPIDR3_CMOD_Msk            (0xFUL << ETM_TRCPIDR3_CMOD_Pos)
#define ETM_TRCPIDR3_REVAND_Pos          4
#define ETM_TRCPIDR3_REVAND_Msk          (0xFUL << ETM_TRCPIDR3_REVAND_Pos)
#define ETM_TRCLAR_KEY_UNLOCK            0xC5ACCE55UL

/* --------  Cortex-M7 CTI (Cross Trigger Interface)  --------------------------- */
typedef struct
{
    __IO uint32_t CONTROL;
    uint32_t RESERVED0[3];
    __O  uint32_t INTACK;
    __IO uint32_t APPSET;
    __O  uint32_t APPCLR;
    __O  uint32_t APPPULSE;
    __IO uint32_t INEN0;
    uint32_t RESERVED1[31];
    __IO uint32_t OUTEN0;
    uint32_t RESERVED2;
    __IO uint32_t OUTEN2;
    __IO uint32_t OUTEN3;
    __IO uint32_t OUTEN4;
    __IO uint32_t OUTEN5;
    uint32_t RESERVED3;
    __IO uint32_t OUTEN7;
    uint32_t RESERVED4[28];
    __I  uint32_t TRIGINSTATUS;
    __I  uint32_t TRIGOUTSTATUS;
    __I  uint32_t CHINSTATUS;
    __I  uint32_t CHOUTSTATUS;
    __IO uint32_t GATE;
    uint32_t RESERVED5[870];
    __O  uint32_t ITCHINACK;
    __O  uint32_t ITTRIGINACK;
    __O  uint32_t ITCHOUT;
    __O  uint32_t ITTRIGOUT;
    __I  uint32_t ITCHOUTACK;
    __I  uint32_t ITTRIGOUTACK;
    __I  uint32_t ITCHIN;
    __I  uint32_t ITTRIGIN;
    uint32_t RESERVED6;
    __IO uint32_t ITCTRL;
    uint32_t RESERVED7[39];
    __IO uint32_t CLAIMSET;
    __IO uint32_t CLAIMCLR;
    __I  uint32_t DEVAFF0;
    __I  uint32_t DEVAFF1;
    __O  uint32_t LOCKACCESS;
    __I  uint32_t LOCKSTATUS;
    __I  uint32_t AUTHSTATUS;
    __I  uint32_t DEVARCH;
    __I  uint32_t DEVID2;
    __I  uint32_t DEVID1;
    __I  uint32_t DEVID;
    __I  uint32_t DEVTYPE;
    __I  uint32_t PID4;
    __I  uint32_t PID5;
    __I  uint32_t PID6;
    __I  uint32_t PID7;
    __I  uint32_t PID0;
    __I  uint32_t PID1;
    __I  uint32_t PID2;
    __I  uint32_t PID3;
    __I  uint32_t CID0;
    __I  uint32_t CID1;
    __I  uint32_t CID2;
    __I  uint32_t CID3;
} CTI_Type;

/* TPIU_Type, TPIU_BASE, TPIU are defined in CMSIS 6 core_cm7.h */
#define TPIU_FFCR_STOPF1_Pos      12
#define TPIU_FFCR_STOPF1_MSK      (0x1UL << TPIU_FFCR_STOPF1_Pos)
#define TPIU_FFSR_FLINPROG_Pos    0
#define TPIU_FFSR_FLINPROG_MSK    (0x1UL << TPIU_FFCR_STOPF1_Pos)
#define TPIU_PIN_TRACEPORT        0
#define TPIU_PIN_SWO_MANCHESTER   1
#define TPIU_PIN_SWO_NRZ          2

/* --------  GPIO (Device-specific)  ---------------------------------------------- */
typedef union
{
    __IO uint32_t WORD;
    __IO uint16_t HALFWORD[2];
    __IO uint8_t  BYTE[4];
} GPIO_Data_TypeDef;

typedef struct
{
    GPIO_Data_TypeDef DATA[256];
    GPIO_Data_TypeDef DIR;
    uint32_t RESERVED[3];
    GPIO_Data_TypeDef IE;
} GPIO_TypeDef;


/******************************************************************************/
/*                         Peripheral memory map                              */
/******************************************************************************/

/* TPIU_BASE defined in CMSIS 6 core_cm7.h */
#define ETM_BASE                  0xE0041000UL
#define CTI_BASE                  0xE0042000UL
#define FPB_BASE                  0xE0002000UL

#define DWT_CTRL_SYNCTAP24        (1 << 10)
#define DWT_CTRL_SYNCTAP26        (2 << 10)
#define DWT_CTRL_SYNCTAP28        (3 << 10)
#define DWT_FUNC_SAMP_PC          0x1
#define DWT_FUNC_SAMP_DATA        0x2
#define DWT_FUNC_SAMP_PC_DATA     0x3
#define DWT_FUNC_PC_WPT           0x4
#define DWT_FUNC_TRIG_PC          0x8
#define DWT_FUNC_TRIG_RD          0x9
#define DWT_FUNC_TRIG_WR          0xA
#define DWT_FUNC_TRIG_RW          0xB
#define DWT_CTRL_POSTPRESET_10    0xA

#define ITM_TER_STIM0             (1 << 0)
#define ITM_TER_STIM1             (1 << 1)
#define ITM_TER_STIM2             (1 << 2)
#define ITM_TCR_TS_GLOBAL_128     (0x01U << ITM_TCR_GTSFREQ_Pos)
#define ITM_TCR_TS_GLOBAL_8192    (0x10U << ITM_TCR_GTSFREQ_Pos)
#define ITM_TCR_TS_GLOBAL_ALL     (0x11U << ITM_TCR_GTSFREQ_Pos)

#define SRAM_BASE                 0x20000000UL
#define PERIPH_BASE               0x40000000UL
#define GPIO_BASE                 PERIPH_BASE
#define GPIO0_BASE                (GPIO_BASE)
#define GPIO1_BASE                (GPIO_BASE + 0x0800UL)
#define GPIO2_BASE                (GPIO_BASE + 0x1000UL)


/* ========================================================================= */
/* ============             Peripheral declaration              ============ */
/* ========================================================================= */

#define ETM                       ((ETM_Type*)ETM_BASE)
/* TPIU defined in CMSIS 6 core_cm7.h */
#define CTI                       ((CTI_Type*)CTI_BASE)
#define GPIO0                     ((GPIO_TypeDef*)GPIO0_BASE)
#define GPIO1                     ((GPIO_TypeDef*)GPIO1_BASE)
#define GPIO2                     ((GPIO_TypeDef*)GPIO2_BASE)

#ifdef __cplusplus
}
#endif

#endif /* CM7IKMCU_H */