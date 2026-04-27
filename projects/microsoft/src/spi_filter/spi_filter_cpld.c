// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "spi_filter/spi_filter_cpld.h"


/**
 * CPLD version that adds support for specifying the size of the flash device to mask out unused
 * address bits during commands.
 *
 * Before this version, all address were assumed to be valid.
 */
#define	CPLD_SUPPORTS_DEVICE_SIZE				0x22

/**
 * CPLD version that adds support for indicating the address mode cannot be changed.
 *
 * Before this version, address mode was always assumed to be changeable.
 */
#define	CPLD_SUPPORTS_FIXED_ADDRESS_MODE		0x23

/**
 * CPLD version that adds support for requiring write enable before changing the address mode.
 *
 * Before this version, write enable was not required to change the address mode of the filter.
 */
#define	CPLD_SUPPORTS_WRITE_ENABLE_REQUIRED		0x23

/**
 * CPLD version that adds support for setting the address mode of the filter on flash reset.
 *
 * Before this version, the address mode was always set to 3-byte mode on flash reset.
 */
#define	CPLD_SUPPORT_RESET_BYTE_MODE			0x23

/**
 * CPLD version that adds support for reporting the write enable state through the status register.
 */
#define	CPLD_SUPPORTS_WRITE_ENABLE_TRACKING		0x23

/**
 * CPLD version that adds support for additional read/write regions.
 */
#define	CPLD_SUPPORTS_EXTRA_RW_REGIONS			0x26

/**
 * CPLD version that adds support for single flash mode.
 */
#define	CPLD_SUPPORTS_SINGLE_FLASH_MODE			0x26


/**
 * Read a register from the CPLD over I2C
 *
 * @param cpld The control interface to the CPLD to read from.
 * @param cpld_reg_name The register address to read.
 * @param data The buffer to hold the register value.
 *
 * @return Transfer status, 0 if success or an error code.
 */
static int spi_filter_cpld_read_register (struct spi_filter_cpld_control *cpld,
	uint8_t cpld_reg_name, uint8_t *data)
{
	return cpld->i2c->read_reg (cpld->i2c, cpld->slave_addr, (0x80 | cpld_reg_name), 1, data, 1);
}

/**
 * Write to a register on the CPLD over I2C
 *
 * @param cpld The control interface to the CPLD to write to.
 * @param cpld_reg_name The register address to write.
 * @param data The value to write to the register.
 *
 * @return Transfer status, 0 if success or an error code.
 */
static int spi_filter_cpld_write_register (struct spi_filter_cpld_control *cpld,
	uint8_t cpld_reg_name, uint8_t data)
{
	return cpld->i2c->write_reg (cpld->i2c, cpld->slave_addr, cpld_reg_name, 1, &data, 1);
}

/**
 * Check if the SPI filter supports the operation
 *
 * @param control The control interface to the CPLD to check
 * @param min_version The minimum filter version that supports the operation
 *
 * @return 0 if supported or an error code
 */
static int spi_filter_cpld_supported_operation (struct spi_filter_cpld_control *control,
	uint8_t min_version)
{
	uint8_t version;
	int status;

	status = spi_filter_cpld_get_version (control, &version);
	if (status != 0) {
		return status;
	}

	if (version < min_version) {
		return SPI_FILTER_UNSUPPORTED_OPERATION;
	}

	return status;
}

/**
 * Write a bitmask to a SPI filter register.  No lock will be taken during the read/modify/write
 * operation.
 *
 * @param control The control interface to the CPLD
 * @param bit The bitmask value to write to the register
 * @param reg_name The name of the register to set
 * @param mask The mask to use for clearing existing register state
 *
 * @return 0 if register write is successful or an error code
 */
static int spi_filter_cpld_write_register_bitmask_no_lock (struct spi_filter_cpld_control *control,
	uint8_t bit, uint8_t reg_name, uint8_t mask)
{
	uint8_t reg;
	int status;

	status = spi_filter_cpld_read_register (control, reg_name, &reg);
	if (status != 0) {
		return status;
	}

	reg &= ~mask;
	reg |= bit;

	return spi_filter_cpld_write_register (control, reg_name, reg);
}

/**
 * Write a bitmask to a SPI filter register.
 *
 * @param control The control interface to the CPLD
 * @param bit The bitmask value to write to the register
 * @param reg_name The name of the register to set
 * @param mask The mask to use for clearing existing register state
 * @param loc The bit location in the register for the value being written
 *
 * @return 0 if register write is successful or an error code
 */
static int spi_filter_cpld_write_register_bit (struct spi_filter_cpld_control *control,	uint8_t bit,
	uint8_t reg_name, uint8_t mask, int loc)
{
	int status;

	platform_mutex_lock (&control->lock);
	status = spi_filter_cpld_write_register_bitmask_no_lock (control, bit << loc, reg_name, mask);
	platform_mutex_unlock (&control->lock);

	return status;
}

/**
 * Get a bitmask value from a SPI filter register.
 *
 * @param control The control interface to the CPLD
 * @param bit Output bit value that is read
 * @param reg_name The name of the register to read
 * @param mask The mask to use to retrieve the register bit
 * @param loc The bit location of value to read
 *
 * @return 0 if register write is successful or an error code
 */
static int spi_filter_cpld_read_register_bit (struct spi_filter_cpld_control *control, uint8_t *bit,
	uint8_t reg_name, uint8_t mask, int loc)
{
	int status;

	platform_mutex_lock (&control->lock);

	status = spi_filter_cpld_read_register (control, reg_name, bit);
	if (status != 0) {
		goto exit;
	}

	*bit = (*bit & mask) >> loc;

exit:
	platform_mutex_unlock (&control->lock);

	return status;
}

/**
 * Get the SPI filter version running on the device.
 *
 * @param control The control interface to the CPLD.
 * @param version Output for the filter version.
 *
 * @return 0 if the version read was successful or an error code.
 */
static int spi_filter_cpld_determine_filter_version (struct spi_filter_cpld_control *control,
	uint8_t *version)
{
	int status;

	status = spi_filter_cpld_read_register (control, CPLD_VERSION, version);
	if (status != 0) {
		return status;
	}

	return ((*version != 0) && (*version != 0xff)) ? 0 : SPI_FILTER_UNKNOWN_VERSION;
}

static int spi_filter_cpld_get_port (const struct spi_filter_interface *filter)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	return (cpld->port == CPLD_FLASH_PORT_0) ? 0 : 1;
}

static int spi_filter_cpld_get_mfg_id (const struct spi_filter_interface *filter, uint8_t *mfg_id)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	int status;

	if ((cpld == NULL) || (mfg_id == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&cpld->control->lock);

	status = spi_filter_cpld_read_register (cpld->control, CPLD_FLASH_MFG_ID, mfg_id);
	if (status != 0) {
		goto exit;
	}

	*mfg_id = (*mfg_id >> (4 * cpld->port)) & 0x0F;

exit:
	platform_mutex_unlock (&cpld->control->lock);

	return status;
}

static int spi_filter_cpld_set_mfg_id (const struct spi_filter_interface *filter, uint8_t mfg_id)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t shift;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	shift = cpld->port * 4;

	return spi_filter_cpld_write_register_bit (cpld->control, mfg_id, CPLD_FLASH_MFG_ID,
		(0x0F << shift), shift);
}

static int spi_filter_cpld_get_flash_size (const struct spi_filter_interface *filter,
	uint32_t *bytes)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg;
	uint8_t version;
	int status;

	if ((cpld == NULL) || (bytes == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_get_version (cpld->control, &version);
	if (status != 0) {
		return status;
	}

	if (version < CPLD_SUPPORTS_DEVICE_SIZE) {
		*bytes = SPI_FILTER_MAX_FLASH_SIZE;

		return 0;
	}

	platform_mutex_lock (&cpld->control->lock);

	status = spi_filter_cpld_read_register (cpld->control,
		CPLD_P0_MAX_ADDR_MASK_LOWER_LSB + (cpld->port * 2), &reg);
	if (status != 0) {
		goto exit;
	}

	*bytes = reg;

	status = spi_filter_cpld_read_register (cpld->control,
		CPLD_P0_MAX_ADDR_MASK_UPPER_MSB + (cpld->port * 2), &reg);
	if (status != 0) {
		goto exit;
	}

	*bytes |= (reg << 8);
	*bytes = (*bytes + 1) << 16;

exit:
	platform_mutex_unlock (&cpld->control->lock);

	return status;
}

static int spi_filter_cpld_set_flash_size (const struct spi_filter_interface *filter,
	uint32_t bytes)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	int status;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_supported_operation (cpld->control, CPLD_SUPPORTS_DEVICE_SIZE);
	if (status != 0) {
		return status;
	}

	platform_mutex_lock (&cpld->control->lock);

	status = spi_filter_cpld_write_register (cpld->control,
		CPLD_P0_MAX_ADDR_MASK_LOWER_LSB + (cpld->port * 2), (bytes - 1) >> 16);
	if (status != 0) {
		goto exit;
	}

	status = spi_filter_cpld_write_register (cpld->control,
		CPLD_P0_MAX_ADDR_MASK_UPPER_MSB + (cpld->port * 2), (bytes - 1) >> 24);

exit:
	platform_mutex_unlock (&cpld->control->lock);

	return status;
}

static int spi_filter_cpld_get_filter_mode (const struct spi_filter_interface *filter,
	spi_filter_flash_mode *mode)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t version;
	uint8_t bypass;
	uint8_t single;
	uint8_t bypass_mask;
	uint8_t bypass_cs_mask;
	uint8_t single_reg;
	int status;

	if ((cpld == NULL) || (mode == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	if (cpld->port == 0) {
		bypass_mask = CPLD_TEST_CTRL_BYPASS_P0_MASK;
		bypass_cs_mask = CPLD_TEST_CTRL_BYPASS_P0_CS_MASK;
		single_reg = CPLD_P0_SINGLE_FLASH_CTRL;
	}
	else {
		bypass_mask = CPLD_TEST_CTRL_BYPASS_P1_MASK;
		bypass_cs_mask = CPLD_TEST_CTRL_BYPASS_P1_CS_MASK;
		single_reg = CPLD_P1_SINGLE_FLASH_CTRL;
	}

	platform_mutex_lock (&cpld->control->lock);

	status = spi_filter_cpld_read_register (cpld->control, CPLD_TEST_CTRL, &bypass);
	if (status != 0) {
		goto exit;
	}

	if (bypass & bypass_mask) {
		if (bypass & bypass_cs_mask) {
			*mode = SPI_FILTER_FLASH_BYPASS_CS1;
		}
		else {
			*mode = SPI_FILTER_FLASH_BYPASS_CS0;
		}
	}
	else {
		status = spi_filter_cpld_determine_filter_version (cpld->control, &version);
		if (status != 0) {
			goto exit;
		}

		if (version >= CPLD_SUPPORTS_SINGLE_FLASH_MODE) {
			status = spi_filter_cpld_read_register (cpld->control, single_reg, &single);
			if (status != 0) {
				goto exit;
			}

			if (single & CPLD_SINGLE_FLASH_MODE_MASK) {
				if (single & CPLD_SINGLE_FLASH_CS_MASK) {
					*mode = SPI_FILTER_FLASH_SINGLE_CS1;
				}
				else {
					*mode = SPI_FILTER_FLASH_SINGLE_CS0;
				}
			}
			else {
				*mode = SPI_FILTER_FLASH_DUAL;
			}
		}
		else {
			*mode = SPI_FILTER_FLASH_DUAL;
		}
	}

exit:
	platform_mutex_unlock (&cpld->control->lock);

	return status;
}

static int spi_filter_cpld_set_filter_mode (const struct spi_filter_interface *filter,
	spi_filter_flash_mode mode)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t version;
	uint8_t bypass_mask;
	uint8_t bypass_cs_mask;
	uint8_t bypass_val;
	uint8_t single_reg;
	uint8_t single_val;
	int status;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	if (cpld->port == 0) {
		bypass_mask = CPLD_TEST_CTRL_BYPASS_P0_MASK;
		bypass_cs_mask = CPLD_TEST_CTRL_BYPASS_P0_CS_MASK;
		single_reg = CPLD_P0_SINGLE_FLASH_CTRL;
	}
	else {
		bypass_mask = CPLD_TEST_CTRL_BYPASS_P1_MASK;
		bypass_cs_mask = CPLD_TEST_CTRL_BYPASS_P1_CS_MASK;
		single_reg = CPLD_P1_SINGLE_FLASH_CTRL;
	}

	switch (mode) {
		case SPI_FILTER_FLASH_DUAL:
			bypass_val = 0;
			single_val = 0;
			break;

		case SPI_FILTER_FLASH_BYPASS_CS0:
			bypass_val = bypass_mask;
			single_val = 0;
			break;

		case SPI_FILTER_FLASH_BYPASS_CS1:
			bypass_val = bypass_mask | bypass_cs_mask;
			single_val = 0;
			break;

		case SPI_FILTER_FLASH_SINGLE_CS0:
			bypass_val = 0;
			single_val = CPLD_SINGLE_FLASH_MODE_MASK;
			break;

		case SPI_FILTER_FLASH_SINGLE_CS1:
			bypass_val = 0;
			single_val = CPLD_SINGLE_FLASH_MODE_MASK | CPLD_SINGLE_FLASH_CS_MASK;
			break;

		default:
			return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_get_version (cpld->control, &version);
	if (status != 0) {
		return status;
	}

	if ((version < CPLD_SUPPORTS_SINGLE_FLASH_MODE) && (single_val != 0)) {
		return SPI_FILTER_UNSUPPORTED_OPERATION;
	}

	platform_mutex_lock (&cpld->control->lock);

	status = spi_filter_cpld_write_register_bitmask_no_lock (cpld->control, bypass_val,
		CPLD_TEST_CTRL, bypass_mask | bypass_cs_mask);
	if (status != 0) {
		goto exit;
	}

	if (version >= CPLD_SUPPORTS_SINGLE_FLASH_MODE) {
		status = spi_filter_cpld_write_register_bitmask_no_lock (cpld->control, single_val,
			single_reg, CPLD_SINGLE_FLASH_MODE_MASK | CPLD_SINGLE_FLASH_CS_MASK);
	}

exit:
	platform_mutex_unlock (&cpld->control->lock);

	return status;
}

static int spi_filter_cpld_get_filter_enabled (const struct spi_filter_interface *filter,
	bool *enabled)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	uint8_t reg;
	int status;

	if ((cpld == NULL) || (enabled == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	platform_mutex_lock (&cpld->control->lock);

	status = spi_filter_cpld_read_register (cpld->control, reg_name, &reg);
	if (status != 0) {
		goto exit;
	}

	*enabled = !!(reg & CPLD_FILTER_CTRL_ENABLE_MASK);

exit:
	platform_mutex_unlock (&cpld->control->lock);

	return status;
}

static int spi_filter_cpld_enable_filter (const struct spi_filter_interface *filter, bool enable)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	return spi_filter_cpld_write_register_bit (cpld->control, (uint8_t) enable, reg_name,
		CPLD_FILTER_CTRL_ENABLE_MASK, CPLD_FILTER_CTRL_ENABLE_SHIFT);
}

static int spi_filter_cpld_get_ro_cs (const struct spi_filter_interface *filter,
	spi_filter_cs *act_sel)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	uint8_t bit;
	int status;

	if ((cpld == NULL) || (act_sel == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	status = spi_filter_cpld_read_register_bit (cpld->control, &bit, reg_name,
		CPLD_FILTER_CTRL_ACV_CS_MASK, CPLD_FILTER_CTRL_ACV_CS_SHIFT);
	if (status == 0) {
		*act_sel = (spi_filter_cs) bit;
	}

	return status;
}

static int spi_filter_cpld_set_ro_cs (const struct spi_filter_interface *filter,
	spi_filter_cs act_sel)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;

	if ((cpld == NULL) || (act_sel < SPI_FILTER_CS_0) || (act_sel > SPI_FILTER_CS_1)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	return spi_filter_cpld_write_register_bit (cpld->control, (uint8_t) act_sel, reg_name,
		CPLD_FILTER_CTRL_ACV_CS_MASK, CPLD_FILTER_CTRL_ACV_CS_SHIFT);
}

/**
 * Determine the current address mode of the SPI filter.  This is the same functionality as
 * {@link struct spi_filter_interface.get_addr_byte_mode}.
 *
 * @param cpld The CPLD control interface.
 * @param port The filter port to query.
 * @param mode Output for the address mode of the filter.
 *
 * @return 0 if the address mode was read successfully or an error code.
 */
int spi_filter_cpld_get_address_mode (struct spi_filter_cpld_control *cpld, cpld_flash_port port,
	spi_filter_address_mode *mode)
{
	uint8_t reg_name;
	uint8_t bit;
	int status;

	if ((cpld == NULL) || (mode == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	switch (port) {
		case CPLD_FLASH_PORT_0:
			reg_name = CPLD_P0_STATUS;
			break;

		case CPLD_FLASH_PORT_1:
			reg_name = CPLD_P1_STATUS;
			break;

		default:
			return SPI_FILTER_UNSUPPORTED_PORT;
	}

	status = spi_filter_cpld_read_register_bit (cpld, &bit, reg_name, CPLD_STATUS_ADDR_MODE_MASK,
		CPLD_STATUS_ADDR_MODE_SHIFT);
	if (status == 0) {
		*mode = (spi_filter_address_mode) bit;
	}

	return status;
}

static int spi_filter_cpld_get_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode *mode)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	return spi_filter_cpld_get_address_mode (cpld->control, (cpld_flash_port) cpld->port, mode);
}

static int spi_filter_cpld_get_fixed_addr_byte_mode (const struct spi_filter_interface *filter,
	bool *fixed)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	uint8_t bit;
	int status;

	if ((cpld == NULL) || (fixed == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	status = spi_filter_cpld_read_register_bit (cpld->control, &bit, reg_name,
		CPLD_FILTER_CTRL_ADDR_MODE_MASK, CPLD_FILTER_CTRL_ADDR_MODE_SHIFT);
	if (status == 0) {
		*fixed = bit;
	}

	return status;
}

static int spi_filter_cpld_set_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode mode)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t ctrl_reg;
	uint8_t reg_name;
	uint8_t mask = CPLD_FILTER_CTRL_ADDR_MODE_MASK | CPLD_FILTER_CTRL_ADDR_SEL_MASK;
	int status;

	if ((cpld == NULL) || (mode < SPI_FILTER_ADDRESS_MODE_3) ||
		(mode > SPI_FILTER_ADDRESS_MODE_4)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_supported_operation (cpld->control, CPLD_SUPPORTS_FIXED_ADDRESS_MODE);
	if (status == 0) {
		mask |= CPLD_FILTER_CTRL_FIXED_ADDR_BYTE_MODE_MASK;
	}
	else if (status != SPI_FILTER_UNSUPPORTED_OPERATION) {
		return status;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	platform_mutex_lock (&cpld->control->lock);

	status = spi_filter_cpld_read_register (cpld->control, reg_name, &ctrl_reg);
	if (status != 0) {
		goto exit;
	}

	ctrl_reg &= ~mask;
	ctrl_reg |= (CPLD_SEL_MODE_OVERRIDE << CPLD_FILTER_CTRL_ADDR_MODE_SHIFT) |
		(((uint8_t) mode) << CPLD_FILTER_CTRL_ADDR_SEL_SHIFT);

	status = spi_filter_cpld_write_register (cpld->control, reg_name, ctrl_reg);
	if (status != 0) {
		goto exit;
	}

	ctrl_reg &= ~CPLD_FILTER_CTRL_ADDR_MODE_MASK;

	status = spi_filter_cpld_write_register (cpld->control, reg_name, ctrl_reg);

exit:
	platform_mutex_unlock (&cpld->control->lock);

	return status;
}

static int spi_filter_cpld_set_fixed_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode mode)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	uint8_t cfg = 0;
	uint8_t mask = CPLD_FILTER_CTRL_ADDR_MODE_MASK | CPLD_FILTER_CTRL_ADDR_SEL_MASK;
	int status;

	if ((cpld == NULL) || (mode < SPI_FILTER_ADDRESS_MODE_3) ||
		(mode > SPI_FILTER_ADDRESS_MODE_4)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_supported_operation (cpld->control, CPLD_SUPPORTS_FIXED_ADDRESS_MODE);
	if (status == 0) {
		cfg = CPLD_FILTER_CTRL_FIXED_ADDR_BYTE_MODE_MASK;
		mask |= CPLD_FILTER_CTRL_FIXED_ADDR_BYTE_MODE_MASK;
	}
	else if (status != SPI_FILTER_UNSUPPORTED_OPERATION) {
		return status;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;
	cfg |= (mode << CPLD_FILTER_CTRL_ADDR_SEL_SHIFT) |
		(CPLD_SEL_MODE_OVERRIDE << CPLD_FILTER_CTRL_ADDR_MODE_SHIFT);

	return spi_filter_cpld_write_register_bit (cpld->control, cfg, reg_name, mask, 0);
}

static int spi_filter_cpld_require_addr_byte_mode_write_enable (
	const struct spi_filter_interface *filter, bool require)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	int status;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_supported_operation (cpld->control,
		CPLD_SUPPORTS_WRITE_ENABLE_REQUIRED);
	if (status != 0) {
		return status;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	return spi_filter_cpld_write_register_bit (cpld->control, (uint8_t) require, reg_name,
		CPLD_FILTER_CTRL_ADDR_BYTE_MODE_WE_MASK, CPLD_FILTER_CTRL_ADDR_BYTE_MODE_WE_SHIFT);
}

static int spi_filter_cpld_get_addr_byte_mode_write_enable_required (
	const struct spi_filter_interface *filter, bool *required)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	uint8_t bit;
	uint8_t version;
	int status;

	if ((cpld == NULL) || (required == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_get_version (cpld->control, &version);
	if (status != 0) {
		return status;
	}

	if (version < CPLD_SUPPORTS_WRITE_ENABLE_REQUIRED) {
		*required = false;

		return 0;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	status = spi_filter_cpld_read_register_bit (cpld->control, &bit, reg_name,
		CPLD_FILTER_CTRL_ADDR_BYTE_MODE_WE_MASK, CPLD_FILTER_CTRL_ADDR_BYTE_MODE_WE_SHIFT);
	if (status == 0) {
		*required = bit;
	}

	return status;
}

static int spi_filter_cpld_get_reset_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode *mode)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	uint8_t bit;
	uint8_t version;
	int status;

	if ((cpld == NULL) || (mode == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_get_version (cpld->control, &version);
	if (status != 0) {
		return status;
	}

	if (version < CPLD_SUPPORT_RESET_BYTE_MODE) {
		*mode = SPI_FILTER_ADDRESS_MODE_3;

		return 0;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	status = spi_filter_cpld_read_register_bit (cpld->control, &bit, reg_name,
		CPLD_FILTER_CTRL_RESET_ADDR_BYTE_MODE_MASK, CPLD_FILTER_CTRL_RESET_ADDR_BYTE_MODE_SHIFT);
	if (status == 0) {
		*mode = (spi_filter_address_mode) bit;
	}

	return status;
}

static int spi_filter_cpld_set_reset_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode mode)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	int status;

	if ((cpld == NULL) || (mode < SPI_FILTER_ADDRESS_MODE_3) ||
		(mode > SPI_FILTER_ADDRESS_MODE_4)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_supported_operation (cpld->control, CPLD_SUPPORT_RESET_BYTE_MODE);
	if (status != 0) {
		return status;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	return spi_filter_cpld_write_register_bit (cpld->control, (uint8_t) mode, reg_name,
		CPLD_FILTER_CTRL_RESET_ADDR_BYTE_MODE_MASK, CPLD_FILTER_CTRL_RESET_ADDR_BYTE_MODE_SHIFT);
}

static int spi_filter_cpld_are_all_single_flash_writes_allowed (
	const struct spi_filter_interface *filter, bool *allowed)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t version;
	uint8_t bit;
	uint8_t reg_name;
	int status;

	if ((cpld == NULL) || (allowed == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_SINGLE_FLASH_CTRL : CPLD_P1_SINGLE_FLASH_CTRL;

	status = spi_filter_cpld_get_version (cpld->control, &version);
	if (status != 0) {
		return status;
	}

	if (version < CPLD_SUPPORTS_SINGLE_FLASH_MODE) {
		*allowed = false;
	}
	else {
		status = spi_filter_cpld_read_register_bit (cpld->control, &bit, reg_name,
			CPLD_SINGLE_FLASH_ALLOW_WRITE_MASK, CPLD_SINGLE_FLASH_ALLOW_WRITE_SHIFT);
		if (status == 0) {
			*allowed = bit;
		}
	}

	return status;
}

static int spi_filter_cpld_allow_all_single_flash_writes (const struct spi_filter_interface *filter,
	bool allowed)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	int status;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_SINGLE_FLASH_CTRL : CPLD_P1_SINGLE_FLASH_CTRL;

	status = spi_filter_cpld_supported_operation (cpld->control, CPLD_SUPPORTS_SINGLE_FLASH_MODE);
	if (status != 0) {
		return status;
	}

	return spi_filter_cpld_write_register_bit (cpld->control, (uint8_t) allowed, reg_name,
		CPLD_SINGLE_FLASH_ALLOW_WRITE_MASK, CPLD_SINGLE_FLASH_ALLOW_WRITE_SHIFT);
}

static int spi_filter_cpld_get_write_enable_detected (const struct spi_filter_interface *filter,
	bool *detected)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	uint8_t bit;
	int status;

	if ((cpld == NULL) || (detected == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_supported_operation (cpld->control,
		CPLD_SUPPORTS_WRITE_ENABLE_TRACKING);
	if (status != 0) {
		return status;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_STATUS : CPLD_P1_STATUS;

	status = spi_filter_cpld_read_register_bit (cpld->control, &bit, reg_name,
		CPLD_STATUS_WRITE_ENABLE_DETECTED_MASK, CPLD_STATUS_WRITE_ENABLE_DETECTED_SHIFT);
	if (status == 0) {
		*detected = bit;
	}

	return status;
}

static int spi_filter_cpld_get_flash_dirty_state (const struct spi_filter_interface *filter,
	spi_filter_flash_state *state)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t reg_name;
	uint8_t bit;
	int status;

	if ((cpld == NULL) || (state == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_STATUS : CPLD_P1_STATUS;

	status = spi_filter_cpld_read_register_bit (cpld->control, &bit, reg_name,
		CPLD_STATUS_FLASH_DIRTY_MASK, CPLD_STATUS_FLASH_DIRTY_SHIFT);
	if (status == 0) {
		*state = (spi_filter_flash_state) bit;
	}

	return status;
}

static int spi_filter_cpld_clear_flash_dirty_state (const struct spi_filter_interface *filter)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t ctrl_reg;
	uint8_t reg_name;
	int status;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	reg_name = (cpld->port == 0) ? CPLD_P0_FILTER_CTRL : CPLD_P1_FILTER_CTRL;

	platform_mutex_lock (&cpld->control->lock);

	status = spi_filter_cpld_read_register (cpld->control, reg_name, &ctrl_reg);
	if (status != 0) {
		goto exit;
	}

	ctrl_reg |= CPLD_FILTER_CTRL_FLASH_DIRTY_MASK;

	status = spi_filter_cpld_write_register (cpld->control, reg_name, ctrl_reg);

exit:
	platform_mutex_unlock (&cpld->control->lock);

	return status;
}

static int spi_filter_cpld_get_filter_rw_region (const struct spi_filter_interface *filter,
	uint8_t region, uint32_t *start_addr, uint32_t *end_addr)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t version;
	uint8_t reg_name;
	uint8_t addr;
	int status;

	if ((cpld == NULL) || (start_addr == NULL) || (end_addr == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	if ((region < CPLD_MIN_FILTER_REGION) || (region > CPLD_NUM_FILTER_REGIONS_EXTENDED)) {
		return SPI_FILTER_UNSUPPORTED_RW_REGION;
	}

	if (region < 4) {
		reg_name = (cpld->port == 0) ?
				CPLD_P0_FILTER_REGION_1_LOWER_MSB : CPLD_P1_FILTER_REGION_1_LOWER_MSB;
		reg_name += (4 * (region - 1));
	}
	else {
		reg_name = (cpld->port == 0) ?
				CPLD_P0_FILTER_REGION_4_LOWER_MSB : CPLD_P1_FILTER_REGION_4_LOWER_MSB;
		reg_name += (4 * (region - 4));
	}

	status = spi_filter_cpld_get_version (cpld->control, &version);
	if (status != 0) {
		return status;
	}

	if ((version < CPLD_SUPPORTS_EXTRA_RW_REGIONS) && (region > CPLD_NUM_FILTER_REGIONS)) {
		*start_addr = 0;
		*end_addr = 0;

		return 0;
	}

	platform_mutex_lock (&cpld->control->lock);

	status = spi_filter_cpld_read_register (cpld->control, reg_name, &addr);
	if (status != 0) {
		goto exit;
	}

	*start_addr = addr << 24;

	status = spi_filter_cpld_read_register (cpld->control, reg_name + 1, &addr);
	if (status != 0) {
		goto exit;
	}

	*start_addr |= (addr << 16);

	status = spi_filter_cpld_read_register (cpld->control, reg_name + 2, &addr);
	if (status != 0) {
		goto exit;
	}

	*end_addr = addr << 24;

	status = spi_filter_cpld_read_register (cpld->control, reg_name + 3, &addr);
	if (status != 0) {
		goto exit;
	}

	*end_addr |= (addr << 16);

exit:
	platform_mutex_unlock (&cpld->control->lock);

	return status;
}

/**
 * Configure a single read/write filter region.  No parameter validation will be performed.
 *
 * @param cpld The SPI filter to update.
 * @param region The read/write region to configure.
 * @param start_addr The starting address for the filter region.
 * @param end_addr The ending address for the filter region.
 *
 * @return 0 if the region was configured successfully or an error code.
 */
static int spi_filter_cpld_configure_rw_region (const struct spi_filter_cpld *cpld, uint8_t region,
	uint32_t start_addr, uint32_t end_addr)
{
	uint8_t reg_name;
	uint8_t addr;
	int status;

	if (region < 4) {
		reg_name = (cpld->port == 0) ?
				CPLD_P0_FILTER_REGION_1_LOWER_MSB : CPLD_P1_FILTER_REGION_1_LOWER_MSB;
		reg_name += (4 * (region - 1));
	}
	else {
		reg_name = (cpld->port == 0) ?
				CPLD_P0_FILTER_REGION_4_LOWER_MSB : CPLD_P1_FILTER_REGION_4_LOWER_MSB;
		reg_name += (4 * (region - 4));
	}

	addr = (start_addr >> 24) & 0xFF;

	platform_mutex_lock (&cpld->control->lock);

	status = spi_filter_cpld_write_register (cpld->control, reg_name, addr);
	if (status != 0) {
		goto exit;
	}

	addr = (start_addr >> 16) & 0xFF;

	status = spi_filter_cpld_write_register (cpld->control, reg_name + 1, addr);
	if (status != 0) {
		goto exit;
	}

	addr = (end_addr >> 24) & 0xFF;

	status = spi_filter_cpld_write_register (cpld->control, reg_name + 2, addr);
	if (status != 0) {
		goto exit;
	}

	addr = (end_addr >> 16) & 0xFF;

	status = spi_filter_cpld_write_register (cpld->control, reg_name + 3, addr);

exit:
	platform_mutex_unlock (&cpld->control->lock);

	return status;
}

static int spi_filter_cpld_set_filter_rw_region (const struct spi_filter_interface *filter,
	uint8_t region, uint32_t start_addr, uint32_t end_addr)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t version;
	int status;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	if (((start_addr & 0x0000FFFF) != 0) || ((end_addr & 0x0000FFFF) != 0)) {
		return SPI_FILTER_MISALIGNED_ADDRESS;
	}

	if ((end_addr != 0) && (end_addr < start_addr)) {
		return SPI_FILTER_INVALID_ADDR_RANGE;
	}

	status = spi_filter_cpld_get_version (cpld->control, &version);
	if (status != 0) {
		return status;
	}

	if ((region < CPLD_MIN_FILTER_REGION) || (region > CPLD_NUM_FILTER_REGIONS_EXTENDED) ||
		((version < CPLD_SUPPORTS_EXTRA_RW_REGIONS) && (region > CPLD_NUM_FILTER_REGIONS))) {
		return SPI_FILTER_UNSUPPORTED_RW_REGION;
	}

	return spi_filter_cpld_configure_rw_region (cpld, region, start_addr, end_addr);
}

static int spi_filter_cpld_clear_filter_rw_regions (const struct spi_filter_interface *filter)
{
	const struct spi_filter_cpld *cpld = (const struct spi_filter_cpld*) filter;
	uint8_t version;
	int i;
	int max_region;
	int status;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	status = spi_filter_cpld_get_version (cpld->control, &version);
	if (status != 0) {
		return status;
	}

	max_region = (version < CPLD_SUPPORTS_EXTRA_RW_REGIONS) ?
			CPLD_NUM_FILTER_REGIONS : CPLD_NUM_FILTER_REGIONS_EXTENDED;

	for (i = CPLD_MIN_FILTER_REGION; i <= max_region; i++) {
		status = spi_filter_cpld_configure_rw_region (cpld, i, 0, 0);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}

/**
 * Initialize the global configuration interface to the CPLD.
 *
 * @param cpld The CPLD control interface to initialize.
 * @param i2c The I2C bus connected to the CPLD.
 * @param slave_addr The 7-bit slave address of the CPLD.
 *
 * @return 0 if initialization success or an error code.
 */
int spi_filter_cpld_control_init (struct spi_filter_cpld_control *cpld,
	struct i2c_master_interface *i2c, uint8_t slave_addr)
{
	int status;

	if ((cpld == NULL) || (i2c == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	memset (cpld, 0, sizeof (struct spi_filter_cpld_control));

	status = platform_mutex_init (&cpld->lock);
	if (status != 0) {
		return status;
	}

	cpld->i2c = i2c;
	cpld->slave_addr = slave_addr;

	return 0;
}

/**
 * Release the resources used by the CPLD global configuration interface.
 *
 * @param cpld The CPLD control interface to release.
 */
void spi_filter_cpld_control_deinit (struct spi_filter_cpld_control *cpld)
{
	if (cpld) {
		platform_mutex_free (&cpld->lock);
	}
}

/**
 * Enable or disable access to the CPLD SPI filter registers.  While register access is disabled,
 * any tasks attempting to access CPLD registers will be suspended until register access is enabled.
 *
 * @param cpld The CPLD to instance to update.
 * @param block Flag indicating if register access should be blocked (true) or allowed (false).
 */
void spi_filter_cpld_block_register_access (struct spi_filter_cpld_control *cpld, bool block)
{
	if (cpld) {
		if (block) {
			platform_mutex_lock (&cpld->lock);
		}
		else {
			platform_mutex_unlock (&cpld->lock);
		}
	}
}

/**
 * Get CPLD FW version
 *
 * @param cpld The CPLD instance to use
 * @param version The version buffer to fill
 *
 * @return Completion status, 0 if success or an error code.
 */
int spi_filter_cpld_get_version (struct spi_filter_cpld_control *cpld, uint8_t *version)
{
	int status;

	if ((cpld == NULL) || (version == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&cpld->lock);
	status = spi_filter_cpld_determine_filter_version (cpld, version);
	platform_mutex_unlock (&cpld->lock);

	return status;
}

/**
 * Get CPLD SPI interrupt status
 *
 * @param cpld The CPLD instance to use
 * @param int_status The status buffer to fill
 *
 * @return Completion status, 0 if success or an error code.
 */
int spi_filter_cpld_get_int_status (struct spi_filter_cpld_control *cpld, uint8_t *int_status)
{
	int status;

	if ((cpld == NULL) || (int_status == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&cpld->lock);
	status = spi_filter_cpld_read_register (cpld, CPLD_SPI_INT, int_status);
	platform_mutex_unlock (&cpld->lock);

	return status;
}

/**
 * Get CPLD SPI interrupt mask register
 *
 * @param cpld The CPLD instance to use
 * @param mask The mask buffer to fill
 *
 * @return Completion status, 0 if success or an error code.
 */
int spi_filter_cpld_get_int_mask (struct spi_filter_cpld_control *cpld, uint8_t *mask)
{
	int status;

	if ((cpld == NULL) || (mask == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&cpld->lock);
	status = spi_filter_cpld_read_register (cpld, CPLD_SPI_INT_MASK, mask);
	platform_mutex_unlock (&cpld->lock);

	return status;
}

/**
 * Set CPLD SPI interrupt mask register
 *
 * @param cpld The CPLD instance to use
 * @param mask Mask to write
 *
 * @return Completion status, 0 if success or an error code.
 */
int spi_filter_cpld_set_int_mask (struct spi_filter_cpld_control *cpld, uint8_t mask)
{
	int status;

	if (cpld == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&cpld->lock);
	status = spi_filter_cpld_write_register (cpld, CPLD_SPI_INT_MASK, mask);
	platform_mutex_unlock (&cpld->lock);

	return status;
}

/**
 * Get the last opcode that was blocked by the SPI filter.
 *
 * @param cpld The CPLD control interface.
 * @param port The filter port to query.
 * @param opcode Output for the blocked opcode.
 *
 * @return 0 if the opcode was read successfully or an error code.
 */
int spi_filter_cpld_get_blocked_opcode (struct spi_filter_cpld_control *cpld, cpld_flash_port port,
	uint8_t *opcode)
{
	int status;

	if ((cpld == NULL) || (opcode == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	if ((port < CPLD_FLASH_PORT_0) || (port > CPLD_FLASH_PORT_1)) {
		return SPI_FILTER_UNSUPPORTED_PORT;
	}

	platform_mutex_lock (&cpld->lock);
	status = spi_filter_cpld_read_register (cpld, CPLD_P0_BLOCKED_OPCODE + port, opcode);
	platform_mutex_unlock (&cpld->lock);

	return status;
}

/**
 * Initialize a SPI filter instance in a CPLD.
 *
 * @param cpld The CPLD that contains the SPI filter.
 * @param filter The SPI filter instance to initialize.
 * @param port The SPI filter instance identifier.
 *
 * @return Initialization status, 0 if success or an error code.
 */
int spi_filter_cpld_init (struct spi_filter_cpld *filter, struct spi_filter_cpld_control *cpld,
	cpld_flash_port port)
{
	if ((cpld == NULL) || (filter == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	if ((port < CPLD_FLASH_PORT_0) || (port >= NUM_CPLD_FLASH_PORTS)) {
		return SPI_FILTER_UNSUPPORTED_PORT;
	}

	memset (filter, 0, sizeof (struct spi_filter_cpld));

	filter->control = cpld;
	filter->port = (uint8_t) port;

	filter->base.get_port = spi_filter_cpld_get_port;
	filter->base.get_mfg_id = spi_filter_cpld_get_mfg_id;
	filter->base.set_mfg_id = spi_filter_cpld_set_mfg_id;
	filter->base.get_flash_size = spi_filter_cpld_get_flash_size;
	filter->base.set_flash_size = spi_filter_cpld_set_flash_size;
	filter->base.get_filter_mode = spi_filter_cpld_get_filter_mode;
	filter->base.set_filter_mode = spi_filter_cpld_set_filter_mode;
	filter->base.get_filter_enabled = spi_filter_cpld_get_filter_enabled;
	filter->base.enable_filter = spi_filter_cpld_enable_filter;
	filter->base.get_ro_cs = spi_filter_cpld_get_ro_cs;
	filter->base.set_ro_cs = spi_filter_cpld_set_ro_cs;
	filter->base.get_addr_byte_mode = spi_filter_cpld_get_addr_byte_mode;
	filter->base.get_fixed_addr_byte_mode = spi_filter_cpld_get_fixed_addr_byte_mode;
	filter->base.set_addr_byte_mode = spi_filter_cpld_set_addr_byte_mode;
	filter->base.set_fixed_addr_byte_mode = spi_filter_cpld_set_fixed_addr_byte_mode;
	filter->base.get_addr_byte_mode_write_enable_required =
		spi_filter_cpld_get_addr_byte_mode_write_enable_required;
	filter->base.require_addr_byte_mode_write_enable =
		spi_filter_cpld_require_addr_byte_mode_write_enable;
	filter->base.get_reset_addr_byte_mode = spi_filter_cpld_get_reset_addr_byte_mode;
	filter->base.set_reset_addr_byte_mode = spi_filter_cpld_set_reset_addr_byte_mode;
	filter->base.are_all_single_flash_writes_allowed =
		spi_filter_cpld_are_all_single_flash_writes_allowed;
	filter->base.allow_all_single_flash_writes = spi_filter_cpld_allow_all_single_flash_writes;
	filter->base.get_write_enable_detected = spi_filter_cpld_get_write_enable_detected;
	filter->base.get_flash_dirty_state = spi_filter_cpld_get_flash_dirty_state;
	filter->base.clear_flash_dirty_state = spi_filter_cpld_clear_flash_dirty_state;
	filter->base.get_filter_rw_region = spi_filter_cpld_get_filter_rw_region;
	filter->base.set_filter_rw_region = spi_filter_cpld_set_filter_rw_region;
	filter->base.clear_filter_rw_regions = spi_filter_cpld_clear_filter_rw_regions;

	return 0;
}

/**
 * Release the resources used by a CPLD SPI filter.
 *
 * @param cpld The CPLD SPI filter instance to release.
 */
void spi_filter_cpld_deinit (struct spi_filter_cpld *cpld)
{
	if (cpld != NULL) {
		memset (cpld, 0, sizeof (struct spi_filter_cpld));
	}
}
