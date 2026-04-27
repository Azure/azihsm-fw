// Copyright (c) Microsoft Corporation. All rights reserved.

#include "tdisp_pcie_utils.h"
#include "common/array_size.h"

/**
 * PCIe BAR0 base address.
 * This is the base address for the PCIe BAR0 register block.
 */
#define TDISP_PCIE_UTILS_BAR0_REGISTER						0xB0180010

/**
 * PCIe MSIX control register base address.
 * This is the base address for the PCIe MSIX control register block.
 */
#define TDISP_PCIE_UTILS_MSIX_CONTROL_REGISTER				0xB01800B0

/**
 * VF function selection register address
 */
#define TDISP_PCIE_UTILS_VF_FUNCTION_SELECTION_REGISTER		0xB01C016C

/**
 * PCIe BAR0 address for VF.
 * This is the base address for the PCIe BAR0 register block for VF functions.
 */
#define TDISP_PCIE_UTILS_VF_BAR0_REGISTER					0xB018021C

/**
 * PCIe MSIX control register address for VF.
 * This is the base address for the PCIe MSIX control register block for VF functions.
 */
#define TDISP_PCIE_UTILS_VF_MSIX_CONTROL_REGISTER			0xB01820B0

/**
 * Helper function to read PF0 registers.
 * This function reads the PCIe BAR registers and MSIX control registers for PF0.
 * It maps the register block, reads the required registers, and then unmaps the block.
 *
 * @param pcie_registers Pointer to the PCIe register block.
 * @param regs Pointer to the interface registers structure where the read values will be stored.
 *
 * @return 0 on success, error code otherwise.
 */
int pcie_utils_read_pf0_registers (const struct mmio_register_block *pcie_registers,
	struct interface_registers *regs)
{
	int status;

	status = pcie_registers->map (pcie_registers);
	if (status != 0) {
		return status;
	}

	/* Read BARs */
	status = pcie_registers->block_read32_by_addr (pcie_registers, TDISP_PCIE_UTILS_BAR0_REGISTER,
		&regs->bars[0].register_value, ARRAY_SIZE (regs->bars));
	if (status != 0) {
		goto exit;
	}

	/* Read MSIX registers */
	status = pcie_registers->block_read32_by_addr (pcie_registers,
		TDISP_PCIE_UTILS_MSIX_CONTROL_REGISTER, &regs->msix_control.value, 3);

exit:
	pcie_registers->unmap (pcie_registers);

	return status;
}

/**
 * Helper function to read VF registers.
 * This function reads the PCIe BAR registers and MSIX control registers for a specific VF.
 * It maps the register block, writes the function index to the appropriate register,
 * reads the required registers, and then unmaps the block.
 *
 * @param pcie_registers Pointer to the PCIe register block.
 * @param function_index The index of the VF function to read (1-based, where
 *                       1 corresponds to VF0, 2 to VF1, ..., 64 to VF63).
 * @param regs Pointer to the interface registers structure where the read values will be stored.
 *
 * @return 0 on success, error code otherwise.
 */
int pcie_utils_read_vf_registers (const struct mmio_register_block *pcie_registers,
	uint32_t function_index, struct interface_registers *regs)
{
	int status;

	status = pcie_registers->map (pcie_registers);
	if (status != 0) {
		return status;
	}

	/* Function_Selection: 0xB01C016C */
	status = pcie_registers->write32_by_addr (pcie_registers,
		TDISP_PCIE_UTILS_VF_FUNCTION_SELECTION_REGISTER, function_index - 1);
	if (status != 0) {
		goto exit;
	}

	status = pcie_registers->block_read32_by_addr (pcie_registers,
		TDISP_PCIE_UTILS_VF_BAR0_REGISTER, &regs->bars[0].register_value, ARRAY_SIZE (regs->bars));
	if (status != 0) {
		goto exit;
	}

	status = pcie_registers->block_read32_by_addr (pcie_registers,
		TDISP_PCIE_UTILS_VF_MSIX_CONTROL_REGISTER, &regs->msix_control.value, 3);

exit:
	pcie_registers->unmap (pcie_registers);

	return status;
}
