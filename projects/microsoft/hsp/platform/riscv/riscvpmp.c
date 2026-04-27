/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    riscvpmp.c

Abstract:

    This file contains RISC-V PMP helper functions

Author:

    Navin Pai (navinp)

--*/

#include <stdbool.h>
#include <stdint.h>
#include "splibs/hsprt/riscvcpu.h"


/*++

Description:

    This function sets the PMP region for RISC-V processor.

Arguments:

    PmpRegion - The PMP region number; 0 < PmpRegion < 16

    RiscvPmpConfig - The configuration information to set

    Address - The address of the region

--*/
void RiscvPmpSetRegion (uint32_t PmpRegion, RiscvPmpConfig PmpConfig, uint32_t Address)
{
	RiscvPmpConfig oldConfig;
	uint32_t oldAddress = 0;

	RiscvPmpGetRegion (PmpRegion, &oldConfig, &oldAddress);

	if (oldConfig.L == RiscvPmpLocked) {
		// cannot modify locked region
		return;
	}

	//
	// Update the address first because if the region is getting locked we
	// cannot change it later
	//

	if (oldAddress != Address) {
		switch (PmpRegion) {
			case 0:
				__asm__ ("csrw pmpaddr0, %[addr]" ::[addr] "r" (Address));
				break;

			case 1:
				__asm__ ("csrw pmpaddr1, %[addr]" ::[addr] "r" (Address));
				break;

			case 2:
				__asm__ ("csrw pmpaddr2, %[addr]" ::[addr] "r" (Address));
				break;

			case 3:
				__asm__ ("csrw pmpaddr3, %[addr]" ::[addr] "r" (Address));
				break;

			case 4:
				__asm__ ("csrw pmpaddr4, %[addr]" ::[addr] "r" (Address));
				break;

			case 5:
				__asm__ ("csrw pmpaddr5, %[addr]" ::[addr] "r" (Address));
				break;

			case 6:
				__asm__ ("csrw pmpaddr6, %[addr]" ::[addr] "r" (Address));
				break;

			case 7:
				__asm__ ("csrw pmpaddr7, %[addr]" ::[addr] "r" (Address));
				break;

			case 8:
				__asm__ ("csrw pmpaddr8, %[addr]" ::[addr] "r" (Address));
				break;

			case 9:
				__asm__ ("csrw pmpaddr9, %[addr]" ::[addr] "r" (Address));
				break;

			case 10:
				__asm__ ("csrw pmpaddr10, %[addr]" ::[addr] "r" (Address));
				break;

			case 11:
				__asm__ ("csrw pmpaddr11, %[addr]" ::[addr] "r" (Address));
				break;

			case 12:
				__asm__ ("csrw pmpaddr12, %[addr]" ::[addr] "r" (Address));
				break;

			case 13:
				__asm__ ("csrw pmpaddr13, %[addr]" ::[addr] "r" (Address));
				break;

			case 14:
				__asm__ ("csrw pmpaddr14, %[addr]" ::[addr] "r" (Address));
				break;

			case 15:
				__asm__ ("csrw pmpaddr15, %[addr]" ::[addr] "r" (Address));
				break;
		}
	}

	PmpConfig.val &= 0xff;
	if (oldConfig.val != PmpConfig.val) {
		//
		// write the config register
		//
		uint32_t cfgmask = 0xff << (8 * (PmpRegion % 4));
		uint32_t cfgval = PmpConfig.val << (8 * (PmpRegion % 4));

		switch (PmpRegion / 4) {
			case 0:
				__asm__ ("csrc pmpcfg0, %[mask]" ::[mask] "r" (cfgmask));
				__asm__ ("csrs pmpcfg0, %[val]" ::[val] "r" (cfgval));
				break;

			case 2:
				__asm__ ("csrc pmpcfg2, %[mask]" ::[mask] "r" (cfgmask));
				__asm__ ("csrs pmpcfg2, %[val]" ::[val] "r" (cfgval));
				break;

			case 1:
				__asm__ ("csrc pmpcfg1, %[mask]" ::[mask] "r" (cfgmask));
				__asm__ ("csrs pmpcfg1, %[val]" ::[val] "r" (cfgval));
				break;

			case 3:
				__asm__ ("csrc pmpcfg3, %[mask]" ::[mask] "r" (cfgmask));
				__asm__ ("csrs pmpcfg3, %[val]" ::[val] "r" (cfgval));
				break;
		}
	}
}


/*++

Description:

    Returns the PMP region information about RISC-V processor

Arguments:

    PmpRegion - The PMP region number; 0 < PmpRegion < 16

    RiscvPmpConfig - Pointer to configuration information structure

    Address - Pointer to address. This can be null if no address is desired

--*/
NOINLINE
void RiscvPmpGetRegion (uint32_t PmpRegion, RiscvPmpConfig *PmpConfig, uint32_t *Address)
{
	uint32_t configval = 0;

	//
	// Read the config register
	//

	switch (PmpRegion / 4) {
		case 0:
			__asm__ ("csrr %[cfg], pmpcfg0" :[cfg] "=r" (configval));
			break;

		case 1:
			__asm__ ("csrr %[cfg], pmpcfg1" :[cfg] "=r" (configval));
			break;

		case 2:
			__asm__ ("csrr %[cfg], pmpcfg2" :[cfg] "=r" (configval));
			break;

		case 3:
			__asm__ ("csrr %[cfg], pmpcfg3" :[cfg] "=r" (configval));
			break;
	}

	PmpConfig->val = (0xff & (configval >> (8 * (PmpRegion % 4))));

	//
	// Read the address register if requested
	//

	if (!Address) {
		return;
	}

	switch (PmpRegion) {
		case 0:
			__asm__ ("csrr %[addr], pmpaddr0" :[addr] "=r" (*Address));
			break;

		case 1:
			__asm__ ("csrr %[addr], pmpaddr1" :[addr] "=r" (*Address));
			break;

		case 2:
			__asm__ ("csrr %[addr], pmpaddr2" :[addr] "=r" (*Address));
			break;

		case 3:
			__asm__ ("csrr %[addr], pmpaddr3" :[addr] "=r" (*Address));
			break;

		case 4:
			__asm__ ("csrr %[addr], pmpaddr4" :[addr] "=r" (*Address));
			break;

		case 5:
			__asm__ ("csrr %[addr], pmpaddr5" :[addr] "=r" (*Address));
			break;

		case 6:
			__asm__ ("csrr %[addr], pmpaddr6" :[addr] "=r" (*Address));
			break;

		case 7:
			__asm__ ("csrr %[addr], pmpaddr7" :[addr] "=r" (*Address));
			break;

		case 8:
			__asm__ ("csrr %[addr], pmpaddr8" :[addr] "=r" (*Address));
			break;

		case 9:
			__asm__ ("csrr %[addr], pmpaddr9" :[addr] "=r" (*Address));
			break;

		case 10:
			__asm__ ("csrr %[addr], pmpaddr10" :[addr] "=r" (*Address));
			break;

		case 11:
			__asm__ ("csrr %[addr], pmpaddr11" :[addr] "=r" (*Address));
			break;

		case 12:
			__asm__ ("csrr %[addr], pmpaddr12" :[addr] "=r" (*Address));
			break;

		case 13:
			__asm__ ("csrr %[addr], pmpaddr13" :[addr] "=r" (*Address));
			break;

		case 14:
			__asm__ ("csrr %[addr], pmpaddr14" :[addr] "=r" (*Address));
			break;

		case 15:
			__asm__ ("csrr %[addr], pmpaddr15" :[addr] "=r" (*Address));
			break;
	}
}
