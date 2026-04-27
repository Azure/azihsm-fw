// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TDISP_DRIVER_MANTICORE_H_
#define TDISP_DRIVER_MANTICORE_H_

#include <stdint.h>
#include "common/observable.h"
#include "mmio/mmio_register_block.h"
#include "pcisig/ide/ide_driver.h"
#include "pcisig/tdisp/tdisp_driver.h"
#include "pcisig/tdisp/tdisp_driver_observer.h"
#include "pcisig/tdisp/tdisp_tdi_context_manager.h"


/**
 * Maximum number of TDI interfaces supported by Manticore HW. 64 VFs + 1 PF
 */
#define TDISP_TDI_MAX_COUNT 65

/**
 * Manticore TDI states
 */
enum {
	TDISP_DRIVER_HW_STATE_NON_TEE = 0x1,			/**< NON TEE interface */
	TDISP_DRIVER_HW_STATE_CONFIG_UNLOCKED = 0x2,	/**< TDISP config unlock state */
	TDISP_DRIVER_HW_STATE_CONFIG_LOCKED = 0x4,		/**< TDISP config locked state */
	TDISP_DRIVER_HW_STATE_ERROR = 0x8,				/**< TDISP error state */
	TDISP_DRIVER_HW_STATE_RUN = 0xF,				/**< TDISP run state */
};


/**
 * Manticore LUT fields masks
 */
enum {
	TDISP_LUT_MASK_MSIX_L = 0x01,	/**< MSIX Lock bit */
	TDISP_LUT_MASK_NON_T_M0 = 0x02,	/**< M0 range NON_T state */
	TDISP_LUT_MASK_NON_T_M1 = 0x04,	/**< M1 range NON_T state */
	TDISP_LUT_MASK_NON_T_M2 = 0x08,	/**< M2 range NON_T state */
	TDISP_LUT_MASK_TDISP_ST = 0x10,	/**< TDI state */
	TDISP_LUT_MASK_ID = 0x20,		/**< TDI ID */
};

/**
 * Manticore TDISP LUT entry structure
 *
 * This structure represents a single entry in the TDISP LUT.
 * It contains fields for MSIX lock, NON_T states for M0, M1, and M2 ranges,
 * TDI state, and TDI ID.
 */
union tdisp_lut_entry {
	uint32_t value;
	struct {
		uint32_t msix_l : 1;
		uint32_t non_t_m2 : 1;
		uint32_t non_t_m1 : 1;
		uint32_t non_t_m0 : 1;
		uint32_t tdisp_st : 4;
		uint32_t id : 8;
	};
};

/**
 * Manticore TDISP device interface report structure
 */
struct tdisp_driver_manticore_interface_report {
	struct tdisp_device_interface_report base;
	struct tdisp_mmio_range mmio_ranges[3];
	uint32_t device_specific_info_length;
};

/**
 * Variable context for the TDISP driver.
 */
struct tdisp_driver_manticore_state {
	struct observable observable;	/**< Observer manager for TDISP events. */
};

/**
 * TDISP driver implementation for Manticore
 */
struct tdisp_driver_manticore {
	struct tdisp_driver base;										/**< TDISP driver interface */
	const struct tdisp_tdi_context_manager *tdi_context_manager;	/**< TDI context manager */
	const struct mmio_register_block *pcie_registers;				/**< MMIO register block interface */
	const struct ide_driver *ide;									/**< IDE driver interface */
	struct tdisp_driver_manticore_state *state;						/**< Variable context for the TDISP driver */
};


int tdisp_driver_manticore_init (struct tdisp_driver_manticore *tdisp_driver,
	const struct tdisp_tdi_context_manager *tdi_context_manager,
	const struct mmio_register_block *pcie_registers, const struct ide_driver *ide,
	struct tdisp_driver_manticore_state *state);
int tdisp_driver_manticore_init_state (const struct tdisp_driver_manticore *tdisp_driver);
void tdisp_driver_manticore_release (const struct tdisp_driver_manticore *tdisp_driver);

int tdisp_driver_manticore_enable (const struct tdisp_driver_manticore *tdisp_driver);

int tdisp_driver_manticore_set_all_error_state (const struct tdisp_driver_manticore *tdisp_driver,
	const struct mmio_register_block *pcie_registers);

int tdisp_driver_manticore_add_tdisp_driver_observer (
	const struct tdisp_driver_manticore *tdisp_driver,
	const struct tdisp_driver_observer *observer);
int tdisp_driver_manticore_remove_tdisp_driver_observer (
	const struct tdisp_driver_manticore *tdisp_driver,
	const struct tdisp_driver_observer *observer);


#endif	// TDISP_DRIVER_MANTICORE_H_
