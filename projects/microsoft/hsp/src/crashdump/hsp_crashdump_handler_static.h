// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_CRASHDUMP_HANDLER_STATIC_H_
#define HSP_CRASHDUMP_HANDLER_STATIC_H_

#include "trap/hsp_interrupt_handler_static.h"


/* Internal functions declared to allow for static initialization. */
bool hsp_crashdump_handler_handle_interrupt (const struct hsp_interrupt_handler *handler,
	uintptr_t param);

/**
 * Initialize a static instance of a crashdump.
 *
 * There is no validation done on the arguments
 * @param[in] fw_version_ptr The build version number for the firmware package.
 * @param[in] fw_version_length The length of build version number for the firmware package.
 * @param[in] reset_callback Reset callback function. Once it is invoked, it would get device into warm reset.
 * @param[in] ram_ptr Persistent ram space for the crash dump handler.
 * @param[in] ram_size Size of persistent ram space for the crash dump handler.
 */
#define hsp_crashdump_handler_static_init(fw_version_ptr, fw_version_length, reset_callback, ram_ptr, ram_size) { \
		.base = \
			hsp_interrupt_handler_static_init (hsp_crashdump_handler_handle_interrupt), \
		.fw_version = fw_version_ptr, \
		.fw_version_len = fw_version_length, \
		.reset = reset_callback, \
		.persistent_ram = ram_ptr, \
		.persistent_ram_size = ram_size, \
	}


#endif	/* HSP_CRASHDUMP_HANDLER_STATIC_H_ */
