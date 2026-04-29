// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2026 Marvell

#include "SysTypes.h"
#include "M7MemMap.h"
#include "cm7ikmcu.h"
#include "FpCommon.h"
#include "vicommon.h"
#include "crashdump.h"
#include "RegTcon.h"
#include "LoggingDebug.h"
#include "HalFps.h"

void Explicit_CrashCatcher_Entry()
{
    unsigned int lrReg, pcReg;
    __asm volatile ("MOV %0, r14\n" : "=r" (lrReg));
    __asm volatile ("MOV %0, r15\n" : "=r" (pcReg));
    CrashCatcherExceptionRegisters crashCatcherExceptionRegisters = {0};
    crashCatcherExceptionRegisters.pc = pcReg;
    crashCatcherExceptionRegisters.lr = lrReg;
    crashCatcherExceptionRegisters.msp = __get_MSP();
    CrashDump_StartDump((const CrashCatcherExceptionRegisters *)&crashCatcherExceptionRegisters, true, true);
}

#ifdef MCR_TEST_HOOKS

// ECC error injection addresses have been hard coded for testing.
// Once test framework is in, these addresses might be passed
#define DTCM_ECC_ERR_INJECT_UECC_ADDR 0x20023DD8
#define DTCM_ECC_ERR_INJECT_CORR_ADDR 0x20023DD0
#define ITCM_ECC_ERR_INJECT_UECC_ADDR 0x7FE0
#define ITCM_ECC_ERR_INJECT_CORR_ADDR 0x7FE4
#define PSRAM_ECC_ERR_INJECT_UECC_ADDR PSRAM_BACKUP_DATA_HEADER_ADDR
#define PSRAM_ECC_ERR_INJECT_CORR_ADDR PSRAM_BACKUP_DATA_HEADER_ADDR + 0x4
void Inject_Correctable_error(uint32_t Addr, CoreId_t coreID, bool isDtcm)
{
    volatile uint32_t tcmReadData = 0;
    writel(InjInitMem, Addr); // Clear TCM memory
    DMB();
    // When Protection Mode enable is set, selects ECC generation/checking if enabled. When clear, selects Parity generation/checking if enabled.
    HalFps_SetCpuTcmEccProtectionCheckAndMode(coreID,(isDtcm ? cCPU_DTCM_ECC_PROTECTION_MODE_ENABLE_BIT : cCPU_ITCM_ECC_PROTECTION_MODE_ENABLE_BIT), true);
    // When Protection check enable is set, checks selected protection. When clear, disables protection check and resets error status.
    HalFps_SetCpuTcmEccProtectionCheckAndMode(coreID,(isDtcm ? cCPU_DTCM_PROTECTION_CHECK_ENABLE_BIT : cCPU_ITCM_PROTECTION_CHECK_ENABLE_BIT), true);
    DMB();
    writel(InjSetMem, Addr); // Write initial pattern to TCM memory
    DMB();
    // Disable memory protection check and perform writes to the memory.
    HalFps_SetDisableTcmWriteProtectionCheck(true, isDtcm); // Disable ECC generation for TCM writes. 
    DMB();
    writel(InjCorrErrMem, Addr); // Introduce Single bit error in TCM memory
    DMB();
    HalFps_SetDisableTcmWriteProtectionCheck(false, isDtcm); // Enable ECC generation for TCM writes. 
    DMB();
    tcmReadData =  readl(Addr); // Read Memory with Single Bit Error
}

void Inject_Uncorrectable_error(uint32_t Addr, CoreId_t coreID, bool isDtcm)
{
    volatile uint32_t tcmReadData = 0;
    writel(InjInitMem, Addr); // Clear TCM memory
    DMB();
    // When Protection Mode enable is set, selects ECC generation/checking if enabled. When clear, selects Parity generation/checking if enabled.
    HalFps_SetCpuTcmEccProtectionCheckAndMode(coreID,(isDtcm ? cCPU_DTCM_ECC_PROTECTION_MODE_ENABLE_BIT : cCPU_ITCM_ECC_PROTECTION_MODE_ENABLE_BIT), true);
    // When Protection check enable is set, checks selected protection. When clear, disables protection check and resets error status.
    HalFps_SetCpuTcmEccProtectionCheckAndMode(coreID,(isDtcm ? cCPU_DTCM_PROTECTION_CHECK_ENABLE_BIT : cCPU_ITCM_PROTECTION_CHECK_ENABLE_BIT), true);
    DMB();
    writel(InjSetMem, Addr); // Write initial pattern to TCM memory
    DMB();
    // Disable memory protection check and perform writes to the memory.
    HalFps_SetDisableTcmWriteProtectionCheck(true, isDtcm); // Disable ECC generation for TCM writes. 
    DMB();
    writel(InjUncorrErrMem, Addr); // Introduce double bit error in TCM memory
    HalFps_SetDisableTcmWriteProtectionCheck(false, isDtcm); // Enable ECC generation for TCM writes.
    DMB();
    tcmReadData =  readl(Addr); // Read Memory with Double Bit Error
}

void Inject_TCM_Ecc_Error(bool isCorrectable, CoreId_t coreID, bool isDtcm)
{
    HalFps_ClearFpsError(); // Set Self-clearing bit which clears all ECC or Parity related errors within the FPS module when set.

    if(isDtcm == MemTypeDtcm)
    {
        if(isCorrectable == MemErrCorrectable)
        {
            // Set correctable error threshold
            HalFps_ConfigureFpsMemoryControlRegister(cFPS_MEM_CTRL_CORR_THRESHOLD_1);
            DMB();
            Inject_Correctable_error(DTCM_ECC_ERR_INJECT_CORR_ADDR, coreID, isDtcm);
        }
        else
        {
            Inject_Uncorrectable_error(DTCM_ECC_ERR_INJECT_UECC_ADDR, coreID, isDtcm);
        }
    }
    else
    {
        if(isCorrectable)
        {
            // Set correctable error threshold
            HalFps_ConfigureFpsMemoryControlRegister(cFPS_MEM_CTRL_CORR_THRESHOLD_1);
            DMB();
            Inject_Correctable_error(ITCM_ECC_ERR_INJECT_CORR_ADDR, coreID, isDtcm);
        }
        else
        {
            Inject_Uncorrectable_error(ITCM_ECC_ERR_INJECT_UECC_ADDR, coreID, isDtcm);
        }
    }
}

void Inject_PSRAM_Ecc_Error(bool isCorrectable)
{
    volatile uint32_t psramreadData = 0;

    HalFps_ClearFpsError(); // Set Self-clearing bit which clears all ECC or Parity related errors within the FPS module when set.
    DMB();

    HalFps_ConfigureFpsMemoryControlRegister(cFPS_MEM_CTRL_CORR_THRESHOLD_1); //config psram initial pattern, enable psram Read-Modift-Write for ecc
    DMB();

    // Modify PSRAM Memory without changing ECC
    if(isCorrectable)
    {
        // Initialize PSRAM Memory to 0
        writel(InjInitMem, PSRAM_ECC_ERR_INJECT_CORR_ADDR);
        DMB();
        HalFps_EnablePsramProtectionCheck(true); //enable protection check
        DMB();
        // Write To PSRAM memory
        writel(InjSetMem, PSRAM_ECC_ERR_INJECT_CORR_ADDR); // Write initial pattern to PSRAM memory.
        DMB();
        // Disable memory protection check and perform writes to the memory.
        HalFps_SetDisablePsramWriteProtectionCheck(true); // Set PSRAM protection write disable bit.
        DMB();
        writel(InjCorrErrMem, PSRAM_ECC_ERR_INJECT_CORR_ADDR); // Introduce single bit error in PSRAM memory.
        DMB();
        HalFps_SetDisablePsramWriteProtectionCheck(false); // Clear PSRAM protection write disable bit
        DMB();

        // read memory to introduce the error.
        psramreadData = readl(PSRAM_ECC_ERR_INJECT_CORR_ADDR); // Correctable error Count 1 or uncorrectable depending on the value of isCorrectable
    }
    else
    {
        // Initialize PSRAM Memory to 0
        writel(InjInitMem, PSRAM_ECC_ERR_INJECT_UECC_ADDR);
        DMB();
        HalFps_EnablePsramProtectionCheck(true); //enable protection check, use ecc
        DMB();
        // Write To PSRAM memory
        writel(InjSetMem, PSRAM_ECC_ERR_INJECT_UECC_ADDR); // Write initial pattern to PSRAM memory.
        DMB();
        // Disable memory protection check and perform writes to the memory.
        HalFps_SetDisablePsramWriteProtectionCheck(true); // Disable ECC generation for PSRAM writes.
        DMB();
        writel(InjUncorrErrMem, PSRAM_ECC_ERR_INJECT_UECC_ADDR); // Introduce double bit error in PSRAM memory. 
        DMB();
        HalFps_SetDisablePsramWriteProtectionCheck(false); // Enable ECC generation for PSRAM writes.
        DMB();

        // read memory to introduce the error.
        psramreadData = readl(PSRAM_ECC_ERR_INJECT_UECC_ADDR); // Correctable error Count 1 or uncorrectable depending on the value of isCorrectable
    }
}

void TriggerCrash(InjectErrorType errorType)
{
    CoreId_t core;
    #if defined (fps_cpu0Core)
    core = cCore0;
    #elif defined (fps_cpu1Core)
    core = cCore1;
    #elif defined (fps_cpu2Core)
    core = cCore2;
    #endif
    switch(errorType)
    {
        case InjErrHardFault:     {
                                volatile int i = 1, j = 0;
                                SCB->CCR = (SCB_CCR_UNALIGN_TRP_Msk | SCB_CCR_DIV_0_TRP_Msk);
                                i =i/j;
                            }
                            break;
        case InjErrExplicitCrash: {
                                Explicit_CrashCatcher_Entry();
                            }
                            break;
        case InjErrHang:          {
                                while(1);
                            }
                            break;
        // Below code to be enabled once ECC test framework is available
        /*
        // Following values should come from DDI tests
        case InjPsramErrUECC:  {
                                Inject_PSRAM_Ecc_Error(MemErrUncorrectable); // PSRAM Uncorrectable
                            }
                            break;
        case InjPsramErrCorrECC:  {
                                Inject_PSRAM_Ecc_Error(MemErrCorrectable); // PSRAM Correctable
                            }
                            break;
        case InjDtcmErrUECC:  {
                                Inject_TCM_Ecc_Error(MemErrUncorrectable, core, MemTypeDtcm); // DTCM Uncorrectable
                            }
                            break;
        case InjDtcmErrCorrECC:  {
                                Inject_TCM_Ecc_Error(MemErrCorrectable, core, MemTypeDtcm); // DTCM Correctable
                            }
                            break;
        case InjItcmErrUECC:  {
                                Inject_TCM_Ecc_Error(MemErrUncorrectable, core, MemTypeItcm); // ITCM Uncorrectable
                            }
                            break;
        case InjItcmErrCorrECC:  {
                                Inject_TCM_Ecc_Error(MemErrCorrectable, core, MemTypeItcm); // ITCM Correctable
                            }
                            break;
        */
    }
}
#endif

uint32_t getFaultCode(bool sendIrq, bool explicitCrash)
{
    uint32_t retval = 0;
    if(!sendIrq)
    {
        retval = OTHER_CORE;
    }
    else if (explicitCrash)
    {
        retval = EXPLICIT_FAILURE;
    }
    else
    {
        retval =  __get_xPSR() & FAILURE_CODE_MASK;
        if(retval <= SYS_USAGEFAULT)
        {
            retval = retval - 1;
            if(retval == NONMASKABLE_INTERRUPT || retval == HARDFAULT)
            {
                CoreId_t core;
                #if defined (fps_cpu0Core)
                core = cCore0;
                #elif defined (fps_cpu1Core)
                core = cCore1;
                #elif defined (fps_cpu2Core)
                core = cCore2;
                #endif
                if(HalFps_GetMemoryErrorStatus(cPSRAM0_PROTECTION_ERROR_BIT))
                {
                    // PSRAM Uncorrectable error Log
                    retval = UNCORRECTABLE_ECC_FAILURE;
                }
                else if(HalFps_GetMemoryErrorStatus(cPSRAM0_ECC_CORRECTABLE_ERROR_BIT))
                {
                    // PSRAM Correctable error Log
                    retval = CORRECTABLE_ECC_FAILURE;
                }
                else if(HalFps_GetMemoryErrorStatus(core))
                {
                    if(HalFps_GetCpuMemoryErrorStatus(core, cCPU_ITCM_PROTECTION_ERROR_BIT))
                    {
                        // ITCM Uncorrectable error Log
                        retval = UNCORRECTABLE_ECC_FAILURE;
                    }
                    else if(HalFps_GetCpuMemoryErrorStatus(core, cCPU_D0TCM_PROTECTION_ERROR_BIT))
                    {
                        // D0TCM Uncorrectable error Log
                        retval = UNCORRECTABLE_ECC_FAILURE;
                    }
                    else if(HalFps_GetCpuMemoryErrorStatus(core, cCPU_ITCM_ECC_CORRECTABLE_ERROR_BIT))
                    {
                        // ITCM Correctable error Log
                        retval = CORRECTABLE_ECC_FAILURE;
                    }
                    else if(HalFps_GetCpuMemoryErrorStatus(core, cCPU_D0TCM_ECC_CORRECTABLE_ERROR_BIT))
                    {
                        // D0TCM Correctable error Log
                        retval = CORRECTABLE_ECC_FAILURE;
                    }
                }
            }
        }
        // TODO SV_CALL and PEND_SV faults have not been tested
        else if(retval == SYS_SVCALL)
        {
            retval = SV_CALL;
        }
        else if(retval == SYS_PENDSV)
        {
            retval = PEND_SV;
        }
        else
        {
            retval = OTHER_CORE;
        }
    }
    return retval;
}
void CrashDump_StartDump(const CrashCatcherExceptionRegisters* pExceptionRegisters, bool sendIrq, bool explicitCrash)
{
    // Disable interrupt
    VicIrqDisable(TCON_INT_WAKE_TIMER_1_NUM);
    ClrPendingIrq(TCON_INT_WAKE_TIMER_1_NUM);

    API_CDMAPause();
    #if defined (fps_cpu0Core)
    volatile CrashDumpInfo* pcrashinfo = (volatile CrashDumpInfo*)M7_FPS_CPU01_CRASHDUMP_INFO_ADDR;
    #elif defined (fps_cpu1Core)
    volatile CrashDumpInfo* pcrashinfo = (volatile CrashDumpInfo*)M7_FPS_CPU12_CRASHDUMP_INFO_ADDR;
    #elif defined (fps_cpu2Core)
    volatile CrashDumpInfo* pcrashinfo = (volatile CrashDumpInfo*)M7_FPS_CPU20_CRASHDUMP_INFO_ADDR;
    #endif
    if (pcrashinfo)
    {
        pcrashinfo->header.magic = DUMP_HEADER_MAGIC_DIRTY;
        pcrashinfo->header.faultCode = getFaultCode(sendIrq, explicitCrash);
        pcrashinfo->header.version = 1;
        #if defined (fps_cpu0Core)
        pcrashinfo->header.coreType = FP0_TYPE;
        #elif defined (fps_cpu1Core)
        pcrashinfo->header.coreType = FP1_TYPE;
        #elif defined (fps_cpu2Core)
        pcrashinfo->header.coreType = FP2_TYPE;
        #endif
        pcrashinfo->header.dumpType = Release;
        if(pcrashinfo->header.faultCode == OTHER_CORE)
        {
            pcrashinfo->header.crashType = (uint8_t)CRASH_TYPE_NORMAL;
        }
        else if(explicitCrash)
        {
            pcrashinfo->header.crashType = (uint8_t)CRASH_TYPE_HANG;
        }
        else
        {
            pcrashinfo->header.crashType = (uint8_t)CRASH_TYPE_CRASH; // ARM Fault occured.
        }

        pcrashinfo->header.payloadSize = sizeof(CrashDumpPayload);

        pcrashinfo->payload.armPayload.common_regs.stack_ptr = (void*) pExceptionRegisters->msp;
        pcrashinfo->payload.armPayload.common_regs.handler_xpsr = __get_xPSR(); // XPSR value in the current context
        pcrashinfo->payload.armPayload.common_regs.frame.r0 = pExceptionRegisters->r0;
        pcrashinfo->payload.armPayload.common_regs.frame.r1 = pExceptionRegisters->r1;
        pcrashinfo->payload.armPayload.common_regs.frame.r2 = pExceptionRegisters->r2;
        pcrashinfo->payload.armPayload.common_regs.frame.r3 = pExceptionRegisters->r3;
        pcrashinfo->payload.armPayload.common_regs.frame.r12 = pExceptionRegisters->r12;
        pcrashinfo->payload.armPayload.common_regs.frame.lr = pExceptionRegisters->lr;
        pcrashinfo->payload.armPayload.common_regs.frame.pc = pExceptionRegisters->pc;
        pcrashinfo->payload.armPayload.common_regs.xpsr = pExceptionRegisters->exceptionpsr; //XPSR value in the Hardfault_handler context
        pcrashinfo->payload.armPayload.common_regs.hfsr = SCB->HFSR;
        pcrashinfo->payload.armPayload.common_regs.cfsr = SCB->CFSR;
        pcrashinfo->payload.armPayload.common_regs.mmfar = SCB->MMFAR;
        pcrashinfo->payload.armPayload.common_regs.bfar = SCB->BFAR;
        pcrashinfo->payload.armPayload.common_regs.afsr = SCB->AFSR;

        #ifdef DEBUG_BUILD
        #if defined (fps_cpu0Core)
        pcrashinfo->payload.fpPayload.fpInitSts= *((uint32_t*)PSRAM_FP_CPU0_STATUS_ADDR);
        pcrashinfo->payload.fpPayload.psramFp0toFp2Pi=*((uint32_t*)PSRAM_INTL_CPU0_2_CPU2_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp2toFp0Ci=*((uint32_t*)PSRAM_INTL_CPU2_2_CPU0_CI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp0toFp1Pi=*((uint32_t*)PSRAM_INTL_CPU0_2_CPU1_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp1toFp0Ci=*((uint32_t*)PSRAM_INTL_CPU1_2_CPU0_CI_ADDR);
        #elif defined (fps_cpu1Core)
        pcrashinfo->payload.fpPayload.fpInitSts = *((uint32_t*)PSRAM_FP_CPU1_STATUS_ADDR);
        pcrashinfo->payload.fpPayload.psramFp1toFp2Pi=*((uint32_t*)PSRAM_INTL_CPU1_2_CPU2_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp2toFp1Ci=*((uint32_t*)PSRAM_INTL_CPU2_2_CPU1_CI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp1toFp0Pi=*((uint32_t*)PSRAM_INTL_CPU1_2_CPU0_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp0toFp1Ci=*((uint32_t*)PSRAM_INTL_CPU0_2_CPU1_CI_ADDR);
        #elif defined (fps_cpu2Core)
        pcrashinfo->payload.fpPayload.fpInitSts = *((uint32_t*)PSRAM_FP_CPU2_STATUS_ADDR);
        pcrashinfo->payload.fpPayload.psramFp2toFp0Pi=*((uint32_t*)PSRAM_INTL_CPU2_2_CPU0_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp0toFp2Ci=*((uint32_t*)PSRAM_INTL_CPU0_2_CPU2_CI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp2toFp1Pi=*((uint32_t*)PSRAM_INTL_CPU2_2_CPU1_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp1toFp2Ci=*((uint32_t*)PSRAM_INTL_CPU1_2_CPU2_CI_ADDR);
        // CP0 to FP2 Req
        pcrashinfo->payload.fpPayload.psramCp0toFp2ReqPi=*((uint32_t*)PSRAM_CP0toFP_REQ_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramCp0toFp2ReqCi=*((uint32_t*)PSRAM_CP0toFP_REQ_CI_ADDR);
        // FP2 to CP0 Res
        pcrashinfo->payload.fpPayload.psramFp2toCp0ResPi=*((uint32_t*)PSRAM_FPtoCP0_RES_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp2toCp0ResCi=*((uint32_t*)PSRAM_FPtoCP0_RES_CI_ADDR);
        // CP1 to FP2 Req
        pcrashinfo->payload.fpPayload.psramCp1toFp2ReqPi=*((uint32_t*)PSRAM_CP1toFP_REQ_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramCp1toFp2ReqCi=*((uint32_t*)PSRAM_CP1toFP_REQ_CI_ADDR);
        // FP2 to CP1 Res
        pcrashinfo->payload.fpPayload.psramFp2toCp1ResPi=*((uint32_t*)PSRAM_FPtoCP1_RES_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp2toCp1ResCi=*((uint32_t*)PSRAM_FPtoCP1_RES_CI_ADDR);

        // FP2 to CP0 Req
        pcrashinfo->payload.fpPayload.psramFp2toCp0ReqPi=*((uint32_t*)PSRAM_FPtoCP0_REQ_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp2toCp0ReqCi=*((uint32_t*)PSRAM_FPtoCP0_REQ_CI_ADDR);
        // CP0 to FP2 Res
        pcrashinfo->payload.fpPayload.psramCp0toFp2ResPi=*((uint32_t*)PSRAM_CP0toFP_RES_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramCp0toFp2ResCi=*((uint32_t*)PSRAM_CP0toFP_RES_CI_ADDR);
        // FP2 to CP1 Req
        pcrashinfo->payload.fpPayload.psramFp2toCp1ReqPi=*((uint32_t*)PSRAM_FPtoCP1_REQ_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramFp2toCp1ReqCi=*((uint32_t*)PSRAM_FPtoCP1_REQ_CI_ADDR);
        // CP1 to FP2 Res
        pcrashinfo->payload.fpPayload.psramCp1toFp2ResPi=*((uint32_t*)PSRAM_CP1toFP_RES_PI_ADDR);
        pcrashinfo->payload.fpPayload.psramCp1toFp2ResCi=*((uint32_t*)PSRAM_CP1toFP_RES_CI_ADDR);

        #endif
        #endif
        pcrashinfo->header.magic = DUMP_HEADER_MAGIC_COMITTED;
    }

    // Send Wakeup1 interrupt to other Cores
    if(sendIrq)
    {
        volatile Tcon_t *tconRegs = (volatile Tcon_t*)TCON_REG_ADDR;
        volatile WakeupCtrl_t* wakeupCtrl = (volatile WakeupCtrl_t*)&tconRegs->wakeupCtrl;
        wakeupCtrl->b.WKINTR_LEVEL_EN = wakeupCtrl->b.WKINTR_LEVEL_EN | 0b10;
        wakeupCtrl->b.WKINTR_RPT_EN = wakeupCtrl->b.WKINTR_RPT_EN & 0b01;
        tconRegs->wakeup1Cnt = 0x1;
        wakeupCtrl->b.WAKEUP_ENABLE = wakeupCtrl->b.WAKEUP_ENABLE | 0b10;
    }
    while (1)
    {
        ;
    }
}
