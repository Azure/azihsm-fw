// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FUSE_CONTROLLER_H_
#define FUSE_CONTROLLER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "platform_api.h"
#include "drivers/fuse_controller_interface.h"
#include "drivers/hsp_security_state.h"
#include "status/msft_module_id.h"

/**
 * Variable context associated with a fuse controller driver.
 */
struct fuse_controller_state {
	platform_mutex lock;	/**< Lock for synchronization. */
};

/**
 * Implementation of the fuse controller driver interface to execute commands against hardware blocks.
 */
struct fuse_controller {
	struct fuse_controller_interface base;	/**< Base driver interface for Fuse commands. */
	struct fuse_controller_state *state;	/**< Variable context for the fuse controller. */
	struct Gfc_regs *regs;					/**< Register interface for the fuse controller. */
};


int fuse_controller_init_state (const struct fuse_controller *fuses);
void fuse_controller_release (const struct fuse_controller *fuses);


/* Internal definitions for use by derived types. */

/**
 * The list of valid commands to send to the fuse controller.
 */
enum fuse_controller_cmd {
	FUSE_CONTROLLER_CMD_CHANGE_TO_TEST = 0x10,			/**< Change the security state to TEST. */
	FUSE_CONTROLLER_CMD_CHANGE_TO_PRODUCTION = 0x20,	/**< Change the security state to PROD. */
	FUSE_CONTROLLER_CMD_CHANGE_TO_SECURE = 0x30,		/**< Change the security state to SECURE. */
	FUSE_CONTROLLER_CMD_CHANGE_TO_RETEST = 0x40,		/**< Change the security state to RETEST. */
	FUSE_CONTROLLER_CMD_PROGRAM_DATA = 0x01,			/**< Program data to the fuses. */
	FUSE_CONTROLLER_CMD_READ_DATA = 0x02,				/**< Read data from the fuses. */
	FUSE_CONTROLLER_CMD_BLANK_CHECK = 0x03,				/**< Check a fuse address to see if is blank. */
};


int fuse_controller_init (struct fuse_controller *fuses, struct fuse_controller_state *state,
	struct Gfc_regs *regs);

int fuse_controller_execute_command (const struct fuse_controller *fuses,
	enum fuse_controller_cmd command, uint16_t address, const uint32_t *data, size_t words);


#endif	/* FUSE_CONTROLLER_H_ */
