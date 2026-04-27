// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "common/array_size.h"
#include "common/unused.h"
#include "spi_filter/spi_filter_hsp.h"


/**
 * Helper macro to translate control register bits.
 */
#define	SPI_FILTER_HSP_CTRL_BIT(x)          \
	CREG_REGS_SPI_FILTER_REGS_SPI_FILTER_REGS_CTRL_TEMPLATE_ ## x ## _FIELD_MASK

/**
 * Helper macro to read control register bits.
 */
#define	SPI_FILTER_HSP_CTRL_BIT_GET_NAME(x) \
	CREG_REGS_SPI_FILTER_REGS_SPI_FILTER_REGS_CTRL_TEMPLATE_ ## x ## _GET
#define	SPI_FILTER_HSP_CTRL_BIT_GET(x, reg) \
	SPI_FILTER_HSP_CTRL_BIT_GET_NAME (x) (reg)

/**
 * Helper macro to translate status register bits.
 */
#define	SPI_FILTER_HSP_STATUS_BIT(x)            \
	CREG_REGS_SPI_FILTER_REGS_SPI_FILTER_REGS_STATUS_TEMPLATE_ ## x ## _FIELD_MASK

/**
 * Helper macro to read status register bits.
 */
#define	SPI_FILTER_HSP_STATUS_BIT_GET_NAME(x)   \
	CREG_REGS_SPI_FILTER_REGS_SPI_FILTER_REGS_STATUS_TEMPLATE_ ## x ## _GET
#define	SPI_FILTER_HSP_STATUS_BIT_GET(x, reg)   \
	SPI_FILTER_HSP_STATUS_BIT_GET_NAME (x) (reg)


int spi_filter_hsp_get_port (const struct spi_filter_interface *filter)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	return hsp->port;
}

int spi_filter_hsp_get_mfg_id (const struct spi_filter_interface *filter, uint8_t *mfg_id)
{
	if ((filter == NULL) || (mfg_id == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	/* Initialize the output to something even though this operation is not supported. */
	*mfg_id = 0;

	/* The filter has no dependence on flash manufacturer.  All opcode filtering is programmable,
	 * leaving it up to firmware to decide what to do based on the flash device. */
	return SPI_FILTER_UNSUPPORTED_OPERATION;
}

int spi_filter_hsp_set_mfg_id (const struct spi_filter_interface *filter, uint8_t mfg_id)
{
	if (filter == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	UNUSED (mfg_id);

	return SPI_FILTER_UNSUPPORTED_OPERATION;
}

int spi_filter_hsp_get_flash_size (const struct spi_filter_interface *filter, uint32_t *bytes)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if ((hsp == NULL) || (bytes == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	*bytes = (hsp->regs->spi_filter_max_address_mask + 1) << 16;

	return 0;
}

int spi_filter_hsp_set_flash_size (const struct spi_filter_interface *filter, uint32_t bytes)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	hsp->regs->spi_filter_max_address_mask = (bytes - 1) >> 16;

	return 0;
}

int spi_filter_hsp_get_filter_mode (const struct spi_filter_interface *filter,
	spi_filter_flash_mode *mode)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;
	uint32_t ctrl;

	if ((hsp == NULL) || (mode == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);

	ctrl = hsp->regs->spi_filter_ctrl;

	if ((ctrl & SPI_FILTER_HSP_CTRL_BIT (BYP_EN)) || hsp->state->bypass_mode) {
		if (ctrl & SPI_FILTER_HSP_CTRL_BIT (BYP_SEL)) {
			*mode = SPI_FILTER_FLASH_BYPASS_CS1;
		}
		else {
			*mode = SPI_FILTER_FLASH_BYPASS_CS0;
		}
	}
	else if (ctrl & SPI_FILTER_HSP_CTRL_BIT (FLASH_MD)) {
		if (ctrl & SPI_FILTER_HSP_CTRL_BIT (SINGLE_FLASH_CHIP_SELECT)) {
			*mode = SPI_FILTER_FLASH_SINGLE_CS1;
		}
		else {
			*mode = SPI_FILTER_FLASH_SINGLE_CS0;
		}
	}
	else {
		*mode = SPI_FILTER_FLASH_DUAL;
	}

	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_set_filter_mode (const struct spi_filter_interface *filter,
	spi_filter_flash_mode mode)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;
	uint32_t ctrl;
	uint32_t bypass_mode = 0;
	int status = 0;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);

	ctrl = hsp->regs->spi_filter_ctrl;

	switch (mode) {
		case SPI_FILTER_FLASH_DUAL:
			/* Disable bypass and single flash modes. */
			ctrl &= ~(SPI_FILTER_HSP_CTRL_BIT (BYP_EN) | SPI_FILTER_HSP_CTRL_BIT (FLASH_MD));
			break;

		case SPI_FILTER_FLASH_SINGLE_CS0:
			/* Disable bypass and enable single flash mode.  Select CS0. */
			ctrl |= SPI_FILTER_HSP_CTRL_BIT (FLASH_MD);
			ctrl &= ~(SPI_FILTER_HSP_CTRL_BIT (BYP_EN) |
				SPI_FILTER_HSP_CTRL_BIT (SINGLE_FLASH_CHIP_SELECT));
			break;

		case SPI_FILTER_FLASH_SINGLE_CS1:
			/* Disable bypass and enable single flash mode.  Select CS1. */
			ctrl &= ~(SPI_FILTER_HSP_CTRL_BIT (BYP_EN));
			ctrl |= SPI_FILTER_HSP_CTRL_BIT (FLASH_MD) |
				SPI_FILTER_HSP_CTRL_BIT (SINGLE_FLASH_CHIP_SELECT);
			break;

		case SPI_FILTER_FLASH_BYPASS_CS0:
		case SPI_FILTER_FLASH_BYPASS_CS1:
			if (mode == SPI_FILTER_FLASH_BYPASS_CS0) {
				/* Enable bypass mode. Select CS0. */
				ctrl &= ~SPI_FILTER_HSP_CTRL_BIT (BYP_SEL);
			}
			else {
				/* Enable bypass mode.  Select CS1. */
				ctrl |= SPI_FILTER_HSP_CTRL_BIT (BYP_SEL);
			}

			/* On this filter implementation, bypass mode enable overrides filter disable, but the
			 * behavior needs to be that disable overrides bypass mode.  To achieve this, only
			 * enable bypass mode if the filter is already enabled.  If it is disabled, make a note
			 * that bypass mode should be enabled next time the filter is enabled. */
			if (ctrl & SPI_FILTER_HSP_CTRL_BIT (FLT_EN)) {
				ctrl |= SPI_FILTER_HSP_CTRL_BIT (BYP_EN);
			}
			else {
				bypass_mode = SPI_FILTER_HSP_CTRL_BIT (BYP_EN);
			}
			break;

		default:
			status = SPI_FILTER_INVALID_ARGUMENT;
			break;
	}

	if (status == 0) {
		hsp->regs->spi_filter_ctrl = ctrl;
		hsp->state->bypass_mode = bypass_mode;
	}

	platform_mutex_unlock (&hsp->state->lock);

	return status;
}

int spi_filter_hsp_get_filter_enabled (const struct spi_filter_interface *filter, bool *enabled)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if ((hsp == NULL) || (enabled == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);
	*enabled = !!(hsp->regs->spi_filter_ctrl & SPI_FILTER_HSP_CTRL_BIT (FLT_EN));
	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_enable_filter (const struct spi_filter_interface *filter, bool enable)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);

	if (enable) {
		/* When enabling the filter, make sure bypass mode gets set as necessary. */
		hsp->regs->spi_filter_ctrl |= (hsp->state->bypass_mode | SPI_FILTER_HSP_CTRL_BIT (FLT_EN));
	}
	else {
		/* When disabling the filter, need to disable bypass mode.  Save the current bypass mode
		 * state. */
		hsp->state->bypass_mode |= hsp->regs->spi_filter_ctrl & SPI_FILTER_HSP_CTRL_BIT (BYP_EN);
		hsp->regs->spi_filter_ctrl &= ~(SPI_FILTER_HSP_CTRL_BIT (BYP_EN) |
			SPI_FILTER_HSP_CTRL_BIT (FLT_EN));
	}

	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_get_ro_cs (const struct spi_filter_interface *filter, spi_filter_cs *act_sel)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if ((hsp == NULL) || (act_sel == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);
	*act_sel = (spi_filter_cs) SPI_FILTER_HSP_CTRL_BIT_GET (ACT_SEL, hsp->regs->spi_filter_ctrl);
	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_set_ro_cs (const struct spi_filter_interface *filter, spi_filter_cs act_sel)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;
	int status = 0;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);
	switch (act_sel) {
		case SPI_FILTER_CS_0:
			hsp->regs->spi_filter_ctrl &= ~SPI_FILTER_HSP_CTRL_BIT (ACT_SEL);
			break;

		case SPI_FILTER_CS_1:
			hsp->regs->spi_filter_ctrl |= SPI_FILTER_HSP_CTRL_BIT (ACT_SEL);
			break;

		default:
			status = SPI_FILTER_INVALID_ARGUMENT;
			break;
	}

	platform_mutex_unlock (&hsp->state->lock);

	return status;
}

int spi_filter_hsp_get_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode *mode)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if ((hsp == NULL) || (mode == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);
	*mode = (spi_filter_address_mode) SPI_FILTER_HSP_STATUS_BIT_GET (BYTE_MODE,
		hsp->regs->spi_filter_status);
	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_get_fixed_addr_byte_mode (const struct spi_filter_interface *filter, bool *fixed)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if ((hsp == NULL) || (fixed == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);
	*fixed = !!(hsp->regs->spi_filter_ctrl & SPI_FILTER_HSP_CTRL_BIT (PERM_BYTE_MD));
	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

/**
 * Apply a firmware override to the address byte mode used by the filter.
 *
 * @param hsp The SPI filter to configure.
 * @param mode The address mode mode to set.
 * @param fixed Flag indicating if the address byte mode is permanent.
 *
 * @return 0 if the address mode successfully configured or an error code.
 */
static int spi_filter_hsp_fw_config_addr_byte_mode (const struct spi_filter_hsp *hsp,
	spi_filter_address_mode mode, uint32_t fixed)
{
	uint32_t ctrl;
	int status = 0;

	platform_mutex_lock (&hsp->state->lock);

	/* Set FW address mode control. */
	ctrl = hsp->regs->spi_filter_ctrl | SPI_FILTER_HSP_CTRL_BIT (BYTE_SEL_MD);

	/* Configure the fixed address mode bit. */
	ctrl &= ~SPI_FILTER_HSP_CTRL_BIT (PERM_BYTE_MD);
	ctrl |= fixed;

	switch (mode) {
		case SPI_FILTER_ADDRESS_MODE_3:
			ctrl &= ~SPI_FILTER_HSP_CTRL_BIT (BYTE_SEL);
			break;

		case SPI_FILTER_ADDRESS_MODE_4:
			ctrl |= SPI_FILTER_HSP_CTRL_BIT (BYTE_SEL);
			break;

		default:
			status = SPI_FILTER_INVALID_ARGUMENT;
			break;
	}

	if (status == 0) {
		hsp->regs->spi_filter_ctrl = ctrl;

		/* Switch address mode control back to the filter. */
		hsp->regs->spi_filter_ctrl &= ~SPI_FILTER_HSP_CTRL_BIT (BYTE_SEL_MD);
	}

	platform_mutex_unlock (&hsp->state->lock);

	return status;
}

int spi_filter_hsp_set_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode mode)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	return spi_filter_hsp_fw_config_addr_byte_mode (hsp, mode, 0);
}

int spi_filter_hsp_set_fixed_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode mode)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	return spi_filter_hsp_fw_config_addr_byte_mode (hsp, mode,
		SPI_FILTER_HSP_CTRL_BIT (PERM_BYTE_MD));
}

int spi_filter_hsp_get_addr_byte_mode_write_enable_required (
	const struct spi_filter_interface *filter, bool *required)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if ((hsp == NULL) || (required == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);
	*required = !!(hsp->regs->spi_filter_ctrl & SPI_FILTER_HSP_CTRL_BIT (WE_BYTE_MD));
	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_require_addr_byte_mode_write_enable (const struct spi_filter_interface *filter,
	bool require)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);

	if (require) {
		hsp->regs->spi_filter_ctrl |= SPI_FILTER_HSP_CTRL_BIT (WE_BYTE_MD);
	}
	else {
		hsp->regs->spi_filter_ctrl &= ~SPI_FILTER_HSP_CTRL_BIT (WE_BYTE_MD);
	}

	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_get_reset_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode *mode)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if ((hsp == NULL) || (mode == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);
	*mode = (spi_filter_address_mode) SPI_FILTER_HSP_CTRL_BIT_GET (BYTE_MD_RST,
		hsp->regs->spi_filter_ctrl);
	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_set_reset_addr_byte_mode (const struct spi_filter_interface *filter,
	spi_filter_address_mode mode)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;
	int status = 0;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);

	switch (mode) {
		case SPI_FILTER_ADDRESS_MODE_3:
			hsp->regs->spi_filter_ctrl &= ~SPI_FILTER_HSP_CTRL_BIT (BYTE_MD_RST);
			break;

		case SPI_FILTER_ADDRESS_MODE_4:
			hsp->regs->spi_filter_ctrl |= SPI_FILTER_HSP_CTRL_BIT (BYTE_MD_RST);
			break;

		default:
			status = SPI_FILTER_INVALID_ARGUMENT;
			break;
	}

	platform_mutex_unlock (&hsp->state->lock);

	return status;
}

int spi_filter_hsp_are_all_single_flash_writes_allowed (const struct spi_filter_interface *filter,
	bool *allowed)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if ((hsp == NULL) || (allowed == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);
	*allowed = !!(hsp->regs->spi_filter_ctrl & SPI_FILTER_HSP_CTRL_BIT (SINGLE_FLASH_ALLOW_WRITE));
	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_allow_all_single_flash_writes (const struct spi_filter_interface *filter,
	bool allowed)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);

	if (allowed) {
		hsp->regs->spi_filter_ctrl |= SPI_FILTER_HSP_CTRL_BIT (SINGLE_FLASH_ALLOW_WRITE);
	}
	else {
		hsp->regs->spi_filter_ctrl &= ~SPI_FILTER_HSP_CTRL_BIT (SINGLE_FLASH_ALLOW_WRITE);
	}

	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_get_write_enable_detected (const struct spi_filter_interface *filter,
	bool *detected)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if ((hsp == NULL) || (detected == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	*detected = !!(hsp->regs->spi_filter_status & SPI_FILTER_HSP_STATUS_BIT (WRITE_EN));

	return 0;
}

int spi_filter_hsp_get_flash_dirty_state (const struct spi_filter_interface *filter,
	spi_filter_flash_state *state)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if ((hsp == NULL) || (state == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	*state = (spi_filter_flash_state) SPI_FILTER_HSP_STATUS_BIT_GET (DIRTY,
		hsp->regs->spi_filter_status);

	return 0;
}

int spi_filter_hsp_clear_flash_dirty_state (const struct spi_filter_interface *filter)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);
	hsp->regs->spi_filter_ctrl |= SPI_FILTER_HSP_CTRL_BIT (DIRTY_CLR);
	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_get_filter_rw_region (const struct spi_filter_interface *filter, uint8_t region,
	uint32_t *start_addr, uint32_t *end_addr)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;
	uint32_t range;

	if ((hsp == NULL) || (start_addr == NULL) || (end_addr == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	if ((region == 0) ||
		(region > ARRAY_SIZE (hsp->regs->spi_filter_address_regions.spi_filter_addr_region))) {
		return SPI_FILTER_UNSUPPORTED_RW_REGION;
	}

	platform_mutex_lock (&hsp->state->lock);

	range = hsp->regs->spi_filter_address_regions.spi_filter_addr_region[region - 1];

	*start_addr = (range & 0xffff) << 16;
	*end_addr = range & 0xffff0000;

	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_set_filter_rw_region (const struct spi_filter_interface *filter, uint8_t region,
	uint32_t start_addr, uint32_t end_addr)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	if ((region == 0) ||
		(region > ARRAY_SIZE (hsp->regs->spi_filter_address_regions.spi_filter_addr_region))) {
		return SPI_FILTER_UNSUPPORTED_RW_REGION;
	}

	if (((start_addr & 0x0000ffff) != 0) || ((end_addr & 0x0000ffff) != 0)) {
		return SPI_FILTER_MISALIGNED_ADDRESS;
	}

	if ((end_addr != 0) && (end_addr < start_addr)) {
		return SPI_FILTER_INVALID_ADDR_RANGE;
	}

	platform_mutex_lock (&hsp->state->lock);

	hsp->regs->spi_filter_address_regions.spi_filter_addr_region[region - 1] =
		((end_addr & 0xffff0000) | ((start_addr & 0xffff0000) >> 16));

	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

int spi_filter_hsp_clear_filter_rw_regions (const struct spi_filter_interface *filter)
{
	const struct spi_filter_hsp *hsp = (const struct spi_filter_hsp*) filter;
	size_t i;

	if (hsp == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&hsp->state->lock);

	for (i = 0; i < ARRAY_SIZE (hsp->regs->spi_filter_address_regions.spi_filter_addr_region);
		i++) {
		hsp->regs->spi_filter_address_regions.spi_filter_addr_region[i] = 0;
	}

	platform_mutex_unlock (&hsp->state->lock);

	return 0;
}

uint32_t spi_filter_hsp_get_interrupt_status (const struct spi_filter_hsp *filter)
{
	if (filter == NULL) {
		return 0;
	}

	return filter->regs->spi_filter_intsts;
}

uint32_t spi_filter_hsp_get_interrupt_enable (const struct spi_filter_hsp *filter)
{
	if (filter == NULL) {
		return 0;
	}

	return filter->regs->spi_filter_inten;
}

void spi_filter_hsp_set_interrupt_enable (const struct spi_filter_hsp *filter, uint32_t enable)
{
	if (filter == NULL) {
		return;
	}

	filter->regs->spi_filter_inten = enable;
}

uint8_t spi_filter_hsp_get_blocked_opcode (const struct spi_filter_hsp *filter)
{
	uint32_t reg;

	if (filter == NULL) {
		return 0;
	}

	reg = filter->regs->spi_filter_block_opcode;

	return reg;
}

int spi_filter_hsp_set_filtered_opcodes (const struct spi_filter_hsp *filter,
	const union spi_filter_hsp_opcode *opcode_list, size_t count)
{
	size_t i;

	if (filter == NULL) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	if (opcode_list == NULL) {
		/* A null list will clear all programable opcodes. */
		count = 0;
	}

	if (count > ARRAY_SIZE (filter->regs->spi_filter_prg_opcodes.spi_filter_prg_opcode)) {
		return SPI_FILTER_OPCODE_CFG_FAILED;
	}

	for (i = 0; i < count; i++) {
		filter->regs->spi_filter_prg_opcodes.spi_filter_prg_opcode[i] = opcode_list[i].reg_value;
	}

	/* Any remaining registers after programming the specified opcode list will be cleared. */
	for (; i < ARRAY_SIZE (filter->regs->spi_filter_prg_opcodes.spi_filter_prg_opcode); i++) {
		filter->regs->spi_filter_prg_opcodes.spi_filter_prg_opcode[i] = 0;
	}

	return 0;
}

/**
 * Initialize a driver for a single SPI filter instance in HSP.
 *
 * @param filter The SPI filter driver to initialize.
 * @param state Variable context for the SPI filter.  This must be uninitialized.
 * @param regs Register interface for the SPI filter instance.
 * @param port Port identifier for the SPI filter instance.
 *
 * @return 0 if the filter was successfully initialized or an error code.
 */
int spi_filter_hsp_init (struct spi_filter_hsp *filter, struct spi_filter_hsp_state *state,
	Creg_regs_spi_filter_regs *regs, uint8_t port)
{
	if ((filter == NULL) || (state == NULL) || (regs == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	memset (filter, 0, sizeof (struct spi_filter_hsp));

	filter->base.get_port = spi_filter_hsp_get_port;
	filter->base.get_mfg_id = spi_filter_hsp_get_mfg_id;
	filter->base.set_mfg_id = spi_filter_hsp_set_mfg_id;
	filter->base.get_flash_size = spi_filter_hsp_get_flash_size;
	filter->base.set_flash_size = spi_filter_hsp_set_flash_size;
	filter->base.get_filter_mode = spi_filter_hsp_get_filter_mode;
	filter->base.set_filter_mode = spi_filter_hsp_set_filter_mode;
	filter->base.get_filter_enabled = spi_filter_hsp_get_filter_enabled;
	filter->base.enable_filter = spi_filter_hsp_enable_filter;
	filter->base.get_ro_cs = spi_filter_hsp_get_ro_cs;
	filter->base.set_ro_cs = spi_filter_hsp_set_ro_cs;
	filter->base.get_addr_byte_mode = spi_filter_hsp_get_addr_byte_mode;
	filter->base.get_fixed_addr_byte_mode = spi_filter_hsp_get_fixed_addr_byte_mode;
	filter->base.set_addr_byte_mode = spi_filter_hsp_set_addr_byte_mode;
	filter->base.set_fixed_addr_byte_mode = spi_filter_hsp_set_fixed_addr_byte_mode;
	filter->base.get_addr_byte_mode_write_enable_required =
		spi_filter_hsp_get_addr_byte_mode_write_enable_required;
	filter->base.require_addr_byte_mode_write_enable =
		spi_filter_hsp_require_addr_byte_mode_write_enable;
	filter->base.get_reset_addr_byte_mode = spi_filter_hsp_get_reset_addr_byte_mode;
	filter->base.set_reset_addr_byte_mode = spi_filter_hsp_set_reset_addr_byte_mode;
	filter->base.are_all_single_flash_writes_allowed =
		spi_filter_hsp_are_all_single_flash_writes_allowed;
	filter->base.allow_all_single_flash_writes = spi_filter_hsp_allow_all_single_flash_writes;
	filter->base.get_write_enable_detected = spi_filter_hsp_get_write_enable_detected;
	filter->base.get_flash_dirty_state = spi_filter_hsp_get_flash_dirty_state;
	filter->base.clear_flash_dirty_state = spi_filter_hsp_clear_flash_dirty_state;
	filter->base.get_filter_rw_region = spi_filter_hsp_get_filter_rw_region;
	filter->base.set_filter_rw_region = spi_filter_hsp_set_filter_rw_region;
	filter->base.clear_filter_rw_regions = spi_filter_hsp_clear_filter_rw_regions;

	filter->get_interrupt_status = spi_filter_hsp_get_interrupt_status;
	filter->get_interrupt_enable = spi_filter_hsp_get_interrupt_enable;
	filter->set_interrupt_enable = spi_filter_hsp_set_interrupt_enable;
	filter->get_blocked_opcode = spi_filter_hsp_get_blocked_opcode;
	filter->set_filtered_opcodes = spi_filter_hsp_set_filtered_opcodes;

	filter->state = state;
	filter->regs = regs;
	filter->port = port;

	return spi_filter_hsp_init_state (filter);
}

/**
 * Initialize only the variable state for a SPI filter driver.  The rest of the driver is assumed to
 * have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param filter The SPI filter driver that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int spi_filter_hsp_init_state (const struct spi_filter_hsp *filter)
{
	if ((filter == NULL) || (filter->state == NULL) || (filter->regs == NULL)) {
		return SPI_FILTER_INVALID_ARGUMENT;
	}

	memset (filter->state, 0, sizeof (struct spi_filter_hsp_state));

	/* Maintain semantics of the previous SPI filter design that required the filter to be enabled
	 * for bypass mode.  In this design, bypass mode overrides other settings.  */
	if (filter->regs->spi_filter_ctrl & SPI_FILTER_HSP_CTRL_BIT (BYP_EN)) {
		filter->regs->spi_filter_ctrl |= SPI_FILTER_HSP_CTRL_BIT (FLT_EN);
	}

	return platform_mutex_init (&filter->state->lock);
}

/**
 * Release the resources used by an HSP SPI filter driver.
 *
 * @param filter The SPI filter to release.
 */
void spi_filter_hsp_release (const struct spi_filter_hsp *filter)
{
	if (filter) {
		platform_mutex_free (&filter->state->lock);
	}
}
