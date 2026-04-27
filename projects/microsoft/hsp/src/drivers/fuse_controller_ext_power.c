// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fuse_controller_ext_power.h"
#include "common/unused.h"
#include "logging/hsp_logging.h"


/**
 * Power down the external power.
 *
 * @param fuses_ext_pwr The external power fuse driver instance.
 */
static void fuse_controller_ext_power_off (const struct fuse_controller_ext_power *fuses_ext_pwr)
{
	int status;

	status = fuses_ext_pwr->power->power_down (fuses_ext_pwr->power);
	if (status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_HSP,
			HSP_LOGGING_EXT_POWER_DOWN_FAILED, status, 0);
	}
}

enum hsp_security_state fuse_controller_ext_power_get_security_state (
	const struct fuse_controller_interface *fuses)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return HSP_SECURITY_STATE_UNKNOWN;
	}

	return fuses_ext_pwr->fuses->get_security_state (fuses_ext_pwr->fuses);
}

int fuse_controller_ext_power_change_security_state (const struct fuse_controller_interface *fuses,
	enum hsp_security_state state)
{
	int status;
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	status = fuses_ext_pwr->power->power_up (fuses_ext_pwr->power);
	if (status != 0) {
		return status;
	}

	status = fuses_ext_pwr->fuses->change_security_state (fuses_ext_pwr->fuses, state);

	fuse_controller_ext_power_off (fuses_ext_pwr);

	return status;
}

int fuse_controller_ext_power_read_registered_socid (const struct fuse_controller_interface *fuses,
	uint8_t *buffer, size_t length)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuses_ext_pwr->fuses->read_registered_socid (fuses_ext_pwr->fuses, buffer, length);
}

int fuse_controller_ext_power_program_socid (const struct fuse_controller_interface *fuses,
	const uint8_t *socid, size_t length)
{
	int status;
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	status = fuses_ext_pwr->power->power_up (fuses_ext_pwr->power);
	if (status != 0) {
		return status;
	}

	status = fuses_ext_pwr->fuses->program_socid (fuses_ext_pwr->fuses, socid, length);

	fuse_controller_ext_power_off (fuses_ext_pwr);

	return status;
}

int fuse_controller_ext_power_read_emc_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t *value)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuses_ext_pwr->fuses->read_emc_register (fuses_ext_pwr->fuses, address, value);
}

int fuse_controller_ext_power_program_emc_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t value)
{
	int status;
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	status = fuses_ext_pwr->power->power_up (fuses_ext_pwr->power);
	if (status != 0) {
		return status;
	}

	status = fuses_ext_pwr->fuses->program_emc_register (fuses_ext_pwr->fuses, address, value);

	fuse_controller_ext_power_off (fuses_ext_pwr);

	return status;
}

int fuse_controller_ext_power_read_aeb_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t *value)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuses_ext_pwr->fuses->read_aeb_register (fuses_ext_pwr->fuses, address, value);
}

int fuse_controller_ext_power_program_aeb_register (const struct fuse_controller_interface *fuses,
	uint16_t address, uint32_t value)
{
	int status;
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	status = fuses_ext_pwr->power->power_up (fuses_ext_pwr->power);
	if (status != 0) {
		return status;
	}

	status = fuses_ext_pwr->fuses->program_aeb_register (fuses_ext_pwr->fuses, address, value);

	fuse_controller_ext_power_off (fuses_ext_pwr);

	return status;
}

int fuse_controller_ext_power_blank_check (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, uint16_t end_addr, uint16_t *not_blank)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuses_ext_pwr->fuses->blank_check (fuses_ext_pwr->fuses, start_addr, end_addr,
		not_blank);
}

int fuse_controller_ext_power_blank_check_socid (const struct fuse_controller_interface *fuses)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuses_ext_pwr->fuses->blank_check_socid (fuses_ext_pwr->fuses);
}

int fuse_controller_ext_power_blank_check_key (const struct fuse_controller_interface *fuses,
	uint8_t key)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuses_ext_pwr->fuses->blank_check_key (fuses_ext_pwr->fuses, key);
}

const struct fuse_controller_fuse_map* fuse_controller_ext_power_get_fuse_map (
	const struct fuse_controller_interface *fuses)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return NULL;
	}

	return fuses_ext_pwr->fuses->get_fuse_map (fuses_ext_pwr->fuses);
}

int fuse_controller_ext_power_read_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, uint8_t *data, size_t length)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuses_ext_pwr->fuses->read_sw_fuses (fuses_ext_pwr->fuses, start_addr, data, length);
}

int fuse_controller_ext_power_program_sw_fuses (const struct fuse_controller_interface *fuses,
	uint16_t start_addr, const uint32_t *data, size_t fuse_words)
{
	int status;
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	status = fuses_ext_pwr->power->power_up (fuses_ext_pwr->power);
	if (status != 0) {
		return status;
	}

	status = fuses_ext_pwr->fuses->program_sw_fuses (fuses_ext_pwr->fuses, start_addr, data,
		fuse_words);

	fuse_controller_ext_power_off (fuses_ext_pwr);

	return status;
}

int fuse_controller_ext_power_read_registered_sw_fuses (
	const struct fuse_controller_interface *fuses, uint16_t start_addr, uint8_t *data,
	size_t length)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuses_ext_pwr->fuses->read_registered_sw_fuses (fuses_ext_pwr->fuses, start_addr, data,
		length);
}

int fuse_controller_ext_power_program_registered_sw_fuses (
	const struct fuse_controller_interface *fuses, uint16_t start_addr, const uint32_t *data,
	size_t fuse_words)
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuses_ext_pwr->fuses->program_registered_sw_fuses (fuses_ext_pwr->fuses, start_addr,
		data, fuse_words);
}

int fuse_controller_ext_power_read_rng_calibration (const struct fuse_controller_interface *fuses,
	uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	return fuses_ext_pwr->fuses->read_rng_calibration (fuses_ext_pwr->fuses, rng_data);
}

int fuse_controller_ext_power_program_rng_calibration (
	const struct fuse_controller_interface *fuses,
	const uint8_t rng_data[FUSE_CONTROLLER_RNG_CALIBRATION_LENGTH])
{
	int status;
	const struct fuse_controller_ext_power *fuses_ext_pwr =
		(const struct fuse_controller_ext_power*) fuses;

	if (fuses_ext_pwr == NULL) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	status = fuses_ext_pwr->power->power_up (fuses_ext_pwr->power);
	if (status != 0) {
		return status;
	}

	status = fuses_ext_pwr->fuses->program_rng_calibration (fuses_ext_pwr->fuses, rng_data);

	fuse_controller_ext_power_off (fuses_ext_pwr);

	return status;
}

/**
 * Initialize the driver for interfacing with the ExtPower HSP fuse controller.
 *
 * @param fuses_ext_pwr The ext power fuse driver instance to initialize.
 * @param fuses driver interface of fuse controller.
 * @param power driver interface of power controller.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int fuse_controller_ext_power_init (struct fuse_controller_ext_power *fuses_ext_pwr,
	const struct fuse_controller_interface *fuses, const struct ext_power_interface *power)
{
	if ((fuses_ext_pwr == NULL) || (fuses == NULL) || (power == NULL)) {
		return FUSE_CONTROLLER_INVALID_ARGUMENT;
	}

	memset (fuses_ext_pwr, 0, sizeof (struct fuse_controller_ext_power));

	fuses_ext_pwr->base.get_security_state = fuse_controller_ext_power_get_security_state;
	fuses_ext_pwr->base.change_security_state = fuse_controller_ext_power_change_security_state;
	fuses_ext_pwr->base.read_registered_socid = fuse_controller_ext_power_read_registered_socid;
	fuses_ext_pwr->base.program_socid = fuse_controller_ext_power_program_socid;
	fuses_ext_pwr->base.read_emc_register = fuse_controller_ext_power_read_emc_register;
	fuses_ext_pwr->base.program_emc_register = fuse_controller_ext_power_program_emc_register;
	fuses_ext_pwr->base.read_aeb_register = fuse_controller_ext_power_read_aeb_register;
	fuses_ext_pwr->base.program_aeb_register = fuse_controller_ext_power_program_aeb_register;
	fuses_ext_pwr->base.blank_check = fuse_controller_ext_power_blank_check;
	fuses_ext_pwr->base.blank_check_socid = fuse_controller_ext_power_blank_check_socid;
	fuses_ext_pwr->base.blank_check_key = fuse_controller_ext_power_blank_check_key;
	fuses_ext_pwr->base.get_fuse_map = fuse_controller_ext_power_get_fuse_map;
	fuses_ext_pwr->base.read_sw_fuses = fuse_controller_ext_power_read_sw_fuses;
	fuses_ext_pwr->base.program_sw_fuses = fuse_controller_ext_power_program_sw_fuses;
	fuses_ext_pwr->base.read_registered_sw_fuses =
		fuse_controller_ext_power_read_registered_sw_fuses;
	fuses_ext_pwr->base.program_registered_sw_fuses =
		fuse_controller_ext_power_program_registered_sw_fuses;
	fuses_ext_pwr->base.read_rng_calibration = fuse_controller_ext_power_read_rng_calibration;
	fuses_ext_pwr->base.program_rng_calibration = fuse_controller_ext_power_program_rng_calibration;

	fuses_ext_pwr->fuses = fuses;
	fuses_ext_pwr->power = power;

	return 0;
}

/**
 * Release the resources used by a fuse controller ext power driver.
 *
 * @param fuses_ext_pwr The ext power fuse driver instance.
 */
void fuse_controller_ext_power_release (struct fuse_controller_ext_power *fuses_ext_pwr)
{
	UNUSED (fuses_ext_pwr);
}
