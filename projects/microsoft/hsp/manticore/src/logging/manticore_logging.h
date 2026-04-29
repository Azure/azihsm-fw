// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_LOGGING_H_
#define MANTICORE_LOGGING_H_

#include "logging/msft_debug_log.h"


/**
 * Component ID for Manticore-specific logging.
 */
#define	DEBUG_LOG_COMPONENT_MANTICORE		MSFT_LOGGING_COMPONENT_MANTICORE_SP


/**
 * Logging messages for Manticore.
 */
enum {
	MANTICORE_LOGGING_IPC_MESSAGE_HANDLER,								/**< Error during handling of IPC message. */
	MANTICORE_LOGGING_IPC_ADMIN_INTERFACE,								/**< Error during IPC Admin command interface processing. */
	MANTICORE_LOGGING_IPC_HSM_INTERFACE,								/**< Error during IPC HSM command interface processing. */
	MANTICORE_LOGGING_MFG_SECURITY_POLICY,								/**< Device was booted using the manufacturing security policy. */
	MANTICORE_LOGGING_COMPONENT_LOAD_ERROR,								/**< Error loading a firmware image component. */
	MANTICORE_LOGGING_CRASHDUMP_PAYLOAD_FW_VERSION,						/**< Crashdump payload FW version. */
	MANTICORE_LOGGING_CRASHDUMP_GET_CORE_STATUS_FAILURE,				/**< Failed to get ARM core status. */
	MANTICORE_LOGGING_CRASHDUMP_SET_CORE_STATUS_FAILURE,				/**< Failed to set ARM core status. */
	MANTICORE_LOGGING_CRASHDUMP_CLEAR_CORE_STATUS_FAILURE,				/**< Failed to clear ARM core status. */
	MANTICORE_LOGGING_CRASHDUMP_GET_ARM_CORE_CRASHDUMP_FAILURE,			/**< Failed to get crashdump from a ARM core. */
	MANTICORE_LOGGING_CRASHDUMP_NOT_ALL_ARM_CORE_CRASHDUMP_AVAIVABLE,	/**< Not all ARM cores collected their crashdump. */
	MANTICORE_LOGGING_CRASHDUMP_TRIGGER_ARM_CORE_INT_FAILURE,			/**< Failed to trigger ARM core interrupt. */
	MANTICORE_LOGGING_TELEMETRY_TEMPERATURE_RUNNING,					/**< Temperature monitor running normally. */
	MANTICORE_LOGGING_TELEMETRY_TEMPERATURE_FAULTED,					/**< Temperature monitor generated error. */
	MANTICORE_LOGGING_TELEMETRY_UNEXPECTED_TEMPERATURE,					/**< Unexpected system temperature */
	MANTICORE_LOGGING_TELEMETRY_PCIE_RUNNING,							/**< PCIe monitor running normally. */
	MANTICORE_LOGGING_TELEMETRY_PCIE_FAULTED,							/**< PCIe monitor generated error. */
	MANTICORE_LOGGING_TELEMETRY_UNEXPECTED_PCIE_LINK,					/**< Unexpected PCIe Link Speed/Width. */
	MANTICORE_LOGGING_CRASHDUMP_RESET_REACHED_MAX,						/**< Number of reset times reached to MAX threshold value. */
	MANTICORE_LOGGING_CRASHDUMP_RESET_FAILURE,							/**< Failed to reset after a crash dump was collected. */
	MANTICORE_LOGGING_CRASHDUMP_SAVE_CRASHDUMP_TO_DEBUG_LOG_FAILURE,	/**< Failed to save the crashdump from RAM to debug log. */
	MANTICORE_LOGGING_LOG_COLLECTOR_RUNNING,							/**< Logging collector running. */
	MANTICORE_LOGGING_LOG_COLLECTOR_FAULTED,							/**< Logging collector faulted. */
	MANTICORE_LOGGING_LOG_COLLECTOR_OVERFLOW_DETECTED,					/**< Overflow detected in the logging collector. */
	MANTICORE_LOGGING_ENABLE_CP_TCM_ECC,								/**< Wipe the CP TCMs and enable ECC. */
	MANTICORE_LOGGING_ENABLE_FP_ECC,									/**< Wipe the FP TCMs and PSRAM and enable ECC. */
	MANTICORE_LOGGING_ENABLE_GSRAM_ECC,									/**< Wipe the GSRAM and enable ECC. */
	MANTICORE_LOGGING_GSRAM_ECC_DISABLED,								/**< The GSRAM ECC has been disabled. */
	MANTICORE_LOGGING_FP_ECC_DISABLED_0,								/**< The ECC for FPS memories has been disabled (FP0/1). */
	MANTICORE_LOGGING_FP_ECC_DISABLED_1,								/**< The ECC for FPS memories has been disabled (FP2/PSRAM). */
	MANTICORE_LOGGING_HW_ERR_ENABLE_ACCESS_INT_FAILED,					/**< Failed to enable access error interrupt. */
	MANTICORE_LOGGING_HW_ERR_ENABLE_BUS_INT_FAILED,						/**< Failed to enable bus error interrupt. */
	MANTICORE_LOGGING_HW_ERR_ENABLE_CHK_INT_FAILED,						/**< Failed to enable HW check point error interrupt. */
	MANTICORE_LOGGING_HW_ERR_ENABLE_DMB_INT_FAILED,						/**< Failed to enable DMB error interrupt. */
	MANTICORE_LOGGING_HW_ERR_ENABLE_MEM_INT_FAILED,						/**< Failed to enable memory error interrupt. */
	MANTICORE_LOGGING_HW_ERR_ENABLE_MPU_INT_FAILED,						/**< Failed to enable MPU error interrupt. */
	MANTICORE_LOGGING_HW_ERR_ENABLE_RNG_INT_FAILED,						/**< Failed to enable RNG error interrupt. */
	MANTICORE_LOGGING_HW_ERR_ENABLE_WATCHDOG_TIMER_INT_FAILED,			/**< Failed to enable watchdog timer timeout interrupt. */
	MANTICORE_LOGGING_HW_ERR_ENABLE_WDT_INT_FAILED,						/**< Failed to enable WDT error interrupt. */
	MANTICORE_LOGGING_EXECUTE_ON_DEMAND_SELF_TESTS,						/**< Start execution of on-demand self-tests. */
	MANTICORE_LOGGING_HW_ERR_ENABLE_MBX_INT_FAILED,						/**< Failed to enable MBX error interrupt. */
	MANTICORE_LOGGING_BLOCK_UPDATE_LOAD_UNTIL_POR,						/**< Block loading a firmware update until a SoC reset. */
	MANTICORE_LOGGING_ALLOW_UPDATE_LOAD_DURING_RESET,					/**< Allow loading a firmware update during warm reset. */
	MANTICORE_LOGGING_IMPACTFUL_RECOVERY_BOOT,							/**< Boot using recovery flash due to a staged impactful update. */
	MANTICORE_LOGGING_FIPS_MODE,										/**< Indication of FIPS approved vs. non-approved execution. */
	MANTICORE_LOGGING_SOC_CRASHDUMP_COLLECTION_FAILED,					/**< Failed to collect SoC crash dumps. */
	MANTICORE_LOGGING_UNLOCK_INCOMPATIBLE,								/**< The running image does not support unlock. */
	MANTICORE_LOGGING_GSRAM_ECC_ERROR,									/**< GSRAM ECC error detected. */
	MANTICORE_LOGGING_ITCM_ECC_ERROR,									/**< ITCM ECC error detected. */
};

/**
 * Identifiers for a firmware image component.
 */
enum {
	MANTICORE_LOGGING_SP_FW_COMPONENT = 0,	/**< A component of the SP firmware image. */
	MANTICORE_LOGGING_CP_FW_COMPONENT = 1,	/**< A component of the CP firmware image. */
	MANTICORE_LOGGING_FP0_FW_COMPONENT = 2,	/**< A component of the FP0 firmware image. */
	MANTICORE_LOGGING_FP1_FW_COMPONENT = 3,	/**< A component of the FP1 firmware image. */
	MANTICORE_LOGGING_FP2_FW_COMPONENT = 4,	/**< A component of the FP2 firmware image. */
	MANTICORE_LOGGING_PHY_FW_COMPONENT = 5,	/**< A component of the PCIe PHY firmware image. */
};


#endif	/* MANTICORE_LOGGING_H_ */
