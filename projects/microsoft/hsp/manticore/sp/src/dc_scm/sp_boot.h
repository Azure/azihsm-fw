// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SP_BOOT_H_
#define SP_BOOT_H_

#include <stdbool.h>
#include "hsp_top.h"
#include "manticore_rom.h"
#include "drivers/hsp_aeb.h"
#include "drivers/hsp_gpio.h"
#include "fips/cmvp_test_case.h"
#include "splibs/inc/spchkptdefs.h"


extern struct manticore_rom_shared_sram *const rom_shared;
extern struct Creg_regs_aeb_regs *const aeb_regs;
extern struct Creg_regs_misc_creg_sw_regs *const sw_regs;

/**
 * Determine the offset within the CREG address space for a particular register address.
 *
 * @param reg_addr Absolute address of the desired register.
 */
#define	CREG_OFFSET(reg_addr)		((reg_addr) - HSP_ADDR_MAP_CREG_ADDRESS)

/**
 * The timeout to use for the hardware watchdog during SPRT execution.  This value is represented in
 * microseconds.
 */
#define	MANTICORE_WATCHDOG_TIMEOUT_US	(2 * 1000000UL)


void determine_hsp_clock_frequency ();
void handle_silicon_errata (const struct hsp_aeb *aeb, uint32_t socid,
	const HSP_CHKPT_CONFIG *chkpt_start, const HSP_CHKPT_CONFIG *chkpt_end);

bool is_a0_bypass ();
bool is_por ();
bool is_sw_reset ();
bool is_graceful_reset ();
bool has_owner_key ();
bool has_secondary_signing_key ();
bool is_dice_valid ();

int configure_gpios (const struct hsp_gpio *gpio);

void configure_error_boot_order ();
void configure_normal_boot_order ();

void boot_error_reset ();
void boot_error_clear_counter ();

void boot_unlocked_device ();
bool has_booted_unlocked ();

void pcr0_extended_with_sprt ();
bool is_pcr0_extended_with_sprt ();

void pcr0_extended_with_aeb_state ();
bool is_pcr0_extended_with_aeb_state ();

void pcr2_extended ();
bool is_pcr2_extended ();

void system_init_done ();
bool is_sys_init_done ();

void impactless_update_applied ();
void impactful_update_applied ();
void impactless_update_done ();
bool has_updated_impactless_firmware ();
bool has_updated_impactful_firmware ();
bool is_impactless_update ();
void clear_graceful_reset ();


/* Support for handling CMVP test scenarios. */
extern uint32_t cmvp_test;
extern bool cmvp_mimic_por;


int get_cmvp_test_case (enum cmvp_test_case_boot_stage boot_stage);
void clear_cmvp_test_case ();
void trigger_cmvp_pct_failure ();


#endif	/* SP_BOOT_H_ */
