// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   LoggingDebug.cpp
//! @brief  The system debugging API implements.
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "LoggingDebug.h"
#include "MemIo.h"
#include "M7Fiber.h"
#include "FpCommon.h"
#if defined (CPU0)
#include "FpsCpu0/FpsCpu0.h"
extern fpsCpu0 gFpsCpu0;
#elif defined (CPU1)
#include "FpsCpu1/FpsCpu1.h"
extern fpsCpu1 gFpsCpu1;
#elif defined (CPU2)
#include "FpsCpu2/FpsCpu2.h"
extern fpsCpu2 gFpsCpu2;
#endif
#include <APILogging.h>

//-----------------------------------------------------------------------------
//  Private Data Type Definitions
//-----------------------------------------------------------------------------

#define cMaxMessageSize 8                                           ///< 8 DWAORD
typedef struct PrintfMessage_t
{
    uint32_t    tokenBuffer[cMaxMessageSize];                   ///< Token Primitive Buffer for one Token Function (32 bytes)
} PrintfMessage_t;

enum class LogFiberId_t : M7FiberId_t
{
    cLogFiberTimestampId,
    cNumberOfFibers
};

//-----------------------------------------------------------------------------
//  Private Constant Definitions
//-----------------------------------------------------------------------------

#ifndef INTEGRATE_TIMESTAMP_TO_FPSCPU
static constexpr uint32_t cFiberWeightTimestamp = 1;                ///< fiber weight of LoggingTimestampFiber
static const char cLoggingTimestampFiberName[] = "LogTimestamp";    ///< fiber name of LoggingTimestampFiber
#endif // End of INTEGRATE_TIMESTAMP_TO_FPSCPU
#define LOG_EN_DIS_UPDATE_DEFAULT_VALUE 0                           ///< default disable

//-----------------------------------------------------------------------------
//  Private Variable Definitions
//-----------------------------------------------------------------------------

static PackedU32_t cpuInfo = {0};                                   ///< CoreID, will be added in the heading of debug log
static LogExt_t* logExtShared = NULL;                               ///< Log Extension. Instance is in PSRAM
volatile LogBufferInfo_t* logBufferInfo = NULL;              ///< Log Buffer Info. Instance is in PSRAM/Shared DTCM.
LogExt_t logExt = {0};                                       ///< Log Extension(single FP_CPU variable). Will sync to logExtShared in PSRAM.

//LogCategory_t gLogCategory;                                         ///< global log category
#ifndef INTEGRATE_TIMESTAMP_TO_FPSCPU
uint32_t localTimestamp = 0;                                        ///< local timestamp, will be updated continuously in fiber
#define TimestampMaskBit23_14 0xFFC000                              ///< 10 bits [23:14]
#endif // End of INTEGRATE_TIMESTAMP_TO_FPSCPU
uint32_t gdmaDqDepth = 0;                                           /// < gdma delivery queue size (for all FPS cpus)
volatile uint32_t* gdmaDqPi = NULL;                                              /// < local gdma delivery queue pi
uint32_t gdmaDqPiBufferTarget = 0;                                  /// < the log buffer index(0 or 1) which current gdmaDqPi points to
M7Fiber fiberTimestamp;                                              /// < The Fiber of Logging Timestamp
#define DEF_FIBERTIMESTAMP(id) (fiberTimestamp)
uint32_t gTimeStampBase = 0;

//-----------------------------------------------------------------------------
//  Function Implement - Core Id
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Function Implement - Fiber
//-----------------------------------------------------------------------------

#ifndef INTEGRATE_TIMESTAMP_TO_FPSCPU
/**
 *  Logging Timestamp Fiber
 *
 *  Always active to check systick. If 10 bits [23:14] of systick changed, store a log of current timestamp.
 *
 *  @return   nothing
 */
static void LoggingTimestampFiber(void)
{
    Cortexm7_t* rCortexm7 = (Cortexm7_t*)CORTEXM7_REG_ADDR;
    uint32_t currentTimestamp = 0xFFFFFF - readl(&rCortexm7->systemControl.systCvr);
    #if defined (CPU0)
    currentTimestamp = (currentTimestamp + gFpsCpu0.gTimerCounterCovert) & SYSTICK_MASK;
    gTimeStampBase = gFpsCpu0.gTimerCounterCovert;
    #elif defined (CPU1)
    currentTimestamp = (currentTimestamp + gFpsCpu1.gTimerCounterCovert) & SYSTICK_MASK;
    gTimeStampBase = gFpsCpu1.gTimerCounterCovert;
    #elif defined (CPU2)
    currentTimestamp = (currentTimestamp + gFpsCpu2.gTimerCounterCovert) & SYSTICK_MASK;
    gTimeStampBase = gFpsCpu2.gTimerCounterCovert;
    #endif

    if ((localTimestamp & TimestampMaskBit23_14) != (currentTimestamp & TimestampMaskBit23_14)) // compare 10 bits [23:14]
    {
        // update local timestamp and record in log buffer
        localTimestamp = currentTimestamp;

        // Note: DebugLogLvDbgInfoInline's param logCategory cannot use variable, due to tokenize tool check the enum "text" but "value".
        #ifdef ENABLE_TIMESTAMP_LOGGING
        #if defined (CPU0)
        //DebugLogLvDbgInfoInline(cLogCPU0Common, cLogInfo, ("logging timestamp: 0x%x\n", localTimestamp));
        #elif defined (CPU1)
        //DebugLogLvDbgInfoInline(cLogCPU1Common, cLogInfo, ("logging timestamp: 0x%x\n", localTimestamp));
        #elif defined (CPU2)
        //DebugLogLvDbgInfoInline(cLogCPU2Common, cLogInfo, ("logging timestamp: 0x%x\n", localTimestamp));
        #endif
        #endif
    } // else do nothing
}
#endif // End of INTEGRATE_TIMESTAMP_TO_FPSCPU

/**
 *  Register Fiber
 *
 *  Fiber must be registered after M7FiberScheduler_Initialize() and M7FiberSchedulerContext_CreateInstance()
 *
 *  @return   nothing
 */
static void LoggingRegisterFiber(void)
{
    uint8_t coreID = 0;
    #ifndef INTEGRATE_TIMESTAMP_TO_FPSCPU
    DEF_FIBERTIMESTAMP(coreID).Register(&LoggingTimestampFiber, cLoggingTimestampFiberName, cM7FiberStateActive);
    #endif // End of INTEGRATE_TIMESTAMP_TO_FPSCPU
}

//-----------------------------------------------------------------------------
//  Non-inline version (for iram size concern)
//-----------------------------------------------------------------------------

static void HandleCurrentFullBuffer(void)
{
    uint8_t coreID = 0;
    volatile LogBufferInfo_t* _logBufferInfo = GetLogBufferInfo(coreID);
    LogExt_t* _logExt = GetLogExt(coreID);

    // in case when disable LogEnDisUpdate  meet empty log buffer
    if (_logBufferInfo->Len[_logBufferInfo->Sel] == 0)
    {
        return;
    } // else do nothing

    _logBufferInfo->Version[_logBufferInfo->Sel]++;
    GetGdmaDqPiBufferTarget(coreID) = _logBufferInfo->Sel; // disable log transfer also call this function, but not switch buf sel

    if (_logExt->LogEnDisUpdate)
    {
        // zero out remind bytes in 2K log buffer
        uint32_t* pDest = 0;
        while ((_logBufferInfo->Len[_logBufferInfo->Sel] + sizeof(uint32_t)) <= LOG_BUFFER_SIZE)
        {
            pDest = (uint32_t*)(_logBufferInfo->Addr[_logBufferInfo->Sel] + _logBufferInfo->Len[_logBufferInfo->Sel]);
            *pDest = 0; // fill 4 bytes zero

            _logBufferInfo->Len[_logBufferInfo->Sel] = _logBufferInfo->Len[_logBufferInfo->Sel] + sizeof(uint32_t);
        }

        // gdma full buffer 2K data from psram to gsram
        *GetGdmaDqPi(coreID) = QUEUE_INC(*GetGdmaDqPi(coreID), gdmaDqDepth - 1);
        #ifdef SUPPORT_FPS_REGISTER
        Fps_t* rFps = (Fps_t*)FPS_REG_ADDR;
        #if defined (CPU0)
        writel(*GetGdmaDqPi(coreID), &(rFps->fpsFp2hweRegRegisters[cFp2HweWq05GdmaDq].fpsFp2hweFpToHweQPiIndirectDataPort));
        #elif defined (CPU1)
        writel(*GetGdmaDqPi(coreID), &(rFps->fpsFp2hweRegRegisters[cFp2HweWq08GdmaDq].fpsFp2hweFpToHweQPiIndirectDataPort));
        #elif defined (CPU2)
        writel(*GetGdmaDqPi(coreID), &(rFps->fpsFp2hweRegRegisters[cFp2HweWq09GdmaDq].fpsFp2hweFpToHweQPiIndirectDataPort));
        #endif
        #endif

    } // else do nothing

    // empty log buffer for futher use
    _logBufferInfo->Len[_logBufferInfo->Sel] = 0;
}

#if 0
static void AddTimestampInBuffer(void)
{
    uint8_t coreID = 0;
    Cortexm7_t* rCortexm7 = (Cortexm7_t*)CORTEXM7_REG_ADDR;
    volatile LogBufferInfo_t* _logBufferInfo = GetLogBufferInfo(coreID);
    uint32_t timestamp = 0;

    timestamp = 0xFFFFFF - readl(&rCortexm7->systemControl.systCvr);
    // update local timestamp
    localTimestamp = timestamp;

    uint16_t timestampToken = (cLogInfo << 14) + (gLogCategory << 10);  // log level, category, index
    uint32_t* pDest = (uint32_t*)(_logBufferInfo->Addr[_logBufferInfo->Sel]);

    SendMessageToBuffer1Argument(timestampToken, pDest, timestamp);
    _logBufferInfo->Len[_logBufferInfo->Sel] += 8;  // 8 bytes fixed size
}
#endif

//-----------------------------------------------------------------------------
//  Function Implement - Initial
//-----------------------------------------------------------------------------

/**
 *  Initializes systick.
 *
 *  @return nothing
 */
static void SystickInit(unsigned long period)
{
    Cortexm7_t* rCortexm7 = (Cortexm7_t*)CORTEXM7_REG_ADDR;
    writel(0x0, &rCortexm7->systemControl.systCsr);
    writel(period - 1, &rCortexm7->systemControl.systRvr);
    writel(0x0, &rCortexm7->systemControl.systCvr);   //any write to current val clears it.
    writel(0x1, &rCortexm7->systemControl.systCsr);   //enable systick and use external clock

    // set systick threshold
    Fps_t* rFps = (Fps_t*)FPS_REG_ADDR;
    uint8_t coreID = cpuInfo.dword;

    // Step 1. Set m7_cfgstcalib register bit[25] from 0x1 to 0x0, to enable alternative reference clock.
    uint32_t val = readl(&rFps->fpsCpuRegRegisters[coreID].fpsCpuCpuCfg1.all);
    val &= ~(BIT(25));
    writel(val, &rFps->fpsCpuRegRegisters[coreID].fpsCpuCpuCfg1.all);

    // Step 2. Set systick threshold register bit[5:0] to SYSTICK_THRESHOLD
    #if defined (CPU0) // config once will effect all FPS
    val = readl(&rFps->fpsBank0RegRegisters.fpsBank0FpsCfg.all);
    val |= SYSTICK_THRESHOLD;
    writel(val, &rFps->fpsBank0RegRegisters.fpsBank0FpsCfg.all);
    #endif
}

/**
 *  Initializes the debug module for logging (one time init).
 *
 *  @return nothing
 */
static void Logging_Initialize(void)
{
    uint8_t coreID = cpuInfo.dword;

    // Log Buffer Info
    // Log Buffer Info - share dtcm
    #if defined (CPU0)
    GetLogBufferInfo(coreID) = (volatile LogBufferInfo_t*)(M7_FPS_CPU01_LOG_BUF_INFO_ADDR);
    GetGdmaDqPi(coreID) = (volatile uint32_t*)(M7_FPS_CPU01_LOG_GDMA_DQ_PI);
    #elif defined (CPU1)
    GetLogBufferInfo(coreID) = (volatile LogBufferInfo_t*)(M7_FPS_CPU12_LOG_BUF_INFO_ADDR);
    GetGdmaDqPi(coreID) = (volatile uint32_t*)(M7_FPS_CPU12_LOG_GDMA_DQ_PI);
    #elif defined (CPU2)
    GetLogBufferInfo(coreID) = (volatile LogBufferInfo_t*)(M7_FPS_CPU20_LOG_BUF_INFO_ADDR);
    GetGdmaDqPi(coreID) = (volatile uint32_t*)(M7_FPS_CPU20_LOG_GDMA_DQ_PI);
    #endif
    *GetGdmaDqPi(coreID) = 0;

    // Fiber
    #ifndef INTEGRATE_TIMESTAMP_TO_FPSCPU
    M7FiberParameters_t fiberParamsTimestamp = {0};
    fiberParamsTimestamp.fiberWeight = cFiberWeightTimestamp;
    DEF_FIBERTIMESTAMP(coreID).Initialize(static_cast<M7CoreId_t>(coreID), cM7CompGroupLog, static_cast<M7FiberId_t>(LogFiberId_t::cLogFiberTimestampId), fiberParamsTimestamp, cM7FiberPriorityLow);
    #endif // End of INTEGRATE_TIMESTAMP_TO_FPSCPU
    M7FiberScheduler_RegisterCompGroup(static_cast<M7CoreId_t>(coreID), cM7CompGroupLog, static_cast<M7FiberId_t>(LogFiberId_t::cNumberOfFibers), LoggingRegisterFiber, NULL);
}

#ifdef LIONPERF_SUPPORT
void LoggingNormalBootInit(void)
{
    uint8_t coreID = cpuInfo.dword;

    // Log Buffer Info
    MEM_CLR((void*)GetLogBufferInfo(coreID), sizeof(LogBufferInfo_t));

    // Log 2K Buffer - psram
    GetLogBufferInfo(coreID)->Addr[0] = (uint32_t)(LOG_BUFFER_BASE_ADDRESS + ((LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU) * coreID));
    GetLogBufferInfo(coreID)->Addr[1] = (uint32_t)(LOG_BUFFER_BASE_ADDRESS + ((LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU) * coreID) + LOG_BUFFER_SIZE);
    MEM_CLR((void*)(GetLogBufferInfo(coreID)->Addr[0]), LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU);
    GetLogBufferInfo(coreID)->Len[0] = 0;
    GetLogBufferInfo(coreID)->Len[1] = 0;
    GetLogBufferInfo(coreID)->Sel = 0; // init use buffer 0
    GetLogBufferInfo(coreID)->Version[0] = 0;
    GetLogBufferInfo(coreID)->Version[1] = 0;

    // Log Extension
    MEM_CLR(logExtShared, LOG_EXTENSION_SIZE);
    writel(cLogInfo, &logExtShared->LevelsEnabledAtRunTime); // init level 'Info'
    GetLogExt(coreID)->LevelsEnabledAtRunTime = cLogInfo; // init level 'Info'
    writel(LOG_EN_DIS_UPDATE_DEFAULT_VALUE, &logExtShared->LogEnDisUpdate);
    GetLogExt(coreID)->LogEnDisUpdate = LOG_EN_DIS_UPDATE_DEFAULT_VALUE;
}

void LoggingResumeBootInit(FWupdateBackupInfo* FWupdateInfo)
{
    uint8_t coreID = cpuInfo.dword;

    #ifdef RESTORE_LOG_INFO_FROM_PSRAM_FOR_CPU0_CPU2
    // Currently in FPGA, loBufferInfo, which stored in shared dtcm, will not be clear after fp reset (except share 1/2 used by cpu1).
    // In case in the future(bootloadr), shared dtcm is cleared, we use psram to back up loBufferInfo.
    M7_MEM_COPY((uint8_t*)GetLogBufferInfo(coreID), (uint8_t*)(LOG_BUFFER_INFO_BASE_ADDRESS + (LOG_BUFFER_INFO_SIZE * coreID)), sizeof(struct LogBufferInfo_t));
    #else
    // in case in the future(bootloadr), all shared dtcm are cleared (for cpu0 and cpu2), but not yet enable assign RESTORE_LOG_INFO_FROM_PSRAM_FOR_CPU0_CPU2
    // assign log buffer address to avoid cpu crash.
    GetLogBufferInfo(coreID)->Addr[0] = (uint32_t)(LOG_BUFFER_BASE_ADDRESS + ((LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU) * coreID));
    GetLogBufferInfo(coreID)->Addr[1] = (uint32_t)(LOG_BUFFER_BASE_ADDRESS + ((LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_PER_FP_CPU) * coreID) + LOG_BUFFER_SIZE);
    #endif
    #if defined (CPU1)
    // cpu1 (share 1/2) must restore from psram
    M7_MEM_COPY((uint8_t*)GetLogBufferInfo(coreID), (uint8_t*)(LOG_BUFFER_INFO_BASE_ADDRESS + (LOG_BUFFER_INFO_SIZE * coreID)), sizeof(struct LogBufferInfo_t));
    #endif

    // Log Extension
    GetLogExt(coreID)->LevelsEnabledAtRunTime = readl(&logExtShared->LevelsEnabledAtRunTime);
    GetLogExt(coreID)->LogEnDisUpdate = readl(&logExtShared->LogEnDisUpdate);
    // restore LogEnDisUpdate info if action is enable
    if (GetLogExt(coreID)->LogEnDisUpdate == 1)
    {
        uint32_t loggingBackupAddr;
        if (FWupdateInfo->sts == cNoSignature)
        {
            loggingBackupAddr = (uint32_t)OLD_PSRAM_LOGGING_BACKUP;
        }
        else
        {
            loggingBackupAddr = (uint32_t)FWupdateInfo->addr;
        }


        *GetGdmaDqPi(coreID) = *(uint32_t*)(loggingBackupAddr + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * coreID) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_CURRENT_PI_OFFSET);
        gdmaDqDepth = *(uint32_t*)(loggingBackupAddr + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * coreID) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_Q_SIZE_OFFSET);
        Fps_t* rFps = (Fps_t*)FPS_REG_ADDR;
        #if defined (CPU0)
        rFps->fpsFp2hweRegRegisters[cFp2HweWq05GdmaDq].fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr =
            *(uint32_t*)(loggingBackupAddr + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * cM7Core0) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_DQ_PI_REG_ADDR_OFFSET);
        rFps->fpsFp2hweRegRegisters[cFp2HweWq05GdmaDq].fpsFp2hweFpToHweQSize.all = FindNextBit32(gdmaDqDepth);
        #elif defined (CPU1)
        rFps->fpsFp2hweRegRegisters[cFp2HweWq08GdmaDq].fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr =
            *(uint32_t*)(loggingBackupAddr + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * cM7Core1) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_DQ_PI_REG_ADDR_OFFSET);
        rFps->fpsFp2hweRegRegisters[cFp2HweWq08GdmaDq].fpsFp2hweFpToHweQSize.all = FindNextBit32(gdmaDqDepth);
        #elif defined (CPU2)
        rFps->fpsFp2hweRegRegisters[cFp2HweWq09GdmaDq].fpsFp2hweFpToHweQPiIndirectAddressPortFp2hweQPiIndirectRegAddr =
            *(uint32_t*)(loggingBackupAddr + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * cM7Core2) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_DQ_PI_REG_ADDR_OFFSET);
        rFps->fpsFp2hweRegRegisters[cFp2HweWq09GdmaDq].fpsFp2hweFpToHweQSize.all = FindNextBit32(gdmaDqDepth);
        #endif
    }

    DMB();
}
#endif

Error_t Logging_OneTimeInit(M7CoreId_t coreID)
{
    // set core id
    cpuInfo.dword = coreID;

    // enable systick
    SystickInit(0xffffff);

    Logging_Initialize();

    return cEcNoError;
}

void InitializeCoreLogging(M7CoreId_t coreID){

    //Initialize logging Memory
    Logging_OneTimeInit(coreID);
    //Initialize Circular Buffer for Cerberus Logging
    API_LoggingProducerOneTimeInit();

}

#ifdef LIONPERF_SUPPORT
//-----------------------------------------------------------------------------
//  Function Implement - CP2FP Message
//-----------------------------------------------------------------------------

bool LogdataTransferAlreadyEnabled(void)
{
    uint8_t coreID = 0;

    return (GetLogExt(coreID)->LogEnDisUpdate);
}

uint32_t LoggingLevelAtRunTime(void)
{
    uint8_t coreID = 0;

    return (GetLogExt(coreID)->LevelsEnabledAtRunTime);
}

void LoggingUpdateGdmaInfo(uint32_t queueDepth, uint32_t sqPi, uint32_t bufferTarget)
{
    uint8_t coreID = cpuInfo.dword;

    gdmaDqDepth = queueDepth;
    Fps_t* rFps = (Fps_t*)FPS_REG_ADDR;
    #if defined (CPU0)
    rFps->fpsFp2hweRegRegisters[cFp2HweWq05GdmaDq].fpsFp2hweFpToHweQSize.all = FindNextBit32(gdmaDqDepth);
    #elif defined (CPU1)
    rFps->fpsFp2hweRegRegisters[cFp2HweWq08GdmaDq].fpsFp2hweFpToHweQSize.all = FindNextBit32(gdmaDqDepth);
    #elif defined (CPU2)
    rFps->fpsFp2hweRegRegisters[cFp2HweWq09GdmaDq].fpsFp2hweFpToHweQSize.all = FindNextBit32(gdmaDqDepth);
    #endif

    *GetGdmaDqPi(coreID) = sqPi;
    if (GetGdmaDqPiBufferTarget(coreID) != bufferTarget)
    {
        // local buffer target is not consistent with CP msg, reset both log buffer
        GetLogBufferInfo(coreID)->Len[0] = 0;
        GetLogBufferInfo(coreID)->Len[1] = 0;
        GetGdmaDqPiBufferTarget(coreID) = bufferTarget;
        GetLogBufferInfo(coreID)->Sel = bufferTarget;
        //AddTimestampInBuffer();

    } // else do nothing

    // backup for fw update
    *(uint32_t*)(LOG_EN_DIS_UPDATE_BACKUP_BASE_ADDRESS + (LOG_EN_DIS_UPDATE_BACKUP_SIZE * coreID) + LOG_EN_DIS_UPDATE_BACKUP_GDMA_Q_SIZE_OFFSET) = gdmaDqDepth;

}

void LoggingUpdateLogExt(LogExt_t* pLogExt, LogExtSet_t logExtSet)
{
    uint8_t coreID = 0;
    LogExt_t* _logExt = GetLogExt(coreID);

    // update local and psram share var
    switch (logExtSet)
    {
        case cLogExtSetLevelsEnabledAtRunTime:
        {
            _logExt->LevelsEnabledAtRunTime = pLogExt->LevelsEnabledAtRunTime;
            writel(_logExt->LevelsEnabledAtRunTime, &logExtShared->LevelsEnabledAtRunTime);
            break;
        }
        case cLogExtSetLogEnDisUpdate:
        {
            if (pLogExt->LogEnDisUpdate == 0)
            {
                // transfer the current log buffer (may not be full) first, then disable LogEnDisUpdate
                HandleCurrentFullBuffer();
            } // else do nothing
            _logExt->LogEnDisUpdate = pLogExt->LogEnDisUpdate;
            writel(_logExt->LogEnDisUpdate, &logExtShared->LogEnDisUpdate);
            break;
        }
        default:
            break;
    }
}

void LoggingUpdateLogExtByLogExtShared(void)
{
    uint8_t coreID = 0;
    LogExt_t* _logExt = GetLogExt(coreID);

    _logExt->LevelsEnabledAtRunTime = readl(&logExtShared->LevelsEnabledAtRunTime);
    if (readl(&logExtShared->LogEnDisUpdate) == 0)
    {
        // transfer the current log buffer (may not be full) first, then disable LogEnDisUpdate
        HandleCurrentFullBuffer();
    } // else do nothing
    _logExt->LogEnDisUpdate = readl(&logExtShared->LogEnDisUpdate);
}

//-----------------------------------------------------------------------------
//  Non-inline version (for iram size concern)
//-----------------------------------------------------------------------------

uint32_t SendMessageToBuffer(uint16_t token, uint32_t* pDest, uint32_t argNum, va_list arguments)
{
    // DW0: the tokenop(16 bits) and the timestamp(16bits)
    GenerateTokenPrimDw0(pDest, token);

    // DW1~3: argument 1~3
    for (uint32_t i = 1; i <= argNum; i++)
    {
        uint32_t argument = va_arg(arguments, uint32_t);
        GenerateTokenPrimArgument((pDest + i), argument);
    }


    return (1 + argNum);   // token primitive size in dword
}
void DebugPrintfToken(uint16_t token, uint32_t argNum, ...)
{
    uint8_t coreID = 0;
    LogExt_t* _logExt = GetLogExt(coreID);

    if (GetTokenLogLevel(token) > _logExt->LevelsEnabledAtRunTime)
    {
        return;
    } // else do nothing

    va_list arguments;
    va_start(arguments, argNum);

    volatile LogBufferInfo_t* _logBufferInfo = GetLogBufferInfo(coreID);
    uint32_t xferSize = 0; // in dword
    uint32_t xferSizeInByte = 0;
    uint32_t* pDest = 0;
    if (likely((_logBufferInfo->Len[_logBufferInfo->Sel] + cTokenPrimitiveMaxSizeInByte) <= LOG_BUFFER_SIZE))
    {
        // current buffer is not full, and enough for 1 new token primitive
        pDest = (uint32_t*)(_logBufferInfo->Addr[_logBufferInfo->Sel] + _logBufferInfo->Len[_logBufferInfo->Sel]);
        xferSize = SendMessageToBuffer(token, pDest, argNum, arguments);
        xferSizeInByte = xferSize << 2;
        _logBufferInfo->Len[_logBufferInfo->Sel] += xferSizeInByte;
    }
    else
    {
        if (((_logBufferInfo->Len[!(_logBufferInfo->Sel)] + cTokenPrimitiveMaxSizeInByte) <= LOG_BUFFER_SIZE))
        {
            HandleCurrentFullBuffer();

            // change to another buffer
            _logBufferInfo->Sel = !(_logBufferInfo->Sel);
            GetGdmaDqPiBufferTarget(coreID) = _logBufferInfo->Sel;
            //AddTimestampInBuffer();
            pDest = (uint32_t*)(_logBufferInfo->Addr[_logBufferInfo->Sel] + _logBufferInfo->Len[_logBufferInfo->Sel]);
            xferSize = SendMessageToBuffer(token, pDest, argNum, arguments);
            xferSizeInByte = xferSize << 2;
            _logBufferInfo->Len[_logBufferInfo->Sel] += xferSizeInByte;
        } // else do nothing (the full buffer will be cleared in HandleCurrentFullBuffer(), so there is no 'both log buffer full' case)
    }
    va_end(arguments);
}
#else
#define LOG_BUFFER_MAX_ARGUMENT_NUM 2
void DebugPrintfToken(uint16_t token, uint32_t argNum, ...)
{
    if( GetTokenLogLevel(token) < cLogDebug)
    {
        va_list arguments;
        va_start(arguments, argNum);
        uint32_t argument[LOG_BUFFER_MAX_ARGUMENT_NUM] = {0};
        for(int i=0; i<((argNum>LOG_BUFFER_MAX_ARGUMENT_NUM)?LOG_BUFFER_MAX_ARGUMENT_NUM:argNum); i++)
        {
            argument[i] = va_arg(arguments, uint32_t);
        }
        API_AddDebugLog(token & 0xff, GetTokenLogLevel(token) & 0xFF, argument[0], argument[1]);
    }
}
#endif
