// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef TDISP_DRIVER_MANTICORE_STATIC_H_
#define TDISP_DRIVER_MANTICORE_STATIC_H_

#include "tdisp_driver_manticore.h"


int tdisp_driver_manticore_get_function_index (const struct tdisp_driver *tdisp_driver,
	uint32_t bdf, uint32_t *function_index);
int tdisp_driver_manticore_get_tdisp_capabilities (const struct tdisp_driver *tdisp_driver,
	const struct tdisp_requester_capabilities *req_caps,
	struct tdisp_responder_capabilities *rsp_caps);
int tdisp_driver_manticore_lock_interface_request (const struct tdisp_driver *tdisp_driver,
	uint32_t function_id, const struct tdisp_lock_interface_param *lock_interface_param);
int tdisp_driver_manticore_get_device_interface_report (const struct tdisp_driver *tdisp_driver,
	uint32_t function_id, uint16_t request_offset, uint16_t request_length,	uint16_t *report_length,
	uint8_t *interface_report, uint16_t *remainder_length);
int tdisp_driver_manticore_get_device_interface_state (const struct tdisp_driver *tdisp_driver,
	uint32_t function_id, uint8_t *tdi_state);
int tdisp_driver_manticore_start_interface_request (const struct tdisp_driver *tdisp_driver,
	uint32_t function_id);
int tdisp_driver_manticore_stop_interface_request (const struct tdisp_driver *tdisp_driver,
	uint32_t function_id);
int tdisp_driver_manticore_get_mmio_ranges (const struct tdisp_driver *tdisp_driver,
	uint32_t function_id, uint32_t mmio_range_count, struct tdisp_mmio_range *mmio_ranges);

/**
 * Constant initializer for the TDISP driver interface.
 */
#define TDISP_DRIVER_MANTICORE_API_INIT { \
	.get_function_index = tdisp_driver_manticore_get_function_index, \
	.get_tdisp_capabilities = tdisp_driver_manticore_get_tdisp_capabilities, \
	.lock_interface_request = tdisp_driver_manticore_lock_interface_request, \
	.get_device_interface_report = tdisp_driver_manticore_get_device_interface_report, \
	.get_device_interface_state = tdisp_driver_manticore_get_device_interface_state, \
	.start_interface_request = tdisp_driver_manticore_start_interface_request, \
	.stop_interface_request = tdisp_driver_manticore_stop_interface_request, \
	.get_mmio_ranges = tdisp_driver_manticore_get_mmio_ranges, \
}

/**
 * Initialize a static instance of a TDISP manticore driver
 *
 * There is no validation done on the arguments
 *
 * @param tdi_context_manager_ptr TDISP TDI context manager
 * @param pcie_registers_ptr MMIO register block for the PCIe device
 * @param ide_driver_ptr IDE driver to use for TDISP driver
 * @param state_ptr Variable context for the TDISP driver
 */
#define tdisp_driver_manticore_static_init(tdi_context_manager_ptr, pcie_registers_ptr, \
	ide_driver_ptr, state_ptr) { \
	.base = TDISP_DRIVER_MANTICORE_API_INIT, \
	.tdi_context_manager = tdi_context_manager_ptr, \
	.pcie_registers = pcie_registers_ptr, \
	.ide = ide_driver_ptr, \
	.state = state_ptr, \
}


#endif	// TDISP_DRIVER_MANTICORE_STATIC_H_
