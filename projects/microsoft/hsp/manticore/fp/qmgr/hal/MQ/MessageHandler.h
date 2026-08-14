// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//
//! @file     MessageHandler.h
//! @brief   Message Handler
//!
//=============================================================================

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
// ----------------------------------------------------------------------------
#include "platform.h"

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------
#define MSG_CONTEXT_MAX_DATA_LENGTH 60

//-----------------------------------------------------------------------------
//  Message Opcode definition
//-----------------------------------------------------------------------------

#define FP_TO_CP_ERR_MSG_LEN 0x4
enum MsgOp
{
    msgOpFpStsChange = 0,
    msgOpErrQSet,
    msgOpFpModeChange,
    msgOpVfSlotSQ2CQMapUpdate,
    msgOpUpdateTimestampAddr,
    msgOpVfUpdate,
    msgOpCpCdmaIo,
    msgOpKeyUpdate,
    msgOpSetLogLevel,
    msgOpFpTokenReq,
    msgOpUcdQuery,
    msgOpTelemetryQuery,
    msgOpLogEnDisUpdate,
    msgOpCDMAStatSet,
    #ifdef QOS_LATENCY_ERROR_HANDLING
    msgOpQoSPenalty,
    #endif
    msgOpChkAlive,
    #ifdef SUPPORT_MSGERROR_INJECTION
    msgOpMsgErrorInjection,
    #endif
    #ifdef SUPPORT_CDMA_RESET_MSG
    msgOpCDMAReset,
    #endif
    msgOpShutdownReq = 0x42,
    msgOpInjectErrorReq = 0x47,
    msgOpNum,
};

enum CPCoreId_t
{
    CP0 = 0,
    CP1,
    CPNum,
};

enum CPMsgQId
{
    CP0ToFP_Req = 0,
    FPToCP0_Res = CP0ToFP_Req,
    CP1ToFP_Req,
    FPToCP1_Res = CP1ToFP_Req,
    FPToCP0_Req,
    CP0ToFP_Res = FPToCP0_Req,
    FPToCP1_Req,
    CP1ToFP_Res = FPToCP1_Req,
    MsgQNum,
};

enum MsgOpErrQsetSubOp
{
    msgSubOpAdminAbort = 0,
    msgSubOpKeyVaultReload
};

enum MsgOpTelemetryQuerySubOp
{
    msgSubOpLogBufferingAddresses = 0,
    msgSubOpFirmwareInformation,
    msgSubOpTelemetryCounters,
    msgSubOpSetWeightRoundRobin,
};

//-----------------------------------------------------------------------------
//  CP2FP/FP2CP message context definition
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgContext_t
{
    uint8_t msgOp : 7;
    uint8_t resp : 1;
    uint8_t tag;
    uint8_t sts;
    uint8_t length;
    uint8_t data[MSG_CONTEXT_MAX_DATA_LENGTH];
}CP2FPMsgContext_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpFpStsChange
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataFpStsChange_t
{
    uint8_t ChangeSts;   /// < ref Fastpath_Status_t

}CP2FPMsgDataFpStsChange_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpErrQSet
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgOpErrQSet_t
{
    /// < CP2FP
    uint32_t subOp;
}CP2FPMsgOpErrQSet_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgSubOpAdminAbort
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgAdminAbort_t
{
    uint32_t subOp; ////< 0x0 msgSubOpAdminAbort
    uint8_t vfId;
    uint8_t ibQId;
    uint16_t cmdId;
    uint32_t abortSts; /// < 0xff invalid, 0x0 failed, 0x1 success, 0x2 wait
    uint8_t adminAbortCompleted;
    uint8_t reserved[3];
}CP2FPMsgAdminAbort_t;

enum MsgAbortSts
{
    abortFailed = 0,
    abortSuccess,
    abortWait,
    abortInvalid = 0xff
};

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpFpModeChange
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataVfModeChange_t
{
    uint8_t VFMode;   /// < 0: greedy, 1:strict

}CP2FPMsgDataVfModeChange_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpVfSlotSQ2CQMapUpdate
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataVfSlotSq2CqMapUpdate_t
{
    uint32_t VFId : 8;   ///< 0~63
    uint32_t SqPId : 8;   ///< UCD IB SQPID
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    uint32_t CqPId : 8;   ///< UCD IB CQPID, valid only for Action 0x1
    uint32_t Action : 8;   ///< 0: remove, 1: create, 2: flush
    #else
    #ifdef NEW_VF_QUEUE_MSG_STRUCTURE
    uint32_t CqPId : 8;   ///< UCD IB CQPID, valid only for Action 0x1
    uint32_t Action : 8;   ///< 0: remove, 1: create, 2: flush
    #else
    uint32_t CqPId : 8;   ///< UCD IB CQPID
    uint32_t Priority : 4;   ///< 0: low, 1: high
    uint32_t Action : 4;   ///< 0: create, 1: remove, 2: flush
    #endif
    #endif
}CP2FPMsgDataVfSlotSq2CqMapUpdate_t;


//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpUcdQuery
//-----------------------------------------------------------------------------

#ifdef SUPPORT_VF65_QB65_UCD_QUERY_EXTEND
typedef struct CP2FPMsgDataUcdQuery_t
{
    uint32_t queueDepth;
    uint32_t dflListBaseAddr;
    uint32_t oslListBaseAddr;
    uint32_t ibcqEBaseAddr;
    uint32_t obcqEBaseAddr;
    uint32_t dflBufBaseAddr;
    uint32_t ibcqPiShadowAddr;
    uint32_t obcqPiShadowAddr;
    uint8_t ucdCoreID;
    uint8_t queueID;
    uint8_t dfl2UcdCoreID;
    uint8_t dfl2QueueID;
    uint32_t dfl2QueueDepth;
    uint32_t dfl2ListBaseAddr;
    uint32_t dfl2BufBaseAddr;
    uint32_t cdmaList2ElementDwordSize : 6;
    uint32_t cdmaList2DescrDwordOffset : 6;
    uint32_t cdmaList2CryptoInDwordOffset : 6;
    uint32_t cdmaList2CryptoOutDwordOffset : 6;
    uint32_t cdmaList2Ifsel : 8;
    uint32_t cdmaList2BaseAddrLow;
    uint32_t cdmaList2BaseAddrHigh;
}CP2FPMsgDataUcdQuery_t;
#else
typedef struct CP2FPMsgDataUcdQuery_t
{
    uint32_t queueDepth;
    uint32_t dflListBaseAddr;
    uint32_t oslListBaseAddr;
    uint32_t ibcqEBaseAddr;
    uint32_t obcqEBaseAddr;
    uint32_t dflBufBaseAddr;
    uint32_t ibcqPiShadowAddr;
    uint32_t obcqPiShadowAddr;
    uint8_t ucdCoreID;
    uint8_t queueID;
    uint16_t reserved;
    uint32_t cdmaList2ElementDwordSize : 6;
    uint32_t cdmaList2DescrDwordOffset : 6;
    uint32_t cdmaList2CryptoInDwordOffset : 6;
    uint32_t cdmaList2CryptoOutDwordOffset : 6;
    uint32_t cdmaList2Ifsel : 8;
    uint32_t cdmaList2BaseAddrLow;
    uint32_t cdmaList2BaseAddrHigh;
    uint32_t cdmaList3ElementDwordSize : 6;
    uint32_t cdmaList3DescrDwordOffset : 6;
    uint32_t cdmaList3CryptoInDwordOffset : 6;
    uint32_t cdmaList3CryptoOutDwordOffset : 6;
    uint32_t cdmaList3Ifsel : 8;
    uint32_t cdmaList3BaseAddrLow;
    uint32_t cdmaList3BaseAddrHigh;
}CP2FPMsgDataUcdQuery_t;
#endif

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpCpCdmaIo
//-----------------------------------------------------------------------------

/* Since FP populates the CDMA SQE from the Host command populated in the PSRAM memory can this structure be removed */
typedef struct CP2FPMsgDataOpCpCdmaIo_t
{
    union
    {
        uint8_t cdmaSqe[8];
        struct
        {
            union
            {
                uint32_t dw0;
                struct
                {
                    uint8_t vfid;
                    uint8_t reserved1[3];
                }Dw0;
            };
            union
            {
                uint32_t dw1;
                struct
                {
                    uint8_t srcDescInterfaceSel;
                    uint8_t srcDataInterfaceSel;
                    uint8_t destDescInterfaceSel;
                    uint8_t destDataInterfaceSel;

                } Dw1;
            };
        }CdmaSqe;
    };
}CP2FPMsgDataOpCpCdmaIo_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpCpCdmaIoResp
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataOpCpCdmaIoResp_t
{
    uint8_t cmdListIdx;
    uint8_t cmdListNum;
    uint8_t cmdStatus;

}CP2FPMsgDataOpCpCdmaIoResp_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpKeyUpdate
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataKeyUpdate_t
{
    uint8_t keySubIndex; // 0-6
    uint8_t resourceGroupId; // 0-65 65: CDMA-IO
    uint8_t vfId; //VF ID 0-65 : 65: CDMA_IO
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    uint8_t action;  // 0: delete key at a KeyIndex, 1: delete all ephemeral keys for a session; 2: delete all keys for an application, 3:create a key
    uint16_t sessionId; // session ID for the key to be updated
    uint8_t appId; // application ID for the key to be updated
    uint8_t flag; // 0: Persistent, 1: Ephemeral
    // u32 array (not u8): vault writes need 32-bit-only access; this matches
    // the source representation to the destination. Wire bytes unchanged.
    uint32_t keyData[8]; // AES-256 key = 8 u32 words (32 bytes)
    #else
    uint8_t action;  // 0: enable, 1: disable
    #endif
}CP2FPMsgDataKeyUpdate_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpVFUpdate
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataVfUpdate_t
{
    uint32_t VFId : 8;   ///< 0~63
    uint32_t Action : 8;   ///< 0: install, 1: teardown
    uint8_t Reserved[2];
}CP2FPMsgDataVfUpdate_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpSetLogLevel
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataSetLogLevel_t
{
    uint8_t LogLevel;   /// < 0: debug and info, 1: only info
}CP2FPMsgDataSetLogLevel_t;


//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpCdmaStatSet
//-----------------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)
typedef struct CP2FPMsgDataCdmaStatSet_t
{
    uint8_t action : 8;
    uint32_t value;
}CP2FPMsgDataCdmaStatSet_t;
#pragma pack(pop)

#ifdef QOS_LATENCY_ERROR_HANDLING

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpQoSPenalty
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataQoSPenalty_t
{
    union
    {
        struct
        {
            uint8_t vfId;   ///< 0~64
            uint8_t qosPenaltyPeriod;   ///< default is 0.   ///< 1~255: valid range;
            uint8_t qosPenaltyCreditRatio;   ///< 1~99 : credit ratio;{0, 100~255}: temporary invalid range
            uint8_t Reserved1;
        } Msg;
        struct
        {
            uint8_t Reserved1;
            uint8_t qosPenaltyPeriod;   ///< default is 0.   ///< 1~255: valid;
            uint8_t qosPenaltyCreditRatio;   ///< default is 100.   ///< 0~99 : valid;100~255: invalid;
            uint8_t Reserved2;
        } Cfg;
    };
}CP2FPMsgDataQoSPenalty_t;

#endif

#ifdef SUPPORT_MSGERROR_INJECTION

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpMsgErrorInjection
//-----------------------------------------------------------------------------

typedef enum InjectSts
{
    Cpu1NoInject = 0,
    Cpu1WaitInject,
    Cpu1Inject,
}InjectSts;

typedef enum MsgErrorInjectionErrType_t
{
    cMsgErrInjectNoErr = 0x0,
    cMsgErrInjectCdmaNonFatalErr,
    cMsgErrInjectCdmaPoorSGLErr,
    cMsgErrInjectCdmaFatalErr,
} MsgErrorInjectionErrType_t;

typedef struct CP2FPMsgDataMsgErrorInjection_t
{
    union
    {
        uint32_t dw0;
        struct
        {
            uint16_t cmdId;
            uint8_t vfId;
            uint8_t ibPhyQId;
        };

    };

    uint8_t errorType : 4;
    uint8_t reErrInjectTimes : 4;
    uint8_t injectSts;
    uint16_t dflIdx;
}CP2FPMsgDataMsgErrorInjection_t;
#endif

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpShutdownReq
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataShutdownReq_t
{
    uint32_t drainTime;   /// < Queue drain time in milliseconds

}CP2FPMsgDataShutdownReq_t;

#ifdef MCR_TEST_HOOKS
typedef enum InjectErrorType
{
    InjErrFpIoLvl1Abrt = 0,
    InjErrHardFault = 1,
    InjErrExplicitCrash = 2,
    InjErrPanic = 3, // Not supported by FP
    InjErrHang = 4,
    InjErrMax = 5,

}InjectErrorType;

typedef struct CP2FPMsgDataInjectErrorReq_t
{
    InjectErrorType errorType;   /// < Error Type
    uint32_t coreid;
}CP2FPMsgDataInjectErrorReq_t;
#endif
#ifndef SUPPORT_UPDATE_TIMESTAMP_IPC

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpTimestampAddr
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataUpdateTimestampAddr_t
{
    uint64_t initTimerCounter;
    uint8_t Ready;   /// < 0: Not ready, 1: Ready
}CP2FPMsgDataUpdateTimestampAddr_t;
#endif

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpLogEnDisUpdate
//-----------------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)
typedef struct CP2FPMsgDataLogEnDisUpdate_t
{
    uint32_t action;                     /// < 0x0 disable; 0x1 enable log transfers
    // fpscpu0
    uint16_t piInfoFpsCpu0 : 12;        /// < log buffer's GDMA delivery queue's current PI
    uint16_t gdmaInsFpsCpu0 : 3;        /// < GDMA delivery queue instance ID
    uint16_t pingPongIndexFpsCpu0 : 1;  /// < ping pong log buffer index pointed by current PI
    uint16_t gdmaQSizeFpsCpu0;          /// < length of GDMA queue
    // fpscpu1
    uint16_t piInfoFpsCpu1 : 12;
    uint16_t gdmaInsFpsCpu1 : 3;
    uint16_t pingPongIndexFpsCpu1 : 1;
    uint16_t gdmaQSizeFpsCpu1;
    // fpscpu2
    uint16_t piInfoFpsCpu2 : 12;
    uint16_t gdmaInsFpsCpu2 : 3;
    uint16_t pingPongIndexFpsCpu2 : 1;
    uint16_t gdmaQSizeFpsCpu2;
}CP2FPMsgDataLogEnDisUpdate_t;
#pragma pack(pop)

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgOpFpTelemetryQuery
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgDataTelemetryQuery_t
{
    /// < CP2FP
    uint32_t subOp;
}CP2FPMsgDataTelemetryQuery_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgSubOpLogBufferingAddresses
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgLogBufferingAddresses_t
{
    /// < CP2FP
    uint32_t subOp;

    /// < FP2CP
    uint32_t logBuffer0AddrFpsCpu0;   /// < Log Buffer0 Address for FPS CPU0
    uint32_t logBuffer1AddrFpsCpu0;   /// < Log Buffer1 Address for FPS CPU0
    uint32_t logBuffer0AddrFpsCpu1;   /// < Log Buffer0 Address for FPS CPU1
    uint32_t logBuffer1AddrFpsCpu1;   /// < Log Buffer1 Address for FPS CPU1
    uint32_t logBuffer0AddrFpsCpu2;   /// < Log Buffer0 Address for FPS CPU2
    uint32_t logBuffer1AddrFpsCpu2;   /// < Log Buffer1 Address for FPS CPU2
    uint32_t gdmaInsCiShadowFpsCpu0;  /// < GDMA instance delivery queue ci shadow address for FPS CPU0
    uint32_t gdmaInsCiShadowFpsCpu1;  /// < GDMA instance delivery queue ci shadow address for FPS CPU1
    uint32_t gdmaInsCiShadowFpsCpu2;  /// < GDMA instance delivery queue ci shadow address for FPS CPU2

}CP2FPMsgLogBufferingAddresses_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgSubOpFirmwareInformation
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgFirmwareInformation_t
{
    /// < CP2FP
    uint32_t subOp;

    /// < FP2CP
    uint32_t fwverBuildNo : 12;/// < Firmware version: Build number
    uint32_t fwverOemNo : 12;  /// < Firmware version: OEM number
    uint32_t fwverMinorNo : 4; /// < Firmware version: Minor number
    uint32_t fwverMajorNo : 4; /// < Firmware version: Major number
    uint8_t fpmode : 2;        /// < FP mode for scheduling command; 0: greedy; 1: strict
    uint8_t loggingLevel : 2;  /// < Logging filter of logging buffer; 0: debug and info, 1: only info
    uint8_t logDataStatus : 2; /// < The status of Log data of involving GDMA to Host; 0: disable, 1: enable
    uint8_t resv : 2;          /// < reserved
    uint8_t injecCountNonFaultErr;  /// < non-fatal error injection request count
    uint8_t injecCountFaultErr;     /// < fatal error injection request count
    uint8_t injecCountPoorSgl;      /// < poorly constructed sgl injection request count
    uint8_t systickThreshold : 6;   /// < value of register systick_timer_pulse_threshold. value from 0x0 to 0x3F.
    uint8_t resv2 : 2;              /// < reserved
    uint8_t resv3[3];               /// < reserved
}CP2FPMsgFirmwareInformation_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgSubOpTelemetryCounters
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgTelemetryCounters_t
{
    /// < CP2FP
    uint32_t subOp;

    /// < FP2CP
    uint32_t outstandingIOCnt;       /// < Infly IO count
    uint64_t accumulateIOCnt;        /// < Accumulate IO Counter
    uint64_t faultErrCnt;            /// < Fatal Error counter
    uint64_t nonFaultErrCnt;         /// < Non-fatal Error counter
    uint64_t poorConstructedSglCnt;  /// < Poorly constructed SGL counter
}CP2FPMsgTelemetryCounters_t;

//-----------------------------------------------------------------------------
//  data structure of message opcode : msgSubOpTelemetrySetWeightRoundRobin
//-----------------------------------------------------------------------------

typedef struct CP2FPMsgTelemetrySetWeightRoundRobin_t
{
    /// < CP2FP
    uint32_t subOp;

    /// < FP2CP
    uint32_t configOp : 8;
    uint32_t weightExp : 8;         /// < weight for Round-robin queue
    uint32_t reserved1 : 16;
}CP2FPMsgTelemetrySetWeightRoundRobin;


//-----------------------------------------------------------------------------
//  Message handler definition in HAL
//-----------------------------------------------------------------------------

typedef struct MsgHandler_t
{
    ///< CP2FP MSG pi address, it will be PSRAM_CP0toFP_PI_ADDR in M7MemMap.h
    volatile uint32_t* pCp2FpMsgPi;
    ///< CP2FP MSG ci address, it will be PSRAM_CP2FP_CI_ADDR in M7MemMap.h
    volatile uint32_t* pCp2FpMsgCi;
    ///< CP2FP MSG address, it will be PSRAM_CP0toFP_MSG_ADDR in M7MemMap.h
    CP2FPMsgContext_t* pCp2FpMsgQ;
    ///< FP2CP MSG pi address, it will be PSRAM_FP2CP_PI_ADDR in M7MemMap.h
    volatile uint32_t* pFp2CpMsgPi;
    ///< FP2CP MSG ci address, it will be PSRAM_FPtoCP0_CI_ADDR in M7MemMap.h
    volatile uint32_t* pFp2CpMsgCi;
    ///< FP2CP MSG address, it will be PSRAM_FPtoCP0_MSG_ADDR in M7MemMap.h
    CP2FPMsgContext_t* pFp2CpMsgQ;
    ///< function call pointer that point to Message queue completion handler API.
    void (*pfApiFp2CP)(void*);

}MsgHandler_t;


//-----------------------------------------------------------------------------
//  CP2FP message status definition
//-----------------------------------------------------------------------------

typedef enum CP2FPMsgSts
{
    // generic status
    msgSuccess = 0x0,              /// < message success
    msgNotSupport = 0x1,             /// < message not support
    msgInvalidField = 0x2,                     /// < message invalid field

    //MSG_OP_FP_STATUS_CHANGE (Message Op: 0x0)
    msgFwUpdateDataFail = 0x3,
    msgFwUpdateTimeout = 0x4,
    //MSG_OP_ERR_REQ (Message Op: 0x1, subOp: 0x0)
    msgVFIsTeardown = 0x3,
    msgQueueIsDelete = 0x4,
    msgCmdNotFound = 0x5,
    msgCmdAlreadyCDMAComplete = 0x6,
    msgCmdAlreadyOutbound = 0x7,
    // MSG_OP_VF_SLOT_SQ2CQ_MAP_UPDATE (Message Op: 0x3)
    msgVfOutOfRange = 0x3,
    msgVfNotInstalled = 0x4,
    msgQueueOutOfRange = 0x5,
    msgQueueInstalledAlready = 0x6,
    msgQueueWithAbortCmdNotDone = 0x7,
    msgQueueNotInstalled = 0x8,
    msgQueueInfoFail = 0x9,
    // MSG_OP_VF_UPDATE (Message Op: 0x5)
    //msgVfOutOfRange = 0x3,
    //msgVfNotInstalled = 0x4,
    msgVfInstalledAlready = 0x4,
    // MSG_OP_QOS_PENALTY  (Message Op: 0xE)
    //msgQoStest = 0x3,
    // MSG_OP_CHK_ALIVE  (Message Op: 0xF)
    msgFP0notAlive = 0x3,
    msgFP1notAlive = 0x4,
    msgFP0andFP1notAlive = 0x5,
    // common error in firmware update status
    msgInFwUpdateState = 0xB,
    // FP internal generic status
    msgInProgress = 0xD,          /// < message in progress
    msgNoEmptyEntry = 0xE,               /// < FP2CP message queue full
    msgNotifyCpu1 = 0xF,          /// < message needs to notify cpu1
}CP2FPMsgSts;
