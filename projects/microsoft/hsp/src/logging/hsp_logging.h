// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_LOGGING_H_
#define HSP_LOGGING_H_

#include "logging/msft_debug_log.h"


/**
 * The component ID for general HSP logging.
 */
#define	DEBUG_LOG_COMPONENT_HSP			MSFT_LOGGING_COMPONENT_HSP


/**
 * Logging messages for HSP.
 */
enum {
	HSP_LOGGING_INVALID_OWNERSHIP_TRANSFER,		/**< Validation of an ownership transfer manifest failed. */
	HSP_LOGGING_HW_FATAL_ERROR,					/**< A HW fatal error was detected. */
	HSP_LOGGING_SVN_REDUNDANCY_DEGRADED,		/**< At least one copy of the fused SVN is known to be wrong. */
	HSP_LOGGING_RTC_COOLDOWN_INIT_ERROR,		/**< Failed to initialize the RTC set_time cooldown timer. */
	HSP_LOGGING_MBOX_PROT_READ_FAILED_FLUSH,	/**< Failed internal flush call during mailbox protocol read. */
	HSP_LOGGING_EXT_POWER_DOWN_FAILED,			/**< Failed Ext Power Down Operation */
	HSP_LOGGING_I2C_SHUTDOWN_FAILED,			/**< Failed to shutdown I2C hardware block. */
	HSP_LOGGING_I2C_RESUME_FAILED,				/**< Failed to resume I2C operation after a failed device reset. */
	HSP_LOGGING_CRASHDUMP_FW_VERSION,			/**< Crashdump FW version. */
	HSP_LOGGING_HW_ERROR_ACCESS,				/**< HW access error. */
	HSP_LOGGING_HW_ERROR_BUS,					/**< HW bus error. */
	HSP_LOGGING_HW_ERROR_MEM,					/**< HW memory error. */
	HSP_LOGGING_HW_ERROR_MPU,					/**< HW MPU error. */
	HSP_LOGGING_HW_ERROR_CHK,					/**< HW check point error. */
	HSP_LOGGING_HW_ERROR_DMB,					/**< HW DMB error. */
	HSP_LOGGING_HW_ERROR_RNG,					/**< HW RNG error. */
	HSP_LOGGING_HW_ERROR_WATCHDOG_TIMEOUT,		/**< HW error watchdog timeout. */
	HSP_LOGGING_HW_ERROR_WDT,					/**< HW AXI WDT error. */
	HSP_LOGGING_I2C_TX_ABORT,					/**< An I2C transaction was aborted by the HW. */
};

/**
 * Identifiers for fatal error log registers.
 */
enum {
	HSP_LOGGING_HSP_FATAL_ERR_LOG = 0x00,			/**< Log entry for HSP_FATAL_ERR_LOG contents. */
	HSP_LOGGING_HSP_FATAL_CRYPTO_ERR_LOG = 0x01,	/**< Log entry for HSP_FATAL_CRYPTO_ERR_LOG contents. */
	HSP_LOGGING_HSP_FATAL_MEM_ERR_LOG = 0x02,		/**< Log entry for HSP_FATAL_MEM_ERR_LOG contents. */
	HSP_LOGGING_HSP_FATAL_HWCHKPT_ERR_LOG = 0x03,	/**< Log entry for HSP_FATAL_HWCHKPT_ERR_LOG contents. */
	HSP_LOGGING_HSP_FATAL_SP_BUS_ERR_LOG = 0x04,	/**< Log entry for HSP_FATAL_SP_BUS_ERR_LOG contents. */
};

/**
 * Tag identifiers for HW error access.
 */
enum {
	HSP_LOGGING_HW_ERROR_ACCESS_INTSTS_TAG = 0x00,			/**< ACCESS error interrupt status tag. */
	HSP_LOGGING_HW_ERROR_ACCESS_ACC_VIO_LOG0_TAG = 0x01,	/**< ACCESS error ACC_VIO_LOG0 tag. */
	HSP_LOGGING_HW_ERROR_ACCESS_ACC_VIO_LOG1_TAG = 0x02,	/**< ACCESS error ACC_VIO_LOG1 tag. */
};

/**
 * Tag identifiers for HW error bus.
 */
enum {
	HSP_LOGGING_HW_ERROR_BUS_INTSTS_TAG = 0x00,				/**< Bus error interrupt status tag. */
	HSP_LOGGING_HW_ERROR_BUS_AXI_RD_ERR_LOG0_TAG = 0x01,	/**< Bus error AXI_RD_ERR_LOG0 tag. */
	HSP_LOGGING_HW_ERROR_BUS_AXI_RD_ERR_LOG1_TAG = 0x02,	/**< Bus error AXI_RD_ERR_LOG1 tag. */
	HSP_LOGGING_HW_ERROR_BUS_AXI_WR_ERR_LOG0_TAG = 0x03,	/**< Bus error AXI_WR_ERR_LOG0 tag. */
	HSP_LOGGING_HW_ERROR_BUS_AXI_WR_ERR_LOG1_TAG = 0x04,	/**< Bus error AXI_WR_ERR_LOG1 tag. */
	HSP_LOGGING_HW_ERROR_BUS_ACC_VIO_0_LOG0_TAG = 0x05,		/**< Bus error ACC_VIO_0_LOG0 tag. */
	HSP_LOGGING_HW_ERROR_BUS_ACC_VIO_0_LOG1_TAG = 0x06,		/**< Bus error ACC_VIO_0_LOG1 tag. */
	HSP_LOGGING_HW_ERROR_BUS_ACC_VIO_1_LOG0_TAG = 0x07,		/**< Bus error ACC_VIO_1_LOG0 tag. */
	HSP_LOGGING_HW_ERROR_BUS_ACC_VIO_1_LOG1_TAG = 0x08,		/**< Bus error ACC_VIO_1_LOG1 tag. */
};

/**
 * Tag identifiers for HW error check point.
 */
enum {
	HSP_LOGGING_HW_ERROR_CHK_INTSTS_TAG = 0x00,	/**< Check point interrupt status tag. */
};

/**
 * Tag identifiers for HW error DMB.
 */
enum {
	HSP_LOGGING_HW_ERROR_DMB_INTSTS_TAG = 0x00,		/**< DMB error interrupt status tag. */
	HSP_LOGGING_HW_ERROR_DMB_ERRLOG1_TAG = 0x01,	/**< DMB error ERRLOG1 tag. */
	HSP_LOGGING_HW_ERROR_DMB_ERRLOG2_TAG = 0x02,	/**< DMB error ERRLOG2 tag. */
};

/**
 * Tag identifiers for HW error memory.
 */
enum {
	HSP_LOGGING_HW_ERROR_MEM_INTSTS_TAG = 0x00,		/**< Memory error interrupt status tag. */
	HSP_LOGGING_HW_ERROR_MEM_PKAR1_TAG = 0x01,		/**< Memory error PKAR1 tag. */
	HSP_LOGGING_HW_ERROR_MEM_PKAR2_TAG = 0x02,		/**< Memory error PKAR2 tag. */
	HSP_LOGGING_HW_ERROR_MEM_KEYSTR_TAG = 0x03,		/**< Memory error KEYSTR tag. */
	HSP_LOGGING_HW_ERROR_MEM_SHAREDRAM_TAG = 0x04,	/**< Memory error SHAREDRAM tag. */
	HSP_LOGGING_HW_ERROR_MEM_SPDRAM_TAG = 0x05,		/**< Memory error SPDRAM tag. */
	HSP_LOGGING_HW_ERROR_MEM_SPIRAM_TAG = 0x06,		/**< Memory error SPIRAM tag. */
	HSP_LOGGING_HW_ERROR_MEM_SPROM_TAG = 0x07,		/**< Memory error SPROM tag. */
};

/**
 * Tag identifiers for HW error MPU.
 */
enum {
	HSP_LOGGING_HW_ERROR_MPU_INTSTS_TAG = 0x00,				/**< MPU error interrupt status tag. */
	HSP_LOGGING_HW_ERROR_MPU_SPDRAM_MPU_STATUS_TAG = 0x01,	/**< MPU error SPDRAM_MPU_STATUS tag. */
	HSP_LOGGING_HW_ERROR_MPU_SPIRAM_MPU_STATUS_TAG = 0x02,	/**< MPU error SPIRAM_MPU_STATUS tag. */
	HSP_LOGGING_HW_ERROR_MPU_SPROM_MPU_STATUS_TAG = 0x03,	/**< MPU error SPROM_MPU_STATUS tag. */
};

/**
 * Tag identifiers for HW error RNG.
 */
enum {
	HSP_LOGGING_HW_ERROR_RNG_INTSTS_TAG = 0x00,		/**< RNG interrupt status tag. */
	HSP_LOGGING_HW_ERROR_RNG_STATUS_LOG_TAG = 0x01,	/**< RNG status log tag. */
};

/**
 * Tag identifiers for HW error watchdog timeout.
 */
enum {
	HSP_LOGGING_HW_ERROR_WATCHDOG_TIMEOUT_INTSTS_TAG = 0x00,	/**< Watchdog timeout interrupt status tag. */
};

/**
 * Tag identifiers for HW error WDT.
 */
enum {
	HSP_LOGGING_HW_ERROR_WDT_INTSTS_TAG = 0x00,			/**< AXI WDT error interrupt status tag. */
	HSP_LOGGING_HW_ERROR_WDT_LOG_DLOCK_1_TAG = 0x01,	/**< AXI WDT error LOG_DLOCK_1 tag. */
	HSP_LOGGING_HW_ERROR_WDT_LOG_DBG_W_1_TAG = 0x02,	/**< AXI WDT error LOG_DBG_W_1 tag. */
	HSP_LOGGING_HW_ERROR_WDT_LOG_DBG_R_1_TAG = 0x03,	/**< AXI WDT error LOG_DBG_R_1 tag. */
	HSP_LOGGING_HW_ERROR_WDT_LOG_SP_W_TAG = 0x04,		/**< AXI WDT error LOG_SP_W tag. */
	HSP_LOGGING_HW_ERROR_WDT_LOG_SP_R_TAG = 0x05,		/**< AXI WDT error LOG_SP_R tag. */
};


#endif	/* HSP_LOGGING_H_ */
