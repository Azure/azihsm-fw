// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   LoggingDebug.h
//! @brief  System debugging API & MACRO.
//!
//=============================================================================

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stdarg.h>
#include <stdint.h>
#include "ErrorCodes.h"
#include "M7MemMap.h"
#include "M7Partition.h"
#include "SysTypes.h"
#include "MemIo.h"
#include "common.h"
#ifdef LOGGING_NEW_SCHEME
#include "LoggingDebugCategory.h"
#include "RegFps.h" // fps reg
#include "FpCommon.h" // fps reg
#endif
#include "RegCortexm7.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define Debug_Log(Category, Level, Message) {};

//-----------------------------------------------------------------------------
//  Public data structure definitions and defines
//-----------------------------------------------------------------------------

#define cDebugTokenSize               4                 ///< DWORD size
#ifdef LOGGING_NEW_SCHEME
#define cTokenPrimitiveMaxSizeInByte 16                 ///< token primitive max size: 16 bytes (for level dbg/info)
#else
#define cTokenPrimitiveMaxSizeInByte 32                 ///< token primitive max size: 32 bytes (for all level)
#endif
//#define RESTORE_LOG_INFO_FROM_PSRAM_FOR_CPU0_CPU2     ///< back up loBufferInfo in psram for fw update

/**
 *  Log Buffer
 */
#define FP_CPU_NUMBER 3                                 ///< 3 FP_CPUs
#define LOG_BUFFER_NUMBER_TOTAL (LOG_BUFFER_NUMBER_PER_FP_CPU * FP_CPU_NUMBER)                                                  ///< Total 6 buffers
#define LOG_BUFFER_BASE_ADDRESS PSRAM_LOG_DUMP_START                                                                           ///< Offset 0
#define LOG_BUFFER_SIZE 0x800                           ///< 2K bytes for one buffer
#define LOG_BUFFER_NUMBER_PER_FP_CPU 2                  ///< 2 buffers for single FP_CPU

typedef struct LogBufferInfo_t
{
    uint32_t Addr[2];                                   ///< Buffer 0 and 1 address in psram
    uint32_t Len[2];                                    ///< Current used log length in buffer 0 and 1 (in bytes)
    uint32_t Sel;                                       ///< Current buffer select 0 or 1
    #ifdef LOGGING_NEW_SCHEME
    uint32_t Version[2];                                ///< Each version[N] means the log buffer N's data version.
    #else
    uint32_t FullBitmap[2];                             ///< Indicate which buffer is full. Both empty and both full are possible
                                                        ///< [0] = 1: buffer 0 full
                                                        ///< [1] = 1: buffer 1 full
    #endif
} LogBufferInfo_t;

typedef enum LoggingSts
{
    loggingSuccess = 0,              /// < logging success
    loggingFail,              /// < logging fail
    loggingmsgFail,                     /// < logging message fail
}LoggingSts;
#define LOG_BUFFER_INFO_SIZE 0x1C                       ///< 28 bytes
#define LOG_BUFFER_INFO_NUMBER_PER_FP_CPU 1             ///< 1 buffer info for single FP_CPU
#define LOG_BUFFER_INFO_NUMBER_TOTAL (LOG_BUFFER_INFO_NUMBER_PER_FP_CPU * FP_CPU_NUMBER)                                        ///< Total 3 buffer info
// Note: If enable LOGGING_NEW_SCHEME, log buffer info will store in share dtcm. So there will be a fragment in psram.
// Note: In LOGGING_NEW_SCHEME, we may use this fragment to backup logBufferInfo for resume boot.
#define LOG_BUFFER_INFO_BASE_ADDRESS (LOG_BUFFER_BASE_ADDRESS + (LOG_BUFFER_SIZE * LOG_BUFFER_NUMBER_TOTAL))                    ///< Offset 0x3000
static_assert(sizeof(LogBufferInfo_t) == LOG_BUFFER_INFO_SIZE, "LogBufferInfo_t size shall be 28 bytes");

/**
 *  Log Extension
 */
typedef struct LogExt_t
{
    uint32_t LevelsEnabledAtRunTime;                     ///< Log can only be shown if log level is equal to or greater then levelsEnabledAtRunTime
    #ifdef LOGGING_NEW_SCHEME
    uint32_t LogEnDisUpdate;                                ///< Need GDMA full Log buffer if LogEnDisUpdate is enabled. 1: enable, 0: disable
    #endif
} LogExt_t;
#ifdef LOGGING_NEW_SCHEME
#define LOG_EXTENSION_SIZE 0x8                           ///< 8 bytes
static_assert(sizeof(LogExt_t) == LOG_EXTENSION_SIZE, "LogExt_t size shall be 8 bytes");
#else
#define LOG_EXTENSION_SIZE 0x4                           ///< 4 bytes
static_assert(sizeof(LogExt_t) == LOG_EXTENSION_SIZE, "LogExt_t size shall be 4 bytes");
#endif
#define LOG_EXTENSION_NUMBER_TOTAL 1                    ///< Only one instance is created for all FP_CPUs

///<  End of psram logging usage: Offset (old)0x3430, (new)0x3434

typedef enum LogExtSet_t
{
    cLogExtSetLevelsEnabledAtRunTime = 0x0,
    #ifdef LOGGING_NEW_SCHEME
    cLogExtSetLogEnDisUpdate,
    #endif
} LogExtSet_t;

/**
 *  Backup struct for LogEnDisUpdate for fw update.
 */
typedef struct LogEnDisUpdateBackup_t
{
    uint32_t gdmaDqPiRegAddr;           /// < log buffer's GDMA delivery queue's PI address
    uint32_t gdmaCurrentPi;             /// < log buffer's GDMA delivery queue's current PI (right before fw update)
    uint32_t gdmaQSize;                 /// < length of GDMA queue
} LogEnDisUpdateBackup_t;
#define LOG_EN_DIS_UPDATE_BACKUP_BASE_ADDRESS PSRAM_LOGGING_BACKUP  ///< logging backup address offset 0x0
#define LOG_EN_DIS_UPDATE_BACKUP_SIZE 0xC                           ///< 12 bytes
static_assert(sizeof(LogEnDisUpdateBackup_t) == LOG_EN_DIS_UPDATE_BACKUP_SIZE, "LogEnDisUpdateBackup_t size shall be 12 bytes");
#define LOG_EN_DIS_UPDATE_BACKUP_NUMBER_PER_FP_CPU 1                ///< 1 backup for single FP_CPU
#define LOG_EN_DIS_UPDATE_BACKUP_NUMBER_TOTAL (LOG_EN_DIS_UPDATE_BACKUP_NUMBER_PER_FP_CPU * FP_CPU_NUMBER)   ///< Total 3 backup
#define LOG_EN_DIS_UPDATE_BACKUP_GDMA_DQ_PI_REG_ADDR_OFFSET 0x0     ///< offset of gdmaDqPiRegAddr
#define LOG_EN_DIS_UPDATE_BACKUP_GDMA_CURRENT_PI_OFFSET 0x4         ///< offset of gdmaCurrentPi
#define LOG_EN_DIS_UPDATE_BACKUP_GDMA_Q_SIZE_OFFSET 0x8             ///< offset of gdmaQSize

extern volatile LogBufferInfo_t* logBufferInfo;
extern LogExt_t logExt;
#define GetLogBufferInfo(coreID) (logBufferInfo)
#define GetLogExt(coreID) (&logExt)

#ifdef LOGGING_NEW_SCHEME
#define SystickCtrl 0xE000E010
#define SystickLoad 0xE000E014
#define SystickCurVal 0xE000E018
#define systick_cal 0xE000E01C
extern uint32_t gdmaDqDepth;
//extern uint32_t localTimestamp;
//extern LogCategory_t gLogCategory;
extern volatile uint32_t* gdmaDqPi;
extern uint32_t gdmaDqPiBufferTarget;
#define GetGdmaDqPi(id) gdmaDqPi
#define GetGdmaDqPiBufferTarget(id) gdmaDqPiBufferTarget
#endif

extern uint32_t gTimeStampBase;

#define SYSTICK_THRESHOLD 0x31 ///< Value from 0x0 to 0x3F.
                               ///< Rule: Bigger threshold makes lower systick cycling. In order to decrease timestamp log count, biggeer threshold is
                               ///< better. However, if the 2 continuous logs have the same timestamp, the threshold shall be decrease.

//-----------------------------------------------------------------------------
//  Tokenization
//-----------------------------------------------------------------------------

/**
 *   Logging API for log level degbug and info
 *
 *   DebugLogLvDbgInfoInline(): It will be replaced to "DebugPrintfToken<N>Argument()" during compile time.
 *                              Inline function increases iram size, only IO log can use it for performance concern.
 *   DebugLogLvDbgInfo(): It will be replaced to "DebugPrintfToken()" during compile time.
 *
 *   @param    logCategory     [in ] log category
 *   @param    logCategory     [in ] logLevel
 *   @param    fmt, ...        [in ] debug msg and arguments
 **/
#define DebugLogLvDbgInfoInline(logCategory, logLevel, fmt, ...)
#define DebugLogLvDbgInfo(logCategory, logLevel, fmt, ...)

//-----------------------------------------------------------------------------
//  Public Function Prototypes (other modules can use these functions by
//  including this .h file):
//-----------------------------------------------------------------------------

/**
 *   One-time initialization of Debug module.\n
 *     - set all logging variables and resource.
 *
 *   @param    coreID     [in ] FPS core ID.
 *   @return error code
 **/
Error_t Logging_OneTimeInit(M7CoreId_t coreID);

/**
 *   Logging initialization.
 *
 *   @param    coreID     [in ] FPS core ID.
 *   @return    nothing
 **/

void InitializeCoreLogging(M7CoreId_t coreID);

#ifdef LIONPERF_SUPPORT
/**
 *  Initializes the debug module for logging for normal boot.
 *
 *  @return nothing
 */
void LoggingNormalBootInit(void);

/**
 *  Initializes the debug module for logging for resume boot.
 *
 *  @return nothing
 */
void LoggingResumeBootInit(FWupdateBackupInfo* FWupdateInfo);
#endif

#ifdef LOGGING_NEW_SCHEME

/**
 *  Check if LogdataTransfer is already enabled or not.
 *  Continuous enable is not accepted.
 */
bool LogdataTransferAlreadyEnabled(void);

/**
 *  Get current logging level.
 *  @return   current logging level
 */
uint32_t LoggingLevelAtRunTime(void);

#ifdef LIONPERF_SUPPORT
/**
 *  Update local gdma variable when receive cp2fp msg LogEnDisUpdate.
 *
 *  @param    queueDepth       [in ] gdma sq depth.
 *  @param    sqPi             [in ] gdma sq pi
 *  @param    bufferTarget     [in ] log buffer target (0 or 1)
 *  @return   nothing
 */
void LoggingUpdateGdmaInfo(uint32_t queueDepth, uint32_t sqPi, uint32_t bufferTarget);
#endif

/**
 *  Logging function for log level debug and info
 *  Auto transferred from "DebugLogLvDbgInfo()" by C# tool Tokenizer.exe
 *  Should not call it directly, instead, please use DebugLogLvDbgInfo()
 *
 *  @param    token           [in ] token generated by C# tool Tokenizer.exe. Token includes log level and category information.
 *  @param    argument1, ...  [in ] arguments of format string
 *  @return   nothing
 **/
void DebugPrintfToken(uint16_t token, uint32_t argNum, ...);
#else

/**
 *  Token Function for Log Level 'Warning' and 'Error'
 *
 *  Generate Token Primitive and store it in Token Primitive Buffer in PSRAM.
 *  Notify FP_CPU2 that there's a new Token Primitive generated thru FP internal notification queue.
 *
 *  @param    argumentCount     [in ] The number of arguments that follow the String Mask.
 *  @param    stringMask        [in ] Indicates which Arguments are strings
 *  @param    tokenOpcode       [in ] Defined in RsvdTokens.dat
 *  @param    ...               [in ] Optional arguments, types and numbers are defined in RsvdTokens.dat
 *  @return   nothing
 */
void DebugSendWarningError(uint32_t argumentCount, const uint32_t stringMask, uint32_t tokenOpcode, ...);
#endif

#ifdef LIONPERF_SUPPORT
/**
 *  Update local variable logExt by input logExt (with specific logExtSet).
 *  Then update psram share logExt by local logExt (whole var).
 *
 * FpsCpu2 use only.
 *
 *  @param    pLogExt       [in ] Input logExt.
 *  @param    logExtSet     [in ] Which item in logExt is going to be update
 *  @return   nothing
 */
void LoggingUpdateLogExt(LogExt_t* pLogExt, LogExtSet_t logExtSet);

/**
 *  Update local variable logExt by logExtShared in PSRAM
 *
 * FpsCpu0 and FpsCpu1 use only.
 *
 *  @return   nothing
 */
void LoggingUpdateLogExtByLogExtShared(void);
#endif


#ifdef LOGGING_NEW_SCHEME
//-----------------------------------------------------------------------------
//  Inline Function Prototypes
//  (Inline for performance concern. Inline function cannot use variable arguments,
//  so there are different functions for different argument number.)
//-----------------------------------------------------------------------------
/**
 *  Logging function for log level debug and info
 *  Auto transferred from "DebugLogLvDbgInfoInline()" by C# tool Tokenizer.exe
 *  Should not call it directly, instead, please use DebugLogLvDbgInfoInline()
 *
 *  @param    token           [in ] token generated by C# tool Tokenizer.exe. Token includes log level and category information.
 *  @param    argument1, ...  [in ] arguments of format string
 *  @return   nothing
 */
static ATTR_ALWAYS_INLINE void DebugPrintfToken1Argument(uint16_t token, uint32_t argument1);
static ATTR_ALWAYS_INLINE void DebugPrintfToken2Argument(uint16_t token, uint32_t argument1, uint32_t argument2);
static ATTR_ALWAYS_INLINE void DebugPrintfToken3Argument(uint16_t token, uint32_t argument1, uint32_t argument2, uint32_t argument3);

/**
 *  Used in DebugPrintfTokenNArgument()
 *  Store token primitive to log 2k buffer
 *
 *  @param    token           [in ] token generated by C# tool Tokenizer.exe. Token includes log level and category information.
 *  @param    pDest           [in ] address and offset of log 2k buffer
 *  @param    argument1, ...  [in ] arguments of format string
 *  @return   token primitive length (in dword)
 */
static ATTR_ALWAYS_INLINE uint32_t SendMessageToBuffer1Argument(uint16_t token, uint32_t* pDest, uint32_t argument1);
static ATTR_ALWAYS_INLINE uint32_t SendMessageToBuffer2Argument(uint16_t token, uint32_t* pDest, uint32_t argument1, uint32_t argument2);
static ATTR_ALWAYS_INLINE uint32_t SendMessageToBuffer3Argument(uint16_t token, uint32_t* pDest, uint32_t argument1, uint32_t argument2, uint32_t argument3);

/**
 *  Used in SendMessageToBufferNArgument()
 *  Store 1 dword of token primitive to log 2k buffer
 */
static ATTR_ALWAYS_INLINE void GenerateTokenPrimDw0(uint32_t* pDest, uint16_t token);
static ATTR_ALWAYS_INLINE void GenerateTokenPrimArgument(uint32_t* pDest, uint32_t argument);
static ATTR_ALWAYS_INLINE unsigned int RevertArgument(unsigned int value);

/**
 *  Used in DebugPrintfTokenNArgument()
 *  Handle full 2k buffer
 */
static ATTR_ALWAYS_INLINE void HandleCurrentFullBufferInline(uint32_t sel);

#if 0 // disabled due to code size concern
/**
 *  Used in DebugPrintfTokenNArgument()
 *  Put 8 bytes timestamp log in the beginning of each buffer as header, for tool to merge cpu0/1/2 buffers.
 *  The 8 bytes is hard code, don't check Tokens.dat when detokenization
 */
static ATTR_ALWAYS_INLINE void AddTimestampInBufferInline(void);
#endif

//-----------------------------------------------------------------------------
//  Inline Function Implements
//-----------------------------------------------------------------------------
#ifdef LIONPERF_SUPPORT
static inline void DebugPrintfToken1Argument(uint16_t token, uint32_t argument1)
{
    uint8_t coreID = 0;
    LogExt_t* _logExt = GetLogExt(coreID);

    if (GetTokenLogLevel(token) > _logExt->LevelsEnabledAtRunTime)
    {
        return;
    } // else do nothing

    volatile LogBufferInfo_t* _logBufferInfo = GetLogBufferInfo(coreID);
    uint32_t logBufferLength, logBufferAddr;
    uint32_t xferSize = 0; // in dword
    uint32_t xferSizeInByte = 0;
    uint32_t* pDest = 0;
    uint32_t sel = _logBufferInfo->Sel;
    uint32_t selReverse = ((sel + 1) & 0x1);

    logBufferLength = _logBufferInfo->Len[sel];
    if (likely((logBufferLength + cTokenPrimitiveMaxSizeInByte) <= LOG_BUFFER_SIZE))
    {
        logBufferAddr = _logBufferInfo->Addr[sel];

        // current buffer is not full, and enough for 1 new token primitive
        pDest = (uint32_t*)(logBufferAddr + logBufferLength);
        xferSize = SendMessageToBuffer1Argument(token, pDest, argument1);
        xferSizeInByte = xferSize << 2;
        logBufferLength += xferSizeInByte;
        writel(logBufferLength, &(_logBufferInfo->Len[sel]));
    }
    else
    {
        logBufferLength = _logBufferInfo->Len[selReverse];
        if (((logBufferLength + cTokenPrimitiveMaxSizeInByte) <= LOG_BUFFER_SIZE))
        {
            logBufferAddr = _logBufferInfo->Addr[selReverse];

            HandleCurrentFullBufferInline(sel);

            // change to another buffer
            _logBufferInfo->Sel = selReverse;
            GetGdmaDqPiBufferTarget(coreID) = selReverse;

            //AddTimestampInBufferInline();
            pDest = (uint32_t*)(logBufferAddr + logBufferLength);
            xferSize = SendMessageToBuffer1Argument(token, pDest, argument1);
            xferSizeInByte = xferSize << 2;
            logBufferLength += xferSizeInByte;
            writel(logBufferLength, &(_logBufferInfo->Len[selReverse]));
        } // else do nothing (the full buffer will be cleared in HandleCurrentFullBufferInline(), so there is no 'both log buffer full' case)
    }
}

static inline void DebugPrintfToken2Argument(uint16_t token, uint32_t argument1, uint32_t argument2)
{
    uint8_t coreID = 0;
    LogExt_t* _logExt = GetLogExt(coreID);

    if (GetTokenLogLevel(token) > _logExt->LevelsEnabledAtRunTime)
    {
        return;

    } // else do nothing

    volatile LogBufferInfo_t* _logBufferInfo = GetLogBufferInfo(coreID);
    uint32_t xferSize = 0; // in dword
    uint32_t xferSizeInByte = 0;
    uint32_t* pDest = 0;

    if (likely((_logBufferInfo->Len[_logBufferInfo->Sel] + cTokenPrimitiveMaxSizeInByte) <= LOG_BUFFER_SIZE))
    {
        // current buffer is not full, and enough for 1 new token primitive
        pDest = (uint32_t*)(_logBufferInfo->Addr[_logBufferInfo->Sel] + _logBufferInfo->Len[_logBufferInfo->Sel]);
        xferSize = SendMessageToBuffer2Argument(token, pDest, argument1, argument2);
        xferSizeInByte = xferSize << 2;
        _logBufferInfo->Len[_logBufferInfo->Sel] += xferSizeInByte;
    }
    else
    {
        if (((_logBufferInfo->Len[!(_logBufferInfo->Sel)] + cTokenPrimitiveMaxSizeInByte) <= LOG_BUFFER_SIZE))
        {
            HandleCurrentFullBufferInline(_logBufferInfo->Sel);

            // change to another buffer
            _logBufferInfo->Sel = !(_logBufferInfo->Sel);
            GetGdmaDqPiBufferTarget(coreID) = _logBufferInfo->Sel;
            //AddTimestampInBufferInline();
            pDest = (uint32_t*)(_logBufferInfo->Addr[_logBufferInfo->Sel] + _logBufferInfo->Len[_logBufferInfo->Sel]);
            xferSize = SendMessageToBuffer2Argument(token, pDest, argument1, argument2);
            xferSizeInByte = xferSize << 2;
            _logBufferInfo->Len[_logBufferInfo->Sel] += xferSizeInByte;
        } // else do nothing (the full buffer will be cleared in HandleCurrentFullBufferInline(), so there is no 'both log buffer full' case)
    }
}

static inline void DebugPrintfToken3Argument(uint16_t token, uint32_t argument1, uint32_t argument2, uint32_t argument3)
{
    uint8_t coreID = 0;
    LogExt_t* _logExt = GetLogExt(coreID);

    if (GetTokenLogLevel(token) > _logExt->LevelsEnabledAtRunTime)
    {
        return;

    } // else do nothing

    volatile LogBufferInfo_t* _logBufferInfo = GetLogBufferInfo(coreID);
    uint32_t xferSize = 0; // in dword
    uint32_t xferSizeInByte = 0;
    uint32_t* pDest = 0;

    if (likely((_logBufferInfo->Len[_logBufferInfo->Sel] + cTokenPrimitiveMaxSizeInByte) <= LOG_BUFFER_SIZE))
    {
        // current buffer is not full, and enough for 1 new token primitive
        pDest = (uint32_t*)(_logBufferInfo->Addr[_logBufferInfo->Sel] + _logBufferInfo->Len[_logBufferInfo->Sel]);
        xferSize = SendMessageToBuffer3Argument(token, pDest, argument1, argument2, argument3);
        xferSizeInByte = xferSize << 2;
        _logBufferInfo->Len[_logBufferInfo->Sel] += xferSizeInByte;
    }
    else
    {
        if (((_logBufferInfo->Len[!(_logBufferInfo->Sel)] + cTokenPrimitiveMaxSizeInByte) <= LOG_BUFFER_SIZE))
        {
            HandleCurrentFullBufferInline(_logBufferInfo->Sel);

            // change to another buffer
            _logBufferInfo->Sel = !(_logBufferInfo->Sel);
            GetGdmaDqPiBufferTarget(coreID) = _logBufferInfo->Sel;
            //AddTimestampInBufferInline();
            pDest = (uint32_t*)(_logBufferInfo->Addr[_logBufferInfo->Sel] + _logBufferInfo->Len[_logBufferInfo->Sel]);
            xferSize = SendMessageToBuffer3Argument(token, pDest, argument1, argument2, argument3);
            xferSizeInByte = xferSize << 2;
            _logBufferInfo->Len[_logBufferInfo->Sel] += xferSizeInByte;
        } // else do nothing (the full buffer will be cleared in HandleCurrentFullBufferInline(), so there is no 'both log buffer full' case)
    }
}
#else
#include "APILogging.h"
static inline void DebugPrintfToken1Argument(uint16_t token, uint32_t argument1)
{
    if( GetTokenLogLevel(token) < cLogDebug)
    {
        API_AddDebugLog(token & 0xff, GetTokenLogLevel(token) & 0xFF, argument1, 0);
    }
}
static inline void DebugPrintfToken2Argument(uint16_t token, uint32_t argument1, uint32_t argument2)
{
    if( GetTokenLogLevel(token) < cLogDebug)
    {
        API_AddDebugLog(token & 0xff, GetTokenLogLevel(token) & 0xFF, argument1, argument2);
    }
}
static inline void DebugPrintfToken3Argument(uint16_t token, uint32_t argument1, uint32_t argument2, uint32_t argument3)
{
    if( GetTokenLogLevel(token) < cLogDebug)
    {
        // NOTE: We only support two arguments to be sent to DebugLogRead buffer.
        API_AddDebugLog(token & 0xff, GetTokenLogLevel(token) & 0xFF, argument1, argument2);
    }
}
#endif

static inline uint32_t SendMessageToBuffer1Argument(uint16_t token, uint32_t* pDest, uint32_t argument1)
{
    // DW0: the tokenop(16 bits) and the timestamp(16bits)
    GenerateTokenPrimDw0(pDest, token);

    // DW1: argument 1
    GenerateTokenPrimArgument((pDest + 1), argument1);


    return 2;   // token primitive size: 2 dword
}

static inline uint32_t SendMessageToBuffer2Argument(uint16_t token, uint32_t* pDest, uint32_t argument1, uint32_t argument2)
{
    // DW0: the tokenop(16 bits) and the timestamp(16bits)
    GenerateTokenPrimDw0(pDest, token);

    // DW1: argument 1
    GenerateTokenPrimArgument((pDest + 1), argument1);

    // DW2: argument 2
    GenerateTokenPrimArgument((pDest + 2), argument2);


    return 3;   // token primitive size: 3 dword
}

static inline uint32_t SendMessageToBuffer3Argument(uint16_t token, uint32_t* pDest, uint32_t argument1, uint32_t argument2, uint32_t argument3)
{
    // DW0: the tokenop(16 bits) and the timestamp(16bits)
    GenerateTokenPrimDw0(pDest, token);

    // DW1: argument 1
    GenerateTokenPrimArgument((pDest + 1), argument1);

    // DW2: argument 2
    GenerateTokenPrimArgument((pDest + 2), argument2);

    // DW3: argument 3
    GenerateTokenPrimArgument((pDest + 3), argument3);


    return 4;   // token primitive size: 4 dword
}

static inline void GenerateTokenPrimDw0(uint32_t* pDest, uint16_t token)
{
    uint32_t timestamp = 0;

    // DW0: the tokenop(16 bits) and the timestamp(16bits)
    timestamp = 0xFFFFFF - readl(SystickCurVal);
    timestamp = (timestamp + gTimeStampBase) & SYSTICK_MASK;
    *pDest = MAKE_U32_4(LOW8(timestamp), HIGH8(timestamp), LOW8(token), HIGH8(token)); // need swap endianness
}

static inline void GenerateTokenPrimArgument(uint32_t* pDest, uint32_t argument)
{
    // DW N: argument N(must be non-string)
    *pDest = RevertArgument(argument);
}

static inline unsigned int RevertArgument(unsigned int value)
{
    return ((value >> 24) & 0xff)       |   // move byte 3 to byte 0
           ((value << 8) & 0xff0000)    |   // move byte 1 to byte 2
           ((value >> 8) & 0xff00)      |   // move byte 2 to byte 1
           ((value << 24) & 0xff000000);    // byte 0 to byte 3
}

static inline void HandleCurrentFullBufferInline(uint32_t sel)
{
    uint8_t coreID = 0;
    volatile LogBufferInfo_t* _logBufferInfo = GetLogBufferInfo(coreID);
    LogExt_t* _logExt = GetLogExt(coreID);
    uint32_t logBufferLength, logBufferAddr;

    _logBufferInfo->Version[sel]++;

    if (_logExt->LogEnDisUpdate)
    {
        // zero out remind bytes in 2K log buffer
        uint32_t* pDest = 0;
        logBufferLength = _logBufferInfo->Len[sel];
        logBufferAddr = _logBufferInfo->Addr[sel];
        while ((logBufferLength + sizeof(uint32_t)) <= LOG_BUFFER_SIZE)
        {
            pDest = (uint32_t*)(logBufferAddr + logBufferLength);
            writel(0, pDest); // fill 4 bytes zero

            logBufferLength = logBufferLength + sizeof(uint32_t);
        }
        writel(logBufferLength, &(_logBufferInfo->Len[sel]));

        // gdma full buffer 2K data from psram to gsram
        *GetGdmaDqPi(coreID) = QUEUE_INC(*GetGdmaDqPi(coreID), (gdmaDqDepth - 1));
        #ifdef SUPPORT_FPS_REGISTER
        Fps_t* _rFps = (Fps_t*)FPS_REG_ADDR;
        #if defined (CPU0)
        writel(*GetGdmaDqPi(coreID), &(_rFps->fpsFp2hweRegRegisters[cFp2HweWq05GdmaDq].fpsFp2hweFpToHweQPiIndirectDataPort));
        #elif defined (CPU1)
        writel(*GetGdmaDqPi(coreID), &(_rFps->fpsFp2hweRegRegisters[cFp2HweWq08GdmaDq].fpsFp2hweFpToHweQPiIndirectDataPort));
        #elif defined (CPU2)
        writel(*GetGdmaDqPi(coreID), &(_rFps->fpsFp2hweRegRegisters[cFp2HweWq09GdmaDq].fpsFp2hweFpToHweQPiIndirectDataPort));
        #endif
        #endif

    } // else do nothing

    // empty log buffer for futher use
    writel(0, &(_logBufferInfo->Len[sel]));

}

#if 0
static inline void AddTimestampInBufferInline(void)
{
    uint8_t coreID = 0;
    volatile LogBufferInfo_t* _logBufferInfo = GetLogBufferInfo(coreID);
    uint32_t timestamp = 0;

    timestamp = 0xFFFFFF - readl(SystickCurVal);
    // update local timestamp
    localTimestamp = timestamp;

    uint16_t timestampToken = (cLogInfo << 14) + (gLogCategory << 10);      // log level, category, index
    uint32_t* pDest = (uint32_t*)(_logBufferInfo->Addr[_logBufferInfo->Sel]);

    SendMessageToBuffer1Argument(timestampToken, pDest, timestamp);
    _logBufferInfo->Len[_logBufferInfo->Sel] += 8;      // 8 bytes fixed size
}
#endif

#endif


#ifdef __cplusplus
}
#endif
