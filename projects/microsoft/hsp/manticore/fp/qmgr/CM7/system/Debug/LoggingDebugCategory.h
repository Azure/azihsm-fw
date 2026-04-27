// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2026 Marvell

//=============================================================================
//
//! @file  LoggingDebugCategory.h
//! @brief Definition of log level, log category, etc.
//!
//=============================================================================

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------------
//  Public data structure definitions and defines
//-----------------------------------------------------------------------------

/**
 *  Available Log Levels
 */
typedef enum LogLevel_t
{
    cLogError             = 0x0,                ///< An error condition occurred.
    cLogWarning           = 0x1,                ///< Warning messages - not an error, but indication that an error may occur if action is not taken.
    cLogInfo              = 0x2,                ///< Normal operational messages - may be harvested for reporting, measuring throughput, etc.
    cLogDebug             = 0x3,                ///< Info useful to developers for debugging the app, not useful during operations.
} LogLevel_t;

/**
 *  Available Log Categories
 */
typedef enum LogCategory_t
{
    /// CPU0: 0x0
    cLogCPU0Common,
    /// CPU1: 0x1
    cLogCPU1Common,
    /// CPU2: 0x2
    cLogCPU2Common,

} LogCategory_t;

#define TokenLogLevelBitmask        0xC000      ///< [15:14]
#define GetTokenLogLevel(token)     (((token) & TokenLogLevelBitmask) >> 14)


//-----------------------------------------------------------------------------
//  Definition from MSFT Cerberus open source
//-----------------------------------------------------------------------------

/**
 * IDs for components that generate log entries.
 */
enum DebugLogComponent
{
    DEBUG_LOG_COMPONENT_INIT = 0,                   /**< Log entry for initialization */
    DEBUG_LOG_COMPONENT_CMD_INTERFACE,              /**< Log entry for command interface */
    DEBUG_LOG_COMPONENT_CRYPTO,                     /**< Log entry for crypto */
    DEBUG_LOG_COMPONENT_HOST_FW,                    /**< Log entry for host firmware management */
    DEBUG_LOG_COMPONENT_CERBERUS_FW,                /**< Log entry for Cerberus firmware images */
    DEBUG_LOG_COMPONENT_STATE_MGR,                  /**< Log entry for state management */
    DEBUG_LOG_COMPONENT_MANIFEST,                   /**< Log entry for manifests */
    DEBUG_LOG_COMPONENT_SPI_FILTER,                 /**< Log entry for the SPI filter */
    DEBUG_LOG_COMPONENT_I2C,                        /**< Log entry for I2C failures */
    DEBUG_LOG_COMPONENT_BOOT,                       /**< Log entry for the bootloader */
    DEBUG_LOG_COMPONENT_FLASH,                      /**< Log entry for flash. */
    DEBUG_LOG_COMPONENT_SPI,                        /**< Log entry for SPI failures */
    DEBUG_LOG_COMPONENT_RECOVERY_IMAGE,             /**< Log entry for recovery images */
    DEBUG_LOG_COMPONENT_MCTP,                       /**< Log entry for MCTP stack */
    DEBUG_LOG_COMPONENT_TPM,                        /**< Log entry for TPM */
    DEBUG_LOG_COMPONENT_RIOT,                       /**< Log entry for RIoT */
    DEBUG_LOG_COMPONENT_SYSTEM,                     /**< Log entry for system management. */
    DEBUG_LOG_COMPONENT_INTRUSION,                  /**< Log entry for chassis intrusion. */

    DEBUG_LOG_COMPONENT_DEVICE_SPECIFIC = 0xf0,     /**< Base component ID for device-specific messages. */
    DEBUG_LOG_COMPONENT_QUEUE,                      /**< Log entry for queue. */            /* Added by MRVL */
    DEBUG_LOG_COMPONENT_KEY,                        /**< Log entry for key management. */   /* Added by MRVL */
    DEBUG_LOG_COMPONENT_CDMA,                       /**< Log entry for CDMA. */             /* Added by MRVL */
    DEBUG_LOG_COMPONENT_UCD,                        /**< Log entry for UCD. */              /* Added by MRVL */
    DEBUG_LOG_COMPONENT_IPC,                        /**< Log entry for IPC. */              /* Added by MRVL */
    DEBUG_LOG_COMPONENT_IO_ASSERT,                  /**< Log entry for IO_ASSERT. */        /* Added by MRVL */
    DEBUG_LOG_COMPONENT_MSG,                        /**< Log entry for CP FP Message. */    /* Added by MRVL */

    /* Component IDs 0xf0 - 0xff are reserved for device-specific logging. */
};

/**
 * The current format identifier for debug log entries.
 */
#define    DEBUG_LOG_ENTRY_FORMAT        1

/**
 * Severity levels for log entries.
 */
enum DebugLogSeverity
{
    DEBUG_LOG_SEVERITY_ERROR = 0,                /**< Log entry documenting an error. */
    DEBUG_LOG_SEVERITY_WARNING,                    /**< Log entry documenting a warning. */
    DEBUG_LOG_SEVERITY_INFO,                    /**< Log entry providing information. */
    DEBUG_LOG_NUM_SEVERITY                        /**< Number of valid severity levels. */
};

/**
 * Format for an entry in the debug log.
 */
#pragma pack(push)
#pragma pack(1)
struct DebugLogEntryInfo_t
{
    uint16_t Format;            /**< Format of the log entry. */
    uint8_t Severity;            /**< Severity level of the entry. */
    uint8_t Component;            /**< System competent that generated the entry. */
    uint8_t MsgIndex;            /**< Identifier for the entry message. */
    uint32_t Arg1;                /**< Message specific argument. */
    uint32_t Arg2;                /**< Message specific argument. */
    uint64_t Time;                /**< Elapsed time in milliseconds since boot. */
};
#pragma pack(pop)

/**
 * Logging messages for queue and command.
 */
enum
{
    QUEUE_LOGGING_DELETED,                /**< Command Aborted due to SQ Deletion. */
    QUEUE_LOGGING_CMD_ABORT,            /**< Command Aborted by host. */
    QUEUE_LOGGING_QUEUE_MSG_FULL,        /**< The message queue is full. */
};

/**
 * Logging messages for CP FP Message.
 */
enum
{
    MSG_LOGGING_MSG_ERR,                /**< Error occurs in CP FP message */
    FW_UPDATE_DATA_LENGTH_MISMATCH,     /**< FW update data length mismatch*/
};

/**
 * Logging messages for key management.
 */
enum
{
    KEY_LOGGING_INVAILD_KEY,                /**< Invalid key index. */
    KEY_LOGGING_INVAILD_KEY_SUB_ID,         /**< Invalid key sub-index. */
    KEY_LOGGING_INVAILD_RG_ID,              /**< Invalid resource group ID. */
    KEY_LOGGING_INVAILD_GCM_TAG,            /**< Invalid GCM tag. */
    KEY_LOGGING_KEY_VAULT_ID_OUT_OF_RANGE,    /**< Key vault index is out of range. */
};

/**
 * Logging messages for CDMA.
 */
enum
{
    CDMA_LOGGING_FATAL_ERR_CAUSED_RETRY,                /**< Retry due to fatal error occurs. */
    CDMA_LOGGING_NON_FATAL_ERR_CAUSED_RETRY,            /**< Retry due to non-fatal error occurs. */
    CDMA_LOGGING_POOR_SGL_CAUSED_RETRY,                 /**< Retry due to poor SGL occurs. */
    CDMA_LOGGING_FATAL_ERR,                             /**< Fatal error. */
    CDMA_LOGGING_NON_FATAL_ERR,                         /**< Non-fatal error. */
    CDMA_LOGGING_POOR_SGL,                              /**< Poor SGL. */
    CDMA_LOGGING_REPORT_HOST,                           /**< Report host. */
    CDMA_LOGGING_UPDATE_RETRY_TIME_UNEXPECTED_STATUS,   /**< Unexpected CE tiny status in update retry times handler. */
    CDMA_LOGGING_KEY_VAULT_MEMORY_CORRECTABLE_ERR,      /**< CDMA key vault memory correctable error */
    CDMA_LOGGING_KEY_VAULT_MEMORY_UNCORRECTABLE_ERR,    /**< CDMA key vault memory uncorrectable error */
    CDMA_LOGGING_QOS_LATENCY_TO_ERR,                    /**< CDMA QoS latency timeout error */
};

/**
 * Logging messages for UCD.
 */
enum
{
    UCD_LOGGING_IB_ERROR,        /**< Inbound error. */
    UCD_LOGGING_OB_ERROR,        /**< Outbound error. */
};

/**
 * Logging messages for IPC.
 */
enum
{
    IPC_LOGGING_INVALID_IRQ,        /**< Unexpected IRQ is asserted. */
};

/**
 * Logging messages for INIT.
 */
enum
{
    INIT_LOGGING_HANDSHAKE_NOT_COMPLETED,    /**< Initialization handshake is not completed yet. */
};

/**
 * Logging messages for IO ASSERT
 */
enum
{
    IO_ASSERT_LOGGING_CE_TINY_NOT_IN_OB,                        /**< CE tiny is not cCETinyStsInOutbound. */
    IO_ASSERT_LOGGING_CDMA_ERROR_HANDLE_UNEXPECTED_RETURN,      /**< Unexpected return value in CDMA error command handler. */
    IO_ASSERT_LOGGING_CDMA_IDLE_CMD_ERROR,                      /**< CDMA idle command CQE status error. */
    IO_ASSERT_LOGGING_CDMA_NON_IDLE_CMD_IN_IDLE_DFL,            /**< CDMA non-idle command in idle DFL list. */
    IO_ASSERT_LOGGING_CDMA_INVALID_DFL,                         /**< CDMA invalid DFL list. */
    IO_ASSERT_LOGGING_CDMA_ERROR_HANDLE_UNEXPECTED_STATUS,              /**< Unexpected CE tiny status in CDMA error command handler. */
    IO_ASSERT_LOGGING_CDMA_ABORT_HANDLE_UNEXPECTED_STATUS,              /**< Unexpected CE status in CDMA abort command handler. */
    IO_ASSERT_LOGGING_CDMA_ZERO_TRANSFER_HANDLE_UNEXPECTED_STATUS,      /**< Unexpected CE status in zero transfer handler. */
    IO_ASSERT_LOGGING_CDMA_ABORT_UPDATE_RETRY_HANDLE_UNEXPECTED_STATUS, /**< Unexpected CE tiny status in abort command update CE retry handler. */
    IO_ASSERT_LOGGING_NO_AVAILABLE_CE,                          /**< There is no available CE */
};

/**
 * Logging messages for host firmware management.
 */
enum
{
    HOST_LOGGING_NVME_CQE_ERROR_STATUS,     /**< Error status in NVME CQE*/
};

#ifdef __cplusplus
}
#endif
