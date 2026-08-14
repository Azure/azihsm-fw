// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TASK_LOG_ID_H_
#define TASK_LOG_ID_H_


/**
 * Defines the list of log IDs to assign to different periodic tasks executing in the system.
 */
enum {
	SYSTEM_CMD_TASK_LOG_ID = 0,				/**< ID for the main command handler task. */
	ATTESTATION_REQ_TASK_LOG_ID = 1,		/**< ID for the attestation requester task. */
	PERSIST_DATA_TASK_LOG_ID = 2,			/**< ID for the task that flushes data to NV storage. */
	SPI_FILTER0_IRQ_TASK_LOG_ID = 3,		/**< ID for the port 0 SPI filter IRQ handler task. */
	SPI_FILTER1_IRQ_TASK_LOG_ID = 4,		/**< ID for the port 1 SPI filter IRQ handler task. */
	RTC_IRQ_TASK_LOG_ID = 5,				/**< ID for the RTC IRQ handler task. */
	IPC_ADMIN_TO_HSP_TASK_LOG_ID = 6,		/**< ID for Admin to HSP IPC task. */
	IPC_HSM_TO_HSP_TASK_LOG_ID = 7,			/**< ID for HSM to HSP IPC task. */
	EPHEMERAL_KEY_GEN_TASK_LOG_ID = 8,		/**< ID for the Ephemeral key generation task. */
	WATCHDOG_TASK_LOG_ID = 9,				/**< ID for the watchdog task. */
	ERROR_STATE_TASK_LOG_ID = 10,			/**< ID for the FIPS error state task. */
	EPHEMERAL_KEY_MONITOR_TASK_LOG_ID = 11,	/**< ID for the ephemeral key monitor task. */
};


#endif	/* TASK_LOG_ID_H_ */
