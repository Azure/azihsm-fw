// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef IDE_DRIVER_MANTICORE_STATIC_H_
#define IDE_DRIVER_MANTICORE_STATIC_H_

#include "ide_driver_manticore.h"


/* Internal functions declared to allow for static initialization. */
int ide_driver_manticore_get_bus_device_segment_info (const struct ide_driver *ide_driver,
	uint8_t port_index, uint8_t *bus_num, uint8_t *device_func_num, uint8_t *segment,
	uint8_t *max_port_index);
int ide_driver_manticore_get_capability_register (const struct ide_driver *ide_driver,
	uint8_t port_index, struct ide_capability_register *capability_register);
int ide_driver_manticore_get_control_register (const struct ide_driver *ide_driver,
	uint8_t port_index, struct ide_control_register *control_register);
int ide_driver_manticore_get_link_ide_register_block (const struct ide_driver *ide_driver,
	uint8_t port_index, uint8_t block_idx,
	struct ide_link_ide_stream_register_block *register_block);
int ide_driver_manticore_get_selective_ide_stream_register_block (
	const struct ide_driver *ide_driver, uint8_t port_index, uint8_t block_idx,
	struct ide_selective_ide_stream_register_block *register_block);
int ide_driver_manticore_key_prog (const struct ide_driver *ide_driver, uint8_t port_index,
	uint8_t stream_id, uint8_t key_set, bool tx_key, uint8_t key_substream, const uint32_t *key,
	uint32_t key_size, const uint32_t *iv, uint32_t iv_size);
int ide_driver_manticore_key_set_go (const struct ide_driver *ide_driver, uint8_t port_index,
	uint8_t stream_id, uint8_t key_set, bool tx_key, uint8_t key_substream);
int ide_driver_manticore_key_set_stop (const struct ide_driver *ide_driver, uint8_t port_index,
	uint8_t stream_id, uint8_t key_set, bool tx_key, uint8_t key_substream);


/**
 * Constant initializer for the IDE driver API.
 */
#define IDE_DRIVER_API_INIT	{ \
		.get_bus_device_segment_info = ide_driver_manticore_get_bus_device_segment_info, \
		.get_capability_register = ide_driver_manticore_get_capability_register, \
		.get_control_register = ide_driver_manticore_get_control_register, \
		.get_link_ide_register_block = ide_driver_manticore_get_link_ide_register_block, \
		.get_selective_ide_stream_register_block = ide_driver_manticore_get_selective_ide_stream_register_block, \
		.key_prog = ide_driver_manticore_key_prog, \
		.key_set_go = ide_driver_manticore_key_set_go, \
		.key_set_stop = ide_driver_manticore_key_set_stop, \
	}

/**
 * Initialize a static IDE driver instance.  This will be used for a Manticore-based platform.
 *
 * There is no validation done on the arguments.
 *
 * @param assist_addr Base address for the PCIe assist registers.
 * @param ide_addr Base address for the IDE registers.
 * @param aes_addr Base address for the AES registers.
 * @param key_addr Base address for the key context memory.
 * @param hsp_dmb The HSP DMB device driver instance to be used as MMU and allow access to SoC
 * memory space.
 * @param state_ptr Variable context for the IDE driver.
 */
#define ide_driver_manticore_static_init(assist_addr, ide_addr, aes_addr, key_addr, hsp_dmb, \
	state_ptr) { \
			.base = IDE_DRIVER_API_INIT,\
			.dmb = hsp_dmb, \
			.assist_reg_base_addr = assist_addr, \
			.reg_base_addr = ide_addr, \
			.aes_reg_base_addr = aes_addr, \
			.key_context_addr = key_addr, \
			.state = state_ptr, \
		}


#endif	/* IDE_DRIVER_MANTICORE_STATIC_H_ */
