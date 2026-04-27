// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Marvell

#pragma once

#include "RegFps.h"
#include "ErrorCodes.h"

/// @brief  Core ID types
typedef enum CoreId_t
{
    cCore0 = 0,                     /// < Core 0
    cCore1,                         /// < Core 1
    cCore2,                         /// < Core 2
    cNumberOfCores                  /// < Number of cores
} CoreId_t;

/// @brief FPS Ecc Correctable threshold values
typedef enum FpsMemCtrLCorrThreshold_t
{
    cFPS_MEM_CTRL_CORR_THRESHOLD_0 = 0,     /// < No Threshold
    cFPS_MEM_CTRL_CORR_THRESHOLD_1 = 1,     /// < Threahold = 1
    cFPS_MEM_CTRL_CORR_THRESHOLD_16 = 2,    /// < Threshold = 16
    cFPS_MEM_CTRL_CORR_THRESHOLD_64 = 3,    /// < Threshold = 64
    cFPS_MEM_CTRL_CORR_THRESHOLD_128 = 4,   /// < Threshold = 128
    cFPS_MEM_CTRL_CORR_THRESHOLD_256 = 5,   /// < Threshold = 256
    cFPS_MEM_CTRL_CORR_THRESHOLD_512 = 6,   /// < Threshold = 512
    cFPS_MEM_CTRL_CORR_THRESHOLD_1024 = 7,  /// < Threshold = 1024
    cFPS_MEM_CTRL_CORR_THRESHOLD_INVALID    /// < Invalid Threshold
} FpsMemCtrLCorrThreshold_t;

/// @brief TCM Protection Bits
typedef enum FpsTcmProtectionBits_t
{
    cCPU_DTCM_ECC_PROTECTION_MODE_ENABLE_BIT    = 28, /// < cpu_dtcm_ecc_protection_mode_enable
    cCPU_DTCM_PROTECTION_CHECK_ENABLE_BIT       = 29, /// < cpu_dtcm_protection_check_enable
    cCPU_ITCM_ECC_PROTECTION_MODE_ENABLE_BIT    = 30, /// < cpu_itcm_ecc_protection_mode_enable
    cCPU_ITCM_PROTECTION_CHECK_ENABLE_BIT       = 31, /// < cpu_itcm_protection_check_enable
} FpsTcmProtectionBits_t;

/// @brief  Memory Error types
typedef enum FpsMemErrorStatus_t
{
    cCPU0_MEMORY_ERROR_BIT              = 0,  /// cpu0_memory_error
    cCPU1_MEMORY_ERROR_BIT              = 1,  /// cpu1_memory_error
    cCPU2_MEMORY_ERROR_BIT              = 2,  /// cpu2_memory_error
    cPSRAM0_PROTECTION_ERROR_BIT        = 16, /// psram0_protection_error
    cPSRAM0_ECC_CORRECTABLE_ERROR_BIT   = 24, /// psram0_ecc_correctable_error
} FpsMemErrorStatus_t;

/// @brief  Fps CPU Memory Error type
typedef enum FpsCpuMemErrorStatus_t
{
    cCPU_ITCM_PROTECTION_ERROR_BIT           = 0,    /// cpu_itcm_protection_error
    cCPU_D0TCM_PROTECTION_ERROR_BIT          = 1,    /// cpu_d0tcm_protection_error
    cCPU_D1TCM_PROTECTION_ERROR_BIT          = 2,    /// cpu_d1tcm_protection_error
    cCPU_ITCM_ECC_CORRECTABLE_ERROR_BIT      = 8,    /// cpu_itcm_ecc_correctable_error
    cCPU_D1TCM_ECC_CORRECTABLE_ERROR_BIT     = 9,    /// cpu_d1tcm_ecc_correctable_error
    cCPU_D0TCM_ECC_CORRECTABLE_ERROR_BIT     = 10,   /// cpu_d0tcm_ecc_correctable_error
    cCPU_D0TCM_ADDR_OUT_OF_RANGE_ERROR_BIT   = 16,   /// cpu_d0tcm_addr_out_of_range_error
    cCPU_D1TCM_ADDR_OUT_OF_RANGE_ERROR_BIT   = 17,   /// cpu_d1tcm_addr_out_of_range_error
} FpsCpuMemErrorStatus_t;

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
Error_t HalFps_ConfigureFpsMemoryControlRegister(FpsMemCtrLCorrThreshold_t threshold);

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
void HalFps_InitPsramMemoryControl();

/**
 *  @brief  Enable / disable PSRAM Protection Check
 *          When set, enables data protection (ECC or parity) checks; When clear, disables data protection checks and resets error status
 *          Sets the following:
 *              - PSRAM_PROTECTION_CHECK_ENABLE
 *  @param  Enable / Disable
 *  @return None
 */
void HalFps_EnablePsramProtectionCheck(bool enable);

/**
 *  @brief  Init Fps Control Register
 *          Sets the following:
 *              - FPS_ERROR_CLR
 *              - PARITY_GEN_ODD
 *              - PARITY_CHECK_ODD
 *  @param  None
 *  @return  None
 */
void HalFps_InitFpsControlRegister(void);

/**
 *  @brief  Fps Error clear
 *          Sets the following:
 *              - FPS_ERROR_CLR
 *  @param  None
 *  @return  None
 */
void HalFps_ClearFpsError(void);

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
void HalFps_EnableFabricParity(void);

/**
 *  @brief  Set / Clear PSRAM Protection Write Disabled bit
 *          Sets or clears PSRAM_PROTECTION_WRITE_DISABLE
 *          When set disables writes to the protection location in memory.
 *  @param  Set / Clear
 *  @return None
 */
void HalFps_SetDisablePsramWriteProtectionCheck(bool isEnable);

/**
 *  @brief  Set / Clear TCM Protection Write Disabled bit
 *          Sets or clears either CPU_DTCM_PROTECTION_WRITE_DISABLE or CPU_ITCM_PROTECTION_WRITE_DISABLE
 *  @param  Set / Clear
 *  @return None
 */
void HalFps_SetDisableTcmWriteProtectionCheck(bool protCheckEnable, bool isDtcm);

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
bool HalFps_GetMemoryErrorStatus(FpsMemErrorStatus_t bit);

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
Error_t HalFps_InitCpuMemoryControl(CoreId_t coreID);

/**
 *  @brief  Enable / Disable TCM Protection Check where core is FP0(0), FP1(1), FP2(2)
 *          Sets the following:
 *              - CPU_DTCM_PROTECTION_CHECK_ENABLE (sets or clears depending on the value of enableTcmProtectionCheck)
 *              - CPU_ITCM_PROTECTION_CHECK_ENABLE (sets or clears depending on the value of enableTcmProtectionCheck)
 *  @param  core Id <0|1|2>
 *  @param  Enable / Disable
 *  @return Error_t
 */
Error_t HalFps_EnableTcmProtectionCheck(CoreId_t coreID, bool enableTcmProtectionCheck);

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
Error_t HalFps_SetCpuTcmEccProtectionCheckAndMode(CoreId_t coreID, FpsTcmProtectionBits_t bit, bool value);

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
bool HalFps_GetCpuMemoryErrorStatus(CoreId_t coreID, FpsCpuMemErrorStatus_t bit);
