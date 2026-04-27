// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_STICKY_REGS_H_
#define MANTICORE_STICKY_REGS_H_

#include "hsp_top.h"


/**
 * Sticky register indicies for various functions.
 */
enum {
	MANTICORE_1SP_BOOT_STATUS = 31,		/**< Manticore boot status for 1SP. */
	MANTICORE_SPRT_RESET_COUNTER = 30,	/**< Cerberus reset counter for the SP. */
	MANTICORE_HOST_RESET_COUNTER = 29,	/**< Cerberus reset counter for external hosts. */
	MANTICORE_PFM_VALID_PORT1 = 28,		/**< Status value for Port 1 PFM authentication. */
	MANTICORE_SHUTDOWN_INDICATOR = 27,	/**< Indicator of a graceful shutdown. */
	MANTICORE_SPRT_BOOT_STATUS = 26,	/**< Manticore boot error and reset status. */
	MANTICORE_CRASHDUMP_18 = 25,		/**< Crash dump. */
	MANTICORE_CRASHDUMP_17 = 24,		/**< Crash dump. */
	MANTICORE_CRASHDUMP_16 = 23,		/**< Crash dump. */
	MANTICORE_CRASHDUMP_15 = 22,		/**< Crash dump. */
	MANTICORE_CRASHDUMP_14 = 21,		/**< Crash dump. */
	MANTICORE_CRASHDUMP_13 = 20,		/**< Crash dump. */
	MANTICORE_CRASHDUMP_12 = 19,		/**< Crash dump. */
	MANTICORE_CRASHDUMP_11 = 18,		/**< Crash dump. */
	MANTICORE_CRASHDUMP_10 = 17,		/**< Crash dump. */
	MANTICORE_CRASHDUMP_9 = 16,			/**< Crash dump. */
	MANTICORE_CRASHDUMP_8 = 15,			/**< Crash dump. */
	MANTICORE_CRASHDUMP_7 = 14,			/**< Crash dump. */
	MANTICORE_CRASHDUMP_6 = 13,			/**< Crash dump. */
	MANTICORE_CRASHDUMP_5 = 12,			/**< Crash dump. */
	MANTICORE_CRASHDUMP_4 = 11,			/**< Crash dump. */
	MANTICORE_CRASHDUMP_3 = 10,			/**< Crash dump. */
	MANTICORE_CRASHDUMP_2 = 9,			/**< Crash dump. */
	MANTICORE_CRASHDUMP_1 = 8,			/**< Crash dump. */
	MANTICORE_CRASHDUMP_0 = 7,			/**< Crash dump. */
	MANTICORE_CRASHDUMP_COUNTER = 6,	/**< Crash dump counter. */
	MANTICORE_ON_DEMAND_SELF_TEST = 4,	/**< Management for on-demand self-test execution. */
	MANTICORE_ROM_EXCEPTION_2 = 3,		/**< ROM exception management. */
	MANTICORE_ROM_EXCEPTION_1 = 2,		/**< ROM exception management. */
	MANTICORE_ROM_FATAL_ERRORS = 1,		/**< ROM accumulation of detected fatal errors. */
	MANTICORE_ROM_BOOT_CONTROL = 0,		/**< ROM boot order control and error management. */
};

/**
 * Get a pointer to a specific HSP sticky register.
 *
 * @param x The sticky register index.
 */
#define	MANTICORE_STICKY_REG(x) \
	((uint32_t*) (HSP_ADDR_MAP_CREG_MISC_REGS_SW_REGS_SW_STICKY_RW_0_ADDRESS + (4 * x)))

/**
 * Get/set the value of a specific HSP sticky register.
 *
 * @param x The sticky register index.
 */
#define	MANTICORE_STICKY_REG_VALUE(x)	(*MANTICORE_STICKY_REG (x))

/**
 * Get a pointer to a specific HSP scratch register.
 *
 * @param x The scratch register index.
 */
#define	MANTICORE_SCRATCH_REG(x)    \
	(((uint32_t*) HSP_ADDR_MAP_CREG_MISC_REGS_CREG_MISC_GROUP_HSP_SCRATCH0_ADDRESS + (4 * x)))

/**
 * Get/set the value of a specific HSP scratch register.
 *
 * @param x The scratch register index.
 */
#define	MANTICORE_SCRATCH_REG_VALUE(x)	(*MANTICORE_SCRATCH_REG (x))


/*************************************
 * Boot status tracking only for 1SP.
 *************************************/

/**
 * Bit mask for the bits used to track unlocked booting.
 */
#define	MANTICORE_1SP_UNLOCKED_BOOT_MASK		(0xffU << 0)

/**
 * Bit indicating that PCR 0 has been extended with SPRT measurements.
 */
#define	MANTICORE_1SP_PCR0_EXTENDED_SPRT_FLAG	(1U << 30)

/**
 * Bit indicating that PCR 2 has been extended with SoC firmware measurements.
 */
#define	MANTICORE_1SP_PCR2_EXTENDED_FLAG		(1U << 28)


/****************************************************
 * Boot status tracking shared between 1SP and SPRT.
 ****************************************************/

/**
 * Bit mask for the bits used to track boot errors.
 */
#define	MANTICORE_SPRT_BOOT_ERROR_COUNTER_MASK	(0x3fU << 0)

/**
 * Update the boot error counter to ensure the recovery image will be used.
 *
 * @param reg The boot status register.
 */
#define	MANTICORE_SPRT_USE_RECOVERY_IMAGE(reg)	reg |= (1U << 4)

/**
 * Determine if the recovery firmware image on external flash should be booted.
 *
 * @param x The current boot error counter value.
 */
#define	MANTICORE_SPRT_BOOT_ERROR_BOOT_RECOVERY_IMAGE(x)	(x & (1U << 4))

/**
 * Determine if the device should be forced into I2C recovery mode.
 *
 * @param x The current boot error counter value.
 */
#define MANTICORE_SPRT_BOOT_ERROR_FORCE_I2C_RECOVERY(x)		(x & (1U << 5))

/**
 * Get the current state of the boot error counter.
 *
 * @param x The register that contains tho boot error counter.
 */
#define	MANTICORE_SPRT_BOOT_ERROR_COUNTER(x)	(x & MANTICORE_SPRT_BOOT_ERROR_COUNTER_MASK)

/**
 * Bit indicating that the system has fully initialized.
 */
#define	MANTICORE_SPRT_SYSTEM_INIT_FLAG			(1U << 31)

/**
 * Bit indicating that PCR 0 has been extended with AEB measurements.  This needs to be in the
 * status register shared with SPRT since the 1SP status register will be locked prior to setting
 * this bit.
 */
#define	MANTICORE_1SP_PCR0_EXTENDED_AEB_FLAG	(1U << 29)

/**
 * An impactless firmware update has been applied to the internal boot flash.
 */
#define	MANTICORE_FIRMWARE_UPDATE_APPLIED		(1U << 27)

/**
 * An impactful firmware update has been applied to the internal boot flash.
 */
#define	MANTICORE_FIRMWARE_IMPACTFUL_UPDATE		(1U << 26)


#endif	/* MANTICORE_STICKY_REGS_H_ */
