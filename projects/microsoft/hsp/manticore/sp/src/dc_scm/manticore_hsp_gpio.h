// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_HSP_GPIO_H_
#define MANTICORE_HSP_GPIO_H_


/**
 * Function assigned to each of the HSP GPIOs.  The value represents the GPIO number.
 */
enum {
	PORT0_RESET_IRQ = 0,			/**< Currently unused. */
	PORT0_RESET_CTRL = 1,			/**< Currently unused. */
	PORT0_CS0 = 2,					/**< Currently unused. */
	PORT0_CS1 = 3,					/**< Currently unused. */
	PORT1_RESET_IRQ = 4,			/**< Reset indication from the host on port 1. */
	PORT1_AUTH_IRQ = 5,				/**< Reset with authentication indication from the host on port 1. */
	PORT1_RESET_CTRL = 6,			/**< Reset control for the host on port 1. */
	PORT0_SPI_FILTER_MUX = 7,		/**< Currently unused. */
	PORT1_SPI_FILTER_MUX = 8,		/**< SPI mux control for the port 1 host flash. */
	IC20_ALERT = 9,					/**< Currently unused. */
	HEARTBEAT_LED = 10,				/**< Control for the Manticore heartbeat LED. */
	UNALLOCATED = 11,				/**< Spare GPIO. */
	MANTICORE_HSP_GPIO_COUNT = 12,	/**< Total number of HSP GPIOs in Manticore. */
};


#endif	/* MANTICORE_HSP_GPIO_H_ */
