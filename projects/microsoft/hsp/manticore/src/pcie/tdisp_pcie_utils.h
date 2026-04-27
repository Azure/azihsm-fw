#ifndef _PCIE_UTILS_H_
#define _PCIE_UTILS_H_

#include <stdint.h>
#include "mmio/mmio_register_block.h"

/**
 * PCIe BAR register structure.
 * This structure represents a PCIe Base Address Register (BAR) and contains fields
 * for the IO type, BAR type, prefetchable flag, and address.
 */
union pcie_bar_register {
	uint32_t register_value;
	struct {
		uint32_t io : 1;
		uint32_t type : 2;
		uint32_t prefetchable : 1;
		uint32_t address : 28;
	} bar;
};

/**
 * MSIX control register structure.
 * This structure represents the MSIX control register and contains fields
 * for the MSIX capability ID, next capability offset, table size, function mask,
 * and enable flags.
 */
union msix_control_register {
	uint32_t value;
	struct {
		uint32_t msix_cap_id : 8;
		uint32_t next_cap_offset : 8;
		uint32_t msix_table_size : 11;
		uint32_t reserved : 3;
		uint32_t msix_function_mask : 1;
		uint32_t msix_enable : 1;
	};
};

/**
 * MSIX BAR offset register structure.
 * This structure represents the MSIX BAR offset register and contains fields
 * for the MSIX BAR offset and MSIX offset.
 * The MSIX BAR offset indicates the BAR number, and the MSIX offset indicates
 * the offset within that BAR where the MSIX table or PBA is located.
 */
union msix_bar_offset_register {
	uint32_t value;
	struct {
		uint32_t msix_bar_offset : 3;
		uint32_t msix_offset : 29;
	};
};

/**
 * PCIe interface registers structure.
 * This structure contains the PCIe BAR registers, MSIX control register,
 * MSIX table offset register, and MSIX PBA offset register.
 * It is used to manage the PCIe interface and its associated registers.
 */
struct interface_registers {
	union pcie_bar_register bars[6];
	union msix_control_register msix_control;
	union msix_bar_offset_register msix_table;
	union msix_bar_offset_register msix_pba;
};


int pcie_utils_read_pf0_registers (const struct mmio_register_block *pcie_registers,
	struct interface_registers *regs);
int pcie_utils_read_vf_registers (const struct mmio_register_block *pcie_registers,
	uint32_t function_index, struct interface_registers *regs);


#endif
