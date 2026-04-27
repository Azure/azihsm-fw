// Copyright (c) Microsoft Corporation. All rights reserved.
//
// ============================================================================
//
//     Copyright (c) Marvell Corporation 2023  -  All rights reserved
//
//  This computer program contains confidential and proprietary information,
//  and  may NOT  be reproduced or transmitted, in whole or in part,  in any
//  form,  or by any means electronic, mechanical, photo-optical, or  other-
//  wise, and  may NOT  be translated  into  another  language  without  the
//  express written permission from Marvell Corporation.
//
// ============================================================================
// =      C O M P A N Y      P R O P R I E T A R Y      M A T E R I A L       =
// ============================================================================

#include <stdio.h>
#include <string.h>
#include "pcie_phy.h"
#include "platform_api.h"
#include "rot_memory_map.h"
#include "soc_shared.h"
#include "sp_boot.h"
#include "common/common_math.h"
#include "crypto/ecdsa.h"
#include "init/init_error.h"
#include "mmio/mmio_register_block_soc_static.h"


/**
 * Expected checksum for PHY firmware images.
 */
#define	PCIE_PHY_MCU_FW_CHECKSUM		0

/**
 * Memory offset for the main PHY firmware image.
 */
#define	PCIE_PHY_MAIN_MCU_FW_OFFSET		0x10000

/**
 * Memory offset for the common PHY firmware image.
 */
#define	PCIE_PHY_COMMON_MCU_FW_OFFSET	0xe000

/**
 * Memory offset for the lane PHY firmware image.
 */
#define	PCIE_PHY_LANE_MCU_FW_OFFSET		0x6000

/* The DMB driver, which is defined externally, and in different locations, for 1SP and SPRT. */
extern const struct hsp_dmb dmb;

/**
 * Variable context for accessing PCIe PHY registers.
 */
static struct mmio_register_block_soc_state phy_regs_context;

/**
 * MMIO handler for accessing PCIe PHY registers.
 */
const struct mmio_register_block_soc phy_regs =
	mmio_register_block_soc_static_init (&phy_regs_context, &dmb, PCIE_PHY_REGS_BASE_ADDR,
	sizeof (struct pcie_phy_regs));


/**
 * Load a single firmware image into the PCIe PHY controller.
 *
 * @param comphy The registers for the PHY.
 * @param fw Firmware image to load.
 * @param comphy_mem PHY memory space where the firmware will be loaded.
 *
 * @return 0 if the PHY firmware was loaded successfully or an error code.
 */
static int pcie_phy_load_single_firmware (uintptr_t comphy, const struct pcie_phy_fw *fw,
	uintptr_t comphy_mem)
{
	uint32_t fw_offset;
	bool checksum_good = false;
	int status;

	/* Initialize checksum calculation. */
	switch (fw->type) {
		case PCIE_PHY_FW_TYPE_MAIN:
			status = mmio_register_block_write_bits (&phy_regs.base,
				comphy + offsetof (struct pcie_phy_regs, comphy0.memoryControl0),
				COMPHY_MEMORY_CTRL0_PROG_RAM_SEL_1_0_BIT,
				COMPHY_MEMORY_CTRL0_PROG_RAM_SEL_1_0_COUNT, 0);
			if (status != 0) {
				return status;
			}

			status = phy_regs.base.write32 (&phy_regs.base,
				comphy + offsetof (struct pcie_phy_regs, comphy0.memoryControl2PmemChecksumExp310),
				PCIE_PHY_MCU_FW_CHECKSUM);
			if (status != 0) {
				return status;
			}

			fw_offset = PCIE_PHY_MAIN_MCU_FW_OFFSET;
			break;

		case PCIE_PHY_FW_TYPE_COMMON:
			status = mmio_register_block_set_bit (&phy_regs.base,
				comphy + offsetof (struct pcie_phy_regs, comphy0.xdataMemChecksumCmn2),
				COMPHY_CHKSUM_COMMON2_XDATA_MEM_CHECKSUM_RESET_CMN_BIT);
			if (status != 0) {
				return status;
			}

			status = mmio_register_block_clear_bit (&phy_regs.base,
				comphy + offsetof (struct pcie_phy_regs, comphy0.xdataMemChecksumCmn2),
				COMPHY_CHKSUM_COMMON2_XDATA_MEM_CHECKSUM_RESET_CMN_BIT);
			if (status != 0) {
				return status;
			}

			status = phy_regs.base.write32 (&phy_regs.base,
				comphy + offsetof (struct pcie_phy_regs,
				comphy0.xdataMemChecksumCmn0XdataMemChecksumExpCmn310),	PCIE_PHY_MCU_FW_CHECKSUM);
			if (status != 0) {
				return status;
			}

			fw_offset = PCIE_PHY_COMMON_MCU_FW_OFFSET;
			break;

		case PCIE_PHY_FW_TYPE_LANE:
			status = phy_regs.base.write32 (&phy_regs.base,
				comphy + offsetof (struct pcie_phy_regs,
				comphy0.xdataMemChecksumLane0XdataMemChecksumExpLane310), PCIE_PHY_MCU_FW_CHECKSUM);
			if (status != 0) {
				return status;
			}

			fw_offset = PCIE_PHY_LANE_MCU_FW_OFFSET;
			break;

		default:
			return INIT_PHY_INIT_FAILED;
	}

	/* Copy firmware data to PHY memory. */
	status = phy_regs.base.block_write32 (&phy_regs.base, comphy_mem + fw_offset, &fw->data,
		fw->length);
	if (status != 0) {
		return status;
	}

	/* Confirm the checksum is correct. */
	switch (fw->type) {
		case PCIE_PHY_FW_TYPE_MAIN:
			status = mmio_register_block_read_bit (&phy_regs.base,
				comphy + offsetof (struct pcie_phy_regs, comphy0.memoryControl4),
				COMPHY_MEMORY_CTRL4_PMEM_CHECKSUM_PASS_BIT, &checksum_good);
			break;

		case PCIE_PHY_FW_TYPE_COMMON:
			status = mmio_register_block_read_bit (&phy_regs.base,
				comphy + offsetof (struct pcie_phy_regs, comphy0.xdataMemChecksumCmn2),
				COMPHY_CHKSUM_COMMON2_XDATA_MEM_CHECKSUM_PASS_CMN_BIT, &checksum_good);
			break;

		case PCIE_PHY_FW_TYPE_LANE:
			status = mmio_register_block_read_bit (&phy_regs.base,
				comphy + offsetof (struct pcie_phy_regs, comphy0.mcuMemReg2Lane),
				COMPHY_MEM_REG2_LANE_XDATA_MEM_CHECKSUM_PASS_LANE_BIT, &checksum_good);
			break;
	}

	if (status == 0) {
		return (checksum_good) ? 0 : INIT_PHY_INIT_FAILED;
	}
	else {
		return status;
	}
}

/**
 * Enable PHY controller memory access.
 *
 * @return 0 if memory access was enabled or an error code.
 */
static int pcie_phy_set_direct_access_enable ()
{
	int status;

	/* Pin direct access enable */
	status = mmio_register_block_set_bit (&phy_regs.base,
		offsetof (struct pcie_phy_regs, pcie_top.ctrl1P0),
		PCIE_TOP_CTRL1PX_DIRECT_ACCESS_EN_PX_BIT);
	if (status != 0) {
		return status;
	}

	status = mmio_register_block_set_bit (&phy_regs.base,
		offsetof (struct pcie_phy_regs, pcie_top.ctrl1P1),
		PCIE_TOP_CTRL1PX_DIRECT_ACCESS_EN_PX_BIT);
	if (status != 0) {
		return status;
	}

	platform_msleep (1);

	return 0;
}

/**
 * Disable PHY controller memory access.
 *
 * @return 0 if memory access was disabled or an error code.
 */
static int pcie_phy_clear_direct_access_enable ()
{
	int status;

	/* Pin direct access disable */
	status = mmio_register_block_clear_bit (&phy_regs.base,
		offsetof (struct pcie_phy_regs, pcie_top.ctrl1P0),
		PCIE_TOP_CTRL1PX_DIRECT_ACCESS_EN_PX_BIT);
	if (status != 0) {
		return status;
	}

	status = mmio_register_block_clear_bit (&phy_regs.base,
		offsetof (struct pcie_phy_regs, pcie_top.ctrl1P1),
		PCIE_TOP_CTRL1PX_DIRECT_ACCESS_EN_PX_BIT);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Load all firmware images into PCIe PHY controllers.
 *
 * @param pcie The descriptor for the PCIe PHY initialization.
 *
 * @return 0 if the PCIe PHY was initialized successfully or an error code.
 */
int pcie_phy_load_firmware (const struct pcie_phy *pcie)
{
	int i;
	int status;

	status = mmio_register_block_soc_init_state (&phy_regs);
	if (status != 0) {
		return status;
	}

	status = phy_regs.base.map (&phy_regs.base);
	if (status != 0) {
		return status;
	}

	/* Execute PCIe Controller Cold Reset Sequence
	 *
	 * Assert Cold Reset
	 * Stall for 1us
	 * Deassert Cold Reset
	 * Stall for 10us */
	status = mmio_register_block_set_bit (&phy_regs.base,
		offsetof (struct pcie_phy_regs, pcie_top.pcieResetControl),
		PCIE_TOP_RESET_CONTROL_PCIE_X4_COLD_RST_BIT);
	if (status != 0) {
		return status;
	}
	platform_msleep (1);

	status = mmio_register_block_clear_bit (&phy_regs.base,
		offsetof (struct pcie_phy_regs, pcie_top.pcieResetControl),
		PCIE_TOP_RESET_CONTROL_PCIE_X4_COLD_RST_BIT);
	if (status != 0) {
		return status;
	}
	platform_msleep (1);

	status = pcie_phy_set_direct_access_enable ();
	if (status != 0) {
		return status;
	}

	/* Lead PHY0 firmware images. */
	for (i = 0; i < 3; i++) {
		status = pcie_phy_load_single_firmware (offsetof (struct pcie_phy_regs, comphy0),
			pcie->fw[i], offsetof (struct pcie_phy_regs, comphy0_mem));
		if (status != 0) {
			return status;
		}
	}

	/* Lead PHY1 firmware images. */
	for (i = 0; i < 3; i++) {
		status = pcie_phy_load_single_firmware (offsetof (struct pcie_phy_regs, comphy1),
			pcie->fw[i], offsetof (struct pcie_phy_regs, comphy1_mem));
		if (status != 0) {
			return status;
		}
	}

	status = pcie_phy_clear_direct_access_enable ();

	phy_regs.base.unmap (&phy_regs.base);

	return status;
}
