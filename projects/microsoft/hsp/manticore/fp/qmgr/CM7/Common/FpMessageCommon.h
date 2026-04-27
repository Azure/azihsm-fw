// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#include "../../hal/Common/common.h"

enum MsgFpStsChangeState
{
    stateFpStsStart = 0,
    stateFpStsMsgtoCpu0 = 1,
    stateFpStsMsgtoCpu1,
    stateFpStsWaitOtherCpuDone,
    stateFpStsCpu2Handle,
    stateFpStsWaitFwUpdateDone,
    stateFpStsSendFP2CP,
};

enum MsgErrorQSetState
{
    stateErrorQSetStart = 0,
    stateErrorQSetMsgtoCpu0 = 1,
    stateErrorQSetMsgtoCpu1,
    stateErrorQSetCpu2Handle,
    stateErrorQSetWaitOtherCpuDone,
    stateErrorQSetSendFP2CP,
};

enum MsgSetLogLevelState
{
    stateSetLogLevelStart = 0,
    stateSetLogLevelCpu2Handle,
    stateSetLogLevelMsgtoCpu0,
    stateSetLogLevelMsgtoCpu1,
    stateSetLogLevelWaitOtherCpuDone,
    stateSetLogLevelSendFP2CP,
};

enum MsgFpVfSlotSq2CqMapUpdateState
{
    stateFpVfSlotSq2CqMapUpdateStart = 0,
    stateFpVfSlotSq2CqMapUpdateMsgtoCpu0,
    stateFpVfSlotSq2CqMapUpdateWaitCpu0Done,
    stateFpVfSlotSq2CqMapUpdateMsgtoCpu1,
    stateFpVfSlotSq2CqMapUpdateWaitCpu1Done,
    stateFpVfSlotSq2CqMapUpdateWaitDeleteQDone,
    #ifdef QOS_LATENCY_ERROR_HANDLING
    stateFpVfSlotSq2CqMapUpdateWaitCpu1UpdateCreditDone,
    #endif
    stateFpVfSlotSq2CqMapUpdateWaitForceComplDone,
    stateFpVfSlotSq2CqMapUpdateSendFP2CP,
};

enum MsgFpVfUpdateState
{
    stateFpVfUpdateStart = 0,
    stateFpVfUpdateMsgtoCpu0,
    stateFpVfUpdateWaitCpu0Done,
    stateFpVfUpdateMsgtoCpu1,
    stateFpVfUpdateWaitCpu1Done,
    stateFpVfUpdateWaitVfTeardown,
    stateFpVfUpdateSendFP2CP,
};

enum MsgCpCdmaIoState
{
    stateCpCdmaIoStart = 0,
    stateCpCdmaIoMsgtoCpu1,
    stateCpCdmaIoWaitOtherCpuDone,
    stateCpCdmaIoWaitCdmaDone,
    stateCpCdmaIoSendFP2CP,
};

enum MsgFpUcdQueryState
{
    stateFpUcdQueryStart = 0,
    stateFpUcdQueryRestore,
    stateFpUcdQueryMsgToCpu0,
    stateFpUcdQueryWaitOtherCpuDone,
    stateFpUcdQueryFillQuery,
    stateFpUcdQuerySendFP2CP,
};

enum MsgKeyUpdateState
{
    stateKeypdateStart = 0,
    stateKeypdateMsgtoCpu1,
    stateKeypdateWaitOtherCpuDone,
    stateKeypdateWaitSendFP2CP,
};

enum MsgFpModeChangeState
{
    stateFpModeChangeStart = 0,
    stateFpModeChangeMsgtoCpu1,
    stateFpModeChangeWaitWaitOtherCpuDone,
    stateFpModeChangeWaitSendFP2CP
};

#ifdef SUPPORT_MSGERROR_INJECTION
enum MsgMsgErrorInjectionState
{
    stateMsgErrorInjectionStart = 0,
    stateMsgErrorInjectionMsgtoCpu1,
    stateMsgErrorInjectionSendFP2CP,
};
#endif

#ifdef LOGGING_NEW_SCHEME
enum MsgLogEnDisUpdate
{
    stateLogEnDisUpdateStart = 0,
    stateLogEnDisUpdateCpu2ConfigGdmaFpReg,
    stateLogEnDisUpdateCpu2UpdateLogExt,
    stateLogEnDisUpdateMsgtoCpu0,
    stateLogEnDisUpdateMsgtoCpu1,
    stateLogEnDisUpdateWaitOtherCpuDone,
    stateLogEnDisUpdateSendFP2CP,
};

enum MsgTelemetryQueryState
{
    stateTelemetryQueryStart = 0,
    stateTelemetryQuerySubOpBufferingAddresses,
    stateTelemetryQuerySubOpFirmwareInformation,
    stateTelemetryQuerySubOpTelemetryCounters,
    stateTelemetryQuerySubOpSetWeightRoundRobin,
    stateTelemetryQuerySendFP2CP,
};
#endif

#ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
enum MsgUpdateTimestampAddrState
{
    stateUpdateTimestampAddrStart = 0,
    stateUpdateTimestampAddrMsgtoCpu0,
    stateUpdateTimestampAddrWaitCpu0Done,
    stateUpdateTimestampAddrMsgtoCpu1,
    stateUpdateTimestampAddrWaitOtherCpuDone,
    stateUpdateTimestampAddrCpu2Handle,
    stateUpdateTimestampAddrSendFP2CP,
};
#endif
enum MsgSubOpAbortState
{
    stateAbortStart = 0,
    stateAbortMsgToCpu0,
    stateAbortMsgToCpu1,
    stateAbortSendFP2CP
};

#ifdef SUPPORT_CDMA_RESET_MSG
enum MsgCDMAResetState
{
    stateCDMAResetStart = 0,
    stateCDMAResetSendFP2CP
};
#endif

enum MsgCDMAStatSetState
{
    stateCDMAStatSetStart = 0,
    stateCDMAStatSetSendFP2CP
};

#ifdef QOS_LATENCY_ERROR_HANDLING
enum MsgQoSPenaltySetupState
{
    stateQoSPenaltySetupStart = 0,
    stateQoSPenaltySetupSendFP2CP
};
#endif

enum MsgChkAlive
{
    stateChkAliveStart = 0,
    stateChkAliveMsgToCpu0,
    stateChkAliveMsgToCpu1,
    stateChkAliveWaitOtherCpuDone,
    stateChkAliveSendFP2CP,
};

enum MsgNotSupportState
{
    stateNotSupportStart = 0,
    stateNotSupportSendFP2CP,
};

enum MsgFpShutdownReqState
{
    stateFpShutdownStart = 0,
    stateFpShutdownMsgtoCpu0,
    stateFpShutdownWaitOtherCpuDone,
    stateFpShutdownWaitFwUpdateDone,
    stateFpShutdownSendFP2CP,
};

#ifdef MCR_TEST_HOOKS
enum MsgFpTriggerCrashReqState
{
    stateFpTriggerCrashStart = 0,
    stateFpTriggerCrashMsgToOtherCpu,
    stateFpTriggerCrashWaitOtherCpuDone,
    stateFpTriggerCrashSendFP2CP,
};

enum MsgFpIoLvl1AbrtReqState
{
    stateFpIoLvl1AbrtStart = 0,
    stateFpIoLvl1AbrtSendFP2CP,
};
#endif

enum FpInterMsgOp
{
    cp2FpMsg = 0,
    loggingErrMsg,
    errorCmdMsg,
    skipAbortMsg,
    fatalErrorMsg,
    cdmaResetMsg,
    ConfigIOMsg,
    doNothingMsg,
    resetHandlingMsg,
    #ifdef SUPPORT_MSGERROR_INJECTION
    errInjectMsg,
    #endif
    #ifdef QOS_LATENCY_ERROR_HANDLING
    qosPenaltyMsg,
    #endif
};

typedef enum ResetRequestMagicNumber_t
{
    noRequest = 0x0,
    VFLR = 0x524C4656,
    PFLR = 0x524C4650,
    PERST = 0x54535250,
} ResetRequsetMagicNumber_t;


typedef struct FPInterMsgHeader
{
    uint8_t fpMsgOp : 7;
    uint8_t resp : 1;
    uint8_t sts;
    union
    {
        uint8_t cmdSpecific[2];
        struct
        {
            uint8_t msgIdx;
            uint8_t msgSrc;
        };
        struct
        {
            uint8_t dataBufIdx;
            uint8_t dataLength;
        };

        uint16_t dflIdx;
        #ifdef SUPPORT_MSGERROR_INJECTION
        struct
        {
            uint8_t enable;
            uint8_t reserved3;
        };
        #endif

        uint16_t skipAbort_ceIndex;

        struct
        {
            uint8_t configIo;
            uint8_t rvsd;
        };
    };
}FPInterMsgHeader;

typedef enum
{
    cPauseIO,
    cResumeIO,
} ConfigIOStatus_t;