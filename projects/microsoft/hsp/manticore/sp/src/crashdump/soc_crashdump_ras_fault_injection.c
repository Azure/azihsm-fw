// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "soc_crashdump_ras_fault_injection.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "dc_scm/mem_map/gsram_mem_map.h"
#include "dc_scm/sp_boot.h"
#include "drivers/hsp_dmb.h"
#include "fips/cmvp_test_case.h"
#include "fips/self_test_manticore.h"
#include "marvell/RegGsram.h"

/**
 * The GSRAM register block address.
 */
#define MANTICORE_SOC_GSRAM_REG_BLOCK_ADDRESS		0xb000c000
#define MANTICORE_SOC_GSRAM_MEM_ADDRESS				0x61005400


/**
 * Inject GSRAM ECC double bit error.
 *
 * @param fault_inject The fault injection to initialize.
 *
 * @return 0 if the fault injection was successfully.
 */
static int cmvp_test_ras_hsp_fault_gsram_ecc_err (
	const struct soc_crashdump_ras_fault_injection *fault_inject)
{
	int status;
	Gsram_t *gsram_regs = NULL;
	uint32_t *ptr = NULL;
	uint32_t read_data;

	// Enable ECC for GSRAM.
	status = fault_inject->dmb->map_soc_address (fault_inject->dmb,
		MANTICORE_SOC_GSRAM_REG_BLOCK_ADDRESS, sizeof (*gsram_regs),
		HSP_DMB_ACCESS_WRITE | HSP_DMB_ACCESS_READ, (void**) &gsram_regs);
	if (status != 0) {
		return status;
	}

	// Set error bits in errinjblo
	gsram_regs->errinjbloErrBitLow = 0x3;

	// Make the address 8-byte aligned to inject error
	gsram_regs->errinjadr.b.ERR_ADDR = (MANTICORE_SOC_GSRAM_MEM_ADDRESS & 0x1FFFFF) >> 3;

	// Write to errinjcts to inject error
	gsram_regs->errinjcts.b.INJ_TRIG = 1;
	while (!gsram_regs->errinjcts.b.INJ_DONE) {
	}

	fault_inject->dmb->unmap_soc_address (fault_inject->dmb, gsram_regs);

	// Map GSRAM address to read-write access
	status = fault_inject->dmb->map_soc_address (fault_inject->dmb, MANTICORE_SOC_GSRAM_MEM_ADDRESS,
		sizeof (*ptr), HSP_DMB_ACCESS_WRITE | HSP_DMB_ACCESS_READ, (void**) &ptr);
	if (status != 0) {
		return status;
	}

	// Write to the address
	*ptr = (0xFFFFFFFF);
	// Read the address
	read_data = *ptr;

	UNUSED (read_data);

	fault_inject->dmb->unmap_soc_address (fault_inject->dmb, ptr);

	return 0;
}

/**
 * Inject a WDT fault error.
 *
 * @param fault_inject The fault injection to initialize.
 *
 * @return 0 if the fault injection was successfully.
 */
static int cmvp_test_case_ras_hsp_fault_inj_wdt_err (
	const struct soc_crashdump_ras_fault_injection *fault_inject)
{
	UNUSED (fault_inject);

	// Watch-dog starve for refresh
	while (1) {
	}

	return 0;
}

/**
 * Inject a MPU fault error.
 *
 * @param fault_inject The fault injection to initialize.
 *
 * @return 0 if the fault injection was successfully.
 */
static int cmvp_test_case_ras_hsp_fault_inj_mpu_err (
	const struct soc_crashdump_ras_fault_injection *fault_inject)
{
	uint8_t *dram_addr = (uint8_t*) HSP_ADDR_MAP_SP_DRAM_ADDRESS;

	UNUSED (fault_inject);

	// Access to the last 12K of DRAM memory to inject a MPU error.
	*(dram_addr + 0x2D000) = 0xFF;

	return 0;
}

/**
 * Inject a BUS fault error.
 *
 * @param fault_inject The fault injection to initialize.
 *
 * @return 0 if the fault injection was successfully or status on error.
 */
static int cmvp_test_case_ras_hsp_fault_inj_bus_err (
	const struct soc_crashdump_ras_fault_injection *fault_inject)
{
	int status;
	uint32_t *ptr;

	// Map SoC memory with READ access.
	status = fault_inject->dmb->map_soc_address (fault_inject->dmb,
		(uint64_t) GSRAM_MEM_MAP_HSP_TO_ADMIN_IPC_TX_QUEUE_CI,
		GSRAM_MEM_MAP_HSP_TO_ADMIN_IPC_TX_QUEUE_CI_SIZE, HSP_DMB_ACCESS_READ, (void**) &ptr);
	if (status != 0) {
		return status;
	}

	// Perform WRITE operation on SoC memory which is mapped with READ access to inject a BUS error.
	*ptr = 0xAA55AA55;

	return 0;
}

int soc_crashdump_ras_fault_injection_run_test (const struct self_test_interface *self_test,
	struct debug_log_entry_info *error_info)
{
	int status = 0;
	const struct soc_crashdump_ras_fault_injection *reg_fault_inj = TO_DERIVED_TYPE (self_test,
		const struct soc_crashdump_ras_fault_injection, base);

	enum cmvp_test_case_get_ras_fault_inj_test fault = CMVP_TEST_CASE_RAS_HSP_FAULT_NONE;

	UNUSED (error_info);

	if ((cmvp_test != 0) &&
		(cmvp_test_case_get_test_type (cmvp_test) == CMVP_TEST_CASE_RAS_ERROR_INJ_TEST)) {
		fault = cmvp_test_case_get_ras_fault_injection_test (cmvp_test);
	}

	switch (fault) {
		case CMVP_TEST_CASE_RAS_HSP_FAULT_GSRAM_ECC_ERR:
			/* This test is implemented to inject GSRAM Double bit ECC Error as faced
			* issue of Hard Fault in CP while testing on A0 Chip.
			*/
			status = cmvp_test_ras_hsp_fault_gsram_ecc_err (reg_fault_inj);
			break;

		case CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_BUS_ERR:
			status = cmvp_test_case_ras_hsp_fault_inj_bus_err (reg_fault_inj);
			break;

		case CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_CHK_ERR:
			// TODO: Write check point error injection register
			break;

		case CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_DMB_ERR:
			// TODO: Write DMB error injection register
			break;

		case CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_MEM_ERR:
			// TODO: Write MEM error injection register
			break;

		case CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_MPU_ERR:
			status = cmvp_test_case_ras_hsp_fault_inj_mpu_err (reg_fault_inj);
			break;

		case CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_WDT_ERR:
			status = cmvp_test_case_ras_hsp_fault_inj_wdt_err (reg_fault_inj);
			break;

		case CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_AXI_WDT_ERR:
			// TODO: Write AXI WDT error injection register
			break;

		default:
			break;
	}

	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Initialize an instance for on-demand RAS HSP faults injection test support using CMVP test.
 *
 * @param fault_inj The fault injection to initialize.
 * @param mmio_creg The CREG register interface (struct Creg_regs).
 * @param dmb A pointer to HSP DMB object used to map and unmap memory regions.
 *
 * @return 0 if the fault injection was initialized successfully or an error code.
 */
int soc_crashdump_ras_fault_injection_init (struct soc_crashdump_ras_fault_injection *fault_inj,
	const struct mmio_register_block *mmio_creg, const struct hsp_dmb *dmb)
{
	if ((fault_inj == NULL) || (mmio_creg == NULL)) {
		return SELF_TEST_MANTICORE_INVALID_ARGUMENT;
	}

	memset (fault_inj, 0, sizeof (*fault_inj));

	fault_inj->base.run_self_test = soc_crashdump_ras_fault_injection_run_test;
	fault_inj->mmio_creg = mmio_creg;
	fault_inj->dmb = dmb;

	return 0;
}

void soc_crashdump_ras_fault_injection_release (
	const struct soc_crashdump_ras_fault_injection *fault_inj)
{
	UNUSED (fault_inj);
}
