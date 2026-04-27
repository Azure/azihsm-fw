// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//  ---------------------------------------------------
//  !
//  ! @file   HalHostLionMSFpCmd.h
//  ! @brief  The Nvme module APIs header file.
//  !
//  ---------------------------------------------------

#pragma once
#pragma pack(push, enterHalHostLionMSFpCmdh)
#pragma pack(1)
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
   //  Dependencies
   -----------------------------------------------------------------------------*/
#include "SysTypes.h"
#include "List.h"
#include "ErrorCodes.h"
#include "platform.h"

/*-----------------------------------------------------------------------------
   //  Public Constant Definitions
   -----------------------------------------------------------------------------*/
#define HOST_SQE_DW1_CIPHER_SHIFT 8
#define HOST_SQE_DW1_PRPSG_SHIFT 2
#define HOST_SQE_DW1_XTS_DATA_UNIT_LENGTH_SHIFT 14
#define HOST_SQE_KEY_INDEX_MASK 0x1ffUL
#define HOST_SQE_KEY_RGID_SHIFT 3
#define HOST_SQE_KEY_SUB_INDEX_MASK 0x7UL

/*
 * Lion CQE Command Status Code
 */
typedef enum LionFPCQEStatusCode
{
    CQE_SC_SUCCESS                  = 0x0,
    CQE_SC_INVALID_FIELD_XTS        = 0x1,
    CQE_SC_INVALID_FIELD_GCM        = 0x2,
    CQE_SC_CRYPTO_ENGINE_ERROR      = 0x3,
    CQE_SC_FETCH_ERROR              = 0x4,
    CQE_SC_DATA_TRANSFER_TIMEOUT    = 0x5,
    CQE_SC_DELETE_QUEUE             = 0x7,
    CQE_SC_ABORT_REQ                = 0x8,
    CQE_SC_CMD_RETRY_TIMES_EXCEEDED = 0x9,
    CQE_SC_QOS_LATENCY_ERROR        = 0xA,

} LionFPCQEStatusCode;

/*
 * Lion CQE Command Error Code
 */
typedef enum LionFPCQEErrorCode
{
    CQE_DEFAULT_ERROR_CODE              = 0x0,

    //SQE Validation, (Sts Code - CQE_SC_INVALID_FIELD_XTS)
    SQE_XTS_INVALID_KEY1            = 0x1,
    SQE_XTS_INVALID_KEY2            = 0x2,
    SQE_XTS_INVALID_VFID            = 0x3,
    SQE_XTS_DIFF_EPHEMERAL_FLAG     = 0x4,
    SQE_XTS_SAME_KEYS_PASSED        = 0x5,
    SQE_XTS_SAME_KEYS_DIFF_INDEX    = 0x5,
    SQE_XTS_INVALID_KEY1_SESSION_ID = 0x7,
    SQE_XTS_INVALID_KEY2_SESSION_ID = 0x8,
    SQE_XTS_INVALID_KEY1_APP_ID     = 0x9,
    SQE_XTS_INVALID_KEY2_APP_ID     = 0xA,
    SQE_XTS_DATA_LENGTH_EXCEEDED    = 0xB,
    SQE_XTS_LENGTH_UNALIGNED_16B    = 0xC,
    SQE_XTS_UNDEFINED_ERR           = 0xD,
    SQE_XTS_INVALID_KEY1_TYPE       = 0xE,
    SQE_XTS_INVALID_KEY2_TYPE       = 0xF,

    //SQE Validation, (StsCode - CQE_SC_INVALID_FIELD_GCM)

    SQE_GCM_INVALID_KEY           = 0x1,
    SQE_GCM_INVALID_VFID          = 0x2,
    SQE_GCM_INVALID_SESSION_ID    = 0x3,
    SQE_GCM_INVALID_APP_ID        = 0x4,
    SQE_GCM_INVALID_AAD_LENGTH    = 0x5,
    SQE_GCM_LENGTH_UNALIGNED_16B  = 0x6,
    SQE_GCM_UNDEFINED_ERR         = 0x7,
    SQE_GCM_INVALID_KEY_TYPE      = 0x8,

    //CP Tag Corr Response Err, (StsCode - CQE_SC_INVALID_FIELD_GCM)

    CP_GCM_INVALID_AES_GCM_REQUEST_PTR    = 0x20,
    CP_GCM_INVALID_SQE_ADDR_PTR           = 0x21,
    CP_GCM_INVALID_UNALIGNED_SRC_DATA_PTR = 0x22,
    CP_GCM_INVALID_UNALIGNED_DST_DATA_PTR = 0x23,
    CP_GCM_INVALID_PCIE_FN                = 0x24,
    CP_GCM_DMA_MEM_ALLOC_FAILED           = 0x25,
    CP_GCM_DMA_IN_OPERATION_ERR           = 0x26,
    CP_GCM_DMA_OUT_OPERATION_ERR          = 0x27,
    CP_GCM_KEY_BLOB_READ_FAILED           = 0x28,
    CP_GCM_TAG_CORR_FAILED                = 0x29,
    CP_GCM_INVALID_DECRYPT_TAG            = 0x2A,
    CP_GCM_INVALID_SQE_INDEX              = 0x2B,
    CP_GCM_INVALID_UNALIGNED_DATA_LEN     = 0x2C,

    //CQE Validation

    //StsCode - CQE_SC_CRYPTO_ENGINE_ERROR

    CQE_DESCM_DEST_AXI_RD_ERR           = 0x1,
    CQE_DESCM_SRC_AXI_RD_ERR            = 0x2,
    CQE_CMPLE_AXI_WR_ERR                = 0x3,
    CQE_DOE_MAX_ELMNT_COUNT_ERR         = 0x4,
    CQE_DOE_OVERRUN_ERR                 = 0x5,
    CQE_DOE_UNDERRUN_ERR                = 0x6,
    CQE_DOE_BUFFER_RD_PARITY_ERR        = 0x7,
    CQE_DOE_AXI_WR_ERR                  = 0x8,
    CQE_CRYPTOE_TAG_MISMATCH_ERR        = 0x9,
    CQE_CRYPTOE_FATAL_ERR               = 0xA,
    CQE_CRYPTOE_TEXT_OUT_READ_ERR       = 0xB,
    CQE_CRYPTOE_REDUNDANCY_MISMATCH_ERR = 0xC,
    CQE_CRYPTOE_KEY_VAULT_MEM_RD_ERR    = 0xD,
    CQE_DBM_BUFFER_RD_PARITY_ERR        = 0xE,
    CQE_DBM_AXI_RD_ERR                  = 0xF,
    CQE_DFE_OVERRUN_ERR                 = 0x10,
    CQE_DFE_UNDERRUN_ERR                = 0x11,
    CQE_DFE_MAX_ELMNT_COUNT_ERR         = 0x12,

    //StsCode - CQE_SC_FETCH_ERROR

    CQE_DEST_PRP_FETCH_ERROR    = 0x1,
    CQE_DEST_SGL_FETCH_ERROR    = 0x2,
    CQE_SRC_PRP_FETCH_ERROR     = 0x3,
    CQE_SRC_SGL_FETCH_ERROR     = 0x4,

    CQE_DESCM_DEST_DESCR_STRUCTURE_ERR  = 0x5,
    CQE_DESCM_SRC_DESCR_STRUCTURE_ERR   = 0x6,
    CQE_CMDE_UNEXPECTED_CMD_PHASE_ERR   = 0x7,
    CQE_CMDE_INVALID_OPCODE_ERR         = 0x8,
    CQE_CMDE_STRUCTURE_ERR              = 0x9,

    CQE_CMD_TRANSFER_LENGTH_UNDERRUN_ERR = 0xA,
    CQE_CMD_TRANSFER_LENGTH_OVERRUN_ERR  = 0xB,

    CQE_SRC_VF_FETCH_ERROR      = 0xC,

    //StsCode - CQE_SC_DATA_TRANSFER_TIMEOUT

    CQE_DEST_XFER_DATA_TIMEOUT  = 0x1,
    CQE_SRC_XFER_DATA_TIMEOUT   = 0X2,

}LionFPCQEErrorCode_t;

/*
 * AES GCM Tag Correction CP Resp Error Code
 */
typedef enum AesGcmExtRespErr{

    AES_GCM_RESP_SUCCESS            = 0x0, // Successful completion
    INVALID_AES_GCM_REQUEST_PTR     = 0x1, // Invalid request pointer
    INVALID_SQE_ADDR_PTR            = 0x2, // Invalid request pointer
    INVALID_UNALIGNED_SRC_DATA_PTR  = 0x3, // Invalid unaligned src data pointer
    INVALID_UNALIGNED_DST_DATA_PTR  = 0x4, // Invalid unaligned dst data pointer
    INVALID_PCIE_FN                 = 0x5, // Invalid PFN value
    DMA_MEM_ALLOC_FAILED            = 0x6, // Dma In Operation failed
    DMA_IN_OPERATION_ERR            = 0x7, // Dma In Operation failed
    DMA_OUT_OPERATION_ERR           = 0x8, // Dma Out Operation failed
    AES_GCM_KEY_BLOB_READ_FAILED    = 0x9, // Key Blob read from CDMA Key Vault failed
    AES_GCM_TAG_CORRECTION_FAILED   = 0xa, // GCM Tag correction failed
    AES_GCM_INVALID_DECRYPT_TAG     = 0xb, // Invalid GCM Tag in decrypt operation
    INVALID_SQE_INDEX               = 0xc, // Invalid SQE Index
    INVALID_UNALIGNED_DATA_LEN      = 0xd, // Invalid Unaligned data length
};

/*-----------------------------------------------------------------------------
   //  Public Macros Definitions
   -----------------------------------------------------------------------------*/
#define PCI_VENDOR_ID_MICROSOFT 0x1414
#define PCI_VENDOR_ID_MARVELL_EXT 0x1b4b
#define PCI_SUB_VENDOR_ID_MARVELL_EXT 0x1b4b
#define SUPPORT_ABORT_CMD_COUNT 1
#define SUPPORT_FW_SLOT_COUNT 2
/*-----------------------------------------------------------------------------
   //  Public Data Type Definitions
   -----------------------------------------------------------------------------*/
//================== NVMe Standard Admin Command ==================
typedef struct
{
    uint64_t addr;
    uint32_t length;
    uint8_t rsvd[3];
    uint8_t type;
} NVMeSglDesc_t;

typedef union
{
    struct
    {
        uint64_t prp1;
        uint64_t prp2;
    };
    NVMeSglDesc_t sgl;
} NVMeDataPtr_t;

typedef struct
{
    uint8_t opcode;
    uint8_t flags;
    uint16_t commandId;
    uint32_t nsid;
    uint32_t cdw2[2];
    uint64_t metadata;
    NVMeDataPtr_t dptr;
    uint32_t cdw10[6];
} NVMeCommonCommand_t;

typedef struct
{
    uint8_t opcode;
    uint8_t flags;
    uint16_t commandId;
    uint32_t rsvd1[5];   ///< DW1~5
    uint64_t prp1;   ///< DW6~7
    uint64_t rsvd8;   ///< DW8~9
    uint16_t cqid;
    uint16_t qsize;
    uint16_t cqFlags;
    uint16_t irqVector;
    uint32_t rsvd12;   ///< DW12
    uint8_t vfid;
    uint8_t rsvd13[3];
    uint32_t rsvd14[2];   ///< DW14~15
} NVMeCreateCq_t;

typedef struct
{
    uint8_t opcode;
    uint8_t flags;
    uint16_t commandId;
    uint32_t rsvd1[5];   ///< DW1~5
    uint64_t prp1;   ///< DW6~7
    uint64_t rsvd8;   ///< DW8~9
    uint16_t sqid;
    uint16_t qsize;
    uint16_t sqFlags;
    uint16_t cqid;
    uint32_t rsvd12;   ///< DW12
    uint8_t vfid;   ///< DW13 [7:0]
    uint8_t rsvd13[3];
    uint32_t rsvd14[2];   ///< DW14~15
} NVMeCreateSq_t;

typedef struct
{
    uint8_t opcode;
    uint8_t flags;
    uint16_t commandId;
    uint32_t rsvd1[9];   ///< DW1~9
    uint16_t qid;
    uint16_t rsvd10;
    uint32_t rsvd11[2];   ///< DW11~12
    uint8_t vfid;   ///< DW13 [7:0]
    uint8_t rsvd13[3];
    uint32_t rsvd14[2];   ///< DW14~15
} NVMeDeleteQ_t;

typedef struct
{
    uint8_t opcode;
    uint8_t flags;
    uint16_t commandId;
    uint32_t nsid;
    uint32_t rsvd2[5]; ///< DW2~5
    NVMeDataPtr_t dptr;
    uint8_t cns; ///< DW10
    uint8_t rsvd;
    uint16_t cntid;
    uint32_t rsvd11[5]; ///< DW11~15
} NVMeIdentify_t;

typedef struct
{
    uint8_t opcode;
    uint8_t flags;
    uint16_t commandId;
    uint32_t nsid;
    uint32_t rsvd2[5]; ///< DW2~5
    NVMeDataPtr_t dptr;
    uint32_t fid; ///< DW10
    uint32_t dw11;
    uint8_t vfid; ///< specfic record
    uint8_t rsvd12[3];
    uint32_t rsvd13[3]; ///< DW13~15
} NVMeSetFeature_t;
//================== UCD API specific Admin Command ==================

/// < @brief Specific command.
typedef struct CreateCq_t
{
    uint8_t PfVfId;
    uint8_t Reserved2;
    uint16_t CqId;
    uint64_t QueueBaseAddr;
    uint16_t QueueSize;
    uint8_t Reserved3[10];
} CreateCq_t;

typedef struct CreateSq_t
{
    uint8_t PfVfId;
    uint8_t Reserved2;
    uint16_t CqId;
    uint16_t SqId;
    uint16_t Priority;
    //uint8_t Priority : 4;
    //uint8_t Reserved3 : 4;
    uint64_t QueueBaseAddr;
    uint16_t QueueSize;
    uint8_t Reserved4[6];
} CreateSq_t;

typedef struct DeleteSq_t
{
    uint8_t PfVfId;
    uint8_t Reserved;   //IsNeedFlush;
    uint16_t SqId;
    uint8_t Reserved3[20];
} DeleteSq_t;

typedef struct DeleteCq_t
{
    uint8_t PfVfId;
    uint8_t Reserved2;
    uint16_t CqId;
    uint8_t Reserved3[20];
} DeleteCq_t;

typedef struct ForceCmpl_t
{
    uint8_t VFId;   ///< 0~63
    uint8_t SqId;   ///< Host SQID
    uint16_t Reserved;
} ForceCmpl_t;

typedef struct ResumeQ_t
{
    uint8_t VFId;
    uint8_t SqId;
    uint16_t Reserved;
} ResumeQ_t;

typedef struct Identify_t
{
    uint32_t Nsid;
    uint8_t Cns;
    uint8_t Reserved2;
    uint16_t Cntid;
    uint64_t Prp1;
    uint64_t Prp2;
} Identify_t;

typedef struct SetFeature_t
{
    uint32_t Fid : 8; //feature id
    uint32_t Sv : 1;
    uint32_t Reserved2 : 17;
    uint32_t dw11;
    uint32_t dw12[4];
} SetFeature_t;

typedef struct VfInstallTearDown_t
{
    uint8_t VfId;               ///> It should be one of 0 to 63.
    uint8_t Action;            ///> 0: install, 1: tear down.
    uint8_t Reserved3[22];
} VfInstallTearDown_t;

/// < @brief Specific command.
typedef struct UcdCpCmd_t
{
    ListHead_t Pointer;   ///> DWORD 0~1
    void (*UcdCpCmplHandlerCallback)(UcdCpCmd_t* pCmd, Error_t status);   ///> DWORD 2
    uint8_t Opcode;   ///> DWORD 3
    uint8_t SubOpcode;   ///< 0: FP IO, 1: CP IO
    uint16_t CmdId;
    union   ///> DWORD 4~9
    {
        CreateCq_t createCq;
        CreateSq_t createSq;
        DeleteCq_t deleteCq;
        DeleteSq_t deleteSq;
        ForceCmpl_t forceCmpl;
        ResumeQ_t resumeQ;
        VfInstallTearDown_t vfInstallTearDown;
    };   ///< 24 bytes
} UcdCpCmd_t;

//================== Message Entry Related Command ==================

#ifdef NEW_AES_KEY_VALIDATION_SUPPORT
/// < @brief key flag type
typedef enum KeyFlagType_t
{
    cPersistentKeyFlag = 0x0,
    cEphemeralKeyFlag,
    cInvalidKeyFlag
} KeyFlagType_t;

typedef enum AesKeyType_t
{
    cAesXts = 0,
    cAesGcmApproved,
    cAesGcmUnapproved
}AesKeyType_t;

typedef struct KeyFlags_t
{
    union 
    {
        uint8_t all;
        struct 
        {
            uint8_t session_only : 1;
            uint8_t keyType : 2;
            uint8_t reserved : 5;
        };
        
    };
    
}KeyFlags_t;

#endif

/// < @brief Specific command.
typedef struct KeyUpdate_t
{
    uint8_t keySubIndex; // 0-6
    uint8_t resourceGroupId; // 0-63
    uint8_t vfId; //VF ID 0-63
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    uint8_t action;  // 0: delete key at a KeyIndex, 1: delete all ephemeral keys for a session; 2: delete all keys for an application, 3:create a key
    uint16_t sessionId; // session ID for the key to be updated
    uint8_t appId; // application ID for the key to be updated
    KeyFlags_t flags;
    uint32_t Reserved3[4];
    #else
    uint8_t action;  // 0: enable, 1: disable
    uint32_t Reserved3[5];
    #endif
} KeyUpdate_t;

#ifdef NEW_AES_KEY_VALIDATION_SUPPORT
#define KEYUPDATE_RGID_MAX  64
#define KEYUPDATE_VF_ID_MAX 64
#define KEYUPDATE_KEY_SUB_IDX_MAX 6
#define KEY_INDEX_MAX ((KEYUPDATE_CDMAIO_RGID + 1) * (KEYUPDATE_KEY_SUB_IDX_MAX + 1)) // Using KEYUPDATE_CDMAIO_RGID as we want to calculate for all 65 Resource Groups
#else
#define KEYUPDATE_RGID_MAX  64
#define KEYUPDATE_VF_ID_MAX 64
///< KeyUpdate_t parameter range
#define KEYUPDATE_KEY_SUB_IDX_MAX 6
#endif
#define KEYUPDATE_CDMAIO_RGID 65
#define CDMAIO_VF_ID 65

/// < @brief Specific command.
typedef struct FpModeChange_t
{
    uint8_t fpMode; // 0-7
    uint8_t Reserved1[3];
    uint32_t Reserved2[5];
} FpModeChange_t;

#ifdef SUPPORT_MSGERROR_INJECTION
/// < @brief Specific command.
typedef struct MsgErrorInjection_t
{
    uint16_t cmdId;
    uint8_t vfId;
    uint8_t errorType;
    uint8_t en;
    uint8_t reErrInjectTimes;
    uint8_t Reserved1[2];
    uint32_t Reserved2[4];
} MsgErrorInjection_t;
#endif

#ifdef LOGGING_NEW_SCHEME
/// < @brief Specific command.
typedef struct LogEnDisUpdate_t
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
    uint32_t Reserved1[2];
} LogEnDisUpdate_t;
#endif

#ifdef SUPPORT_TELEMETRY
/// < @brief Specific command.
typedef struct Telemetry_t
{
    uint8_t subop; // 0-7
    uint8_t Reserved1[3];
    uint32_t Reserved2[5];
} Telemetry_t;
#endif

/// < @brief Specific command - Host(non-ucd) Command
typedef struct HostCpCmd_t
{
    //uint32_t Reserved1[2];      ///> DWORD 0~1
    ListHead_t Pointer;   ///> DWORD 0~1
    void (*HostCmdHandlerCallback)(HostCpCmd_t* pCmd, Error_t status);   ///> DWORD 2
    uint8_t Opcode;             ///> DWORD 3
    uint8_t Reserved2;
    uint16_t CmdId;
    union                       ///> DWORD 4~9
    {
        struct
        {
            uint8_t loglevel;
            uint8_t Reserved3[3];
            uint32_t Reserved4[5];
        } setLoglevel;          ///> cNvmeAdminSetLogLevel

        KeyUpdate_t keyUpdate; ///> cNvmeAdminKeyUpdate
        Identify_t identify;   ///> cNvmeAdminIdentify
        SetFeature_t setfeature; ///> cNvmeAdminSetFeatures
        FpModeChange_t fpModeChange; ///> cNvmeAdminFpModeChange
        #ifdef SUPPORT_MSGERROR_INJECTION
        MsgErrorInjection_t msgErrorInjection; ///> cNvmeAdminMsgErrorInjection
        #endif
        #ifdef LOGGING_NEW_SCHEME
        LogEnDisUpdate_t logEnDisUpdate; ///> cNvmeAdminLogEnDisUpdate
        #endif
        #ifdef SUPPORT_TELEMETRY
        Telemetry_t telemetry;
        #endif
    };
    uint32_t Resrved[6];      ///< DW10~15: 24 bytes
} HostCpCmd_t;
COMPILE_ASSERT(sizeof(HostCpCmd_t) == 64, "HostCpCmd size shall be 64");
//=================== LionMS Command ========================

// @brief Prp Descriptor by Spec.
typedef struct
{
    union
    {
        uint64_t Prp;
        struct
        {
            uint32_t Lo;
            uint32_t Hi;
        };
    };
} DataPtrPrp_t;

// @brief Lion Host NVMe Fastpath SQE Descriptor by Spec.
typedef struct
{
    uint32_t PASID;    ///< DW0: PASID
    union
    {
        uint32_t DW1;
        struct
        {
            union
            {
                uint16_t CmdTypeAttr; ///< Command Type + Command Attributes
                struct
                {
                    uint16_t EnDecrypt   : 1;  ///< 0: Encrypt   1: Decrypt
                    uint16_t reserved1   : 1;
                    uint16_t prpSgl      : 1;  ///< 0: PRP  1: SGL
                    uint16_t cmdType     : 3;  ///< 101b: fp cmd   000b: sp cmd  other: invalid
                    uint16_t reserved2   : 2;
                    uint16_t cipher      : 1;  ///< 0: GCM   1: XTS
                    uint16_t reserved3   : 5;
                    uint16_t dataUnitLen : 2;  ///< 00b total size, 01b: 512bytes, 10b: 4k, 11b: 8k
                };
            };
            uint16_t HostCid;
        };
    };
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    union
    {
        uint32_t DW2;
        struct
        {
            uint16_t sessionID;
            uint8_t appID;
            uint8_t reservedDW2;
        };
    };

    union
    {
        uint32_t DW3;

        #ifdef SUPPORT_CDMA_ERROR_INJECTION
        uint32_t NonFatalErrInjInfo1;   ///< Non Fatal error injected offset [31:8], Non Fatal inject error num [7:0]
        uint32_t SglErrInjInfo1;        ///< poorly SGL error injected offset [31:8], poorly SGL inject error num [7:0]
        #endif
    };

    union
    {
        uint32_t DW4;
        #ifdef SUPPORT_CDMA_ERROR_INJECTION
        uint32_t SglErrInjInfo2;   ///< poorly SGL error injected offset [31:8], poorly SGL inject error num [7:0]
        #endif

        struct
        {
            uint8_t PoorSGLRetryTimes : 2;   ///< Poor SGL Retry Times [1:0]
            uint8_t Reserved : 6;            ///< reserved [7:2]
            uint8_t Reserved2;
            uint16_t chunkSize;              ///< cdma chunk size
        };                                   ///< Reserved field is using for FP cdma retry error handling
    };
    #else
    union
    {
        uint32_t DW2;
        struct
        {
            uint8_t PoorSGLRetryTimes : 2;   ///< Poor SGL Retry Times [1:0]
            uint8_t Reserved : 6;            ///< reserved [7:2]
            uint8_t Reserved2;
            uint16_t chunkSize;              ///< cdma chunk size
        };                                   ///< Reserved field is using for FP cdma retry error handling
    };
    union
    {
        uint64_t AuxCmdPtr;           ///< DW3:DW4 Auxillary Cmd Pointer
        struct
        {
            uint32_t Lo;              ///< Auxillary Cmd Pointer [31:0]
            uint32_t Hi;              ///< Auxillary Cmd Pointer [63:32]
        };
        #ifdef SUPPORT_CDMA_ERROR_INJECTION
        struct
        {
            uint32_t NonFatalErrInjInfo1;   ///< Non Fatal error injected offset [31:8], Non Fatal inject error num [7:0]
            uint32_t Reserved3;
        };   ///< Reserved field is using for PcSim cdma error injection
        struct
        {
            uint32_t SglErrInjInfo1;   ///< poorly SGL error injected offset [31:8], poorly SGL inject error num [7:0]
            uint32_t SglErrInjInfo2;   ///< poorly SGL error injected offset [31:8], poorly SGL inject error num [7:0]
        };   ///< Reserved field is using for PcSim cdma error injection
        #endif
    };
    #endif
    union
    {
        uint32_t DW5;
        #ifdef SUPPORT_CDMA_ERROR_INJECTION
        struct
        {
            uint8_t NonFatalErrInjIdx1;   ///< content index of NonFatalErrInjInfo1
            uint8_t Reserved51[3];
        };   ///< Reserved field is using for PcSim cdma error injection
        struct
        {
            uint8_t SglErrInjIdx1;   ///< content index of SglErrInjInfo1
            uint8_t SglErrInjIdx2;   ///< content index of SglErrInjInfo2
            uint8_t Reserved52[2];
        };   ///< Reserved field is using for PcSim cdma error injection
        #endif
    };
    uint32_t SrcDataLen;              ///< DW6: Source Data Length
    union
    {
        DataPtrPrp_t SrcDataPtr[2]; ///<DW7 8 9 10: Source Data Pointer PRP1 & PRP2
        NVMeSglDesc_t SrcSglDesc;
    };

    uint32_t DstDataLen;              ///< DW11: Destination Data Length
    union
    {
        DataPtrPrp_t DstDataPtr[2];    ///<DW12 13 14 15: Source Data Point Pointer PRP1 & PRP2
        NVMeSglDesc_t DstSglDesc;
    };
} LionNvmeSQDescriptor_t;
static_assert(sizeof(LionNvmeSQDescriptor_t) == 64, \
              "LionMS Nvme SQ size shall be 64");

typedef struct
{
    uint32_t keySubIndex : 3;
    uint32_t resourceGroupID : 7;
    uint32_t reserved : 22;
} KeyIndexStruct_t;

/// @brief Lion Host Fastpath Auxillary AEX-XTS Command by Spec.
typedef struct
{
    union
    {
        uint32_t DW0;
        struct
        {
            uint32_t FrameId : 8;   /// < Frame ID
            uint32_t Reserved0 : 8;
            uint32_t Cipher : 8;   /// < Cipher
            uint32_t Reserved1 : 8;
        };
    };
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    KeyIndexStruct_t HostKeyIdx[2];     /// <DW1 2: AEX-XTS: Host Key1 2 Index[31:0]
    #else
    uint32_t HostKey1Idx;     /// <DW1: AEX-XTS: Host Key1 Index[31:0]
    uint32_t HostKey2Idx;     /// <DW2: AEX-XTS: Host Key2 Index[31:0]
    #endif
    uint32_t Tweak[4];        /// <DW3 4 5 6: AEX-XTS: Tweak[127:0]
    #ifdef SUPPORT_ERROR_INJECTION
    uint32_t Reserved3[4];    /// <DW7 8 9 10 : Reserved
    uint32_t Signature;       /// <DW11 : MSFT signature
    uint32_t ErrorType0 : 8;       /// <DW12: Error Type 1st inject
    uint32_t ErrorType1 : 8;       /// <DW12: Error Type 2nd inject
    uint32_t Reserved4 : 16;
    uint32_t Reserved5[3];    /// <DW13 14 15: Reserved
    #else
    uint32_t Reserved3[9];
    #endif
} AesXtsCmd_t;
/// @brief Lion Host Fastpath Auxillary AEX-GCM Command by Spec.
typedef struct
{
    union
    {
        uint32_t DW0;
        struct
        {
            uint32_t FrameId : 8;   /// < Frame ID
            uint32_t Rsvd0 : 8;
            uint32_t Cipher : 8;   /// < Cipher
            uint32_t Rsvd1 : 8;
        };
    };
    uint32_t HostKeyIdx;     /// <DW1: AES-GCM: Host Key Index
    uint32_t UnpaddedAADLen; /// <DW2: AES-GCM: Unpadded AAD length (Unaligned)
    uint32_t Tag[4];         /// <DW3 4 5 6: AES-GCM: Tag[127:0]
    uint32_t IV[3];          /// <DW7 8 9: AES-GCM: IV[96:0]
    uint32_t AADLen;         /// <DW10: AES-GCM: AAD Length in Src data

    union 
    {
        uint32_t DW11;
        struct
        {
            uint32_t UnalignedSrcDataLen : 8;   /// < Frame ID
            uint32_t UnalignedDstDataLen : 8;
            uint32_t Rsvd2 : 16;
        };
    };

    uint32_t UnalignedSrcDataPtr[2]; /// <DW12 13: AES-GCM: Unaligned Src data length

    uint32_t UnalignedDstDataPtr[2]; /// <DW12 13: AES-GCM: Unaligned Dest data length

} AesGcmCmd_t;
/// @brief Lion Host Fastpath Auxillary Command by Spec.
typedef struct
{
    union
    {
        AesXtsCmd_t AesXtsCmd;
        AesGcmCmd_t AesGcmCmd;
    };
} LionHostFpAuxCmd_t;
COMPILE_ASSERT(sizeof(LionHostFpAuxCmd_t) == 64, \
               "LionMS Host Aux command size shall be 64");
/// @brief Lion Host Nvme Fastpath CQE by Spec.
#ifdef LIONPERF_SUPPORT
// FIPS mandates the IV for encryption must be generated inside the FIPS boundary.
// Decryption IV must be passed from outside FIPS module. CQE is extended to pass IV back to the host.
// Marvell Internal Tool does not support does not support 16 DW CQE. Hence LIONPERF_SUPPORT
// flag is introduced to maintain compatibility with Marvell Internal Tool.
typedef struct
{
    union
    {
        uint32_t DW0;
        struct
        {
            uint32_t CmdTypeAttr : 16;   /// < Command Type + Command Attributes
            uint32_t CmdId : 16;         /// < Host Command ID
        };
    };
    /// < DW1 2 3 4: Output Crypto Context [127:0] (GCM-Output Tag)
    uint32_t CryptoCtx[4];
    uint32_t DataLen; /// < DW5: Output Data Length  ///< Use this as Admin Cmd specfic CQE DW0
    union
    {
        uint32_t DW6;
        struct
        {
            uint32_t SqHead : 16;        /// < Submission header index
            uint32_t SqId : 16;          /// < submission  Identifier
        };
    };
    union
    {
        uint32_t DW7;
        struct
        {
            uint32_t ErrCode : 16;          ///< Error Code
            uint32_t PhaseTag : 1;          ///< Phase Tag
            #ifndef SUPPORT_ERROR_INJECTION
            uint32_t StsCode : 15;          ///< Status Code
            #else
            uint32_t StsCode : 8;          ///< Status Code
            uint32_t ErrTypeNum : 6;       ///< Err type number
            uint32_t ErrInjectionTimes : 1;  ///< Err type inject times 0: 1st injection 1: 2nd injection
            #endif
        };
    };
} LionFPCQE_t;
COMPILE_ASSERT(sizeof(LionFPCQE_t) == 32, \
               "LionMS Host NVMe Fastpath CQE size shall be 32");
#else
typedef struct
{
    union
    {
        uint32_t DW0;
        struct
        {
            union
            {
                uint16_t CmdTypeAttr; ///< Command Type + Command Attributes
                struct
                {
                    uint16_t EnDecrypt   : 1;  ///< 0: Encrypt   1: Decrypt
                    uint16_t reserved1   : 1;
                    uint16_t prpSgl      : 1;  ///< 0: PRP  1: SGL
                    uint16_t cmdType     : 3;  ///< 101b: fp cmd   000b: sp cmd  other: invalid
                    uint16_t reserved2   : 2;
                    uint16_t cipher      : 1;  ///< 0: GCM   1: XTS
                    uint16_t reserved3   : 5;
                    uint16_t dataUnitLen : 2;  ///< 00b total size, 01b: 512bytes, 10b: 4k, 11b: 8k
                };
            };
            uint16_t CmdId;
        };
    };
    /// < DW1 2 3 4: Output Crypto Context [127:0] (GCM-Output Tag)
    union{
        uint32_t CryptoCtx[4];
        struct
        {
            uint32_t DW1;
            uint32_t DW2;
            uint32_t DW3;
            uint32_t DW4;
        };
    };

    union
    {
        uint32_t IV[3];                         ///< DW5, DW6, DW7
        struct
        {
            uint32_t DW5;
            uint32_t DW6;
            uint32_t DW7;
        };
    };
    union
    {
        uint32_t Reserved1[4];                ///< DW8, DW9, DW10, DW11
        struct
        {
            uint32_t DW8;
            uint32_t DW9;
            uint32_t DW10;
            uint32_t DW11;
        };
    };
    union
    {
        uint32_t DW12;
        uint32_t ServiceIndicator; /// < DW12: Service Indicator
    };

    union
    {
        uint32_t DW13;
        uint32_t DataLen; /// < DW13: Output Data Length  ///< Use this as Admin Cmd specfic CQE DW0
    };
    union
    {
        uint32_t DW14;
        struct
        {
            uint32_t SqHead : 16;        /// < Submission header index
            uint32_t SqId : 16;          /// < submission  Identifier
        };
    };
    union
    {
        uint32_t DW15;
        struct
        {
            uint32_t ErrCode : 16;          ///< Error Code
            uint32_t PhaseTag : 1;          ///< Phase Tag
            #ifndef SUPPORT_ERROR_INJECTION
            uint32_t StsCode : 15;          ///< Status Code
            #else
            uint32_t StsCode : 8;          ///< Status Code
            uint32_t ErrTypeNum : 6;       ///< Err type number
            uint32_t ErrInjectionTimes : 1;  ///< Err type inject times 0: 1st injection 1: 2nd injection
            #endif
        };
    };
} LionFPCQE_t;
COMPILE_ASSERT(sizeof(LionFPCQE_t) == 64, \
               "LionMS Host NVMe Fastpath CQE size shall be 64");
#endif
/// < @brief Lion Concatenated 128B FastPath Command + Meta Data by Spec.
typedef struct
{
    union
    {
        LionNvmeSQDescriptor_t sqe;
        #ifdef LIONPERF_SUPPORT
        LionFPCQE_t cqe;
        uint8_t Resrved1[8];
        #else
        LionFPCQE_t cqe;
        #endif
    };
    LionHostFpAuxCmd_t meta;
} LionFPCmdMetaData_t;

typedef struct _psd_t
{
    struct
    {
        uint32_t mp : 16;
        uint32_t rsv : 8;
        uint32_t mps : 1;
        uint32_t nops : 1;
        uint32_t rsv2 : 6;
    } dw0;

    struct
    {
        uint32_t enlat;
    } dw1;

    struct
    {
        uint32_t exlat;
    } dw2;

    struct
    {
        uint32_t rrt : 5;
        uint32_t rsv1 : 3;
        uint32_t rrl : 5;
        uint32_t rsv2 : 3;
        uint32_t rwt : 5;
        uint32_t rsv3 : 3;
        uint32_t rwl : 5;
        uint32_t rsv4 : 3;
    } dw3;
    uint32_t dw4;
    uint32_t dw5;
    uint32_t dw6;
    uint32_t dw7;
} psd_t;

#define IDENTIFY_DATA_SIZE 4096
typedef struct NVMeIdentifyCtrl_t
{
    //Controller Capabilities and Features
    uint16_t vid;
    uint16_t ssvid;
    uint8_t sn[20];
    uint8_t mn[40];
    uint8_t fr[8];
    uint8_t rab;
    uint8_t ieee[3];
    uint8_t cmic;
    uint8_t mdts;
    uint16_t cntlid;
    uint8_t ver[4];
    uint32_t rtd3r;
    uint32_t rtd3e;
    uint32_t oaes;
    #ifndef SUPPORT_NVMSET
    uint8_t rsv1[160];
    #else
    union
    {
        uint32_t ctratt_all;
        struct
        {
            uint32_t Supports128bitsHostID : 1;
            uint32_t SupportsNonOperationalPowerStatePermissiveMode : 1;
            uint32_t SupportsNVMSets : 1;
            uint32_t SupportsReadRecoveryLevels : 1;
            uint32_t SupportsEnduranceGroups : 1;
            uint32_t SupportsPredictableLatencyMode : 1;
            uint32_t SupportsTrafficBasedKeepAliveSupportTBKAS : 1;
            uint32_t SupportsNamespaceGranularity : 1;
            uint32_t SupportsSQAssociations : 1;
            uint32_t SupportsUUIDList : 1;
            uint32_t SupportsReserved : 22;
        } ctratt;
    };
    uint8_t rsv1[156];
    #endif
    union
    {
        uint16_t oacs_all;
        struct
        {
            uint16_t SupportsSecuritySendSecurityReceive : 1;
            uint16_t SupportsFormatNVM : 1;
            uint16_t SupportsFirmwareActivateFirmwareDownload : 1;
            uint16_t SupportsNamespaceMgmtAndAttachment : 1;
            uint16_t SupportsDeviceSelfTest : 1;
            uint16_t SupportsDirectives : 1;
            uint16_t SupportsNVMeMI : 1;
            uint16_t SupportsVirtualizationManagement : 1;
            uint16_t SupportsDoorbellBufferConfig : 1;
            uint16_t Reserved : 7;
        } oacs;
    };
    uint8_t acl;
    uint8_t aerl;
    uint8_t frmw;
    uint8_t lpa;
    uint8_t elpe;
    uint8_t npss;
    uint8_t avscc;
    uint8_t apsta; //Optional
    uint16_t wctemp;
    uint16_t cctemp;
    uint16_t mtfa;
    uint32_t hmpre;
    uint32_t hmmin;
    uint8_t tnvmcap[16];
    uint8_t unvmcap[16];
    uint32_t rpmbs;
    uint16_t edstt;
    uint8_t dsto;
    uint8_t fwug;
    uint16_t kas;
    uint16_t hctma;
    uint16_t mntmt;
    uint16_t mxtmt;
    uint32_t sanicap;
    #ifndef SUPPORT_NVMSET
    uint8_t rsv2[180];
    #else
    uint8_t rsv2_1[6];
    uint16_t nsetidmax;
    uint16_t endgidmax;
    uint8_t rsv2_2[170];
    #endif

    //NVM Command Set Attributes
    uint8_t sqes;
    uint8_t cqes;
    uint16_t maxcmd;
    uint32_t nn_v;
    uint16_t oncs;
    uint16_t fuses;
    uint8_t fna;
    uint8_t vwc;
    uint16_t awun;
    uint16_t awupf;
    uint8_t nvscc;
    uint8_t rsv4;
    uint16_t acwu;
    uint8_t rsv5[2];
    uint8_t sgls[4];
    uint8_t rsv6[228];
    uint8_t subnqn[256];
    //I/O Command Set Attributes
    uint8_t rsv7[1024];

    //Power State Descriptors
    psd_t psd0;
    psd_t psd1; //Optional
    psd_t psd2;
    psd_t psd3;
    psd_t psd4;
    psd_t psd5;
    psd_t psd6;
    psd_t psd7;
    psd_t psd8;
    psd_t psd9;
    psd_t psd10;
    psd_t psd11;
    psd_t psd12;
    psd_t psd13;
    psd_t psd14;
    psd_t psd15;
    psd_t psd16;
    psd_t psd17;
    psd_t psd18;
    psd_t psd19;
    psd_t psd20;
    psd_t psd21;
    psd_t psd22;
    psd_t psd23;
    psd_t psd24;
    psd_t psd25;
    psd_t psd26;
    psd_t psd27;
    psd_t psd28;
    psd_t psd29;
    psd_t psd30;
    psd_t psd31;

    //Vendor Specific
    uint8_t vs[1024];
} NVMeIdentifyCtrl_t;

/*-----------------------------------------------------------------------------
   //  Public Function Declarations
   -----------------------------------------------------------------------------*/

/**
 *  @brief Configure the one DW of HW IO SRAM per command index, DW index and mask
 *
 *  @param[in]  cmdIndex         Command Index
 *  @param[in]  dwIndex          DW index in selected HW IO SRAM entry per cmdIndex
 *  @param[in]  data             data to be written into the HW IO SRAM per mask
 *  @param[in]  mask             mask the data bits
 *  @return     error codes      If success, return cEcNoError
 *                               If fail, return cEcOutOfResources.
 */

#ifdef __cplusplus
}
#endif
#pragma pack(pop, enterHalHostLionMSFpCmdh)
