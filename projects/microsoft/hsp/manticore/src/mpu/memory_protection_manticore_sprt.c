// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "memory_protection_manticore_sprt.h"
#include "common/array_size.h"
#include "common/unused.h"
#include "mpu/fence_manticore_static.h"
#ifdef MANTICORE_MEMORY_FENCING_SPRT_CONFIG_AEBS
#include "platform_io_api.h"
#include "drivers/hsp_aeb_static.h"
#include "system/manticore_aeb.h"
#endif


#ifdef MANTICORE_MEMORY_FENCING_SPRT_CONFIG_AEBS
/**
 * Variable context for the AEB driver.
 */
static struct hsp_aeb_state aeb_context;

/**
 * Interface for configuring AEBs.
 */
static const struct hsp_aeb aeb = hsp_aeb_static_init (&aeb_context,
	(struct Creg_regs_aeb_regs*) HSP_ADDR_MAP_CREG_AEB_INTERFACE_ADDRESS);
#endif


/**
 * Memory fencing memory map showing the protected range of addresses for each fencing block
 *
 * Start Address	End Address	Size 	Fence Module
 * 0x60000000	0x607FFFFF	0x00800000	FENCE_DUAL_CP
 * 0x61000000	0x615FFFFF	0x00600000	FENCE_GSRAM
 * 0xA0000000	0xA00FFFFF	0x00100000	FENCE_GDMA
 * 0xA0100000	0xA01FFFFF	0x00100000	FENCE_PCIE
 * 0xA0200000	0xA02FFFFF	0x00100000	FENCE_AES
 * 0xA0300000	0xA03FFFFF	0x00100000	FENCE_RNG
 * 0xA0400000	0xA04FFFFF	0x00100000	FENCE_hssha
 * 0xA0C00000	0xA0FFFFFF	0x00400000	FENCE_BCP
 * 0xA1000000	0xA4FFFFFF	0x04000000	FENCE_NQM
 * 0xB0000000	0xB01FFFFF	0x00200000	FENCE_APB
 * 0xB0400000	0xB05FFFFF	0x00200000
 * 0xC0000000	0xC0007FFF	0x00008000	FENCE_UPKAB0
 * 0xC0008000	0xC000FFFF	0x00008000	FENCE_UPKAB1
 */

/**
 * Memory fencing entries for APB block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_apb_entries[] = {
	/* HSP_MAILBOX_0 */
	{
		.memory_region = {
			.start = (const void*) 0xB0000000,
			.length = 0x1000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1,
	},

	/* POR */
	{
		.memory_region = {
			.start = (const void*) 0xB0004000,
			.length = 0x1000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_CP0,
		.write_access_bits = FENCE_INITIATOR_MASK_CP0,
	},

	/* TCON/INTC */
	{
		.memory_region = {
			.start = (const void*) 0xB0005000,
			.length = 0x2000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_ALL,
		.write_access_bits = FENCE_INITIATOR_MASK_ALL,
	},

	/* UART/SPIS_0/SPIS_1/GSRAM_REG */
	{
		.memory_region = {
			.start = (const void*) 0xB0009000,
			.length = 0x4000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1,
	},

	/* PCIe_PHY */
	{
		.memory_region = {
			.start = (const void*) 0xB0100000,
			.length = 0x61000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0,
	},

	/* PCIe_EP */
	{
		.memory_region = {
			.start = (const void*) 0xB0180000,
			.length = 0x31000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0,
	},

	/* PCIe_TDISP/DOE/IDE */
	{
		.memory_region = {
			.start = (const void*) 0xB01C0000,
			.length = 0x24000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0,
	},

	{
		.memory_region = {
			.start = (const void*) 0xB0000000,
			.length = 0x00600000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP,
	},
};

/**
 * Memory fencing entries for CP 0/1 block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_cp_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0x60000000,
			.length = 0x00800000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP |
			FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP |
			FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1,
	},
};

/**
 * Memory fencing entries for NQM block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_nqm_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0xA1100000,
			.length = 0x103000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_CP0,
		.write_access_bits = FENCE_INITIATOR_MASK_CP0,
	},

	{
		.memory_region = {
			.start = (const void*) 0xA1280000,
			.length = 0x80000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1 |
			FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER |
			FENCE_INITIATOR_MASK_QMGR_MSIX,
		.write_access_bits = FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1 |
			FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER |
			FENCE_INITIATOR_MASK_QMGR_MSIX,
	},

	{
		.memory_region = {
			.start = (const void*) 0xA1300000,
			.length = 0x200000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_PCIE,
		.write_access_bits = FENCE_INITIATOR_MASK_PCIE,
	},

	{
		.memory_region = {
			.start = (const void*) 0xA1800000,
			.length = 0x19000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_CP0,
		.write_access_bits = FENCE_INITIATOR_MASK_CP0,
	},

	{
		.memory_region = {
			.start = (const void*) 0xA1900000,
			.length = 0x180000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_PCIE,
		.write_access_bits = FENCE_INITIATOR_MASK_PCIE,
	},

	{
		.memory_region = {
			.start = (const void*) 0xA1E00000,
			.length = 0x1240000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_CP0,
		.write_access_bits = FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_CP0,
	},

	{
		.memory_region = {
			.start = (const void*) 0xA3E00000,
			.length = 0x8000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1 |
			FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER |
			FENCE_INITIATOR_MASK_QMGR_MSIX,
		.write_access_bits = FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1 |
			FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER |
			FENCE_INITIATOR_MASK_QMGR_MSIX,
	},
};

/**
 * Memory fencing entries for BCP block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_bcp_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0xA0C01000,
			.length = 0X4000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1 |
			FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER |
			FENCE_INITIATOR_MASK_QMGR_MSIX | FENCE_INITIATOR_MASK_BCP_CDMA,
		.write_access_bits = FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1 |
			FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER |
			FENCE_INITIATOR_MASK_QMGR_MSIX | FENCE_INITIATOR_MASK_BCP_CDMA,
	},
	{
		.memory_region = {
			.start = (const void*) 0xA0C00000,
			.length = 0x00400000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_QMGR_FP |
			FENCE_INITIATOR_MASK_BCP_CDMA |	FENCE_INITIATOR_MASK_GDMA |
			FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_QMGR_FP |
			FENCE_INITIATOR_MASK_BCP_CDMA |	FENCE_INITIATOR_MASK_GDMA |
			FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER,
	},
#ifdef MANTICORE_ENABLE_CP_CDMA_ACCESS
	{
		.memory_region = {
			.start = (const void*) 0xA0C00000,
			.length = 0X1000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1 |
			FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER |
			FENCE_INITIATOR_MASK_QMGR_MSIX | FENCE_INITIATOR_MASK_BCP_CDMA,
		.write_access_bits = FENCE_INITIATOR_MASK_CP0 | FENCE_INITIATOR_MASK_CP1 |
			FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER |
			FENCE_INITIATOR_MASK_QMGR_MSIX | FENCE_INITIATOR_MASK_BCP_CDMA,
	},
#endif
};

/**
 * Memory fencing entries for GDMA block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_gdma_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0xA0000000,
			.length = 0x00100000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_ALL,
		.write_access_bits = FENCE_INITIATOR_MASK_ALL,
	},
};

/**
 * Memory fencing entries for GSRAM block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_gsram_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0x61000000,
			.length = 0x1000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_HSSHA,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP,
	},
	{
		.memory_region = {
			.start = (const void*) 0x61001000,
			.length = 0x2000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1,
	},
	{
		.memory_region = {
			.start = (const void*) 0x61003000,
			.length = 0x2000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP,
	},
	{
		.memory_region = {
			.start = (const void*) 0x61005000,
			.length = 0x1000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_BCP_CDMA,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_QMGR_FP | FENCE_INITIATOR_MASK_BCP_CDMA,
	},
	{
		.memory_region = {
			.start = (const void*) 0x61006000,
			.length = 0x3000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1,
	},
	{
		.memory_region = {
			.start = (const void*) 0x61009000,
			.length = 0x6000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER,
		.write_access_bits = FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_QMGR_CMD_FETCHER,
	},
	{
		.memory_region = {
			.start = (const void*) 0x6100F000,
			.length = 0xA9000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_GDMA | FENCE_INITIATOR_MASK_AES |
			FENCE_INITIATOR_MASK_HSSHA |
			FENCE_INITIATOR_MASK_UPKA0 | FENCE_INITIATOR_MASK_UPKA1 | FENCE_INITIATOR_MASK_UPKA2 |
			FENCE_INITIATOR_MASK_UPKA3 |
			FENCE_INITIATOR_MASK_UPKA4 | FENCE_INITIATOR_MASK_UPKA5 | FENCE_INITIATOR_MASK_UPKA6 |
			FENCE_INITIATOR_MASK_UPKA7 | FENCE_INITIATOR_MASK_UPKA8 | FENCE_INITIATOR_MASK_UPKA9 |
			FENCE_INITIATOR_MASK_UPKA10 | FENCE_INITIATOR_MASK_UPKA11 |
			FENCE_INITIATOR_MASK_UPKA12 | FENCE_INITIATOR_MASK_UPKA13 |
			FENCE_INITIATOR_MASK_UPKA14 | FENCE_INITIATOR_MASK_UPKA15,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_GDMA | FENCE_INITIATOR_MASK_AES |
			FENCE_INITIATOR_MASK_HSSHA |
			FENCE_INITIATOR_MASK_UPKA0 | FENCE_INITIATOR_MASK_UPKA1 | FENCE_INITIATOR_MASK_UPKA2 |
			FENCE_INITIATOR_MASK_UPKA3 |
			FENCE_INITIATOR_MASK_UPKA4 | FENCE_INITIATOR_MASK_UPKA5 | FENCE_INITIATOR_MASK_UPKA6 |
			FENCE_INITIATOR_MASK_UPKA7 | FENCE_INITIATOR_MASK_UPKA8 | FENCE_INITIATOR_MASK_UPKA9 |
			FENCE_INITIATOR_MASK_UPKA10 | FENCE_INITIATOR_MASK_UPKA11 |
			FENCE_INITIATOR_MASK_UPKA12 | FENCE_INITIATOR_MASK_UPKA13 |
			FENCE_INITIATOR_MASK_UPKA14 | FENCE_INITIATOR_MASK_UPKA15,
	},
	{
		.memory_region = {
			.start = (const void*) 0x610B8000,
			.length = 0x148000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_AES | FENCE_INITIATOR_MASK_HSSHA |
			FENCE_INITIATOR_MASK_UPKA0 | FENCE_INITIATOR_MASK_UPKA1 | FENCE_INITIATOR_MASK_UPKA2 |
			FENCE_INITIATOR_MASK_UPKA3 |
			FENCE_INITIATOR_MASK_UPKA4 | FENCE_INITIATOR_MASK_UPKA5 | FENCE_INITIATOR_MASK_UPKA6 |
			FENCE_INITIATOR_MASK_UPKA7 | FENCE_INITIATOR_MASK_UPKA8 | FENCE_INITIATOR_MASK_UPKA9 |
			FENCE_INITIATOR_MASK_UPKA10 | FENCE_INITIATOR_MASK_UPKA11 |
			FENCE_INITIATOR_MASK_UPKA12 | FENCE_INITIATOR_MASK_UPKA13 |
			FENCE_INITIATOR_MASK_UPKA14 | FENCE_INITIATOR_MASK_UPKA15,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_AES | FENCE_INITIATOR_MASK_HSSHA |
			FENCE_INITIATOR_MASK_UPKA0 | FENCE_INITIATOR_MASK_UPKA1 | FENCE_INITIATOR_MASK_UPKA2 |
			FENCE_INITIATOR_MASK_UPKA3 |
			FENCE_INITIATOR_MASK_UPKA4 | FENCE_INITIATOR_MASK_UPKA5 | FENCE_INITIATOR_MASK_UPKA6 |
			FENCE_INITIATOR_MASK_UPKA7 | FENCE_INITIATOR_MASK_UPKA8 | FENCE_INITIATOR_MASK_UPKA9 |
			FENCE_INITIATOR_MASK_UPKA10 | FENCE_INITIATOR_MASK_UPKA11 |
			FENCE_INITIATOR_MASK_UPKA12 | FENCE_INITIATOR_MASK_UPKA13 |
			FENCE_INITIATOR_MASK_UPKA14 | FENCE_INITIATOR_MASK_UPKA15,
	},
	{
		.memory_region = {
			.start = (const void*) 0x61000000,
			.length = 0x00600000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP,
	},
};

/**
 * Memory fencing entries for PCIE block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_pcie_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0xA0100000,
			.length = 0x00100000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_PCIE,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_PCIE,
	},
};

/**
 * Memory fencing entries for UPKAB0 block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_upka0_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0xC0000000,
			.length = 0x00008000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_UPKA0 |
			FENCE_INITIATOR_MASK_UPKA1 | FENCE_INITIATOR_MASK_UPKA2 | FENCE_INITIATOR_MASK_UPKA3 |
			FENCE_INITIATOR_MASK_UPKA4 | FENCE_INITIATOR_MASK_UPKA5 | FENCE_INITIATOR_MASK_UPKA6 |
			FENCE_INITIATOR_MASK_UPKA7 | FENCE_INITIATOR_MASK_UPKA8 | FENCE_INITIATOR_MASK_UPKA9 |
			FENCE_INITIATOR_MASK_UPKA10 | FENCE_INITIATOR_MASK_UPKA11 |
			FENCE_INITIATOR_MASK_UPKA12 | FENCE_INITIATOR_MASK_UPKA13 |
			FENCE_INITIATOR_MASK_UPKA14 | FENCE_INITIATOR_MASK_UPKA15,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_UPKA0 |
			FENCE_INITIATOR_MASK_UPKA1 | FENCE_INITIATOR_MASK_UPKA2 | FENCE_INITIATOR_MASK_UPKA3 |
			FENCE_INITIATOR_MASK_UPKA4 | FENCE_INITIATOR_MASK_UPKA5 | FENCE_INITIATOR_MASK_UPKA6 |
			FENCE_INITIATOR_MASK_UPKA7 | FENCE_INITIATOR_MASK_UPKA8 | FENCE_INITIATOR_MASK_UPKA9 |
			FENCE_INITIATOR_MASK_UPKA10 | FENCE_INITIATOR_MASK_UPKA11 |
			FENCE_INITIATOR_MASK_UPKA12 | FENCE_INITIATOR_MASK_UPKA13 |
			FENCE_INITIATOR_MASK_UPKA14 | FENCE_INITIATOR_MASK_UPKA15,
	},
};

/**
 * Memory fencing entries for UPKAB1 block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_upka1_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0xC0008000,
			.length = 0x00008000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_UPKA0 |
			FENCE_INITIATOR_MASK_UPKA1 | FENCE_INITIATOR_MASK_UPKA2 | FENCE_INITIATOR_MASK_UPKA3 |
			FENCE_INITIATOR_MASK_UPKA4 | FENCE_INITIATOR_MASK_UPKA5 | FENCE_INITIATOR_MASK_UPKA6 |
			FENCE_INITIATOR_MASK_UPKA7 | FENCE_INITIATOR_MASK_UPKA8 | FENCE_INITIATOR_MASK_UPKA9 |
			FENCE_INITIATOR_MASK_UPKA10 | FENCE_INITIATOR_MASK_UPKA11 |
			FENCE_INITIATOR_MASK_UPKA12 | FENCE_INITIATOR_MASK_UPKA13 |
			FENCE_INITIATOR_MASK_UPKA14 | FENCE_INITIATOR_MASK_UPKA15,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_UPKA0 |
			FENCE_INITIATOR_MASK_UPKA1 | FENCE_INITIATOR_MASK_UPKA2 | FENCE_INITIATOR_MASK_UPKA3 |
			FENCE_INITIATOR_MASK_UPKA4 | FENCE_INITIATOR_MASK_UPKA5 | FENCE_INITIATOR_MASK_UPKA6 |
			FENCE_INITIATOR_MASK_UPKA7 | FENCE_INITIATOR_MASK_UPKA8 | FENCE_INITIATOR_MASK_UPKA9 |
			FENCE_INITIATOR_MASK_UPKA10 | FENCE_INITIATOR_MASK_UPKA11 |
			FENCE_INITIATOR_MASK_UPKA12 | FENCE_INITIATOR_MASK_UPKA13 |
			FENCE_INITIATOR_MASK_UPKA14 | FENCE_INITIATOR_MASK_UPKA15,
	},
};

/**
 * Memory fencing entries for HSSHA block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_hssha_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0xA0400000,
			.length = 0x00100000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_HSSHA,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_HSSHA,
	},
};

/**
 * Memory fencing entries for AES block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_aes_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0xA0200000,
			.length = 0x00100000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_AES,
		.write_access_bits = FENCE_INITIATOR_MASK_HSP | FENCE_INITIATOR_MASK_CP0 |
			FENCE_INITIATOR_MASK_CP1 | FENCE_INITIATOR_MASK_AES,
	},
};

/**
 * Memory fencing entries for RNG block.
 */
static const struct fence_policy_entry memory_protection_manticore_sprt_rng_entries[] = {
	{
		.memory_region = {
			.start = (const void*) 0xA0300000,
			.length = 0x00100000,
		},
		.read_access_bits = FENCE_INITIATOR_MASK_ALL,
		.write_access_bits = FENCE_INITIATOR_MASK_ALL,
	},
};


/**
 * Helper macro to define default memory fencing policy entry
 *
 * @param id Memory fencing block ID
 */
#define MEMORY_FENCING_ENTRY(id, name) \
{ \
		.fence_block_id = id, \
		.block_entries = memory_protection_manticore_sprt_##name##_entries, \
		.block_entries_count = ARRAY_SIZE (memory_protection_manticore_sprt_##name##_entries), \
}

/**
 * Default memory fencing policy for SPRT memory fencing
 */
static const struct fence_policy_block manticore_sprt_default_fencing_policy[] = {
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_APB, apb),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_DUAL_CP, cp),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_NQM, nqm),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_BCP, bcp),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_GDMA, gdma),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_GSRAM, gsram),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_PCIE, pcie),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_UPKAB0, upka0),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_UPKAB1, upka1),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_HSSHA, hssha),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_AES, aes),
	MEMORY_FENCING_ENTRY (FENCE_BLOCK_RNG, rng),
};


int memory_protection_manticore_sprt_configure_soc_fences (
	const struct memory_protection *mem_protect)
{
	const struct memory_protection_manticore_sprt *manticore =
		(const struct memory_protection_manticore_sprt*) mem_protect;

	if (manticore == NULL) {
		return MEMORY_PROTECTION_INVALID_ARGUMENT;
	}

	int status = 0;
#ifdef MANTICORE_MEMORY_FENCING_SPRT_CONFIG_AEBS
	uint32_t i;
	uint32_t fence_aebs[] = {
		MANTICORE_AEB_DISABLE_FENCE_RNG,
		MANTICORE_AEB_DISABLE_FENCE_APB_BRIDGE,
		MANTICORE_AEB_DISABLE_FENCE_DUAL_CP,
		MANTICORE_AEB_DISABLE_FENCE_NQM,
		MANTICORE_AEB_DISABLE_FENCE_BCP,
		MANTICORE_AEB_DISABLE_FENCE_GDMA,
		MANTICORE_AEB_DISABLE_FENCE_GSRAM,
		MANTICORE_AEB_DISABLE_FENCE_UPKA,
		MANTICORE_AEB_DISABLE_FENCE_AES,
		MANTICORE_AEB_DISABLE_FENCE_HSSHA,
		MANTICORE_AEB_DISABLE_FENCE_PCIE_CTRL,
	};

	status = hsp_aeb_init_state (&aeb);
	if (status != 0) {
		return status;
	}
#endif

	status = manticore->fence->apply (manticore->fence, manticore_sprt_default_fencing_policy,
		ARRAY_SIZE (manticore_sprt_default_fencing_policy));
	if (status != 0) {
		return status;
	}

#ifdef MANTICORE_MEMORY_FENCING_SPRT_CONFIG_AEBS
	platform_printf ("Disabling the AEBs" NEWLINE);
	for (i = 0; i < ARRAY_SIZE (fence_aebs); i++) {
		status = aeb.disable_aeb (&aeb, fence_aebs[i]);
		if (status != 0) {
			return status;
		}
	}
#endif

	return status;
}

/**
 * Initialize a handler for configuring SPRT memory protections.
 *
 * @param mem_protect The configuration handler to initialize.
 * @param fence The driver for SoC memory fencing.
 * @param mpu The MPU driver for HSP.
 * @param regions A list of memory regions that should be configured in the MPU.
 * @param count The number of memory regions in the list.
 *
 * @return 0 if the handler was initialized successfully or an error code.
 */
int memory_protection_manticore_sprt_init (struct memory_protection_manticore_sprt *mem_protect,
	const struct fence_interface *fence, const struct mpu_interface *mpu,
	const struct memory_protection_mpu_only_region *regions, size_t count)
{
	int status;

	if ((mem_protect == NULL) || (fence == NULL)) {
		return MEMORY_PROTECTION_INVALID_ARGUMENT;
	}

	memset (mem_protect, 0, sizeof (struct memory_protection_manticore_sprt));

	status = memory_protection_mpu_only_init (&mem_protect->base, mpu, regions, count);
	if (status != 0) {
		return status;
	}

	mem_protect->base.base.configure_soc_fences =
		memory_protection_manticore_sprt_configure_soc_fences;

	mem_protect->fence = fence;

	return 0;
}

/**
 * Release the resources used by SPRT memory protections.
 *
 * @param mem_protect The configuration handler to release.
 */
void memory_protection_manticore_sprt_release (
	const struct memory_protection_manticore_sprt *mem_protect)
{
	UNUSED (mem_protect);
}
