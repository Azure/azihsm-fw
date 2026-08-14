// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#pragma once

#include "../../hal/Common/common.h"

#define MAX_FP_RGID_NUM 65
#define RGID_NO_OWNER_VF 0xFF
#define MAP_FUNCTION_ID(fid)  fid

#define CA_2_IBPHYQID(caIndex)   (caIndex < 64) ? (caIndex << 1) : (((caIndex - 64) << 1) + 1)
#define IBPHYQID_2_CA(ibPhyQId)                                                                 \
    (ibPhyQId & (uint8_t)LOW_PRIORITY_QUEUE) ?                                                  \
    ((uint8_t)LOW_PRIORITY_QUEUE_START_CA_INDEX + (ibPhyQId >> (uint8_t)IB_PHY_Q_2_CA_SHIFT)) : \
    (ibPhyQId >> (uint8_t)IB_PHY_Q_2_CA_SHIFT)


#define CDMA_LIST_2_DFL_NUM(cdmaListNum) (cdmaListNum == CDMA_FP_DFL_0_LIST ? 0 : 1)

#ifdef INTEGRATE_TIMESTAMP_TO_FPSCPU
#define TimestampMaskBit23_14 0xFFC000
#endif

#define DFL_0_BUFF_PHYSICAL_ADDR  0xA3200000  // CPU1 local TCM
#define DFL_1_BUFF_PHYSICAL_ADDR 0xA3220000 // CPU12 share TCM

#define DFL_BUFF_PHYSICAL_ADDR_MASK 0xFFFF0000
#define DFL_1_BUFF_PHYSICAL_ADDR_LOW_MASK 0x20000

#define GET_DFL_BUFF_ADDR_FROM_CDMA_LIST(cdmaDflList) ((cdmaDflList ==  CDMA_FP_DFL_0_LIST) ?  (M7_FPS_CPU1_DFL_BUFF_ADDR) : (M7_FPS_CPU12_DFL_1_BUFF_ADDR))
#define GET_CDMA_LIST_FROM_DFL_NUM(dflNum) ((dflNum == DFL_0) ? (CDMA_FP_DFL_0_LIST) : (CDMA_FP_DFL_3_LIST))
#define DFL_IDX_CARRY_BIT (BIT(9))
#define GET_MSG_DFL_IDX(cdmaDflList, dflIdx) ((cdmaDflList == CDMA_FP_DFL_3_LIST) ? (dflIdx | DFL_IDX_CARRY_BIT) : (dflIdx))
#define GET_DFL_IDX(dflIdx) (dflIdx & 0x1FF)
#define GET_CDMA_DFL_LIST(dflIdx) ((dflIdx & DFL_IDX_CARRY_BIT) ? (CDMA_FP_DFL_3_LIST) : (CDMA_FP_DFL_0_LIST))
#define GET_VF_IDX(VFId) ((VFId == MAX_VF_NUM) ? 0 : VFId)
#define GET_DFL_NUM_FROM_ADDR(dflAddr) (((dflAddr & DFL_BUFF_PHYSICAL_ADDR_MASK) == DFL_0_BUFF_PHYSICAL_ADDR) ? DFL_0 : DFL_1)
#define CHK_DFL_BUFF_ADDR(dflAddr)                                            \
    (((dflAddr & DFL_BUFF_PHYSICAL_ADDR_MASK) == DFL_0_BUFF_PHYSICAL_ADDR) || \
     ((dflAddr & DFL_BUFF_PHYSICAL_ADDR_MASK) == DFL_1_BUFF_PHYSICAL_ADDR))

#define CP_CDMA_IO_CMD_ID 0x208

#define MSG_STATE_START 0
#define ALL_MSG_STATE_DONE 0xFF
#define MSG_CPU(cpu) BIT(cpu)

#define LOW_PRIORITY_QUEUE  0x1
#define LOW_PRIORITY_QUEUE_START_CA_INDEX 64
#define IB_PHY_Q_2_CA_SHIFT 0x1
#define QBLK_2_PHYQ_SHIFT 0x1

#define HIGH_LOW_PHYSICAL_Q_OFFSET 65
#define QB65_HIGH_PHYSICAL_Q_INDEX 64
#define QB65_LOW_PHYSICAL_Q_INDEX  129
#define SQ_PID_2_QBIDX(SqPid)   (SqPid > 64) ? (SqPid - 65) : SqPid
#define QBIDX_2_HIGH_SQ_PID(qbIndex)   qbIndex
#define QBIDX_2_LOW_SQ_PID(qbIndex)    (qbIndex + HIGH_LOW_PHYSICAL_Q_OFFSET)
#define GET_DFL_PHYSICAL_BUF_ADDR(cdmaListNum, dflOffset) (cdmaListNum == CDMA_FP_DFL_3_LIST) ? (dflOffset + DFL_1_BUFF_PHYSICAL_ADDR) : (dflOffset + DFL_0_BUFF_PHYSICAL_ADDR)

#define QBLOCK_CMD_EXIST_BITMAP 0x3
#define QUEUE_BLOCK_INDEX_SHIFT 4
#define QUEUE_BLOCK_OFFSET_MASK 0x1F

#define CHUNK_SIZE 0x1000UL //4096
#define CREDIT_SIZE 4096

#define CA_SIZE 0x4UL//4
#define CA_SIZE_SHIFT 0x2UL  //< 4
#define CA_MASK (CA_SIZE - 1)
#define CA_EXTRA_BIT_MASK CA_SIZE
//#define CA_ROLLOVER_MASK (((uint8_t)CA_SIZE << 1) - 1)
#define CA_ROLLOVER_MASK (((uint8_t)CA_SIZE << 2) - 1)
#define DFL_BUF_SZ_SHIFT 7  ///< 128 bytes
#define DFL_BUF_INDEX_MASK 0x1ff  ///< 512
#define DFL_1_BUF_INDEX_MASK 0x1f ///< 31
#define CMD_ARRAY_SHIFT 4
#define FPS_IO_QUEUE_DEPTH 0x200
#define FPS_IO_QUEUE_DEPTH_MASK ((uint16_t)FPS_IO_QUEUE_DEPTH - 1)
#define FPS_IO_QUEUE_1_DEPTH 0x20
#define FPS_IO_QUEUE_1_DEPTH_MASK ((uint16_t)FPS_IO_QUEUE_1_DEPTH - 1)

#ifdef QOS_LATENCY_ERROR_HANDLING
#define DEFAULT_QOS_CREDIT_RATIO 0x64   ///< default 100: disable QoS penalty
#endif

#define FPS_CDMA_SLOT_NUM    68U

#ifdef CDMA_CMD_COUNT
#define MAX_IO_CMD_SLOT_COUNT (FPS_CDMA_SLOT_NUM - 1)   ///< 1 cmd slot for idle cmd using
#define FPS_CDMA_IO_COMMAND_QUEUE_DEPTH 0x80UL
#define FPS_CDMA_IO_COMMAND_QUEUE_DEPTH_MASK (FPS_CDMA_IO_COMMAND_QUEUE_DEPTH - 1)
#endif // End of CDMA_CMD_COUNT

#define CDMA_FP_DFL_0_LIST    0
#define CDMA_FP_IDLE_DFL_LIST 1
#define CDMA_CP_DFL_LIST      2
#define CDMA_FP_DFL_3_LIST    3

#define CDMA_CMD_SLOT_ERR_STS_REG_ID_1 1

#define CDMA_CMD_SLOT_ERR_CHECK_EN_REG_ID_0 0
#define CDMA_CMD_SLOT_ERR_CHECK_EN_REG_ID_1 1

#define CDMA_CMD_SLOT_ERR_CHECK_EN_REG_ID_1_MASK 0xffffffff

#define FPS_SLOT_ARRAY_NUM  128
#define FPS_QUEUE_BLOCK_NUM (FPS_SLOT_ARRAY_NUM >> 1)
#define FPS_QUEUE_BLOCK_65  1
#define FPS_QUEUE_BLOCK_65_INDEX 64
#define FPS_SLOT_LOW_PRIORITY_START_INDEX 64
#define VF_QB_32_SHIFT      5

#define FPS2HW_Q_SZ_POW 6  ///< 64 entries

#define GET_VF_ID(ifSel)   ((ifSel > cPCIeFuctcionInterfacePf0) ? UCD_IFSEL_TO_VFID(ifSel) : cUcdHiuPf0)

#define VF_ID_MASK 0x3F
#define QID_INVALID 0xff
#define DFL_BUFFER_INVALID 0xffffffff
#define MAX_CHUNK_TRANS_SIZE 0x1000
#define HOST_CMD_16B_ALIGN_MASK  0xf

#define FPS_ERROR_PENDING_QUEUE_DEPTH 0x40
#define FPS_ERROR_PENDING_QUEUE_DEPTH_MASK ((uint8_t)FPS_ERROR_PENDING_QUEUE_DEPTH - 1)

#define FPS_CE_QPID_SHIFT 4
#define FPS_CE_IFSEL_SHIFT 12
#define FPS_CE_DFL_ID_SHIFT 20
#define FPS_CE_DFL_NUM_SHIFT 30
#define FPS_INTL_MSG_CMD_SPECIFIC_SHIFT 0x8UL

#define CMD_NON_FATAL_ERROR_MAX_RETRY_TIMES 1
#define CMD_POOR_SGL_MAX_RETRY_TIMES 2    ///< 4
#define CMD_FATAL_ERROR_MAX_RETRY_TIMES 1

#define CMD_NON_FATAL_ERROR_MAX_INJECT_TIMES (CMD_NON_FATAL_ERROR_MAX_RETRY_TIMES + 1)
#define CMD_POOR_SGL_MAX_INJECT_TIMES (CMD_POOR_SGL_MAX_RETRY_TIMES + 1)
#define CMD_FATAL_ERROR_MAX_INJECT_TIMES (CMD_FATAL_ERROR_MAX_RETRY_TIMES + 1)

#define UCD_IBCQ_IFSEL_SHIFT 0x10UL  //16
#define UCD_IBCQ_ERR_STS_SHIFT 30
#define UCD_OBCQ_IFSEL_SHIFT 0x10UL  //16
#define UCD_OBCQ_QID_SHIFT 0x18UL  //24
#define UCD_OBCQ_ERR_STS_SHIFT 30

#define QB64_HIGH_QUEUE 128
#define QB64_LOW_QUEUE 129
#define QB64_HIGH_QUEUE_LOW_QUEUE_MASK 0x3

#define SYSTICK_MASK 0xFFFFFF
#define SYSTICK_DELTA_MASK 0xFF

#define DEFAULT_FP_MODE FP_MODE_GREEDY  // {FP_MODE_STRICT, FP_MODE_GREEDY}
#define DEFAULT_WEIGHT 1                // {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024}

#define FP_GCM_REQ_QUEUE_FULL_TIMEOUT 5000

//-----------------------------------------------------------------------------
//  CPU1 static Member Variable Definitions
//-----------------------------------------------------------------------------

#define NO_EXIST_CMD  0

#define XTS_CMD 1
#define GCM_CMD 3

#define CHUNK_SIZE_WITH_RETRY(retryTime) (retryTime == 0 ? CHUNK_SIZE : CHUNK_SIZE >> ((0x1UL << (uint32_t)retryTime) + 1))

#define AES_KEY_LEN_IN_BYTES 32
#define AES_KEY_LEN_IN_WORDS (AES_KEY_LEN_IN_BYTES / sizeof(uint32_t))   // 8 u32 words = 32 bytes
#define DFL_KEY_STAGING_OFFSET 20  // Byte offset in DFL (CQE) where bulk key is staged for tag correction

//-----------------------------------------------------------------------------
//  CPU2 static Member Variable Definitions
//-----------------------------------------------------------------------------

#define CDMA_CMD_COMPLETED  BIT(0)
#define CDMA_CMD_SKIPPED    BIT(1)
#define CDMA_CMD_ABORTED    BIT(2)
#define CDMA_CMD_IDLED      BIT(3)
#define CDMA_CMD_ERROR      BIT(4)

#define FP_CMD_ERR_MASK (CDMA_CMD_ERROR | CDMA_CMD_ABORTED)
#define CP_CMD_ERR_MASK (CDMA_CMD_ERROR | CDMA_CMD_ABORTED)
#define FP_IDLE_CMD_ERR_MASK (CDMA_CMD_ERROR)
#define CDMA_CMD_SLOT_ERR_STS_REG_POORLY_SGL_ERR_MASK (BIT(20) | BIT(0))
#define CDMA_LIST_IFSEL_MASK (0xFF000000) //Bit 31:24

// fps_bank0_reg_event_status_0 0x40000000
#define HWE2FP_WQ_00_UCD_IB_CQ0_EMPTY_BIT   BIT(0)
#define HWE2FP_WQ_01_UCD_IB_CQ1_EMPTY_BIT   BIT(1)
#define HWE2FP_WQ_02_UCD_OB_CQ0_EMPTY_BIT   BIT(2)
#define HWE2FP_WQ_03_UCD_OB_CQ1_EMPTY_BIT   BIT(3)
#define HWE2FP_WQ_04_CDMA_CQ_EMPTY_BIT      BIT(4)
#define HWE2FP_WQ_05_GDMA_CQ_EMPTY_BIT      BIT(5)
#define HWE2FP_WQ_06_CP2FP_CMD_SQ_EMPTY_BIT BIT(6)
#define HWE2FP_WQ_07_FP2CP_ERR_CQ_EMPTY_BIT BIT(7)
#define HWE2FP_WQ_08_EMPTY_BIT              BIT(8)
#define HWE2FP_WQ_09_EMPTY_BIT              BIT(9)
#define HWE2FP_WQ_10_EMPTY_BIT              BIT(10)
#define HWE2FP_WQ_11_EMPTY_BIT              BIT(11)
#define FP2HWE_WQ_00_FULL_BIT               BIT(12)
#define FP2HWE_WQ_01_FULL_BIT               BIT(13)
#define FP2HWE_WQ_02_FULL_BIT               BIT(14)
#define FP2HWE_WQ_03_FULL_BIT               BIT(15)
#define FP2HWE_WQ_04_CDMA_SQ_FULL_BIT       BIT(16)
#define FP2HWE_WQ_05_GDMA_SQ_FULL_BIT       BIT(17)
#define FP2HWE_WQ_06_CP2FP_CMD_CQ_FULL_BIT  BIT(18)
#define FP2HWE_WQ_07_FP2CP_ERR_SQ_FULL_BIT  BIT(19)
#define FP2HWE_WQ_08_FULL_BIT               BIT(20)
#define FP2HWE_WQ_09_FULL_BIT               BIT(21)
#define FP2HWE_WQ_10_FULL_BIT               BIT(22)
#define FP2HWE_WQ_11_FULL_BIT               BIT(23)
#define CPUX_TO_CPUY_WQ_EMPTY_7_0_BIT       BIT(24)
#define CPUX_TO_CPUY_WQ_FULL_7_0_BIT        BIT(25)
#define SLOT_ARRAY_EMPTY_131_0_BIT          BIT(26)
#define SLOT_ARRAY_FULL_131_0_BIT           BIT(27)
#define FPS_CPU_STAGE1_WDTO_BIT             BIT(28)
#define FPS_CPU_STAGE2_WDTO_BIT             BIT(29)
#define FPS_FABRIC_PARITY_ERROR_BIT         BIT(30)
#define FPS_MEMORY_PARITY_ERROR_BIT         BIT(31)

// fps_bank1_reg_event_status_0 0x40000004
#define HWE2FP_WQ_00_UCD_IB_CQ0_FULL_BIT    BIT(0)
#define HWE2FP_WQ_01_UCD_IB_CQ1_FULL_BIT    BIT(1)
#define HWE2FP_WQ_02_UCD_OB_CQ0_FULL_BIT    BIT(2)
#define HWE2FP_WQ_03_UCD_OB_CQ1_FULL_BIT    BIT(3)
#define HWE2FP_WQ_04_CDMA_CQ_FULL_BIT       BIT(4)
#define HWE2FP_WQ_05_GDMA_CQ_FULL_BIT       BIT(5)
#define HWE2FP_WQ_06_CP2FP_CMD_SQ_FULL_BIT  BIT(6)
#define HWE2FP_WQ_07_FP2CP_ERR_CQ_FULL_BIT  BIT(7)
#define HWE2FP_WQ_08_FULL_BIT               BIT(8)
#define HWE2FP_WQ_09_FULL_BIT               BIT(9)
#define HWE2FP_WQ_10_FULL_BIT               BIT(10)
#define HWE2FP_WQ_11_FULL_BIT               BIT(11)
#define FP2HWE_WQ_00_EMPTY_BIT              BIT(12)
#define FP2HWE_WQ_01_EMPTY_BIT              BIT(13)
#define FP2HWE_WQ_02_EMPTY_BIT              BIT(14)
#define FP2HWE_WQ_03_EMPTY_BIT              BIT(15)
#define FP2HWE_WQ_04_CDMA_SQ_EMPTY_BIT      BIT(16)
#define FP2HWE_WQ_05_GDMA_SQ_EMPTY_BIT      BIT(17)
#define FP2HWE_WQ_06_CP2FP_CMD_CQ_EMPTY_BIT BIT(18)
#define FP2HWE_WQ_07_FP2CP_ERR_SQ_EMPTY_BIT BIT(19)
#define FP2HWE_WQ_08_EMPTY_BIT              BIT(20)
#define FP2HWE_WQ_09_EMPTY_BIT              BIT(21)
#define FP2HWE_WQ_10_EMPTY_BIT              BIT(22)
#define FP2HWE_WQ_11_EMPTY_BIT              BIT(23)

// fps_bank0_reg_indirect_register_write_disable 0x400000F0
#define FP2HWE_Q_PI_04_WR_BIT    BIT(16)
#define SOC_REG_0_WR_BIT         BIT(24)
#define SOC_REG_1_WR_BIT         BIT(25)
#define SOC_REG_2_WR_BIT         BIT(26)
#define SOC_REG_3_WR_BIT         BIT(27)


// Fw update usage
#define FW_UPDATE_SIGNATURE 0x46575544
#define FW_UPDATE_BACKUP_DATA_BLK_CNT 0x2
#define FW_UPDATE_STS_SHIFT 4
typedef enum
{
    cUCDData = 0,
    cLoggingData,
    cTotalBlkCnt,
} RecoverDataBlk;

typedef enum
{
    cNewVer = 0,
    cNoSignature,
    cOldVer,
    cChkSumFail,
} FwUdSts;

typedef struct FwUpdateDataHeader
{
    uint32_t signature;
    uint16_t totalDataLength;
    uint16_t dataBlkCnt;
    uint32_t checkSum;
}FwUpdateDataHeader;
typedef struct FWupdateBackupInfo
{
    uint32_t addr;
    uint32_t length : 24;
    uint32_t sts : 8;
}FWupdateBackupInfo;
// Error injection usage
#if defined (SUPPORT_MSGERROR_INJECTION) || defined (SUPPORT_ERROR_INJECTION)
#define INVALID_CDMA_OPCODE 5
#define INVALID_REFERENCE_WITH_BYTE 0xFF
#define INVALID_REFERENCE_WITH_DOUBLE 0xFFFF
#define INVALID_REFERENCE_WITH_DWORD 0xFFFFFFFF
#define COMPARED_ID_WITH_BYTE(comparedId, Id) ((comparedId == INVALID_REFERENCE_WITH_BYTE) ? true : (comparedId == Id))
#define COMPARED_ID_WITH_2BYTE(comparedId, Id) ((comparedId == INVALID_REFERENCE_WITH_DOUBLE) ? true : (comparedId == Id))
#define MSFT_SIGNATURE 0x4D534654

typedef enum ErrorType_t
{
    cNoErr = 0,
    cDescmDestAxiRdErr = 1,
    cDescmSrcAxiRdErr = 2,
    cCmdeUnexpectedCmdPhaseErr = 3,
    cCmdeInvalidOpcodeErr = 4,
    cCmdeAxiRdErr = 5,
    cCmpleAxiWrErr = 6,
    cCmdTransferLengthUnderrunErr = 7,
    cCmdTransferLengthOverrunErr = 8,
    cDoeBufferRdParityErr = 9,
    cCryptoeTextOutReadErr = 10,
    cCryptoeRedundancyMismatchErr = 11,
    cDbmBufferRdParityErr = 12,
    cDbmAxiRdErr = 13,
    cDoeOverrunErr = 14,  // fatal error start
    cDoeUnderrunErr = 15,
    cDoeAxiWrErr = 16,
    cCryptoeFatalErr = 17,
    cCryptoeKeyVaultMemRdErr = 18,
    cDfeOverrunErr = 19,
    cDfeUnderrrunErr = 20,
    #ifdef LIONMS_B0
    cCommandSlotErrSts1 = 21, // non fatal not retry
    #endif
};

#endif

typedef enum Fastpath_Status_t
{
    FP_STS_HALT = 0,
    FP_STS_INIT_START = 1,
    FP_STS_INIT_DONE = 2,
    FP_STS_NORMAL_BOOT = 3,
    FP_STS_FP_FW_UPDATE_START = 4,
    FP_STS_FP_FW_UPDATE_RESP = 5,
    FP_STS_FP_START = 6,
    FP_STS_FP_ERR = 7,
    FP_STS_FP_WAIT = 8,
    FP_STS_FP_CORE_DUMP = 9
} Fastpath_Status_t;

typedef enum Fastpath_OP_Mode_t
{
    FP_MODE_GREEDY = 0,
    FP_MODE_STRICT = 1
}Fastpath_OP_Mode_t;

/*
 * Status for the command entry structure.
 */
typedef enum CmdEntryStatus_t
{
    cCEStsInValid            = 0x0,
    cCEStsValid              = 0x1,
    cCEStsRetry              = 0x2,
    cCEStsPoorSGLRetry       = 0x3,
    cCEStsCorrKeyErrHandling = 0x4,
    cCEStsInvalidXTSField    = 0x5,
    cCEStsInvalidGCMField    = 0x6,
    cCEStsFatalError         = 0x7,
    cCEStsDelQ               = 0x8,
    cCEStsCdmaAbort          = 0x9,
    cCEStsTearDown           = 0xA,
    cCEStsCryptoEngineError  = 0xB,
    cCEStsCpCdmaError        = 0xC,
    cCEStsFetchError         = 0xD,
    cCEStsDataTranferTimeout = 0xE,
    cCEStsQoSError           = 0xF,

} CmdEntryStatus_t;

/*
 * Error status for the command entry tiny structure.
 */
typedef enum CmdEntryTinyErrStatus_t
{
    cCETinyStsNoErr = 0x0,
    cCETinyStsNonFatalErr = 0x1,
    cCETinyStsPoorSGLErr = 0x2,
    cCETinyStsZeroXfer = 0x3,
    cCETinyStsFatalErr = 0x4,
    cCETinyStsQosErr = 0x5,
    cCETinyStsReportHost = 0x6,
} CmdEntryTinyErrStatus_t;

/*
 * Error code for the Host in command entry tiny structure.
 */
typedef enum CmdEntryTinyHostErrCode_t
{
    cCETinyHostErrDefaultErrorCode            = 0x0,

    //SQE Validation, (Sts Code - cCEStsInvalidXTSField)
    cCETinyHostErrSqeXtsInvalidKey1           = 0x1,
    cCETinyHostErrSqeXtsInvalidKey2           = 0x2,
    cCETinyHostErrSqeXtsInvalidVfID           = 0x3,
    cCETinyHostErrSqeXtsDiffEphemeralFlag     = 0x4,
    cCETinyHostErrSqeXtsSameKeysPassed        = 0x5,
    cCETinyHostErrSqeXtsSameKeysDiffIndex     = 0x6,
    cCETinyHostErrSqeXtsInvalidKey1SessionID  = 0x7,
    cCETinyHostErrSqeXtsInvalidKey2SessionID  = 0x8,
    cCETinyHostErrSqeXtsInvalidKey1AppID      = 0x9,
    cCETinyHostErrSqeXtsInvalidKey2AppID      = 0xA,
    cCETinyHostErrSqeXtsDataLengthExceeded    = 0xB,
    cCETinyHostErrSqeXtsLengthUnaligned16B    = 0xC,
    cCETinyHostErrSqeXtsUndefinedErr          = 0xD,
    cCETinyHostErrSqeXtsInvalidKey1Type       = 0xE,
    cCETinyHostErrSqeXtsInvalidKey2Type       = 0xF,

    //SQE Validation, (StsCode - cCEStsInvalidGCMField)

    cCETinyHostErrSqeGcmInvalidKey           = 0x1,
    cCETinyHostErrSqeGcmInvalidVfID          = 0x2,
    cCETinyHostErrSqeGcmInvalidKeySessionID  = 0x3,
    cCETinyHostErrSqeGcmInvalidKeyAppID      = 0x4,
    cCETinyHostErrSqeGcmInvalidAadLength     = 0x5,
    cCETinyHostErrSqeGcmLengthUnaligned16B   = 0x6,
    cCETinyHostErrSqeGcmUndefinedErr         = 0x7,
    cCETinyHostErrSqeGcmInvalidKeyType       = 0x8,

    //CQE Validation

    //StsCode - cCEStsCryptoEngineError
    cCETinyHostErrCqeDescmDestAxiRdErr              = 0x1,
    cCETinyHostErrCqeDescmSrcAxiRdErr               = 0x2,
    cCETinyHostErrCqeCmpleAxiWrErr                  = 0x3,
    cCETinyHostErrCqeDoeMaxElmntCountErr            = 0x4,
    cCETinyHostErrCqeDoeOverrunErr                  = 0x5,
    cCETinyHostErrCqeDoeUnderrunErr                 = 0x6,
    cCETinyHostErrCqeDoeBufferRdParityErr           = 0x7,
    cCETinyHostErrCqeDoeAxiWrErr                    = 0x8,
    cCETinyHostErrCqeCryptoeTagMismatchErr          = 0x9,
    cCETinyHostErrCqeCryptoeFatalErr                = 0xA,
    cCETinyHostErrCqeCryptoeTextOutReadErr          = 0xB,
    cCETinyHostErrCqeCryptoeRedundancyMismatchErr   = 0xC,
    cCETinyHostErrCqeCryptoeKeyVaultMemRdErr        = 0xD,
    cCETinyHostErrCqeDbmBufferRdParityErr           = 0xE,
    cCETinyHostErrCqeDbmAxiRdErr                    = 0xF,
    cCETinyHostErrCqeDfeOverrunErr                  = 0x10,
    cCETinyHostErrCqeDfeUnderrunErr                 = 0x11,
    cCETinyHostErrCqeDfeMaxElmntCountErr            = 0x12,

    //StsCode - cCEStsFetchError

    cCETinyHostErrCqeDestPrpFetchError    = 0x1,
    cCETinyHostErrCqeDestSglFetchError    = 0x2,
    cCETinyHostErrCqeSrcPrpFetchError     = 0x3,
    cCETinyHostErrCqeSrcSglFetchError     = 0x4,

    cCETinyHostErrCqeDescmDestDescrStructureErr  = 0x5,
    cCETinyHostErrCqeDescmSrcDescrStructureErr   = 0x6,
    cCETinyHostErrCqeCmdeUnexpectedCmdPhaseErr   = 0x7,
    cCETinyHostErrCqeCmdeInvalidOpcodeErr        = 0x8,
    cCETinyHostErrCqeCmdeStructureErr            = 0x9,

    cCETinyHostErrCqeCmdTransferLengthUnderrunErr = 0xA,
    cCETinyHostErrCqeCmdTransferLengthOverrunErr  = 0xB,

    cCETinyHostErrCqeSrcVfFetchError      = 0xC,

    //StsCode - cCEStsDataTranferTimeout

    cCETinyHostErrCqeDestXferDataTimeout  = 0x1,
    cCETinyHostErrCqeSrcXferDataTimeout   = 0X2,

}CmdEntryTinyHostErrCode_t;


/*
 * CE status for the command entry tiny structure.
 */
typedef enum CmdEntryTinyAbortStatus_t
{
    cCETinyNormal = 0,
    cCETinyAdminAbort = 1,
} CmdEntryTinyStatus_t;

typedef enum QB_Index_t
{
    QB0_QB31      = 0,
    QB32_QB63     = 1,
    QB64          = 2,
    QB_MAX        = 3
} QB_Index_t;

typedef enum VF_Index_t
{
    VF0_VF31      = 0,
    VF32_VF63     = 1,
    VF64          = 2,
    VF_MAX        = 3   // control VF for loop in FpsCpu1QueueManagerFiber
} VF_Index_t;

typedef enum IBCQ_Index_t
{
    IBCQ_0      = 0,
    IBCQ_1      = 1,
    IBCQ_END    = 2,
} IBCQ_Index_t;
typedef enum OBCQ_Index_t
{
    OBCQ_0      = 0,
    OBCQ_1      = 1,
    OBCQ_END    = 2,
} OBCQ_Index_t;
typedef enum OSL_Index_t
{
    OSL_0     = 0,
    OSL_1     = 1,
    OSL_END   = 2
} OSL_Index_t;

typedef enum DFL_Index_t
{
    DFL_0      = 0,
    DFL_1      = 1,
    DFL_END    = 2,
} DFL_Index_t;

// Command Entry
typedef struct CmdEntry_t
{
    union
    {
        uint32_t Dw0;
        struct
        {
            uint32_t Status    : 4;    ///< 0x0: invalid, 0x1: valid, 0x2:retry, ...
            uint32_t PhyIbqId  : 8;
            uint32_t IFSel     : 8;
            uint32_t DFLIdx    : 10;   ///< 0-511
            uint32_t cdmaListNum : 2;  ///< 0-3
        };
    };
} CmdEntry_t;

// Command Entry Tiny
typedef struct CmdEntryTiny_t
{
    union
    {
        uint32_t Dw0;
        struct
        {
            uint32_t RetryTimes : 2;   ///< retry times
            uint32_t ErrStatus  : 4;   ///< Maintain the error status of the command entry. 0x0: no error, 0x1: non-fatal error, ...
            uint32_t abortStatus : 1;  ///< 0x0: normal status, 0x1: this command was aborted by host
            uint32_t HostErrCode : 8 ; ///< Error code for Host Status
            uint32_t Reserved   : 17;
        };
    };
} CmdEntryTiny_t;

// Error pending queue context
typedef struct ErrorQueueContext_t
{
    union
    {
        uint32_t Dw0;
        struct
        {
            uint32_t DFLIdx : 10;
            uint32_t DFListNum : 6;
            uint32_t Reserved : 16;
        };
    };
} ErrorQueueContext_t;

typedef struct FpsUcdIbq_t
{
    uint32_t* pHwStatus;
    volatile uint32_t* pHwIbCqPi[IBCQ_END];
    volatile uint32_t* pHwIbCqCi[IBCQ_END];
    UcdCqEntry_t* pIbCqe[IBCQ_END];
    volatile uint32_t* pHwDflPi[DFL_END];
    volatile uint32_t* pHwDflCi[DFL_END];
    uint64_t* pDflEntries[DFL_END];
} FpsUcdIbq_t;

typedef struct FpsUcdObq_t
{
    uint32_t* pHwStatus;
    volatile uint32_t* pHwOslPi[OSL_END];
    volatile uint32_t* pHwOslCi[OSL_END];
    volatile uint32_t* pHwObCqPi[OBCQ_END];
    volatile uint32_t* pHwObCqCi[OBCQ_END];
    struct nvme_completion* pCplEntries;
    UcdOslEntry_t* pOslEntries[OSL_END];
    UcdCqEntry_t* pCqEntries[OBCQ_END];
} FpsUcdObq_t;

typedef struct CdmaSq_t
{
    CdmaSqCmdDescr_t* pCdmaSqBase;
    uint32_t* pHwPi;
    uint32_t* pHwCi;
    uint32_t* pHwStatus;
    #ifdef DISABLE_INDIRECT_REG_WRITE
    uint32_t PiHwAddr;
    #endif
} CdmaSq_t;

typedef struct CdmaCq_t
{
    CdmaCqCmdDescr_t* pCdmaCqBase;
    uint32_t* pHwPi;
    uint32_t* pHwCi;
    uint32_t* pHwStatus;
} CdmaCq_t;

typedef struct Ucd_Query_Parameters
{
    uint32_t queueDepth;
    uint32_t dflListBaseAddr;
    uint32_t oslListBaseAddr;
    uint32_t ibcqEBaseAddr;
    uint32_t obcqEBaseAddr;
    uint32_t dflBufBaseAddr;
    uint32_t ibcqPiShadowAddr;
    uint32_t obcqPiShadowAddr;
} UcdQueryPara_t;

#pragma pack(push)
#pragma pack(1)
//AES key vault
// u32 array (not u8) because the CDMA key vault MMIO region requires
// 32-bit-only accesses. Using a u32 array makes word-aligned access
// the natural representation and prevents compilers from emitting
// STRD/STRB via memcpy/memset on the byte-array form.
typedef struct AesKeyVault_t
{
    uint32_t key[AES_KEY_LEN_IN_WORDS];
} AesKeyVault_t ;
#pragma pack(pop)
COMPILE_ASSERT(sizeof(AesKeyVault_t) == (sizeof(uint32_t) * AES_KEY_LEN_IN_WORDS), "Invalid AesKeyVault_t size");

typedef enum
{
    cHwe2FpWq00UcdIbCq0 = 0,
    cHwe2FpWq01UcdIbCq1,
    cHwe2FpWq02UcdObCq0,
    cHwe2FpWq03UcdObCq1,
    cHwe2FpWq04CdmaCq,
    cHwe2FpWq05GdmaCq,
    cHwe2FpWq06Cp2FpSq,
    cHwe2FpWq07Fp2CpErrCq,
    cHwe2FpWq08,
    cHwe2FpWq09,
    cHwe2FpWq10,
    cHwe2FpWq11,
    cHwe2FpEnd = 12
} FpRegHwe2Fp;

typedef enum
{
    cFpSocFwd00Ucd1Dfl0 = 0,
    cFpSocFwd01Ucd1Osl0,
    cFpSocFwd02Ucd1Dfl3,
    cFpSocFwd03Ucd1Osl1,
    cFpSocFwd04,
    cFpSocFwd05,
    cFpSocFwd06,
    cFpSocFwd07,
    cFpRegSocFwEnd = 8
} FpRegSocFw;

typedef enum
{
    cFp2HweWq00 = 0,
    cFp2HweWq01,
    cFp2HweWq02,
    cFp2HweWq03,
    cFp2HweWq04CdmaSq,
    cFp2HweWq05GdmaDq,
    cFp2HweWq06Cp2FpCq,
    cFp2HweWq07Fp2CpErrSq,
    cFp2HweWq08GdmaDq,
    cFp2HweWq09GdmaDq,
    cFp2HweWq10,
    cFp2HweWq11,
    cFp2HweEnd = 12
} FpRegFp2Hwe;

typedef enum
{
    cCpuX2CpuYWq00 = 0,
    cCpuX2CpuYWq01,
    cCpuX2CpuYWq02,
    cCpuX2CpuYWq03,
    cCpuX2CpuYWq04,
    cCpuX2CpuYWq05,
    cCpuX2CpuYWq06,
    cCpuX2CpuYWq07,
    cFpRegCpuXtoYEnd = 8
} FpRegCpuXtoY;
