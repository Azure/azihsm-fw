// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SPI_FILTER_HSP_STATIC_H_
#define SPI_FILTER_HSP_STATIC_H_

#include "spi_filter/spi_filter_hsp.h"


/* Internal functions declared to allow for static initialization. */
int spi_filter_hsp_get_port (const struct spi_filter_interface *filter);
int spi_filter_hsp_get_mfg_id (const struct spi_filter_interface *filter, uint8_t *mfg_id);
int spi_filter_hsp_set_mfg_id (const struct spi_filter_interface *filter, uint8_t mfg_id);
int spi_filter_hsp_get_flash_size (const struct spi_filter_interface *filter, uint32_t *bytes);
int spi_filter_hsp_set_flash_size (const struct spi_filter_interface *filter, uint32_t bytes);
int spi_filter_hsp_get_filter_mode (const struct spi_filter_interface *filter,
	spi_filter_flash_mode *mode);
int spi_filter_hsp_set_filter_mode (const struct spi_filter_interface *filter,
	spi_filter_flash_mode mode);
int spi_filter_hsp_get_filter_enabled (const struct spi_filter_interface *filter, bool *enabled);
int spi_filter_hsp_enable_filter (const struct spi_filter_interface *filter, bool enable);
int spi_filter_hsp_get_ro_cs (const struct spi_filter_interface *filter, spi_filter_cs *act_sel);
int spi_filter_hsp_set_ro_cs (const struct spi_filter_interface *filter, spi_filter_cs act_sel);
int spi_filter_hsp_get_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode *mode);
int spi_filter_hsp_get_fixed_addr_byte_mode (const struct spi_filter_interface *filter,
	bool *fixed);
int spi_filter_hsp_set_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode mode);
int spi_filter_hsp_set_fixed_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode mode);
int spi_filter_hsp_get_addr_byte_mode_write_enable_required (
	const struct spi_filter_interface *filter, bool *required);
int spi_filter_hsp_require_addr_byte_mode_write_enable (const struct spi_filter_interface *filter,
	bool require);
int spi_filter_hsp_get_reset_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode *mode);
int spi_filter_hsp_set_reset_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode mode);
int spi_filter_hsp_are_all_single_flash_writes_allowed (const struct spi_filter_interface *filter,
	bool *allowed);
int spi_filter_hsp_allow_all_single_flash_writes (const struct spi_filter_interface *filter,
	bool allowed);
int spi_filter_hsp_get_write_enable_detected (const struct spi_filter_interface *filter,
	bool *detected);
int spi_filter_hsp_get_flash_dirty_state (const struct spi_filter_interface *filter,
	spi_filter_flash_state *state);
int spi_filter_hsp_clear_flash_dirty_state (const struct spi_filter_interface *filter);
int spi_filter_hsp_get_filter_rw_region (const struct spi_filter_interface *filter, uint8_t region,
	uint32_t *start_addr, uint32_t *end_addr);
int spi_filter_hsp_set_filter_rw_region (const struct spi_filter_interface *filter, uint8_t region,
	uint32_t start_addr, uint32_t end_addr);
int spi_filter_hsp_clear_filter_rw_regions (const struct spi_filter_interface *filter);

uint32_t spi_filter_hsp_get_interrupt_status (const struct spi_filter_hsp *filter);
uint32_t spi_filter_hsp_get_interrupt_enable (const struct spi_filter_hsp *filter);
void spi_filter_hsp_set_interrupt_enable (const struct spi_filter_hsp *filter, uint32_t enable);
uint8_t spi_filter_hsp_get_blocked_opcode (const struct spi_filter_hsp *filter);
int spi_filter_hsp_set_filtered_opcodes (const struct spi_filter_hsp *filter,
	const union spi_filter_hsp_opcode *opcode_list, size_t count);


/**
 * Constant initializer for the hash API.
 */
#define	SPI_FILTER_HSP_API_INIT	{ \
		.get_port = spi_filter_hsp_get_port, \
		.get_mfg_id = spi_filter_hsp_get_mfg_id, \
		.set_mfg_id = spi_filter_hsp_set_mfg_id, \
		.get_flash_size = spi_filter_hsp_get_flash_size, \
		.set_flash_size = spi_filter_hsp_set_flash_size, \
		.get_filter_mode = spi_filter_hsp_get_filter_mode, \
		.set_filter_mode = spi_filter_hsp_set_filter_mode, \
		.get_filter_enabled = spi_filter_hsp_get_filter_enabled, \
		.enable_filter = spi_filter_hsp_enable_filter, \
		.get_ro_cs = spi_filter_hsp_get_ro_cs, \
		.set_ro_cs = spi_filter_hsp_set_ro_cs, \
		.get_addr_byte_mode = spi_filter_hsp_get_addr_byte_mode, \
		.get_fixed_addr_byte_mode = spi_filter_hsp_get_fixed_addr_byte_mode, \
		.set_addr_byte_mode = spi_filter_hsp_set_addr_byte_mode, \
		.set_fixed_addr_byte_mode = spi_filter_hsp_set_fixed_addr_byte_mode, \
		.get_addr_byte_mode_write_enable_required = \
			spi_filter_hsp_get_addr_byte_mode_write_enable_required, \
		.require_addr_byte_mode_write_enable = spi_filter_hsp_require_addr_byte_mode_write_enable, \
		.get_reset_addr_byte_mode = spi_filter_hsp_get_reset_addr_byte_mode, \
		.set_reset_addr_byte_mode = spi_filter_hsp_set_reset_addr_byte_mode, \
		.are_all_single_flash_writes_allowed = spi_filter_hsp_are_all_single_flash_writes_allowed, \
		.allow_all_single_flash_writes = spi_filter_hsp_allow_all_single_flash_writes, \
		.get_write_enable_detected = spi_filter_hsp_get_write_enable_detected, \
		.get_flash_dirty_state = spi_filter_hsp_get_flash_dirty_state, \
		.clear_flash_dirty_state = spi_filter_hsp_clear_flash_dirty_state, \
		.get_filter_rw_region = spi_filter_hsp_get_filter_rw_region, \
		.set_filter_rw_region = spi_filter_hsp_set_filter_rw_region, \
		.clear_filter_rw_regions = spi_filter_hsp_clear_filter_rw_regions \
	}


/**
 * Initialize a static CCS/KSU driver instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the CCS driver instance.
 * @param regs_ptr Base address for the CCS registers.
 * @param port_id Port identifier for the SPI filter instance.
 */
#define	spi_filter_hsp_static_init(state_ptr, regs_ptr, port_id)	{ \
		.base = SPI_FILTER_HSP_API_INIT, \
		.get_interrupt_status = spi_filter_hsp_get_interrupt_status, \
		.get_interrupt_enable = spi_filter_hsp_get_interrupt_enable, \
		.set_interrupt_enable = spi_filter_hsp_set_interrupt_enable, \
		.get_blocked_opcode = spi_filter_hsp_get_blocked_opcode, \
		.set_filtered_opcodes = spi_filter_hsp_set_filtered_opcodes, \
		.state = state_ptr, \
		.regs = regs_ptr, \
		.port = port_id \
	}


#endif	/* SPI_FILTER_HSP_STATIC_H_ */
