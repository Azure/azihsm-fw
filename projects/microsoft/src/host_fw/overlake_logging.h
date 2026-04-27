// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVERLAKE_LOGGING_H_
#define OVERLAKE_LOGGING_H_

#include "logging/msft_debug_log.h"


/**
 * Component ID for Overlake logging.
 */
#define	DEBUG_LOG_COMPONENT_OVERLAKE		MSFT_LOGGING_COMPONENT_OVERLAKE

/**
 * Logging messages for Overlake.
 */
enum {
	OVERLAKE_LOGGING_ERASE_SHMOO,							/**< Erasing the shmoo data as part of SoC update. */
	OVERLAKE_LOGGING_PERIODIC_RESET_TIMER_FAILURE,			/**< Periodic reset timer creation failure */
	OVERLAKE_LOGGING_PERIODIC_RESET_FORCE_RESET,			/**< Periodic reset component issued unconditional reset */
	OVERLAKE_LOGGING_PERIODIC_RESET_GRACEFUL_RESET,			/**< Periodic reset component issued graceful reset */
	OVERLAKE_LOGGING_PERIODIC_RESET_FORCE_FLASH_OWNERSHIP,	/**< Periodic reset force flash ownership before reset */
	OVERLAKE_LOGGING_PERIODIC_RESET_FLASH_POSTPONE,			/**< Periodic reset was postponed because flash is owned by SOC */
	OVERLAKE_LOGGING_PERIODIC_RESET_UPDATE_POSTPONE,		/**< Periodic reset was postponed because there is an active update */
	OVERLAKE_LOGGING_PERIODIC_RESET_INITIALIZED,			/**< Periodic reset was successfully initialized */
};


#endif	/* OVERLAKE_LOGGING_H_ */
