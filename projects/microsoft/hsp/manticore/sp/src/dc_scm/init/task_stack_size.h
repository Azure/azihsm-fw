// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TASK_STACK_SIZE_H_
#define TASK_STACK_SIZE_H_

#include "FreeRTOS.h"


/****************************
 * Task stack allocation
 *
 * TODO:  Confirm stack requirements on this platform.
 ****************************/

/**
 * Stack size for the I2C command handler task.
 */
#define	SYSTEM_CMD_TASK_STACK_WORDS				(6 * 256)

/**
 * Stack size for the background command handler task.
 */
#define	CMD_BACKGROUND_TASK_STACK_WORDS			(6 * 256)

/**
 * Stack size for the firmware update handler task.
 */
#define	FW_UPDATE_TASK_STACK_WORDS				(6 * 256)

/**
 * Stack size for the manifest command handler task.
 */
#define	MANIFEST_CMD_TASK_STACK_WORDS			(6 * 256)

/**
 * Stack size for the IPC handler task for the Admin core.
 */
#define	ADMIN_TO_HSP_TASK_STACK_WORDS			(8 * 256)

/**
 * Stack size for the IPC handler task for the HSM core.
 */
#define	HSM_TO_HSP_TASK_STACK_WORDS				(8 * 256)

/**
 * Stack size for the state persistence task.
 */
#define	PERSISTENCE_TASK_STACK_WORDS			((1 * 256) + 128)

/**
 * Stack size for the RTC intrusion interrupt handler task.
 */
#define	RTC_IRQ_TASK_STACK_WORDS				(1 * 256)

/**
 * Stack size for the SPI filter interrupt handler task.
 *
 * TODO:  Confirm stack requirements.  The task doesn't do much, but generates logs.  Worst case
 * may be when the log memory needs to get flushed in real-time rather than in the background.
 */
#define	FILTER_IRQ_TASK_STACK_WORDS				(1 * 256)

/**
 * Stack size for the SP watchdog task.
 */
#define	WATCHDOG_TASK_STACK_WORDS				(2 * 256)

/**
 * Stack size for the FIPS error state task.
 */
#define	ERROR_STATE_TASK_STACK_WORDS			(1 * 256)

/**
 * Stack size for the host Port 1 handler task.
 */
#define	HOST_PORT1_TASK_STACK_WORDS				(6 * 256)

/**
 * Stack size for the Ephemeral key manager task
 */
#define	EPHEMERAL_KEY_MANAGER_TASK_STACK_WORDS	(4 * 256)


#endif	/* TASK_STACK_SIZE_H_ */
