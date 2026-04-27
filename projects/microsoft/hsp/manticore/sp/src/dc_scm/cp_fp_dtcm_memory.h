// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CP_FP_DTCM_MEMORY_H_
#define CP_FP_DTCM_MEMORY_H_

#include <stdint.h>


/**
 * Macros (formulas) to convert a local dTCM address to a global address.
 */
#define DTCM_OFFSET_MASK							0xFFFFF
#define CP_GLOBAL_ADDRESS(core_num,\
		local_address)	(0x60200000 + (0x400000 * core_num) + (local_address & DTCM_OFFSET_MASK))
#define FP_GLOBAL_ADDRESS(core_num,\
		local_address)	(0xA3000000 + (0x200000 * core_num) + (local_address & DTCM_OFFSET_MASK))

/**
 * CP0 dTCM status and crashdump local addresses and their globla addresses
 */
#define CPU_CP0_DTCM_STATUS_ADDR			0x2003FB00
#define CPU_CP0_DTCM_CRASHDUMP_START		0x2003FB04
#define CPU_CP0_DTCM_STATUS_GLOBAL_ADDR		CP_GLOBAL_ADDRESS(0, CPU_CP0_DTCM_STATUS_ADDR)
#define CPU_CP0_DTCM_CRASHDUMP_GLOBAL_START	CP_GLOBAL_ADDRESS(0, CPU_CP0_DTCM_CRASHDUMP_START)

/**
 * CP1 dTCM status and crashdump local addresses and their globla addresses
 */
#define CPU_CP1_DTCM_STATUS_ADDR			0x2003F800
#define CPU_CP1_DTCM_CRASHDUMP_START		0x2003F400
#define CPU_CP1_DTCM_STATUS_GLOBAL_ADDR		CP_GLOBAL_ADDRESS(1, CPU_CP1_DTCM_STATUS_ADDR)
#define CPU_CP1_DTCM_CRASHDUMP_GLOBAL_START	CP_GLOBAL_ADDRESS(1, CPU_CP1_DTCM_CRASHDUMP_START)

/**
 * FP0 dTCM status and crashdump local addresses and their globla addresses
 */
#define CPU_FP0_DTCM_STATUS_ADDR			0x20023DDC
#define CPU_FP0_DTCM_CRASHDUMP_START		0x20021750
#define CPU_FP0_DTCM_STATUS_GLOBAL_ADDR		FP_GLOBAL_ADDRESS(0, CPU_FP0_DTCM_STATUS_ADDR)
#define CPU_FP0_DTCM_CRASHDUMP_GLOBAL_START	FP_GLOBAL_ADDRESS(0, CPU_FP0_DTCM_CRASHDUMP_START)

/**
 * FP1 dTCM status and crashdump local addresses and their globla addresses
 */
#define CPU_FP1_DTCM_STATUS_ADDR			0x20023DDC
#define CPU_FP1_DTCM_CRASHDUMP_START		0x20021530
#define CPU_FP1_DTCM_STATUS_GLOBAL_ADDR		FP_GLOBAL_ADDRESS(1, CPU_FP1_DTCM_STATUS_ADDR)
#define CPU_FP1_DTCM_CRASHDUMP_GLOBAL_START	FP_GLOBAL_ADDRESS(1, CPU_FP1_DTCM_CRASHDUMP_START)

/**
 * FP2 dTCM status and crashdump local addresses and their globla addresses
 */
#define CPU_FP2_DTCM_STATUS_ADDR			0x20023DDC
#define CPU_FP2_DTCM_CRASHDUMP_START		0x20020a60
#define CPU_FP2_DTCM_STATUS_GLOBAL_ADDR		FP_GLOBAL_ADDRESS(2, CPU_FP2_DTCM_STATUS_ADDR)
#define CPU_FP2_DTCM_CRASHDUMP_GLOBAL_START	FP_GLOBAL_ADDRESS(2, CPU_FP2_DTCM_CRASHDUMP_START)


#endif	/* CP_FP_DTCM_MEMORY_H_ */
