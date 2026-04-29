// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MSFT_BASE_COMMANDS_STATIC_H_
#define MSFT_BASE_COMMANDS_STATIC_H_

#include "msft_base_commands.h"


/**
 * Initializes a static instance of a supported command set in the MSFT protocol.
 *
 * There is no validation done on the arguments.
 *
 * @param set_id_arg The command set ID to associate with the entry.
 * @param protocol_versions_ptr List of protocol versions supported for the command set.
 * @param version_count_arg The number of supported protocol versions.
 */
#define	msft_base_supported_command_set_static_init(set_id_arg, protocol_versions_ptr, \
	version_count_arg) { \
		.set_id = set_id_arg, \
		.versions = protocol_versions_ptr, \
		.version_count = version_count_arg, \
	}


#endif	/* MSFT_BASE_COMMANDS_STATIC_H_ */
