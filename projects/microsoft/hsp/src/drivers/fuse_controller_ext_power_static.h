// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FUSE_CONTROLLER_EXT_POWER_STATIC_H_
#define FUSE_CONTROLLER_EXT_POWER_STATIC_H_

#include "fuse_controller_ext_power.h"


/* Internal functions declared to allow for static initialization. */
enum hsp_security_state fuse_controller_ext_power_get_security_state (
	const struct fuse_controller_interface *fuses);


int fuse_controller_ext_power_change_security_state (
	const struct fuse_controller_interface *fuses, enum hsp_security_state state);
int fuse_controller_ext_power_blank_check_socid (
	const struct fuse_controller_interface *fuses);
int fuse_controller_ext_power_read_registered_socid (
	const struct fuse_controller_interface *fuses, uint8_t *socid, size_t length);
int fuse_controller_ext_power_program_socid (
	const struct fuse_controller_interface *fuses, const uint8_t *socid, size_t length);
int fuse_controller_ext_power_read_emc_register (
	const struct fuse_controller_interface *fuses, uint16_t address, uint32_t *value);
int fuse_controller_ext_power_program_emc_register (
	const struct fuse_controller_interface *fuses, uint16_t address, uint32_t value);
int fuse_controller_ext_power_read_aeb_register (
	const struct fuse_controller_interface *fuses, uint16_t address, uint32_t *value);
int fuse_controller_ext_power_program_aeb_register (
	const struct fuse_controller_interface *fuses, uint16_t address, uint32_t value);
int fuse_controller_ext_power_blank_check (
	const struct fuse_controller_interface *fuses, uint16_t start_addr, uint16_t end_addr,
	uint16_t *not_blank);
int fuse_controller_ext_power_blank_check_key (
	const struct fuse_controller_interface *fuses, uint8_t key);
const struct fuse_controller_fuse_map* fuse_controller_ext_power_get_fuse_map (
	const struct fuse_controller_interface *fuses);


int fuse_controller_ext_power_read_sw_fuses (
	const struct fuse_controller_interface *fuses, uint16_t start_addr, uint8_t *data,
	size_t length);
int fuse_controller_ext_power_read_registered_sw_fuses (
	const struct fuse_controller_interface *fuses, uint16_t start_addr, uint8_t *data,
	size_t length);
int fuse_controller_ext_power_program_sw_fuses (
	const struct fuse_controller_interface *fuses, uint16_t start_addr,	const uint32_t *data,
	size_t fuse_words);
int fuse_controller_ext_power_program_registered_sw_fuses (
	const struct fuse_controller_interface *fuses, uint16_t start_addr,	const uint32_t *data,
	size_t fuse_words);
int fuse_controller_ext_power_read_rng_calibration (const struct fuse_controller_interface *fuses,
	uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH]);
int fuse_controller_ext_power_program_rng_calibration (
	const struct fuse_controller_interface *fuses,
	const uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH]);

/**
 * Static initialization of the Fuse Controller driver API for external fuse controller power.
 */
#define	FUSE_CONTROLLER_EXT_POWER_API_INIT	{ \
		.get_security_state = fuse_controller_ext_power_get_security_state, \
		.change_security_state = fuse_controller_ext_power_change_security_state, \
		.blank_check_socid = fuse_controller_ext_power_blank_check_socid, \
		.read_registered_socid = fuse_controller_ext_power_read_registered_socid, \
		.program_socid = fuse_controller_ext_power_program_socid, \
		.read_emc_register = fuse_controller_ext_power_read_emc_register, \
		.program_emc_register = fuse_controller_ext_power_program_emc_register, \
		.read_aeb_register = fuse_controller_ext_power_read_aeb_register, \
		.program_aeb_register = fuse_controller_ext_power_program_aeb_register, \
		.blank_check = fuse_controller_ext_power_blank_check, \
		.blank_check_key = fuse_controller_ext_power_blank_check_key, \
		.get_fuse_map = fuse_controller_ext_power_get_fuse_map, \
		.read_sw_fuses = fuse_controller_ext_power_read_sw_fuses, \
		.read_registered_sw_fuses = fuse_controller_ext_power_read_registered_sw_fuses, \
		.program_sw_fuses = fuse_controller_ext_power_program_sw_fuses, \
		.program_registered_sw_fuses = fuse_controller_ext_power_program_registered_sw_fuses, \
		.read_rng_calibration = fuse_controller_ext_power_read_rng_calibration, \
		.program_rng_calibration = fuse_controller_ext_power_program_rng_calibration, \
	}

/**
 * Initialize a static fuse controller driver instance.
 *
 * There is no validation done on the arguments.
 *
 * @param fuses_ptr interface of fuse controller.
 * @param power_ptr interface of power controller.
 */
#define	fuse_controller_ext_power_static_init(fuses_ptr, power_ptr) { \
		.base =  FUSE_CONTROLLER_EXT_POWER_API_INIT, \
		.fuses = fuses_ptr, \
		.power = power_ptr, \
	}


#endif	/* FUSE_CONTROLLER_EXT_POWER_STATIC_H_ */
