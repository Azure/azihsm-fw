// Copyright (c) Microsoft Corporation. All rights reserved.

#include <memory.h>
#include <stddef.h>
#include <stdlib.h>
#include "tdisp_driver_manticore.h"
#include "tdisp_pcie_utils.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "marvell/RegPcieAssist.h"

/**
 * TDISP TDI state register address.
 */
#define TDISP_DRIVER_MANTICORE_TDISP_ST_REGISTER			0xB01B0010

/**
 * PCIE VF count register address
 */
#define TDISP_DRIVER_MANTICORE_PCIE_SRIOV_NUM_VFS_REGISTER	0xB0180208

/**
 * TDISP LUT register address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_LUT_REGISTER			0xB01A0300

/**
 * TDISP LUT configuration register for HC0
 */
#define TDISP_DRIVER_MANTICORE_TDISP_LUT_CFG_HC0_REGISTER	0xB01A0568

/**
 * PCIe core configuration register address
 */
#define TDISP_DRIVER_MANTICORE_PCIE_CORE_CFG_REGISTER		0xB01C0068

/**
 * TDISP control register address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_CTRL_REGISTER			0xB01A0000

/**
 * TDISP CII control register address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_CII_CTRL_REGISTER		0xB01A0200

/**
 * TDISP EC IDE control register 0 address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL0_REGISTER	0xB01A0188

/**
 * TDISP EC IDE control register 1 address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL1_REGISTER	0xB01A018C

/**
 * TDISP EC IDE control register 2 address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL2_REGISTER	0xB01A0190

/**
 * TDISP EC IDE control register 3 address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL3_REGISTER	0xB01A0194

/**
 * TDISP EC IDE control register 4 address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL4_REGISTER	0xB01A0198

/**
 * TDISP EC IDE control register 5 address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL5_REGISTER	0xB01A019C

/**
 * TDISP EC (error controller) ST register 0 address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_EC_ST_CTRL0_REGISTER	0xB01A01A0

/**
 * TDISP EC (error controller) ST register 1 address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_EC_ST_CTRL1_REGISTER	0xB01A01A4

/**
 * TDISP EC (error controller) ST register 2 address
 */
#define TDISP_DRIVER_MANTICORE_TDISP_EC_ST_CTRL2_REGISTER	0xB01A01A8

/**
 * TDISP US (upper stream manager) control register
 */
#define TDISP_DRIVER_MANTICORE_TDISP_US_CTRL_REGISTER		0xB01A0080

/**
 * PCIE IDE global configuration register
 */
#define TDISP_DRIVER_MANTICORE_PCIE_IDE_GLBL_CFG_REGISTER	0xB01E0008

/**
 * PCIE IDE interrupt enable register
 */
#define TDISP_DRIVER_MANTICORE_PCIE_IDE_IRQ_EN_REGISTER		0xB01E02D8

/**
 * TDISP LUT CFG FC0 register
 */
#define TDISP_DRIVER_MANTICORE_TDISP_LUT_CFG_FC0_REGISTER	0xB01A0500

/**
 * TDISP LUT CFG FC22 register
 */
#define TDISP_DRIVER_MANTICORE_TDISP_LUT_CFG_FC22_REGISTER	0xB01A0558

/**
 * PCIe core interrupt enable register
 */
#define TDISP_DRIVER_MANTICORE_PCIE_CORE_INT_EN_REGISTER	0xB01C00D8

/**
 * TDISP interrupt status register
 */
#define TDISP_DRIVER_MANTICORE_TDISP_INT_STATUS_REGISTER	0xB01A0004

/**
 * TDISP interrupt enable register
 */
#define TDISP_DRIVER_MANTICORE_TDISP_INT_EN_REGISTER		0xB01A0008

/**
 * TDISP error status register for VF0-31.
 */
#define TDISP_DRIVER_MANTICORE_TDISP_ERR_ST0				0xB01A0010

/**
 * TDISP error status register for VF32-63.
 */
#define TDISP_DRIVER_MANTICORE_TDISP_ERR_ST1				0xB01A0014

/**
 * TDISP error status register for PF.
 */
#define TDISP_DRIVER_MANTICORE_TDISP_ERR_ST2				0xB01A0018

/**
 * TDISP error interrupt enable for VF0-31.
 */
#define TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN0			0xB01A001C

/**
 * TDISP error interrupt enable for VF32-63.
 */
#define TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN1			0xB01A0020

/**
 * TDISP error interrupt enable for PF.
 */
#define TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN2			0xB01A0024


//#define TDISP_DRIVER_UART_SPEW

#ifdef TDISP_DRIVER_UART_SPEW
#include "platform_io_api.h"
/**
 * Dump MMIO register block contents
 *
 * @param register_block Register block to dump
 * @param base Base address of the register block
 * @param offset Offset inside the register block to start dumping from
 * @param count Number of 32-bit registers to dump
 */
static void tdisp_driver_manticore_dump_register_block (
	const struct mmio_register_block *register_block, uint32_t base, uint32_t offset,
	uint32_t count)
{
	int status;
	uint32_t i;
	uint32_t value;

	status = register_block->map (register_block);
	if (status != 0) {
		return;
	}

	for (i = 0; i < count; i++) {
		if ((i != 0) && ((i % 8) == 0)) {
			platform_printf ("\n");
		}

		if ((i % 8) == 0) {
			platform_printf ("0x%x: ", base + offset + i * sizeof (uint32_t));
		}

		status = register_block->read32_by_addr (register_block, base + offset + i *
			sizeof (uint32_t), &value);
		if (status != 0) {
			goto exit;
		}

		platform_printf ("0x%x ", value);
	}
	platform_printf ("\n");

exit:
	register_block->unmap (register_block);
}
#endif	// TDISP_DRIVER_UART_SPEW

/**
 * Helper function to convert PCIE function index into manticore compatible index
 * Incoming function index:
 *   0 - PF0
 *   1 - VF0
 *   2 - VF1
 *   ...
 *   64 - VF63
 *
 * Hardware expects:
 *   0 - VF0
 *   1 - VF1
 *   ...
 *   63 - VF63
 *   64 - PF0
 *
 * @param function_index Requester provided function index. Manticore  device is configured
 * with VF offset = 1 and VF stride = 1, so that would create continues numbers from 0 to 64
 * inclusive.
 *
 * @return HW compatible function ID
 */
static uint32_t tdisp_driver_manticore_hw_function_index (uint32_t function_index)
{
	if (function_index == 0) {
		return TDISP_TDI_MAX_COUNT - 1;
	}
	else {
		return function_index - 1;
	}
}

/**
 * Get TDISP hardware state for specific function
 *
 * @param tdisp_driver TDISP driver instance
 * @param hw_function_index Hardware function index to query
 * @param tdi_hw_state Output parameter which will hold TDISP hardware state
 *
 * @return 0 if successful, error code otherwise
 */
static int tdisp_driver_manticore_get_tdi_hw_state (
	const struct tdisp_driver_manticore *tdisp_driver, uint32_t hw_function_index,
	uint32_t *tdi_hw_state)
{
	int status;

	status = tdisp_driver->pcie_registers->map (tdisp_driver->pcie_registers);
	if (status != 0) {
		return status;
	}

	/* 0xB01B0010 - TDI states */
	status = tdisp_driver->pcie_registers->read32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ST_REGISTER +
		(hw_function_index * sizeof (uint32_t)), tdi_hw_state);

	tdisp_driver->pcie_registers->unmap (tdisp_driver->pcie_registers);

	return status;
}

/**
 * Set TDISP hardware state for specific function
 *
 * @param tdisp_driver TDISP driver instance
 * @param hw_function_index Hardware function index to set
 * @param tdi_hw_state TDISP hardware state to set
 *
 * @return 0 if successful, error code otherwise
 */
static int tdisp_driver_manticore_set_tdi_hw_state (
	const struct tdisp_driver_manticore *tdisp_driver, uint32_t hw_function_index,
	uint32_t tdi_hw_state)
{
	int status;

	status = tdisp_driver->pcie_registers->map (tdisp_driver->pcie_registers);
	if (status != 0) {
		return status;
	}

	/* 0xB01B0010 */
	status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ST_REGISTER +
		(hw_function_index * sizeof (uint32_t)), tdi_hw_state);

	tdisp_driver->pcie_registers->unmap (tdisp_driver->pcie_registers);

	return status;
}

/**
 * Get number of VFs supported by the device
 *
 * @param tdisp_driver TDISP driver instance
 * @param vf_count Output number of available VF functions
 *
 * @return 0 if successful, error code otherwise
 */
static int tdisp_driver_manticore_get_vf_count (const struct tdisp_driver_manticore *tdisp_driver,
	uint32_t *vf_count)
{
	int status;
	uint32_t sriov_numvf;

	status = tdisp_driver->pcie_registers->map (tdisp_driver->pcie_registers);
	if (status != 0) {
		return status;
	}

	status = tdisp_driver->pcie_registers->read32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_PCIE_SRIOV_NUM_VFS_REGISTER, &sriov_numvf);
	if (status != 0) {
		goto exit;
	}

	*vf_count = sriov_numvf & 0xffff;

exit:
	tdisp_driver->pcie_registers->unmap (tdisp_driver->pcie_registers);

	return status;
}

#ifdef TDISP_DRIVER_UART_SPEW
/**
 * Get TDISP LUT entry for specific function
 *
 * @param mdriver Manticore TDISP driver instance
 * @param hw_function_index Hardware function ID to query
 * @param lut_entry Output parameter which will hold the LUT entry
 *
 * @return 0 if successful, error code otherwise
 */
static int tdisp_driver_manticore_get_lut_entry (const struct tdisp_driver_manticore *mdriver,
	uint32_t hw_function_index, union tdisp_lut_entry *lut_entry)
{
	int status;

	status = mdriver->pcie_registers->map (mdriver->pcie_registers);
	if (status != 0) {
		return status;
	}

	status = mdriver->pcie_registers->read32_by_addr (mdriver->pcie_registers, 0xB01A0300 +
		(hw_function_index * sizeof (uint32_t)), &lut_entry->value);

	mdriver->pcie_registers->unmap (mdriver->pcie_registers);

	if (status == 0) {
		platform_printf ("TDISP: get_lut_entry: hw_function_id=%d\n", hw_function_index);
		platform_printf ("TDISP: get_lut_entry: value=0x%x\n", lut_entry->value);
		platform_printf ("TDISP: get_lut_entry: non_t_m0=%d\n", lut_entry->non_t_m0);
		platform_printf ("TDISP: get_lut_entry: non_t_m1=%d\n", lut_entry->non_t_m1);
		platform_printf ("TDISP: get_lut_entry: non_t_m2=%d\n", lut_entry->non_t_m2);
		platform_printf ("TDISP: get_lut_entry: msix_l=%d\n", lut_entry->msix_l);
		platform_printf ("TDISP: get_lut_entry: id=%d\n", lut_entry->id);
		platform_printf ("TDISP: get_lut_entry: hw_st=0x%x\n", lut_entry->tdisp_st);
	}

	return status;
}
#endif

/**
 * Set TDISP LUT entry for specific function
 *
 * @param mdriver Manticore TDISP driver instance
 * @param hw_function_index Hardware function ID to set
 * @param lut_entry LUT entry to set
 * @param mask Bitmask indicating which fields in the LUT entry to set
 *
 * @return 0 if successful, error code otherwise
 */
static int tdisp_driver_manticore_set_lut_entry (const struct tdisp_driver_manticore *mdriver,
	uint32_t hw_function_index, const union tdisp_lut_entry *lut_entry, uint32_t mask)
{
	int status;
	union tdisp_lut_entry entry = {};

	status = mdriver->pcie_registers->map (mdriver->pcie_registers);
	if (status != 0) {
		return status;
	}

	status = mdriver->pcie_registers->read32_by_addr (mdriver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_LUT_REGISTER +
		(hw_function_index * sizeof (uint32_t)), &entry.value);
	if (status != 0) {
		goto exit;
	}

	if ((mask & TDISP_LUT_MASK_MSIX_L) != 0) {
		entry.msix_l = lut_entry->msix_l;
	}
	if ((mask & TDISP_LUT_MASK_NON_T_M0) != 0) {
		entry.non_t_m0 = lut_entry->non_t_m0;
	}
	if ((mask & TDISP_LUT_MASK_NON_T_M1) != 0) {
		entry.non_t_m1 = lut_entry->non_t_m1;
	}
	if ((mask & TDISP_LUT_MASK_NON_T_M2) != 0) {
		entry.non_t_m2 = lut_entry->non_t_m2;
	}
	if ((mask & TDISP_LUT_MASK_ID) != 0) {
		entry.id = lut_entry->id;
	}

	status = mdriver->pcie_registers->write32_by_addr (mdriver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_LUT_REGISTER +
		(hw_function_index * sizeof (uint32_t)), entry.value);

exit:
	mdriver->pcie_registers->unmap (mdriver->pcie_registers);

	return status;
}

/**
 * Check if the IDE stream is in a secure state
 *
 * @param mdriver Manticore TDISP driver instance
 * @param stream_id Stream ID to check
 *
 * @return 0 if the stream is secure, error code otherwise
 */
static int tdisp_driver_manticore_check_ide_stream_state (
	const struct tdisp_driver_manticore *mdriver, uint8_t stream_id)
{
	int status;
	struct ide_link_ide_stream_register_block link_stream_regs;
	struct ide_selective_ide_stream_register_block sel_stream_regs;

	status = mdriver->ide->get_link_ide_register_block (mdriver->ide, 0, 0, &link_stream_regs);
	if (status != 0) {
		return status;
	}

	status = mdriver->ide->get_selective_ide_stream_register_block (mdriver->ide, 0, 0,
		&sel_stream_regs);
	if (status != 0) {
		return status;
	}

	if (link_stream_regs.stream_control_register.link_ide_stream_enable) {
		if (link_stream_regs.stream_control_register.stream_id != stream_id) {
#ifdef TDISP_DRIVER_UART_SPEW
			platform_printf (
				"TDISP: lock_interface: Enabled Link IDE stream ID is not matching ex(%x), ac(%x)\n",
				stream_id, link_stream_regs.stream_control_register.stream_id);
#endif

			return TDISP_DRIVER_IDE_NOT_SECURE;
		}
		if (link_stream_regs.stream_control_register.tc != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
			platform_printf ("TDISP: lock_interface: Enabled Link IDE stream TC != 0 (%d)\n",
				link_stream_regs.stream_control_register.tc);
#endif

			return TDISP_DRIVER_IDE_NOT_SECURE;
		}

		if (link_stream_regs.stream_status_register.link_ide_stream_state != 0x2) {
#ifdef TDISP_DRIVER_UART_SPEW
			platform_printf ("TDISP: lock_interface: Enabled Link IDE stream is not SECURE\n");
#endif

			return TDISP_DRIVER_IDE_NOT_SECURE;
		}
	}
	else if (sel_stream_regs.sel_ide_stream_control_reg.selective_ide_stream_enable) {
		if (sel_stream_regs.sel_ide_stream_control_reg.stream_id != stream_id) {
#ifdef TDISP_DRIVER_UART_SPEW
			platform_printf (
				"TDISP: lock_interface: Enabled Selective IDE stream ID is not matching ex(%x), ac(%x)\n",
				stream_id, sel_stream_regs.sel_ide_stream_control_reg.stream_id);
#endif

			return TDISP_DRIVER_IDE_NOT_SECURE;
		}
		if (sel_stream_regs.sel_ide_stream_control_reg.tc != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
			platform_printf ("TDISP: lock_interface: Enabled Selective IDE stream TC != 0 (%d)\n",
				link_stream_regs.stream_control_register.tc);
#endif

			return TDISP_DRIVER_IDE_NOT_SECURE;
		}
		if (sel_stream_regs.sel_ide_stream_status_reg.selective_ide_stream_state != 0x2) {
#ifdef TDISP_DRIVER_UART_SPEW
			platform_printf ("TDISP: lock_interface: Enabled Selective IDE stream is not SECURE\n");
#endif

			return TDISP_DRIVER_IDE_NOT_SECURE;
		}
	}
	else {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: NO IDE stream enabled\n");
#endif

		return TDISP_DRIVER_IDE_NOT_SECURE;
	}

	return 0;
}

/**
 * Set hardware control bits for a specific configuration entry
 *
 * @param regs MMIO register block to use for setting the hardware control bits
 * @param conf_addr Configuration address to set the hardware control bits for
 * @param hc Hardware control bits to set
 *
 * @return 0 if successful, error code otherwise
 */
static int tdisp_driver_manticore_set_hc (const struct mmio_register_block *regs,
	uint32_t conf_addr, uint32_t hc)
{
	uint32_t reg_index;
	uint32_t pos_index;
	int status;
	uint32_t value;

	/* Each HC register covers 16 configuration registers (2 bits per register). Each configuration
	register is 4 bytes, configuration register address / 4 = configuration register index, because
	16 configuration registers per 1 HC register, then HC register index = configuration register
	address / (16* 4) */
	if (conf_addr > 0x1000) {
		// VF
		reg_index = 17 + (conf_addr - 0x1000) / (16 * 4);
	}
	else if (conf_addr > 0xE00) {
		reg_index = 16;
	}
	else {
		reg_index = conf_addr / (16 * 4);
	}
	pos_index = (conf_addr / 4) % 16;

	status = regs->read32_by_addr (regs, TDISP_DRIVER_MANTICORE_TDISP_LUT_CFG_HC0_REGISTER +
		reg_index * 4, &value);
	if (status != 0) {
		return status;
	}

	value &= ~(((uint32_t) 0x3) << (pos_index * 2));
	value |= ((hc & 0x3) << (pos_index * 2));

	status = regs->write32_by_addr (regs, TDISP_DRIVER_MANTICORE_TDISP_LUT_CFG_HC0_REGISTER +
		reg_index * 4, value);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Determines if TDISP hardware controller enabled or not.
 *
 * @param tdisp_driver Manticore TDISP driver instance
 * @param enabled Output, 0 - controller is disabled, 1 - controller is enabled
 *
 * @return 0 if successful, error code otherwise
 */
static int tdisp_driver_manticore_is_tdisp_controller_enabled (
	const struct tdisp_driver_manticore *tdisp_driver, uint32_t *enabled)
{
	int status;
	uint32_t tdisp_status;

	if ((tdisp_driver == NULL) || (enabled == NULL)) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	*enabled = 0;

	status = tdisp_driver->pcie_registers->map (tdisp_driver->pcie_registers);
	if (status != 0) {
		return status;
	}

	status = tdisp_driver->pcie_registers->read32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_CTRL_REGISTER, &tdisp_status);
	if (status != 0) {
		goto exit;
	}

	/* Is TDISP controller enabled? */
	if ((tdisp_status & 0x1) != 0) {
		*enabled = 1;
	}

exit:
	tdisp_driver->pcie_registers->unmap (tdisp_driver->pcie_registers);

	return status;
}

int tdisp_driver_manticore_get_function_index (const struct tdisp_driver *tdisp_driver,
	uint32_t bdf, uint32_t *function_index)
{
	int status;
	const struct tdisp_driver_manticore *mdriver = TO_DERIVED_TYPE (tdisp_driver,
		const struct tdisp_driver_manticore, base);
	PcieCoreGeneralCfg_t cfg_reg = {0};
	uint32_t pf_bdf;
	uint32_t vf_count;

	if ((tdisp_driver == NULL) || (function_index == NULL)) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	status = tdisp_driver_manticore_enable (mdriver);
	if (status != 0) {
		return status;
	}

	status = tdisp_driver_manticore_get_vf_count (mdriver, &vf_count);
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: get_function_index: vf_count=%d\n", vf_count);
	platform_printf ("TDISP: get_function_index: bus=%x, device=%x, function=%x\n",	(bdf >> 8),
		((bdf >> 3) & 0x1F), (bdf & 0x7));
#endif
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: get_function_index: failed to get VF count, status=%x\n", status);
#endif

		return status;
	}

	status = mdriver->pcie_registers->map (mdriver->pcie_registers);
	if (status != 0) {
		return status;
	}

	/* 0xB01C0068 */
	status = mdriver->pcie_registers->read32_by_addr (mdriver->pcie_registers,
		TDISP_DRIVER_MANTICORE_PCIE_CORE_CFG_REGISTER, &cfg_reg.all);
	if (status != 0) {
		goto exit;
	}

	/* TODO: consider VF functions STRIDE to properly calculate function index */
	pf_bdf = (cfg_reg.b.CFG_PBUS_NUM << 8) | (cfg_reg.b.CFG_PBUS_DEV_NUM << 3);

	if ((bdf < pf_bdf) || (bdf > (pf_bdf + vf_count))) {
		status = TDISP_DRIVER_INVALID_INTERFACE;
		goto exit;
	}

	*function_index = bdf - pf_bdf;
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: get_function_index: function_index=%d\n", (*function_index));
#endif
exit:
	mdriver->pcie_registers->unmap (mdriver->pcie_registers);

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: get_function_index: REGISTERS:\n");
	// TDISP_CFG B01A0000 - 0xB01800000 =
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20000, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20010, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20080, 5);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20100, 4);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20150, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20180, 11);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20200, 9);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20300, 65);

	// PCIE_IDE
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60000, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6001C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60040, 8);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6024C, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602D4, 7);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602FC, 20);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6042C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60448, 2);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60500, 1);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x62000, 13);
#endif

	return status;
}


int tdisp_driver_manticore_get_tdisp_capabilities (const struct tdisp_driver *tdisp_driver,
	const struct tdisp_requester_capabilities *req_caps,
	struct tdisp_responder_capabilities *rsp_caps)
{
#ifdef TDISP_DRIVER_UART_SPEW
	const struct tdisp_driver_manticore *mdriver = TO_DERIVED_TYPE (tdisp_driver,
		const struct tdisp_driver_manticore, base);
#endif

	if ((tdisp_driver == NULL) || (req_caps == NULL) || (rsp_caps == NULL)) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	memset (rsp_caps, 0, sizeof (*rsp_caps));
	rsp_caps->dsm_caps = 0;

	rsp_caps->lock_interface_flags_supported = TDISP_LOCK_INTERFACE_FLAGS_NO_FW_UPDATE |
		TDISP_LOCK_INTERFACE_FLAGS_CACHE_LINE_SIZE | TDISP_LOCK_INTERFACE_FLAGS_LOCK_MSIX;

	rsp_caps->dev_addr_width = 64;

	/* TODO: figure out proper values for these fields */
	rsp_caps->num_req_this = 1;
	rsp_caps->num_req_all = 1;

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: get_tdisp_capabilities: REGISTERS:\n");
	// TDISP_CFG B01A0000 - 0xB01800000 =
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20000, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20010, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20080, 5);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20100, 4);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20150, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20180, 11);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20200, 9);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20300, 65);

	// PCIE_IDE
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60000, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6001C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60040, 8);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6024C, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602D4, 7);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602FC, 20);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6042C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60448, 2);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60500, 1);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x62000, 13);
#endif

	return 0;
}

int tdisp_driver_manticore_lock_interface_request (const struct tdisp_driver *tdisp_driver,
	uint32_t function_index, const struct tdisp_lock_interface_param *lock_interface_param)
{
	int status;
	const struct tdisp_driver_manticore *mdriver = TO_DERIVED_TYPE (tdisp_driver,
		const struct tdisp_driver_manticore, base);
	uint32_t hw_function_index = tdisp_driver_manticore_hw_function_index (function_index);
	uint32_t tdi_hw_state;
	uint32_t vf_count;
	union tdisp_lut_entry lut_entry = {};
	uint32_t is_enabled = 0;

	if ((tdisp_driver == NULL) || (lock_interface_param == NULL)) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: invalid argument\n");
#endif

		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: lock_interface: lock_interface_param->default_stream_id=%x\n",
		lock_interface_param->default_stream_id);
	platform_printf ("TDISP: lock_interface: lock_interface_param->flags=%x\n",
		lock_interface_param->flags);
	platform_printf ("TDISP: lock_interface: lock_interface_param->mmio_reporting_offset=%x\n",
		lock_interface_param->mmio_reporting_offset);
#endif

	status = tdisp_driver_manticore_is_tdisp_controller_enabled (mdriver, &is_enabled);
	if (status != 0) {
		return status;
	}

	if (is_enabled == 0) {
		return TDISP_DRIVER_CONTROLLER_NOT_ENABLED;
	}

	status = tdisp_driver_manticore_get_vf_count (mdriver, &vf_count);
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: lock_interface: vf_count=%d\n", vf_count);
	platform_printf ("TDISP: lock_interface: function_index=%d\n", function_index);
	platform_printf ("TDISP: lock_interface: hw_function_index=%d\n", hw_function_index);
#endif
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: failed to get VF count, status=%x\n", status);
#endif

		return status;
	}

	/* function_index is 1 based */
	if (function_index > vf_count) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: invalid interface, function_id=%d, vf_count=%d\n",
			function_index, vf_count);
#endif

		return TDISP_DRIVER_INVALID_INTERFACE;
	}

	status = tdisp_driver_manticore_check_ide_stream_state (mdriver,
		lock_interface_param->default_stream_id);
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: IDE is not SECURE\n");
#endif

		return status;
	}

	status = tdisp_driver_manticore_get_tdi_hw_state (mdriver, hw_function_index, &tdi_hw_state);
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: lock_interface: tdi_hw_state=%x\n", tdi_hw_state);
#endif
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: failed to get TDI state, status=%x\n", status);
#endif

		return status;
	}

	if ((tdi_hw_state != TDISP_DRIVER_HW_STATE_NON_TEE) &&
		(tdi_hw_state != TDISP_DRIVER_HW_STATE_CONFIG_UNLOCKED)) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: invalid interface state, state=%x\n",
			tdi_hw_state);
#endif

		return TDISP_DRIVER_INTERFACE_INVALID_STATE;
	}

	/* HW says we are in a good state, so lets clear out context and start with clean */
	status = mdriver->tdi_context_manager->clear_tdi_context (mdriver->tdi_context_manager,
		function_index);
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: failed to clear context, status=%x\n", status);
#endif

		return status;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: lock_interface: lock_interface_param->flags.value=%x\n",
		lock_interface_param->flags.value);
	platform_printf ("TDISP: lock_interface: lock_interface_param->default_stream_id=%x\n",
		lock_interface_param->default_stream_id);
	platform_printf ("TDISP: lock_interface: lock_interface_param->mmio_reporting_offset_lsb=%x\n",
		lock_interface_param->mmio_reporting_offset & 0xFFFFFFFF);
	platform_printf ("TDISP: lock_interface: lock_interface_param->mmio_reporting_offset_msb=%x\n",
		(lock_interface_param->mmio_reporting_offset >> 32) & 0xFFFFFFFF);
	platform_printf ("TDISP: lock_interface: lock_interface_param->bind_p2p_address_mask=%x\n",
		lock_interface_param->bind_p2p_address_mask);
#endif

	status = mdriver->tdi_context_manager->set_lock_flags (mdriver->tdi_context_manager,
		function_index, lock_interface_param->flags.value);
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: failed to set lock flags, status=%x\n", status);
#endif

		return status;
	}

	status = mdriver->tdi_context_manager->set_default_ide_stream (mdriver->tdi_context_manager,
		function_index, lock_interface_param->default_stream_id);
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: failed to set default IDE stream, status=%x\n",
			status);
#endif

		return status;
	}

	status = mdriver->tdi_context_manager->set_mmio_reporting_offset (mdriver->tdi_context_manager,
		function_index, lock_interface_param->mmio_reporting_offset);
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: failed to set MMIO offset, status=%x\n", status);
#endif

		return status;
	}

	status = mdriver->tdi_context_manager->set_bind_p2p_address_mask (mdriver->tdi_context_manager,
		function_index, lock_interface_param->bind_p2p_address_mask);
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: failed to set P2P address mask, status=%x\n",
			status);
#endif

		return status;
	}

	// TODO: not locking MSIX to work around driver loading
	lut_entry.msix_l = 0;	//lock_interface_param->flags.lock_msix;
	lut_entry.non_t_m0 = 0;
	lut_entry.non_t_m1 = 0;
	lut_entry.non_t_m2 = 1;
	status = tdisp_driver_manticore_set_lut_entry (mdriver, hw_function_index, &lut_entry,
		TDISP_LUT_MASK_MSIX_L | TDISP_LUT_MASK_NON_T_M0 | TDISP_LUT_MASK_NON_T_M1 |
		TDISP_LUT_MASK_NON_T_M2);
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: failed to set MSIX lock, status=%x\n", status);
#endif

		return status;
	}

	status = tdisp_driver_manticore_set_tdi_hw_state (mdriver, hw_function_index,
		TDISP_DRIVER_HW_STATE_CONFIG_LOCKED);

	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: lock_interface: failed to set HW state, status=%x\n", status);
#endif

		return status;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	status = tdisp_driver_manticore_get_tdi_hw_state (mdriver, hw_function_index, &tdi_hw_state);
	if (status != 0) {
		platform_printf ("TDISP: lock_interface: failed to get HW state, status=%x\n", status);

		return status;
	}

	platform_printf ("TDISP: lock_interface: final tdi_hw_state=%x\n", tdi_hw_state);

	memset (&lut_entry, 0, sizeof (lut_entry));
	platform_printf ("TDISP: lock_interface:\n");
	status = tdisp_driver_manticore_get_lut_entry (mdriver, hw_function_index, &lut_entry);
	if (status != 0) {
		platform_printf ("TDISP: lock_interface: failed to get LUT entry, status=%x\n", status);

		return status;
	}

	platform_printf ("TDISP: lock_interface: REGISTERS:\n");
	// TDISP_CFG B01A0000 - 0xB01800000 =
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20000, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20010, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20080, 5);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20100, 4);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20150, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20180, 11);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20200, 9);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20300, 65);

	// PCIE_IDE
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60000, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6001C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60040, 8);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6024C, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602D4, 7);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602FC, 20);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6042C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60448, 2);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60500, 1);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x62000, 13);
#endif

	return status;
}

int tdisp_driver_manticore_get_device_interface_report (const struct tdisp_driver *tdisp_driver,
	uint32_t function_index, uint16_t request_offset, uint16_t request_length,
	uint16_t *report_length, uint8_t *interface_report, uint16_t *remainder_length)
{
	int status;
	const struct tdisp_driver_manticore *mdriver = TO_DERIVED_TYPE (tdisp_driver,
		const struct tdisp_driver_manticore, base);
	uint32_t hw_function_index = tdisp_driver_manticore_hw_function_index (function_index);
	struct tdisp_driver_manticore_interface_report report = {};
	uint32_t vf_count;
	uint32_t tdi_hw_state;
	struct tdisp_tdi_context tdi_context;
	uint32_t portion_length;
	struct interface_registers interface_info;
	const uint8_t *report_memory = (const uint8_t*) &report;
	uint32_t is_enabled = 0;

	if ((tdisp_driver == NULL) || (report_length == NULL) || (interface_report == NULL) ||
		(remainder_length == NULL)) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	if (request_offset >= sizeof (report)) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	status = tdisp_driver_manticore_is_tdisp_controller_enabled (mdriver, &is_enabled);
	if (status != 0) {
		return status;
	}

	if (is_enabled == 0) {
		return TDISP_DRIVER_CONTROLLER_NOT_ENABLED;
	}

	status = tdisp_driver_manticore_get_vf_count (mdriver, &vf_count);
	if (status != 0) {
		return status;
	}

	/* function_id is 1 based */
	if (function_index > vf_count) {
		return TDISP_DRIVER_INVALID_INTERFACE;
	}

	status = tdisp_driver_manticore_get_tdi_hw_state (mdriver, hw_function_index, &tdi_hw_state);
	if (status != 0) {
		return status;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: get_device_interface_report: final tdi_hw_state=%x\n", tdi_hw_state);
#endif

	if ((tdi_hw_state != TDISP_DRIVER_HW_STATE_RUN) &&
		(tdi_hw_state != TDISP_DRIVER_HW_STATE_CONFIG_LOCKED)) {
		return TDISP_DRIVER_INTERFACE_INVALID_STATE;
	}

	status = mdriver->tdi_context_manager->get_tdi_context (mdriver->tdi_context_manager,
		function_index, TDISP_TDI_CONTEXT_MASK_ALL, &tdi_context);
	if (status != 0) {
		return status;
	}

	if (((tdi_context.tdi_context_mask & TDISP_TDI_CONTEXT_MASK_LOCK_FLAGS) == 0) ||
		((tdi_context.tdi_context_mask & TDISP_TDI_CONTEXT_MASK_MMIO_REPORTING_OFFSET) == 0)) {
		return TDISP_DRIVER_INTERFACE_INVALID_STATE;
	}

	if (function_index == 0) {
		status = pcie_utils_read_pf0_registers (mdriver->pcie_registers, &interface_info);
	}
	else {
		status = pcie_utils_read_vf_registers (mdriver->pcie_registers, function_index,
			&interface_info);
	}
	if (status != 0) {
		return status;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: lock_interface: tdi_context.lock_flags=%x\n", tdi_context.lock_flags);
	platform_printf ("TDISP: lock_interface: tdi_context.default_ide_stream_id=%x\n",
		tdi_context.default_ide_stream_id);
	platform_printf ("TDISP: lock_interface: tdi_context.mmio_reporting_offset_lsb=%x\n",
		tdi_context.mmio_reporting_offset & 0xFFFFFFFF);
	platform_printf ("TDISP: lock_interface: tdi_context.mmio_reporting_offset_msb=%x\n",
		(tdi_context.mmio_reporting_offset >> 32) & 0xFFFFFFFF);
	platform_printf ("TDISP: lock_interface: tdi_context.bind_p2p_address_mask=%x\n",
		tdi_context.bind_p2p_address_mask);
	platform_printf ("TDISP: lock_interface: tdi_context.tdi_context_mask=%x\n",
		tdi_context.tdi_context_mask);

	platform_printf ("get_device_interface_report function_id=%d\n", function_index);
	platform_printf ("get_device_interface_report interface_info.bars[0].register_value=%x\n",
		interface_info.bars[0].register_value);
	platform_printf ("get_device_interface_report interface_info.bars[1].register_value=%x\n",
		interface_info.bars[1].register_value);
	platform_printf ("get_device_interface_report interface_info.bars[2].register_value=%x\n",
		interface_info.bars[2].register_value);
	platform_printf ("get_device_interface_report interface_info.bars[3].register_value=%x\n",
		interface_info.bars[3].register_value);
	platform_printf ("get_device_interface_report interface_info.bars[4].register_value=%x\n",
		interface_info.bars[4].register_value);
	platform_printf ("get_device_interface_report interface_info.bars[5].register_value=%x\n",
		interface_info.bars[5].register_value);
#endif

	report.base.interface_info.no_fw_update =
		(tdi_context.lock_flags & TDISP_LOCK_INTERFACE_FLAGS_NO_FW_UPDATE) != 0;
	report.base.interface_info.ats_supported = 0;
	report.base.interface_info.dma_requests_with_pasid = 0;
	report.base.interface_info.dma_requests_without_pasid = 1;
	report.base.interface_info.prs_supported = 0;

	report.base.msi_x_message_control = (uint16_t) (interface_info.msix_control.value & 0xffff);
	report.base.lnr_control = 0;
	report.base.tph_control = 0;
	report.base.mmio_range_count = 3;

	report.mmio_ranges[0].first_page = ((uint64_t) (interface_info.bars[0].register_value &
		0xfffffff0)) |
		(((uint64_t) (interface_info.bars[1].register_value & 0xfffffff0)) << 32);
	if (function_index != 0) {
		report.mmio_ranges[0].first_page += (function_index - 1) * 0x1000;
	}
	report.mmio_ranges[0].first_page += tdi_context.mmio_reporting_offset;

	report.mmio_ranges[0].number_of_pages = 1;
	report.mmio_ranges[0].range_attributes.is_mem_attr_updatable = 0;
	report.mmio_ranges[0].range_attributes.is_non_tee_mem = 0;
	report.mmio_ranges[0].range_attributes.msi_x_pba = 0;
	report.mmio_ranges[0].range_attributes.msi_x_table = 0;
	report.mmio_ranges[0].range_attributes.range_id = 0;

	report.mmio_ranges[1].first_page = ((uint64_t) (interface_info.bars[2].register_value &
		0xfffffff0)) |
		(((uint64_t) (interface_info.bars[3].register_value & 0xfffffff0)) << 32);
	if (function_index != 0) {
		report.mmio_ranges[1].first_page += (function_index - 1) * 0x1000;
	}
	report.mmio_ranges[1].first_page += tdi_context.mmio_reporting_offset;

	report.mmio_ranges[1].number_of_pages = 1;
	report.mmio_ranges[1].range_attributes.is_mem_attr_updatable = 0;
	report.mmio_ranges[1].range_attributes.is_non_tee_mem = 0;
	report.mmio_ranges[1].range_attributes.msi_x_pba = 0;
	report.mmio_ranges[1].range_attributes.msi_x_table = 0;
	report.mmio_ranges[1].range_attributes.range_id = 2;

	report.mmio_ranges[2].first_page = ((uint64_t) (interface_info.bars[4].register_value &
		0xfffffff0)) |
		(((uint64_t) (interface_info.bars[5].register_value & 0xfffffff0)) << 32);
	if (function_index != 0) {
		report.mmio_ranges[2].first_page += (function_index - 1) * 0x1000;
	}
	report.mmio_ranges[2].first_page += tdi_context.mmio_reporting_offset;

	report.mmio_ranges[2].number_of_pages = 1;
	report.mmio_ranges[2].range_attributes.is_mem_attr_updatable = 0;
	report.mmio_ranges[2].range_attributes.is_non_tee_mem = 0;
	report.mmio_ranges[2].range_attributes.msi_x_pba = 1;
	report.mmio_ranges[2].range_attributes.msi_x_table = 1;
	report.mmio_ranges[2].range_attributes.range_id = 4;

	report.device_specific_info_length = 0;

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("get_device_interface_report request_offset=%d\n", request_offset);
	platform_printf ("get_device_interface_report *report_length=%d\n", *report_length);
#endif
	portion_length = sizeof (report) - request_offset;
	if (portion_length > request_length) {
		portion_length = request_length;
	}
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("get_device_interface_report sizeof (report)=%d\n", sizeof (report));
	platform_printf ("get_device_interface_report portion_length=%d\n", portion_length);
#endif

	memcpy (interface_report, report_memory + request_offset, portion_length);

	*report_length = portion_length;
	*remainder_length = sizeof (report) - request_offset - portion_length;

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("get_device_interface_report *remainder_length=%d\n", *remainder_length);

	platform_printf ("TDISP: get_device_interface_report:\n");

	union tdisp_lut_entry lut_entry = {};

	status = tdisp_driver_manticore_get_lut_entry (mdriver, hw_function_index, &lut_entry);
	if (status != 0) {
		platform_printf ("TDISP: get_device_interface_report: failed to get LUT entry, status=%x\n",
			status);

		return status;
	}

	platform_printf ("TDISP: get_device_interface_report: REGISTERS:\n");
	// TDISP_CFG B01A0000 - 0xB01800000 =
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20000, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20010, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20080, 5);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20100, 4);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20150, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20180, 11);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20200, 9);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20300, 65);

	// PCIE_IDE
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60000, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6001C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60040, 8);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6024C, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602D4, 7);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602FC, 20);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6042C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60448, 2);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60500, 1);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x62000, 13);
#endif

	return 0;
}

int tdisp_driver_manticore_get_device_interface_state (const struct tdisp_driver *tdisp_driver,
	uint32_t function_index, uint8_t *tdi_state)
{
	int status;
	const struct tdisp_driver_manticore *mdriver = TO_DERIVED_TYPE (tdisp_driver,
		const struct tdisp_driver_manticore, base);
	uint32_t tdi_hw_state;
	uint32_t hw_function_index = tdisp_driver_manticore_hw_function_index (function_index);
	uint32_t vf_count;
	uint32_t is_enabled = 0;

	if ((tdisp_driver == NULL) || (tdi_state == NULL)) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: get_device_interface_state: invalid arg\n");
#endif

		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	status = tdisp_driver_manticore_is_tdisp_controller_enabled (mdriver, &is_enabled);
	if (status != 0) {
		return status;
	}

	if (is_enabled == 0) {
		return TDISP_DRIVER_CONTROLLER_NOT_ENABLED;
	}

	status = tdisp_driver_manticore_get_vf_count (mdriver, &vf_count);
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: get_device_interface_state: failed to get VF count, status=%x\n",
			status);
#endif

		return status;
	}

	/* function_id is 1 based */
	if (function_index > vf_count) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: get_device_interface_state: function_id=%d, vf_count=%d\n",
			function_index, vf_count);
#endif

		return TDISP_DRIVER_INVALID_INTERFACE;
	}

	status = tdisp_driver_manticore_get_tdi_hw_state (mdriver, hw_function_index, &tdi_hw_state);
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: get_device_interface_state: tdi_hw_state=0x%x\n", tdi_hw_state);
#endif
	if (status != 0) {
#ifdef TDISP_DRIVER_UART_SPEW
		platform_printf ("TDISP: get_device_interface_state: failed to HW state, status=%x\n",
			status);
#endif

		return status;
	}

	switch (tdi_hw_state) {
		case TDISP_DRIVER_HW_STATE_NON_TEE:
		case TDISP_DRIVER_HW_STATE_CONFIG_UNLOCKED:
			*tdi_state = TDISP_TDI_STATE_CONFIG_UNLOCKED;
#ifdef TDISP_DRIVER_UART_SPEW
			platform_printf (
				"TDISP: get_device_interface_state: tdi_state = TDISP_TDI_STATE_CONFIG_UNLOCKED\n");
#endif
			break;

		case TDISP_DRIVER_HW_STATE_CONFIG_LOCKED:
			*tdi_state = TDISP_TDI_STATE_CONFIG_LOCKED;
#ifdef TDISP_DRIVER_UART_SPEW
			platform_printf (
				"TDISP: get_device_interface_state: tdi_state = TDISP_TDI_STATE_CONFIG_LOCKED\n");
#endif
			break;

		case TDISP_DRIVER_HW_STATE_ERROR:
			*tdi_state = TDISP_TDI_STATE_ERROR;
#ifdef TDISP_DRIVER_UART_SPEW
			platform_printf (
				"TDISP: get_device_interface_state: tdi_state = TDISP_TDI_STATE_ERROR\n");
#endif
			break;

		case TDISP_DRIVER_HW_STATE_RUN:
			*tdi_state = TDISP_TDI_STATE_RUN;
#ifdef TDISP_DRIVER_UART_SPEW
			platform_printf (
				"TDISP: get_device_interface_state: tdi_state = TDISP_TDI_STATE_RUN\n");
#endif
			break;

		default:
			status = TDISP_DRIVER_INTERFACE_INVALID_STATE;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: get_device_interface_state:\n");
	union tdisp_lut_entry lut_entry = {};

	status = tdisp_driver_manticore_get_lut_entry (mdriver, hw_function_index, &lut_entry);
	if (status != 0) {
		platform_printf ("TDISP: get_device_interface_state: failed to get LUT entry, status=%x\n",
			status);

		return status;
	}

	platform_printf ("TDISP: get_device_interface_state: REGISTERS:\n");
	// TDISP_CFG B01A0000 - 0xB01800000 =
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20000, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20010, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20080, 5);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20100, 4);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20150, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20180, 11);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20200, 9);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20300, 65);

	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x400D4, 1);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x400D8, 1);

	// PCIE_IDE
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60000, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6001C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60040, 8);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6024C, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602D4, 7);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602FC, 20);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6042C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60448, 2);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60500, 1);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x62000, 13);
#endif

	return status;
}

int tdisp_driver_manticore_start_interface_request (const struct tdisp_driver *tdisp_driver,
	uint32_t function_index)
{
	int status;
	const struct tdisp_driver_manticore *mdriver = TO_DERIVED_TYPE (tdisp_driver,
		const struct tdisp_driver_manticore, base);
	uint32_t tdi_hw_state;
	uint32_t hw_function_index = tdisp_driver_manticore_hw_function_index (function_index);
	uint32_t vf_count;
	uint32_t is_enabled = 0;

	if (tdisp_driver == NULL) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	status = tdisp_driver_manticore_is_tdisp_controller_enabled (mdriver, &is_enabled);
	if (status != 0) {
		return status;
	}

	if (is_enabled == 0) {
		return TDISP_DRIVER_CONTROLLER_NOT_ENABLED;
	}

	status = tdisp_driver_manticore_get_vf_count (mdriver, &vf_count);
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: start_interface: vf_count=%d\n", vf_count);
#endif
	if (status != 0) {
		return status;
	}

	/* function_id is 1 based */
	if (function_index > vf_count) {
		return TDISP_DRIVER_INVALID_INTERFACE;
	}

	status = tdisp_driver_manticore_get_tdi_hw_state (mdriver, hw_function_index, &tdi_hw_state);
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: start_interface: tdi_hw_state=%x\n", tdi_hw_state);
#endif
	if (status != 0) {
		return status;
	}

	if (tdi_hw_state != TDISP_DRIVER_HW_STATE_CONFIG_LOCKED) {
		return TDISP_DRIVER_INTERFACE_INVALID_STATE;
	}

	status = tdisp_driver_manticore_set_tdi_hw_state (mdriver, hw_function_index,
		TDISP_DRIVER_HW_STATE_RUN);
	if (status != 0) {
		return status;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: start_interface_request:\n");
	union tdisp_lut_entry lut_entry = {};

	status = tdisp_driver_manticore_get_lut_entry (mdriver, hw_function_index, &lut_entry);
	if (status != 0) {
		platform_printf ("TDISP: start_interface_request: failed to get LUT entry, status=%x\n",
			status);

		return status;
	}

	platform_printf ("TDISP: start_interface_request: REGISTERS:\n");
	// TDISP_CFG B01A0000 - 0xB01800000 =
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20000, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20010, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20080, 5);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20100, 4);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20150, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20180, 11);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20200, 9);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20300, 65);

	// PCIE_IDE
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60000, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6001C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60040, 8);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6024C, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602D4, 7);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602FC, 20);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6042C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60448, 2);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60500, 1);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x62000, 13);
#endif

	observable_notify_observers_with_ptr (&mdriver->state->observable,
		offsetof (struct tdisp_driver_observer, on_start_interface), &function_index);

	return status;
}

int tdisp_driver_manticore_stop_interface_request (const struct tdisp_driver *tdisp_driver,
	uint32_t function_index)
{
	int status;
	const struct tdisp_driver_manticore *mdriver = TO_DERIVED_TYPE (tdisp_driver,
		const struct tdisp_driver_manticore, base);
	uint32_t hw_function_index = tdisp_driver_manticore_hw_function_index (function_index);
	uint32_t vf_count;
	uint32_t tdi_hw_state;
	uint32_t is_enabled = 0;

	if (tdisp_driver == NULL) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	status = tdisp_driver_manticore_is_tdisp_controller_enabled (mdriver, &is_enabled);
	if (status != 0) {
		return status;
	}

	if (is_enabled == 0) {
		return TDISP_DRIVER_CONTROLLER_NOT_ENABLED;
	}

	status = tdisp_driver_manticore_get_vf_count (mdriver, &vf_count);
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: stop_interface: vf_count=%d\n", vf_count);
#endif
	if (status != 0) {
		return status;
	}

	/* function_id is 1 based */
	if (function_index > vf_count) {
		return TDISP_DRIVER_INVALID_INTERFACE;
	}

	status = tdisp_driver_manticore_get_tdi_hw_state (mdriver, hw_function_index, &tdi_hw_state);
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: stop_interface: hw_state = %x\n", tdi_hw_state);
#endif
	if (status != 0) {
		return status;
	}

	status = tdisp_driver_manticore_set_tdi_hw_state (mdriver, hw_function_index,
		TDISP_DRIVER_HW_STATE_CONFIG_UNLOCKED);
	if (status != 0) {
		return status;
	}

	status = mdriver->tdi_context_manager->clear_tdi_context (mdriver->tdi_context_manager,
		function_index);

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP: stop_interface: REGISTERS:\n");
	// TDISP_CFG B01A0000 - 0xB01800000 =
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20000, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20010, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20080, 5);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20100, 4);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20150, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20180, 11);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20200, 9);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x20300, 65);

	// PCIE_IDE
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60000, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6001C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60040, 8);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6024C, 3);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602D4, 7);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x602FC, 20);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x6042C, 6);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60448, 2);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x60500, 1);
	tdisp_driver_manticore_dump_register_block (mdriver->pcie_registers, 0xB0180000, 0x62000, 13);
#endif

	observable_notify_observers_with_ptr (&mdriver->state->observable,
		offsetof (struct tdisp_driver_observer, on_stop_interface), &function_index);

	return status;
}

int tdisp_driver_manticore_get_mmio_ranges (const struct tdisp_driver *tdisp_driver,
	uint32_t function_id, uint32_t mmio_range_count, struct tdisp_mmio_range *mmio_ranges)
{
	UNUSED (tdisp_driver);
	UNUSED (function_id);
	UNUSED (mmio_range_count);
	UNUSED (mmio_ranges);

	return TDISP_DRIVER_NOT_IMPLEMENTED;
}

/**
 * Set all TDISP interfaces in LOCKED or RUN state to ERROR state.  This function will not trigger
 * any related interrupts.  This can be useful in scenarios where the device crashed and the
 * synchronization between the firmware state and hardware state needs to be restored.
 *
 * @param tdisp_driver instance of TDISP driver
 * @param pcie_registers MMIO register block for the PCIe device
 *
 * @return 0 if all LOCKED and RUN interfaces were successfully set to ERROR state or an error code.
 */
int tdisp_driver_manticore_set_all_error_state (const struct tdisp_driver_manticore *tdisp_driver,
	const struct mmio_register_block *pcie_registers)
{
	uint32_t i;
	uint32_t hw_state;
	int status = 0;
	uint32_t is_enabled = 0;

	if ((tdisp_driver == NULL) || (pcie_registers == NULL)) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	status = tdisp_driver_manticore_is_tdisp_controller_enabled (tdisp_driver, &is_enabled);
	if (status != 0) {
		return status;
	}

	/* if TDISP controller is not enable, return success */
	if (is_enabled == 0) {
		return 0;
	}

	status = pcie_registers->map (pcie_registers);
	if (status != 0) {
		return status;
	}

	/* Disable TDISP ERROR-related interrupts. */
	/* Disable TDISP ERROR interrupt. */
	status = mmio_register_block_clear_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_INT_EN_REGISTER, 0);
	if (status != 0) {
		goto exit;
	}

	/* Disable TDISP ERROR transition interrupt for VF0-31. */
	status = mmio_register_block_write_bits_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN0, 0, 32, 0x00000000);
	if (status != 0) {
		goto exit;
	}

	/* Disable TDISP ERROR transition interrupt for VF32-63. */
	status = mmio_register_block_write_bits_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN1, 0, 32, 0x00000000);
	if (status != 0) {
		goto exit;
	}

	/* Disable TDISP ERROR transition interrupt for PF. */
	status = mmio_register_block_clear_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN2, 0);
	if (status != 0) {
		goto exit;
	}

	/* Unmap registers to match previous map. Helper functions doing their own map/unmap. */
	pcie_registers->unmap (pcie_registers);

	for (i = 0; i < TDISP_TDI_MAX_COUNT; i++) {
		status = tdisp_driver_manticore_get_tdi_hw_state (tdisp_driver, i, &hw_state);
		if (status != 0) {
			return status;
		}

		if ((hw_state == TDISP_DRIVER_HW_STATE_CONFIG_LOCKED) ||
			(hw_state == TDISP_DRIVER_HW_STATE_RUN)) {
			status = tdisp_driver_manticore_set_tdi_hw_state (tdisp_driver, i,
				TDISP_DRIVER_HW_STATE_ERROR);
			if (status != 0) {
				return status;
			}
		}
	}

	/* Re-map registers again */
	status = pcie_registers->map (pcie_registers);
	if (status != 0) {
		return status;
	}

	/* Re-enable TDISP ERROR-related interrupts. */
	/* Enable TDISP ERROR interrupt. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_INT_EN_REGISTER, 0);
	if (status != 0) {
		goto exit;
	}

	/* Enable TDISP ERROR transition interrupt for VF0-31. */
	status = mmio_register_block_write_bits_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN0, 0, 32, 0xFFFFFFFF);
	if (status != 0) {
		goto exit;
	}

	/* Enable TDISP ERROR transition interrupt for VF32-63. */
	status = mmio_register_block_write_bits_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN1, 0, 32, 0xFFFFFFFF);
	if (status != 0) {
		goto exit;
	}

	/* Enable TDISP ERROR transition interrupt for PF. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN2, 0);

exit:
	pcie_registers->unmap (pcie_registers);

	return status;
}

/**
 * Initialize TDISP driver
 *
 * @param tdisp_driver instance of TDISP driver to initialize
 * @param tdi_context_manager TDI context manager to use for TDISP driver
 * @param pcie_registers MMIO register block for the PCIe device
 * @param ide IDE driver to use for TDISP driver
 * @param state The variable context for the TDISP driver
 *
 * @return 0 if successfully initialized or error code otherwise
 */
int tdisp_driver_manticore_init (struct tdisp_driver_manticore *tdisp_driver,
	const struct tdisp_tdi_context_manager *tdi_context_manager,
	const struct mmio_register_block *pcie_registers, const struct ide_driver *ide,
	struct tdisp_driver_manticore_state *state)
{
	if ((tdisp_driver == NULL) || (tdi_context_manager == NULL) || (pcie_registers == NULL) ||
		(ide == NULL) || (state == NULL)) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	memset (tdisp_driver, 0, sizeof (*tdisp_driver));

	tdisp_driver->state = state;

	tdisp_driver->base.get_function_index = tdisp_driver_manticore_get_function_index;
	tdisp_driver->base.get_tdisp_capabilities = tdisp_driver_manticore_get_tdisp_capabilities;
	tdisp_driver->base.lock_interface_request = tdisp_driver_manticore_lock_interface_request;
	tdisp_driver->base.get_device_interface_report =
		tdisp_driver_manticore_get_device_interface_report;
	tdisp_driver->base.get_device_interface_state =
		tdisp_driver_manticore_get_device_interface_state;
	tdisp_driver->base.start_interface_request = tdisp_driver_manticore_start_interface_request;
	tdisp_driver->base.stop_interface_request = tdisp_driver_manticore_stop_interface_request;
	tdisp_driver->base.get_mmio_ranges = tdisp_driver_manticore_get_mmio_ranges;

	tdisp_driver->tdi_context_manager = tdi_context_manager;
	tdisp_driver->pcie_registers = pcie_registers;
	tdisp_driver->ide = ide;

	return tdisp_driver_manticore_init_state (tdisp_driver);
}

/**
 * Initialize only the variable state for the TDISP driver.  The rest of the driver is assumed to
 * have been initialized.
 *
 * @param tdisp_driver instance of TDISP driver to initialize the state for
 *
 * @return 0 if the TDISP driver state was successfully initialized or an error code.
 */
int tdisp_driver_manticore_init_state (const struct tdisp_driver_manticore *tdisp_driver)
{
	if ((tdisp_driver == NULL) || (tdisp_driver->state == NULL)) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	return observable_init (&tdisp_driver->state->observable);
}

/**
 * Release resources used by TDISP driver
 *
 * @param tdisp_driver instance of TDISP driver to be cleaned up
 */
void tdisp_driver_manticore_release (const struct tdisp_driver_manticore *tdisp_driver)
{
	if ((tdisp_driver) && (tdisp_driver->state)) {
		observable_release (&tdisp_driver->state->observable);
	}
}

/**
 * Enable TDISP driver
 *
 * @param tdisp_driver instance of TDISP driver to enable
 *
 * @return 0 if successfully enabled or error code otherwise
 */
int tdisp_driver_manticore_enable (const struct tdisp_driver_manticore *tdisp_driver)
{
	int status;
	uint32_t value;
	uint32_t fc_addr;
	uint32_t is_enabled = 0;

	if (tdisp_driver == NULL) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif

	status = tdisp_driver_manticore_is_tdisp_controller_enabled (tdisp_driver, &is_enabled);
	if (status != 0) {
		return status;
	}

	/* if it is already enbaled, return success */
	if (is_enabled != 0) {
		return 0;
	}

	status = tdisp_driver->pcie_registers->map (tdisp_driver->pcie_registers);
	if (status != 0) {
		return status;
	}

	/* TDISP controller is to be enabled.  Ensure that error state registers are cleared. */
	status = mmio_register_block_write_bits_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_INT_STATUS_REGISTER, 1, 8, 0xFF);
	if (status != 0) {
		goto exit;
	}

	status = mmio_register_block_write_bits_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_ST0, 0, 32, 0xFFFFFFFF);
	if (status != 0) {
		goto exit;
	}

	status = mmio_register_block_write_bits_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_ST1, 0, 32, 0xFFFFFFFF);
	if (status != 0) {
		goto exit;
	}

	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_ST2, 0);
	if (status != 0) {
		goto exit;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	/* Enable TDISP controller	PCIE_TDISP_CFG 0xB01A0000 */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_CTRL_REGISTER, 0);
	if (status != 0) {
		goto exit;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	/* Enable TDISP CII controller */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_CII_CTRL_REGISTER, 0);
	if (status != 0) {
		goto exit;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	/* Enable ERROR transition when IDE link stream goes unsecure */
	status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL0_REGISTER, 0xffffffff);
	if (status != 0) {
		goto exit;
	}
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL1_REGISTER, 0xffffffff);
	if (status != 0) {
		goto exit;
	}
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL2_REGISTER, 0x00000001);
	if (status != 0) {
		goto exit;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	/* Enable ERROR transition when IDE selective stream goes unsecure */
	status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL3_REGISTER, 0xffffffff);
	if (status != 0) {
		goto exit;
	}
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL4_REGISTER, 0xffffffff);
	if (status != 0) {
		goto exit;
	}
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_EC_IDE_CTRL5_REGISTER, 0x00000001);
	if (status != 0) {
		goto exit;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	/* Enable ERROR transition when error/configuration happens */
	status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_EC_ST_CTRL0_REGISTER, 0xffffffff);
	if (status != 0) {
		goto exit;
	}
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_EC_ST_CTRL1_REGISTER, 0xffffffff);
	if (status != 0) {
		goto exit;
	}
#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_EC_ST_CTRL2_REGISTER, 0x00000001);
	if (status != 0) {
		goto exit;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	/* Block MSI-X request generated in CONFIG_UNLOCKED state (regardless of US_ALLOW_REQ_CU). */
	status = mmio_register_block_clear_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_US_CTRL_REGISTER, 2);
	if (status != 0) {
		goto exit;
	}

	/* Block MSI-X request generated in ERROR state. */
	status = mmio_register_block_clear_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_US_CTRL_REGISTER, 3);
	if (status != 0) {
		goto exit;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable %d\n", __LINE__);
#endif
	/* Enable ERROR transition when upstream request is rejected */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_US_CTRL_REGISTER, 4);
	if (status != 0) {
		goto exit;
	}

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("tdisp_driver_manticore_enable: enable TBit sourcing\n");
#endif
	/* IDE_TBIT_IF_SRC_SEL = 1 T bit received on the Controller IDE interface */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_PCIE_IDE_GLBL_CFG_REGISTER, 7);
	if (status != 0) {
		goto exit;
	}

	/* 0xB01A0500 - 0xB01A0558
	Run through all FC configuration entries and make sure all FC = 3 are being
	downgraded to FC = 2, giving HW full control for now.
	TODO: make sure IDE registers are still could trigger FW override flow, so
	FW could intercept IDE registers modifications.	*/
	for (fc_addr = TDISP_DRIVER_MANTICORE_TDISP_LUT_CFG_FC0_REGISTER;
		fc_addr <= TDISP_DRIVER_MANTICORE_TDISP_LUT_CFG_FC22_REGISTER;
		fc_addr += sizeof (uint32_t)) {
		status = tdisp_driver->pcie_registers->read32_by_addr (tdisp_driver->pcie_registers,
			fc_addr, &value);
		if (status != 0) {
			goto exit;
		}

		uint32_t new_value = 0;

		for (uint32_t j = 0; j < 16; j++) {
			if ((value & 0x3) == 0x3) {
				new_value |= (0x2 << (j * 2));
			}
			else {
				new_value |= ((value & 0x3) << (j * 2));
			}
			value >>= 2;
		}

		status = tdisp_driver->pcie_registers->write32_by_addr (tdisp_driver->pcie_registers,
			fc_addr, new_value);
		if (status != 0) {
			goto exit;
		}
	}

	/* STATUS_COMMAND_REG */
	status = tdisp_driver_manticore_set_hc (tdisp_driver->pcie_registers, 0x004, 1);
	/* BARx_REG  */
	status |= tdisp_driver_manticore_set_hc (tdisp_driver->pcie_registers, 0x010, 1);
	status |= tdisp_driver_manticore_set_hc (tdisp_driver->pcie_registers, 0x014, 1);
	status |= tdisp_driver_manticore_set_hc (tdisp_driver->pcie_registers, 0x018, 1);
	status |= tdisp_driver_manticore_set_hc (tdisp_driver->pcie_registers, 0x01C, 1);
	status |= tdisp_driver_manticore_set_hc (tdisp_driver->pcie_registers, 0x020, 1);
	status |= tdisp_driver_manticore_set_hc (tdisp_driver->pcie_registers, 0x024, 1);
	status |= tdisp_driver_manticore_set_hc (tdisp_driver->pcie_registers, 0x030, 1);
	/* IDE_LINK_STREAM_CTRL_0 */
	status |= tdisp_driver_manticore_set_hc (tdisp_driver->pcie_registers, 0x3D0, 1);

	if (status != 0) {
		goto exit;
	}

	/* CII_CTRL 0xB01A0200
	 * LUT_CFG_BF.VALUE upate enable. Regardless of CII_CTRL_EN, LUT_CFG_BF is updated
	 * when CII_LUT_UPT_EN = 1. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_CII_CTRL_REGISTER, 5);
	if (status != 0) {
		goto exit;
	}

	/* Enable IDE interrupts. */

	/* Enable interrupt for TX Kbit toggling. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_PCIE_IDE_IRQ_EN_REGISTER, 7);
	if (status != 0) {
		goto exit;
	}

	/* Enable interrupt for RX Kbit toggling. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_PCIE_IDE_IRQ_EN_REGISTER, 8);
	if (status != 0) {
		goto exit;
	}

	/* Enable interrupt for stream transition to INSECURE. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_PCIE_IDE_IRQ_EN_REGISTER, 12);
	if (status != 0) {
		goto exit;
	}

	/* Enable global IDE interrupt. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_PCIE_IDE_IRQ_EN_REGISTER, 31);
	if (status != 0) {
		goto exit;
	}

	/* Enable TDISP interrupts. */

	/* Enable PCIe Core TDISP interrupt. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_PCIE_CORE_INT_EN_REGISTER, 21);
	if (status != 0) {
		goto exit;
	}

	/* Enable TDISP error interrupt. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_INT_EN_REGISTER, 0);
	if (status != 0) {
		goto exit;
	}

	/* Enable TDISP error detection interrupt. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_INT_EN_REGISTER, 6);
	if (status != 0) {
		goto exit;
	}

	/* Enable TDISP ERROR transition interrupt for VF0-31. */
	status = mmio_register_block_write_bits_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN0, 0, 32, 0xFFFFFFFF);
	if (status != 0) {
		goto exit;
	}

	/* Enable TDISP ERROR transition interrupt for VF32-63. */
	status = mmio_register_block_write_bits_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN1, 0, 32, 0xFFFFFFFF);
	if (status != 0) {
		goto exit;
	}

	/* Enable TDISP ERROR transition interrupt for PF. */
	status = mmio_register_block_set_bit_by_addr (tdisp_driver->pcie_registers,
		TDISP_DRIVER_MANTICORE_TDISP_ERR_INT_EN2, 0);

exit:
	tdisp_driver->pcie_registers->unmap (tdisp_driver->pcie_registers);

#ifdef TDISP_DRIVER_UART_SPEW
	platform_printf ("TDISP INIT DONE\nREGISTERS:\n");
	// TDISP_CFG B01A0000 - 0xB01800000 =
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20000,
		3);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20010,
		6);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20080,
		5);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20100,
		4);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20150,
		3);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20180,
		11);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20200,
		9);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20300,
		65);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20500,
		23);	// LUT_CFG_FC
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20568,
		23);	// LUT_CFG_HC
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x20600,
		96);	// LUT_BF

	// PCIE_IDE
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x60000,
		6);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x6001C,
		6);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x60040,
		8);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x6024C,
		3);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x602D4,
		7);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x602FC,
		20);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x6042C,
		6);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x60448,
		2);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x60500,
		1);
	tdisp_driver_manticore_dump_register_block (tdisp_driver->pcie_registers, 0xB0180000, 0x62000,
		13);
#endif

	return status;
}

/**
 * Add an observer for TDISP driver notifications.
 *
 * @param tdisp_driver The TDISP driver instance to register with.
 * @param observer The observer to add.
 *
 * @return 0 if the observer was successfully added or an error code.
 */
int tdisp_driver_manticore_add_tdisp_driver_observer (
	const struct tdisp_driver_manticore *tdisp_driver, const struct tdisp_driver_observer *observer)
{
	if (tdisp_driver == NULL) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	return observable_add_observer (&tdisp_driver->state->observable, (void*) observer);
}

/**
 * Remove an observer from TDISP driver notifications.
 *
 * @param tdisp The TDISP driver instance to deregister from.
 * @param observer The observer to remove.
 *
 * @return 0 if the observer was successfully removed or an error code.
 */
int tdisp_driver_manticore_remove_tdisp_driver_observer (
	const struct tdisp_driver_manticore *tdisp_driver, const struct tdisp_driver_observer *observer)
{
	if (tdisp_driver == NULL) {
		return TDISP_DRIVER_INVALID_ARGUMENT;
	}

	return observable_remove_observer (&tdisp_driver->state->observable, (void*) observer);
}
