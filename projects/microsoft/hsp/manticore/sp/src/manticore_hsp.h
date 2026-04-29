// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_HSP_H_
#define MANTICORE_HSP_H_

#include <stdint.h>


/* List of HSP configuration parameters that can be specified for a given platform.  Undefined
 * values will use the default value. */


/*********
 * HSP
 *********/

/**
 * The total length of the tenancy counter.  By default, it will be set to match the size of the
 * RSVD0 fuse slot.
 */
// #define	HW_ROT_TENANCY_COUNTER_LENGTH		HSP_FUSES_LENGTH (RSVD0)

/**
 * The clock frequency of the main HSP clock, in Hz.  There is no default for this value.
 */
#ifdef BUILD_FOR_FPGA
/* This is the clock rate used by the MPS3 FPGA. */
#define	HSP_CLOCK_FREQUENCY_HZ				16666667
#define	HSP_CLOCK_PLL_OUT_FREQUENCY_HZ		HSP_CLOCK_FREQUENCY_HZ
#elif defined BUILD_FOR_HAPS
/* This is the clock rate used by the HAPS FPGA. */
#define	HSP_CLOCK_FREQUENCY_HZ				2500000
#define	HSP_CLOCK_PLL_OUT_FREQUENCY_HZ		HSP_CLOCK_FREQUENCY_HZ
#else
/* Manticore will normally run at 375MHz, but will run directly on the reference clock when A0
 * bypass is asserted.  Need to allow for this clock frequency to be determined at run-time. */
extern uint32_t hsp_clock_freq;

#define	HSP_CLOCK_FREQUENCY_HZ				hsp_clock_freq

#define	HSP_CLOCK_PLL_OUT_FREQUENCY_HZ		375000000
#define	HSP_CLOCK_REF_CLK_FREQUENCY_HZ		25000000
#endif

/**
 * The clock frequency used as the RTC clock for the RISC-V core.
 */
// #define	RISCV_RTC_FREQUENCY_HZ				(HSP_CLOCK_FREQUENCY_HZ / 32)

/**
 * The clock frequency used as the RTC clock for the HSP.
 */
// #define	HSP_RTC_FREQUENCY_HZ				(1U << CREG_REGS_CREG_RTC_GROUP_COUNT_TO_SECOND_COUNT_TO_SECOND_WIDTH)

/**
 * The number of bits that should be used for generating a random delay after printing a postcode,
 * or whenever else a short, random delay is needed.
 */
// #define	HSP_CPI_RANDOM_DELAY_BITS			8

/**
 * The total number of DMB segments available in the hardware.
 */
// #define	HSP_DMB_SEGMENTS					14

/**
 * The base address for HSP addresses that are mapped by the DMB.
 */
// #define	HSP_DMB_BASE_MAPPING_ADDRESS		0x90000000

/**
 * The max crypto command execution wait time (ms).
 */
//#define	HSP_CRYPTO_DRIVER_CMD_EXE_WAIT_TIME		2000


#endif	/* MANTICORE_HSP_H_ */
