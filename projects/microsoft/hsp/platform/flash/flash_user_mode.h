// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FLASH_USER_MODE_H_
#define FLASH_USER_MODE_H_

#include "flash/spi_flash.h"
#include "splibs/hsprt/riscvcpu.h"
#include "splibs/inc/spstatus.h"
#include "splibs/inc/sptypes.h"


/**
 * Flash interface wrapper for a another flash device that will jump to user mode before reading
 * data into memory from that flash device.
 */
struct flash_user_mode {
	struct flash base;					/**< Base flash instance. */
	const struct flash *device;			/**< Flash that is wrapped for user mode execution. */
	const RiscvPmpSetting *pmp_config;	/**< The PMP settings to apply when going into user mode. */
	size_t pmp_regions;					/**< Number of PMP regions defined. */
};


int flash_user_mode_init (struct flash_user_mode *flash, const struct flash *device,
	const RiscvPmpSetting *pmp_config, size_t pmp_regions);
void flash_user_mode_release (const struct flash_user_mode *flash);


#endif	/* FLASH_USER_MODE_H_ */
