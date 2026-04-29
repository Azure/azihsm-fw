// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FUSE_CONTROLLER_STATIC_H_
#define FUSE_CONTROLLER_STATIC_H_

#include "drivers/fuse_controller.h"


/* Internal functions declared to allow for static initialization. */
enum hsp_security_state fuse_controller_get_security_state (
	const struct fuse_controller_interface *fuses);


int fuse_controller_change_security_state (const struct fuse_controller_interface *fuses,
	enum hsp_security_state state);
int fuse_controller_blank_check_socid (const struct fuse_controller_interface *fuses);
int fuse_controller_read_registered_socid (const struct fuse_controller_interface *fuses,
	uint8_t *socid, size_t length);
int fuse_controller_program_socid (const struct fuse_controller_interface *fuses,
	const uint8_t *socid, size_t length);
int fuse_controller_read_emc_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t *value);
int fuse_controller_program_emc_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t value);
int fuse_controller_read_aeb_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t *value);
int fuse_controller_program_aeb_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t value);
int fuse_controller_blank_check (const struct fuse_controller_interface *fuses, uint16_t start_addr,
	uint16_t end_addr, uint16_t *not_blank);
int fuse_controller_blank_check_key (const struct fuse_controller_interface *fuses, uint8_t key);
const struct fuse_controller_fuse_map* fuse_controller_get_fuse_map (
	const struct fuse_controller_interface *fuses);


int fuse_controller_read_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, uint8_t *data, size_t length);
int fuse_controller_read_registered_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, uint8_t *data, size_t length);
int fuse_controller_program_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, const uint32_t *data, size_t fuse_words);
int fuse_controller_program_registered_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, const uint32_t *data, size_t fuse_words);

/**
 * Static initialization of the Fuse Controller driver APIs.
 */
#define	FUSE_CONTROLLER_API_INIT    \
		.get_security_state = fuse_controller_get_security_state, \
		.change_security_state = fuse_controller_change_security_state, \
		.blank_check_socid = fuse_controller_blank_check_socid, \
		.read_registered_socid = fuse_controller_read_registered_socid, \
		.program_socid = fuse_controller_program_socid, \
		.read_emc_register = fuse_controller_read_emc_register, \
		.program_emc_register = fuse_controller_program_emc_register, \
		.read_aeb_register = fuse_controller_read_aeb_register, \
		.program_aeb_register = fuse_controller_program_aeb_register, \
		.blank_check = fuse_controller_blank_check, \
		.blank_check_key = fuse_controller_blank_check_key, \
		.get_fuse_map = fuse_controller_get_fuse_map, \
		.read_sw_fuses = fuse_controller_read_sw_fuses, \
		.read_registered_sw_fuses = fuse_controller_read_registered_sw_fuses, \
		.program_sw_fuses = fuse_controller_program_sw_fuses, \
		.program_registered_sw_fuses = fuse_controller_program_registered_sw_fuses,


/**
 * Initialize common fields of a static fuse controller driver instance.  This should be included as
 * part of complete initialization implementation that includes handlers for RNG calibration data.
 *
 * There is no validation done on the arguments.
 *
 * @param api The type of fuse controller initialization.
 * @param state_ptr The variable context for the driver instance.
 * @param regs_ptr Base address of the hardware registers.
 */
#define	fuse_controller_static_init(api, state_ptr, regs_ptr)   \
		.base =  api, \
		.state = state_ptr, \
		.regs = regs_ptr


#endif	/* FUSE_CONTROLLER_STATIC_H_ */
