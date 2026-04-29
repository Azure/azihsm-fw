// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2026 Marvell

//=============================================================================
//
//! @file   vicommon.c
//! @brief  for vic and interrupt setting
//!
//=============================================================================
#include "vicommon.h"
#include "MessageHandler.h"
#include "M7MemMap.h"

#if defined (fps_cpu0Core) || defined (fps_cpu2Core) || defined (fps_cpu1Core) || defined (fps_cpu3Core) || defined (fps_cpu4Core) || defined (fps_cpu5Core) || defined (fps_cpu6Core) || defined (fps_cpu7Core) || defined (cp_cpu0Core) || defined (cp_cpu1Core)

#include "cm7ikmcu.h"
#include "core_cm7.h"
#else
#include "cr5ikmcu.h"
#endif
/**
 *  @brief VIC IRQ enable
 *
 *    @param[in]   irq - interrupt number
 *
 *  @return none
 */
void VicIrqEnable (uint32_t irq)
{

    #if defined (fps_cpu0Core) || defined (fps_cpu2Core)  || defined (fps_cpu1Core) || defined (fps_cpu3Core) || defined (fps_cpu4Core) || defined (fps_cpu5Core) || defined (fps_cpu6Core) || defined (fps_cpu7Core)  || defined (cp_cpu0Core) || defined (cp_cpu1Core)

    NVIC_EnableIRQ(irq);
    #else
    if (irq < 32)
    {
        vic0REG->VICIntEnable = 1 << irq;
    }
    else if (irq < 64)
    {
        vic1REG->VICIntEnable = 1 << (irq - 32);
    }
    else
    {
        vic2REG->VICIntEnable = 1 << (irq - 64);
    }
    #endif
}
/**
 *  @brief VIC IRQ Disable
 *
 *    @param[in]   irq - interrupt number
 *
 *  @return none
 */
void VicIrqDisable (uint32_t irq)
{

    #if defined (fps_cpu0Core) || defined (fps_cpu2Core) || defined (fps_cpu1Core) || defined (fps_cpu3Core) || defined (fps_cpu4Core) || defined (fps_cpu5Core) || defined (fps_cpu6Core) || defined (fps_cpu7Core)  || defined (cp_cpu0Core) || defined (cp_cpu1Core)

    NVIC_DisableIRQ(irq);
    #else
    if (irq < 32)
    {
        vic0REG->VICIntEnClear = 1 << irq;
    }
    else if (irq < 64)
    {
        vic1REG->VICIntEnClear = 1 << (irq - 32);
    }
    else
    {
        vic2REG->VICIntEnClear = 1 << (irq - 64);
    }
    #endif
}
/**
 *  @brief VIC IRQ Disable ALL
 *
 *    @param[in]   none
 *
 *  @return none
 */
void VicIrqDisableAll(void)
{
    #if defined (fps_cpu0Core) || defined (fps_cpu2Core) || defined (fps_cpu1Core) || defined (fps_cpu3Core) || defined (fps_cpu4Core) || defined (fps_cpu5Core) || defined (fps_cpu6Core) || defined (fps_cpu7Core)  || defined (cp_cpu0Core) || defined (cp_cpu1Core)

    NVIC->ICER[0] = 0xffffffff;
    NVIC->ICER[1] = 0xffffffff;
    NVIC->ICER[2] = 0xffffffff;
    NVIC->ICER[3] = 0xffffffff;
    #else
    vic0REG->VICIntEnClear = 0xffffffff;
    vic1REG->VICIntEnClear = 0xffffffff;
    vic2REG->VICIntEnClear = 0xffffffff;
    #endif

}

/**
 *  @brief Get IPC pending
 *
 *    @param[in]   cpuIdx : CPU index
 *
 *  @return with pending value
 */
uint32_t GetIpcPending(uint32_t cpuIdx)
{
    uint32_t value;
    // value = *(uint32_t *)(INT_PEND_SET_0 + IPC_CPU_Idx >>2);
    value = vicreadl(INT_PEND_SET_0 + (cpuIdx << 2));
    return value;
}
/**
 *  @brief Set IPC Enable
 *
 *    @param[in]  IPC_CPU_Idx : CPU index , DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcIntEnableSet(uint32_t cpuIdx, uint32_t DescIdx)
{
    //*((uint32_t *)(INT_ENAB_SET_0 + cpuIdx << 2)) |= (1<<(DescIdx));
    uint32_t addr = (INT_ENAB_SET_0 + (cpuIdx << 2));
    vicwritel((0x1UL << (DescIdx)), addr);
}
/**
 *  @brief Trigger IPC descriptor
 *
 *    @param[in]  DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcDescTrigger(uint32_t DescIdx, uint32_t EnableVal)
{
    //*((uint32_t *)(DESC_REG_00 + Src_IPC_CPU_Idx >>2)) = 1;
    vicwritel(EnableVal, (DESC_REG_00 + (DescIdx << 2)));
}
/**
 *  @brief Trigger IPC descriptor with specific value
 *
 *    @param[in]  DescIdx : Descriptor index
 *    @param[in]  value : Value to write to descriptor register
 *
 *  @return none
 */
void IpcDescTriggerWithValue(uint32_t DescIdx, uint32_t value)
{
    //*((uint32_t *)(DESC_REG_00 + Src_IPC_CPU_Idx >>2)) = 1;
    vicwritel(value, (DESC_REG_00 + (DescIdx << 2)));
}

/**
 *  @brief Clear IPC pending
 *
 *    @param[in]  cpuIdx : CPU index , DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcIntPendingClr(uint32_t cpuIdx, uint32_t DescIdx)
{
    //*((uint32_t *)(INT_PEND_CLR_0 + IPC_CPU_Idx >>2)) = (1<<(Desc_Idx));
    vicwritel((0x1UL << (DescIdx)), (INT_PEND_CLR_0 + (cpuIdx << 0x2UL)));
}
/**
 *  @brief Clear IPC enable
 *
 *    @param[in]  IPC_CPU_Idx : CPU index , DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcIntEnableClr(uint32_t cpuIdx, uint32_t DescIdx)
{
    //*((uint32_t *)(INT_ENAB_CLR_0 + IPC_CPU_Idx >>2)) = (1<<(Desc_Idx));
    vicwritel((0x1UL << (DescIdx)), (INT_ENAB_CLR_0 + (cpuIdx << 0x2UL)));
}

/**
 *  @brief Set IPC Mask
 *
 *    @param[in]  IPC_CPU_Idx : CPU index , DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcIntMaskSet(uint32_t cpuIdx, uint32_t DescIdx)
{
    vicwritel((0x1UL << (DescIdx)), (INT_MASK_SET_0 + (cpuIdx << 0x2UL)));
}

/**
 *  @brief Clear IPC Mask
 *
 *    @param[in]  IPC_CPU_Idx : CPU index , DescIdx : Descriptor index
 *
 *  @return none
 */
void IpcIntMaskClr(uint32_t cpuIdx, uint32_t DescIdx)
{
    vicwritel((0x1UL << (DescIdx)), (INT_MASK_CLR_0 + (cpuIdx << 0x2UL)));
}

/**
 *  @brief Get value of IPC descriptor
 *
 *    @param[in]  DescIdx : Descriptor index
 *
 *  @return Value of IPC descriptor of given index
 */
uint32_t IpcIntGetDescValue(uint32_t DescIdx)
{
    return vicreadl((DESC_REG_00 + (DescIdx << 2)));
}

/**
 *  @brief Get interrupt active
 *
 *    @param[in]  irq : Interrupt number
 *
 *  @return  0:  Interrupt status is not active.
 *            1:  Interrupt status is active.
 */
bool GetIrqActive(uint32_t irq)
{
    uint32_t temp;
    uint8_t idx = (irq >> 5);
    temp  = NVIC->IABR[idx];
    temp  = temp >> (irq & 0x1FUL);
    temp &= 0x1UL;

    return temp;
}
/**
 *  @brief Get pending interrupt
 *
 *    @param[in]  irq : Interrupt number
 *
 *  @return 0:  Interrupt status is not pending.
 *             1:  Interrupt status is pending.
 */
uint32_t VicIrqGetPending(uint32_t irq)
{
    #if defined (fps_cpu0Core) || defined (fps_cpu2Core) ||  defined (fps_cpu1Core) || defined (fps_cpu3Core) || defined (fps_cpu4Core) || defined (fps_cpu5Core) || defined (fps_cpu6Core) || defined (fps_cpu7Core)  || defined (cp_cpu0Core) || defined (cp_cpu1Core)

    return NVIC_GetPendingIRQ(irq);
    #else
    if (irq < 32)
    {
        return (uint32_t)(((vic0REG->VICRawIntr) & (1 << irq)) ? 1 : 0);
    }
    else if (irq < 64)
    {
        return (uint32_t)(((vic1REG->VICRawIntr) & (1 << (irq - 32))) ? 1 : 0);
    }
    else
    {
        return (uint32_t)(((vic2REG->VICRawIntr) & (1 << (irq - 64))) ? 1 : 0);
    }
    #endif
}

/**
 *  @brief  Clear Pending Interrupt
 *
 *    @param[in]  irq : Interrupt number
 *
 *  @return none
 */
void ClrPendingIrq(uint32_t irq)
{
    uint8_t idx = (irq >> 5);
    NVIC->ICPR[idx] = 0x1UL << (irq & 0x1FUL);
}

/**
 *  @brief     Calculate checksum
 *
 *  @param     uint32_t* pData - data pointer
 *             uint32_t len -  data length
 *             uint8_t check - return checksum based on check value
 *
 *  @return    checksum value
 */
uint32_t CalCheckSum(uint32_t* pData, uint32_t len, uint8_t check)
{
    uint32_t sum = 0;
    uint32_t* p = (uint32_t*)pData;
    uint32_t i = 0;
    for (i = 0; i < (len / 4); i++)
    {
        sum += p[i];
    }

    if (check)
    {
        return sum;
    } // else do nothing

    sum = (~sum) + 1;
    return sum;
}

/**
 *  @brief  Check backup data
 *
 *  @param[in]  none
 *
 *  @return status
 */
FwUdSts CheckBackupData(void)
{
    uint32_t backupDataHeaderAddr = (uint32_t)PSRAM_BACKUP_DATA_HEADER_ADDR;
    FwUpdateDataHeader* pDataHeader = (FwUpdateDataHeader*)backupDataHeaderAddr;
    if (pDataHeader->signature != FW_UPDATE_SIGNATURE) // this will be from old structure to new
    {
        return cNoSignature;
    } // else do nothing

    uint32_t checksum = CalCheckSum((uint32_t*)pDataHeader, pDataHeader->totalDataLength, 1);
    if (checksum)
    {
        return cChkSumFail;
    } // else do nothing

    return cNewVer;
}

/**
 *  @brief     Get Firmware update status
 *
 *  @param     FwUdSts FwSts - Checsum Status
 *
 *  @return    Firmware update status
 */
uint8_t GetFwUpdateStatus(FwUdSts FwSts)
{
    uint8_t FwUpdateStatus = msgSuccess;
    switch (FwSts)
    {
        case cNewVer:
        {
            break;
        }
        case cNoSignature:
        {
            FwUpdateStatus = msgFwUpdateDataFail;
            // TBD: Error level logging
            break;
        }
        case cChkSumFail:
        {
            FwUpdateStatus = msgFwUpdateDataFail;
            // TBD: Error level logging
            break;
        }
        default:
        {
            FwUpdateStatus = msgInvalidField;
            break;
        }
    }
    return FwUpdateStatus;
}
