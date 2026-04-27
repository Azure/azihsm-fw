// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVERLAKE_FLASH_MANAGER_FPGA_H_
#define OVERLAKE_FLASH_MANAGER_FPGA_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "flash/spi_flash.h"
#include "host_fw/host_control.h"
#include "host_fw/host_flash_initialization.h"
#include "host_fw/host_state_manager.h"
#include "host_fw/overlake_board_id.h"
#include "host_fw/overlake_flash_manager.h"
#include "manifest/pfm/pfm.h"


/**
 * The bitmask is for checking incoming FPGA boot mode arguments from the command interface for
 * valid values.
 *
 * Field			| Bits	| Description
 * -----------------|-------|-----------
 * boot_slot		| 1:0	| values 0-3
 * -----------------|-------|-----------
 * reserved			| 6:2	| reserved
 * -----------------|-------|-----------
 * Fail to failsafe | 7		| debug only
 */
#define OVERLAKE_FPGA_BOOT_MODE_MASK 0x83
#define OVERLAKE_FPGA_BOOT_MODE_INVALID 0xff

/**
 * Management of the protected flash device for the Overlake FPGA.
 */
struct overlake_flash_manager_fpga {
	struct overlake_flash_manager base;
};


int overlake_flash_manager_fpga_init (struct overlake_flash_manager_fpga *manager,
	struct spi_flash *flash, const struct host_control *control,
	const struct host_state_manager *host_state_boot, enum overlake_board_id board_id,
	volatile uint32_t *boot_mode_cache_reg, bool is_por);
int overlake_flash_manager_fpga_init_with_managed_flash_initialization (
	struct overlake_flash_manager_fpga *manager, struct spi_flash *flash,
	const struct host_control *control, const struct host_state_manager *host_state_fpga,
	const struct host_flash_initialization *flash_init, enum overlake_board_id board_id,
	volatile uint32_t *boot_mode_cache_reg, bool is_por);
void overlake_flash_manager_fpga_release (struct overlake_flash_manager_fpga *manager);


#endif	/* OVERLAKE_FLASH_MANAGER_FPGA_H_ */
