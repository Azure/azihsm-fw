// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Marvell

#include "HalFps.h"
#include "M7MemMap.h"

/**
 *  @brief  Configure Fps Memory Control Register
 *          Set the following:
 *              - ECC_CORRECTABLE_ERROR_COUNT_THRESHOLD
 *              - CPU_TCM_ECC_CORRECTABLE_ERROR_WRTBK_EN
 *              - PSRAM_ECC_CORRECTABLE_ERROR_WRTBK_EN
 *              - PSRAM_ECC_PARTIAL_WRITE_RMW_EN
 *  @param  threshold
 *  @return Error_t.
 */
Error_t HalFps_ConfigureFpsMemoryControlRegister(FpsMemCtrLCorrThreshold_t threshold)
{
    Error_t status = cEcNoError;
    Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
    if(threshold == cFPS_MEM_CTRL_CORR_THRESHOLD_0)
    {
        pFps->fpsBank1RegRegisters.fpsBank1FpsMemoryControl.all = 0; // reset all
        pFps->fpsBank1RegRegisters.fpsBank1FpsMemoryControl.b.ECC_CORRECTABLE_ERROR_COUNT_THRESHOLD = threshold;
        pFps->fpsBank1RegRegisters.fpsBank1FpsMemoryControl.b.CPU_TCM_ECC_CORRECTABLE_ERROR_WRTBK_EN = 1;
        pFps->fpsBank1RegRegisters.fpsBank1FpsMemoryControl.b.PSRAM_ECC_CORRECTABLE_ERROR_WRTBK_EN = 1;
        pFps->fpsBank1RegRegisters.fpsBank1FpsMemoryControl.b.PSRAM_ECC_PARTIAL_WRITE_RMW_EN = 1;
        pFps->fpsBank1RegRegisters.fpsBank1FpsMemoryControl.b.ELEVATE_ECC_CORRECTABLE_ERROR2FATAL = 1;
    }
    else if (threshold <= cFPS_MEM_CTRL_CORR_THRESHOLD_INVALID)
    {
        pFps->fpsBank1RegRegisters.fpsBank1FpsMemoryControl.b.ECC_CORRECTABLE_ERROR_COUNT_THRESHOLD = threshold;
    }
    else
    {
        status = cEcError;
    }
    return status;
}

/**
 *  @brief  Init Fps Control Register
 *          Sets the following:
 *              - FPS_ERROR_CLR
 *              - PARITY_GEN_ODD
 *              - PARITY_CHECK_ODD
 *  @param  None
 *  @return  None
 */
void HalFps_InitFpsControlRegister(void)
{
    Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
    pFps->fpsBank0RegRegisters.fpsBank0FpsControl.all = 0; // reset All
    pFps->fpsBank0RegRegisters.fpsBank0FpsControl.b.FPS_ERROR_CLR = 1;
    pFps->fpsBank0RegRegisters.fpsBank0FpsControl.b.PARITY_GEN_ODD = 1;
    pFps->fpsBank0RegRegisters.fpsBank0FpsControl.b.PARITY_CHECK_ODD = 1;
}

/**
 *  @brief  Enable Fabric Parity
 *          Sets the following:
 *              - FABRIC_PSRAM0_S_PORT_WR_PARITY_CHECK_ENABLE
 *              - FABRIC_CPU0_M_PORT_PARITY_CHECK_ENABLE
 *              - FABRIC_CPU1_M_PORT_PARITY_CHECK_ENABLE
 *              - FABRIC_CPU2_M_PORT_PARITY_CHECK_ENABLE
 *              - FABRIC_CPU0_S_PORT_PARITY_CHECK_ENABLE
 *              - FABRIC_CPU1_S_PORT_PARITY_CHECK_ENABLE
 *              - FABRIC_CPU2_S_PORT_PARITY_CHECK_ENABLE
 *              - FABRIC_REG_M_PORT_PARITY_CHECK_ENABLE
 *              - FABRIC_REG_S_PORT_PARITY_CHECK_ENABLE
 *              - FABRIC_REG_M_PORT_RESPONSE_CHECK_ENABLE
 *              - FABRIC_MAIN_AXI_PORT_PARITY_CHECK_ENABLE
 *  @param  None
 *  @return None
 */
void HalFps_EnableFabricParity(void)
{
    Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.all = 0; // reset all
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_PSRAM0_S_PORT_WR_PARITY_CHECK_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_CPU0_M_PORT_PARITY_CHECK_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_CPU1_M_PORT_PARITY_CHECK_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_CPU2_M_PORT_PARITY_CHECK_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_CPU0_S_PORT_PARITY_CHECK_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_CPU1_S_PORT_PARITY_CHECK_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_CPU2_S_PORT_PARITY_CHECK_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_REG_M_PORT_PARITY_CHECK_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_REG_S_PORT_PARITY_CHECK_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_REG_M_PORT_RESPONSE_CHECK_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1FabricErrorControl.b.FABRIC_MAIN_AXI_PORT_PARITY_CHECK_ENABLE = 1;
}

/**
 *  @brief  Fps Error clear
 *          Sets the following:
 *              - FPS_ERROR_CLR
 *  @param  None
 *  @return  None
 */
void HalFps_ClearFpsError(void)
{
    Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
    pFps->fpsBank0RegRegisters.fpsBank0FpsControl.b.FPS_ERROR_CLR = 1;
}

/**
 *  @brief  Init PSRAM Memory Control 
 *          Sets the following:
 *              - PSRAM_RTC
 *              - PSRAM_ECC_PROTECTION_MODE_ENABLE
 *              - PSRAM_INIT_EN
 *              - PSRAM_SCRUB_EN
 *              - PSRAM_PROTECTION_CHECK_ENABLE
 *  @return None
 */
void HalFps_InitPsramMemoryControl()
{
    Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
    pFps->fpsBank1RegRegisters.fpsBank1Psram0MemoryControl.all = 0; // reset all
    pFps->fpsBank1RegRegisters.fpsBank1Psram0MemoryControl.b.PSRAM_RTC = 1;
    pFps->fpsBank1RegRegisters.fpsBank1Psram0MemoryControl.b.PSRAM_ECC_PROTECTION_MODE_ENABLE = 1;
    pFps->fpsBank1RegRegisters.fpsBank1Psram0MemoryControl.b.PSRAM_INIT_EN = 1;
    pFps->fpsBank1RegRegisters.fpsBank1Psram0MemoryControl.b.PSRAM_SCRUB_EN = 1;
    pFps->fpsBank1RegRegisters.fpsBank1Psram0MemoryControl.b.PSRAM_PROTECTION_CHECK_ENABLE = 0;
}

/**
 *  @brief  Enable / disable PSRAM Protection Check
 *          When set, enables data protection (ECC or parity) checks; When clear, disables data protection checks and resets error status
 *          Sets the following:
 *              - PSRAM_PROTECTION_CHECK_ENABLE
 *  @param  Enable / Disable
 *  @return None
 */
void HalFps_EnablePsramProtectionCheck(bool enable)
{
    Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
    pFps->fpsBank1RegRegisters.fpsBank1Psram0MemoryControl.b.PSRAM_PROTECTION_CHECK_ENABLE = enable;
}

/**
 *  @brief  Set / Clear PSRAM Protection Write Disabled bit
 *          Sets or clears PSRAM_PROTECTION_WRITE_DISABLE
 *          When set disables writes to the protection location in memory.
 *  @param  Set / Clear
 *  @return None
 */
void HalFps_SetDisablePsramWriteProtectionCheck(bool isEnable)
{
    Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
    pFps->fpsBank1RegRegisters.fpsBank1Psram0MemoryControl.b.PSRAM_PROTECTION_WRITE_DISABLE = isEnable;
}

/**
 *  @brief  Set / Clear TCM Protection Write Disabled bit
 *          Sets or clears either CPU_DTCM_PROTECTION_WRITE_DISABLE or CPU_ITCM_PROTECTION_WRITE_DISABLE
 *  @param  Set / Clear
 *  @return None
 */
void HalFps_SetDisableTcmWriteProtectionCheck(bool protCheckEnable, bool isDtcm)
{
    Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
    if(isDtcm)
    {
        pFps->fpsBank1RegRegisters.fpsBank1FpsMemoryControl.b.CPU_DTCM_PROTECTION_WRITE_DISABLE = protCheckEnable;
    }
    else
    {
        pFps->fpsBank1RegRegisters.fpsBank1FpsMemoryControl.b.CPU_ITCM_PROTECTION_WRITE_DISABLE = protCheckEnable;
    }
}

/**
 *  @brief  Get Memory Error Status
 *  @param  One fo the following:
 *              - cCPU0_MEMORY_ERROR_BIT
 *              - cCPU1_MEMORY_ERROR_BIT
 *              - cCPU2_MEMORY_ERROR_BIT
 *              - cPSRAM0_PROTECTION_ERROR_BIT
 *              - cPSRAM0_ECC_CORRECTABLE_ERROR_BIT
 *  @return bool
 */
bool HalFps_GetMemoryErrorStatus(FpsMemErrorStatus_t bit)
{
    bool retval = false;
    Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
    switch(bit)
    {
        case cCPU0_MEMORY_ERROR_BIT: retval = pFps->fpsBank1RegRegisters.fpsBank1MemoryErrorStatus.b.CPU0_MEMORY_ERROR;
                                 break;
        case cCPU1_MEMORY_ERROR_BIT: retval = pFps->fpsBank1RegRegisters.fpsBank1MemoryErrorStatus.b.CPU1_MEMORY_ERROR;
                                 break;
        case cCPU2_MEMORY_ERROR_BIT: retval = pFps->fpsBank1RegRegisters.fpsBank1MemoryErrorStatus.b.CPU2_MEMORY_ERROR;
                                 break;
        case cPSRAM0_PROTECTION_ERROR_BIT: retval = pFps->fpsBank1RegRegisters.fpsBank1MemoryErrorStatus.b.PSRAM0_PROTECTION_ERROR;
                                 break;
        case cPSRAM0_ECC_CORRECTABLE_ERROR_BIT: retval = pFps->fpsBank1RegRegisters.fpsBank1MemoryErrorStatus.b.PSRAM0_ECC_CORRECTABLE_ERROR;
                                 break;
        default: break;
    }
    return retval;
}

/**
 *  @brief  Init TCM Protection Check where core is FP0(0), FP1(1), FP2(2)
 *          Sets the following:
 *              - CPU_ITCM_RTC
 *              - CPU_BANK0_D0TCM_RTC
 *              - CPU_BANK0_D1TCM_RTC
 *              - CPU_BANK1_D0TCM_RTC
 *              - CPU_BANK1_D0TCM_WTC
 *              - CPU_BANK1_D1TCM_RTC
 *              - CPU_BANK1_D1TCM_WTC
 *              - CPU_DTCM_ECC_PROTECTION_MODE_ENABLE
 *              - CPU_ITCM_ECC_PROTECTION_MODE_ENABLE
 *              - CPU_DTCM_PROTECTION_CHECK_ENABLE
 *              - CPU_ITCM_PROTECTION_CHECK_ENABLE
 *  @param  core Id <0|1|2>
 *  @return Error_t
 */
Error_t HalFps_InitCpuMemoryControl(CoreId_t coreID)
{
    Error_t retval = cEcNoError;
    if(coreID < cNumberOfCores)
    {
        Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.all = 0; // reset all
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_ITCM_RTC = 1;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_BANK0_D0TCM_RTC = 1;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_BANK0_D1TCM_RTC = 1;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_BANK1_D0TCM_RTC = 1;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_BANK1_D0TCM_WTC = 1;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_BANK1_D1TCM_RTC = 1;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_BANK1_D1TCM_WTC = 1;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_DTCM_ECC_PROTECTION_MODE_ENABLE = 1;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_ITCM_ECC_PROTECTION_MODE_ENABLE = 1;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_DTCM_PROTECTION_CHECK_ENABLE = 0;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_ITCM_PROTECTION_CHECK_ENABLE = 0;
    }
    else
    {
        retval = cEcError;
    }
    return retval;
}

/**
 *  @brief  Enable / Disable TCM Protection Check where core is FP0(0), FP1(1), FP2(2)
 *          Sets the following:
 *              - CPU_DTCM_PROTECTION_CHECK_ENABLE (sets or clears depending on the value of enableTcmProtectionCheck)
 *              - CPU_ITCM_PROTECTION_CHECK_ENABLE (sets or clears depending on the value of enableTcmProtectionCheck)
 *  @param  core Id <0|1|2>
 *  @param  Enable / Disable
 *  @return Error_t
 */
Error_t HalFps_EnableTcmProtectionCheck(CoreId_t coreID, bool enableTcmProtectionCheck)
{
    Error_t retval = cEcNoError;
    if(coreID < cNumberOfCores)
    {
        Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_DTCM_PROTECTION_CHECK_ENABLE = enableTcmProtectionCheck;
        pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_ITCM_PROTECTION_CHECK_ENABLE = enableTcmProtectionCheck;
    }
    else
    {
        retval = cEcError;
    }
    return retval;
}

/**
 *  @brief  CPU Protection Enable where core is FP0(0), FP1(1), FP2(2)
 *  @param  core Id <0|1|2>
 *  @param  BITS
 *              - cCPU_DTCM_ECC_PROTECTION_MODE_ENABLE_BIT
 *              - cCPU_DTCM_PROTECTION_CHECK_ENABLE_BIT
 *              - cCPU_ITCM_ECC_PROTECTION_MODE_ENABLE_BIT
 *              - cCPU_ITCM_PROTECTION_CHECK_ENABLE_BIT
 *  @param  Set / Clear
 *  @return Error_t
 */
Error_t HalFps_SetCpuTcmEccProtectionCheckAndMode(CoreId_t coreID, FpsTcmProtectionBits_t bit, bool isEnable)
{
    Error_t status = cEcNoError;
    if(coreID < cNumberOfCores)
    {
        Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
        switch(bit)
        {
            case cCPU_DTCM_ECC_PROTECTION_MODE_ENABLE_BIT:
                    pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_DTCM_ECC_PROTECTION_MODE_ENABLE = isEnable;
                break;
            case cCPU_DTCM_PROTECTION_CHECK_ENABLE_BIT:
                    pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_DTCM_PROTECTION_CHECK_ENABLE = isEnable;
                break;
            case cCPU_ITCM_ECC_PROTECTION_MODE_ENABLE_BIT:
                    pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_ITCM_ECC_PROTECTION_MODE_ENABLE = isEnable;
                break;
            case cCPU_ITCM_PROTECTION_CHECK_ENABLE_BIT:
                    pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryControl.b.CPU_ITCM_PROTECTION_CHECK_ENABLE = isEnable;
                break;
            default :
                status = cEcError;
        }
    }
    else
    {
        // Invalid Parameter
        status = cEcError;
    }
    return status;
}

/**
 *  @brief  Get CPU Memory Error Status where core is FP0(0), FP1(1), FP2(2)
 *  @param  core Id <0|1|2>
 *  @param  BITS
 *              - cCPU_ITCM_PROTECTION_ERROR_BIT
 *              - cCPU_D0TCM_PROTECTION_ERROR_BIT
 *              - cCPU_D1TCM_PROTECTION_ERROR_BIT
 *              - cCPU_ITCM_ECC_CORRECTABLE_ERROR_BIT
 *              - cCPU_D1TCM_ECC_CORRECTABLE_ERROR_BIT
 *              - cCPU_D0TCM_ECC_CORRECTABLE_ERROR_BIT
 *              - cCPU_D0TCM_ADDR_OUT_OF_RANGE_ERROR_BIT
 *              - cCPU_D1TCM_ADDR_OUT_OF_RANGE_ERROR_BIT
 *  @return bool
 */
bool HalFps_GetCpuMemoryErrorStatus(CoreId_t coreID, FpsCpuMemErrorStatus_t bit)
{
    bool retval = false;
    Fps_t* pFps = (Fps_t*)FPS_REG_ADDR;
    switch(bit)
    {
        case cCPU_ITCM_PROTECTION_ERROR_BIT: retval = pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryErrorStatus.b.CPU_ITCM_PROTECTION_ERROR;
                                 break;
        case cCPU_D0TCM_PROTECTION_ERROR_BIT: retval = pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryErrorStatus.b.CPU_D0TCM_PROTECTION_ERROR;
                                 break;
        case cCPU_D1TCM_PROTECTION_ERROR_BIT: retval = pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryErrorStatus.b.CPU_D1TCM_PROTECTION_ERROR;
                                 break;
        case cCPU_ITCM_ECC_CORRECTABLE_ERROR_BIT: retval = pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryErrorStatus.b.CPU_ITCM_ECC_CORRECTABLE_ERROR;
                                 break;
        case cCPU_D1TCM_ECC_CORRECTABLE_ERROR_BIT: retval = pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryErrorStatus.b.CPU_D1TCM_ECC_CORRECTABLE_ERROR;
                                 break;
        case cCPU_D0TCM_ECC_CORRECTABLE_ERROR_BIT: retval = pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryErrorStatus.b.CPU_D0TCM_ECC_CORRECTABLE_ERROR;
                                 break;
        case cCPU_D0TCM_ADDR_OUT_OF_RANGE_ERROR_BIT: retval = pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryErrorStatus.b.CPU_D0TCM_ADDR_OUT_OF_RANGE_ERROR;
                                 break;
        case cCPU_D1TCM_ADDR_OUT_OF_RANGE_ERROR_BIT: retval = pFps->fpsCpuRegRegisters[coreID].fpsCpuCpuMemoryErrorStatus.b.CPU_D1TCM_ADDR_OUT_OF_RANGE_ERROR;
                                 break;
        default: break;
    }
    return retval;
}
