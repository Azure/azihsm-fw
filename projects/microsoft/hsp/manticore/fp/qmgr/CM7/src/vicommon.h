// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   vicommon.h
//! @brief  vic and interrupt header
//!
//=============================================================================
#ifndef COMMON_H
#define COMMON_H
#pragma once

#include "SysTypes.h"
#include <stdint.h>
#include <stdbool.h>
#include "FpCommon.h"

// define PLATFORM
#define CHIP 0x1
#define NRAE 0x2
//In INTC, IPC registers definitions
#define INT_PEND_CLR_0 0xB0006000 //CPU0 - CPU5 : CP0,CP1, FP0,FP1,FP2, HSP
#define INT_PEND_SET_0 0xB0006018 //CPU0 - CPU5 : CP0,CP1, FP0,FP1,FP2, HSP
#define INT_MASK_CLR_0 0xB0006030 //CPU0 - CPU5 : CP0,CP1, FP0,FP1,FP2, HSP
#define INT_MASK_SET_0 0xB0006048 //CPU0 - CPU5 : CP0,CP1, FP0,FP1,FP2, HSP
#define INT_ENAB_CLR_0 0xB0006060 //CPU0 - CPU5 : CP0,CP1, FP0,FP1,FP2, HSP
#define INT_ENAB_SET_0 0xB0006078 //CPU0 - CPU5 : CP0,CP1, FP0,FP1,FP2, HSP
#define DESC_00_CNT    0xB0006090
#define DESC_01_CNT    0xB0006094
#define DESC_02_CNT    0xB0006098
#define DESC_03_CNT    0xB000609C
#define DESC_31_CNT    0xB000610C
#define DESC_REG_00    0xB0006110 //0 - 31

#define CP0toFP_REQ_DESC_REG 0xB0006110
#define FPtoCP0_RESP_DESC_REG 0xB0006114
#define CPU0toCPU1_DESC_REG 0xB0006118
#define CPU0toCPU2_DESC_REG 0xB000611C
#define CPU1toCPU0_DESC_REG 0xB0006120
#define CPU1toCPU2_DESC_REG 0xB0006124
#define CPU2toCPU0_DESC_REG 0xB0006128
#define CPU2toCPU1_DESC_REG 0xB000612C

#define CP0_INT_NUM 128
#define CP1_INT_NUM 129
#define FP0_INT_NUM 130
#define FP1_INT_NUM 131
#define FP2_INT_NUM 132
#define HSP_INT_NUM 133
#define CDMA_INT_1_NUM 59
#define TCON_INT_WAKE_TIMER_0_NUM 88
#define TCON_INT_WAKE_TIMER_1_NUM 89

#define IPC_TRIGGER_VAL 0xFF
typedef enum IPC_CPU_INDX
{
    IPC_CP0 = 0,
    IPC_CP1 = 1,
    IPC_FP0 = 2,
    IPC_FP1 = 3,
    IPC_FP2 = 4,
    IPC_HSP = 5,
} IPC_CPU_INDX_t;

typedef enum IPC_DESC
{
    CP0toFP_REQ_DESC = 0,
    FPtoCP0_RES_DESC = 1,
    CPU0toCPU1_DESC = 2,
    CPU0toCPU2_DESC = 3,
    CPU1toCPU0_DESC = 4,
    CPU1toCPU2_DESC = 5,
    CPU2toCPU0_DESC = 6,
    CPU2toCPU1_DESC = 7,
    IN_USE_BY_CP = 8,
    // UNUSED DESC= 9,
    FPtoCP1_REQ_DESC = 10,
    CP1toFP_RES_DESC = 11,
    // CP1 TO CP0 REQUEST DESC = 12,
    // CP0 TO CP1 RESPONSE DESC = 13,
    UPDATE_TIMESTAMP_CPU2_TO_3CPU = 14,
    CP1toFP_REQ_DESC = 15,
    FPtoCP1_RES_DESC = 16,
    ResetCP2FP = 17, // CP0 to FPS Reset Request
    ResetFP2CP = 18, // FPS to CP0 Reset Response
    // HSM to CP0 CRASHDUMP REQUEST = 19,
    // CP0 to HSP REQUEST = 20,
    // HSP to CP0 REPONSE = 21,
    // HSP to CP0 REQUEST = 22,
    // CP0 to HSP RESPONSE = 23,
    // CP1 to HSP REQUEST = 24,
    // HSP to CP1 RESPONSE = 25,
    FPtoCP0_REQ_DESC = 26,
    CP0toFP_RES_DESC = 27,
    // CP0 to CP1 RESET REQUES = 28,
    // CP1 to CP0 RESET RESPONSE = 29,
    // CP0 to CP1 REQUEST = 30,
    // CP1 to CP0 RESPONSE = 31,
} IPC_DESC_t;

typedef enum FIBER_RESUME
{
    CP2FPMSG_FIBER = 0,
    FP2FPMSG_FIBER = 1,
    FPALIVE_FIBER = 2,
    CDMA_FIBER = 3,
    RESET_FIBER = 4,
} FIBER_RESUME_t;

typedef enum ResetType_t
{
    cPor = 0,                  // Power On Reset
    cWarmReset = 1,            // Warm Reset, issued during any fault recovery
    cFwUpdateWarmReset = 2,    // Firmware Update Warm Reset
    cInvalidResetType = 0xFF,
}ResetType_t;

/*
 * Linux style IOs
 */
#define vicwriteb(data, addr) (*((volatile uint8_t*)(addr)) = (uint8_t)(data))
#define vicwritew(data, addr) (*((volatile uint16_t*)(addr)) = (uint16_t)(data))
#define vicwritel(data, addr) (*((volatile uint32_t*)(addr)) = (uint32_t)(data))
#define vicwriteq(data, addr) (*((volatile uint64_t*)(addr)) = (uint64_t)(data))

#define vicreadb(addr) (*((volatile uint8_t*)(addr)))
#define vicreadw(addr) (*((volatile uint16_t*)(addr)))
#define vicreadl(addr) (*((volatile uint32_t*)(addr)))
#define vicreadq(addr) (*((volatile uint64_t*)(addr)))


/*
 * GNU Pre-processor macors
 */
#define stringify(s)    (#s)

typedef uint32_t phys_addr_t;

static inline void DSB(void)
{
    __asm volatile ("dsb");
}

/*
 * CPUID
 */
enum platform_cpuid
{
    CCP_CPU = 0xC271,
    DQP_CPU = 0xC271,
    MEP_CPU = 0xC153,
};


/**
 *  @brief VIC IRQ enable
 *
 *    @param[in]   irq - interrupt number
 *
 *  @return none
 */
void VicIrqEnable(uint32_t irq);
/**
 *  @brief VIC IRQ Disable
 *
 *    @param[in]   irq - interrupt number
 *
 *  @return none
 */
void VicIrqDisable(uint32_t irq);
/**
 *  @brief VIC IRQ Disable ALL
 *
 *    @param[in]   none
 *
 *  @return none
 */
void VicIrqDisableAll(void);
/**
 *  @brief Get IPC pending
 *
 *    @param[in]   cpuIdx : CPU index
 *
 *  @return with pending value
 */
uint32_t GetIpcPending(uint32_t IPC_CPU_Idx);
/**
 *  @brief Set IPC Enable
 *
 *    @param[in]  IPC_CPU_Idx : CPU index , DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcIntEnableSet(uint32_t IPC_CPU_Idx, uint32_t Desc_Idx);
/**
 *  @brief Trigger IPC descriptor
 *
 *    @param[in]  DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcDescTrigger(uint32_t DescIdx, uint32_t EnableVal);
/**
 *  @brief Trigger IPC descriptor with specific value
 *
 *    @param[in]  DescIdx : Descriptor index
 *    @param[in]  value : Value to write to descriptor register
 *
 *  @return none
 */
void IpcDescTriggerWithValue(uint32_t DescIdx, uint32_t value);

/**
 *  @brief Clear IPC pending
 *
 *    @param[in]  cpuIdx : CPU index , DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcIntPendingClr(uint32_t IPC_CPU_Idx, uint32_t Desc_Idx);
/**
 *  @brief Clear IPC enable
 *
 *    @param[in]  IPC_CPU_Idx : CPU index , DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcIntEnableClr(uint32_t IPC_CPU_Idx, uint32_t Desc_Idx);
/**
 *  @brief Set IPC Mask
 *
 *    @param[in]  IPC_CPU_Idx : CPU index , DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcIntMaskSet(uint32_t IPC_CPU_Idx, uint32_t Desc_Idx);
/**
 *  @brief Clear IPC Mask
 *
 *    @param[in]  IPC_CPU_Idx : CPU index , DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcIntMaskClr(uint32_t IPC_CPU_Idx, uint32_t Desc_Idx);

/**
 *  @brief Get value of IPC descriptor
 *
 *    @param[in]  DescIdx : Descriptor index
 *
 *  @return Value of IPC descriptor of given index
 */
uint32_t IpcIntGetDescValue(uint32_t DescIdx);

/**
 *  @brief Get interrupt active
 *
 *    @param[in]  irq : Interrupt number
 *
 *  @return  0:  Interrupt status is not active.
 *            1:  Interrupt status is active.
 */
bool GetIrqActive(uint32_t ID);
/**
 *  @brief Get pending interrupt
 *
 *    @param[in]  irq : Interrupt number
 *
 *  @return 0:  Interrupt status is not pending.
 *             1:  Interrupt status is pending.
 */
uint32_t VicIrqGetPending(uint32_t irq);
/**
 *  @brief  Clear Pending Interrupt
 *
 *    @param[in]  irq : Interrupt number
 *
 *  @return none
 */
void ClrPendingIrq(uint32_t ID);

/**
 *  @brief     Calculate check sum
 *
 *  @param     uint32_t* pData - Calculated data pointer
 *             uint32_t len -  data length
 *             uint8_t check - return checksum based on check value
 *
 *  @return    checksum value
 */
uint32_t CalCheckSum(uint32_t* pData, uint32_t len, uint8_t check);

/**
 *  @brief  Check backup data
 *
 *  @param[in]  none
 *
 *  @return status
 */
FwUdSts CheckBackupData(void);

/**
 *  @brief     Get Firmware update status
 *
 *  @param     FwUdSts FwSts - Checsum Status
 *
 *  @return    Firmware update status
 */
uint8_t GetFwUpdateStatus(FwUdSts FwSts);

#endif
