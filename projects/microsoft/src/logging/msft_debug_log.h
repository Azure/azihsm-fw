// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MSFT_DEBUG_LOG_H_
#define MSFT_DEBUG_LOG_H_

#include "logging/debug_log.h"


/**
 * IDs for MSFT components that generate log entries.
 */
enum {
	MSFT_LOGGING_COMPONENT_HSP_ROM = 0xf0,		/**< Log entry for HSP ROM, or ROM in general. */
	MSFT_LOGGING_COMPONENT_HSP,					/**< Log entry for HSP firmware messages. */
	MSFT_LOGGING_COMPONENT_MVDP,				/**< Log entry for MVDP message handling. */
	MSFT_LOGGING_COMPONENT_MANTICORE_SP,		/**< Log entry for Manticore firmware messages. */
	MSFT_LOGGING_COMPONENT_MANTICORE_CP0,		/**< Log entry for Manticore CP0 firmware messages. */
	MSFT_LOGGING_COMPONENT_MANTICORE_CP1,		/**< Log entry for Manticore CP1 firmware messages. */
	MSFT_LOGGING_COMPONENT_MANTICORE_FP0,		/**< Log entry for Manticore FP0 firmware messages. */
	MSFT_LOGGING_COMPONENT_MANTICORE_FP1,		/**< Log entry for Manticore FP1 firmware messages. */
	MSFT_LOGGING_COMPONENT_MANTICORE_FP2,		/**< Log entry for Manticore FP2 firmware messages. */
	MSFT_LOGGING_COMPONENT_OVERLAKE_CMC = 0xfd,	/**< Log entry for CMC firmware messages. */
	MSFT_LOGGING_COMPONENT_OVERLAKE = 0xfe,		/**< Log entry for Overlake firmware messages. */
	MSFT_LOGGING_COMPONENT_LPC,					/**< Log entry for NXP LPC firmware messages. */
};


#endif	/* MSFT_DEBUG_LOG_H_ */
