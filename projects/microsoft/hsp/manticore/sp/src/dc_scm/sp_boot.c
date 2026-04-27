// Copyright (c) Microsoft Corporation. All rights reserved.

#include "manticore_hsp_gpio.h"
#include "manticore_sticky_regs.h"
#include "platform_api.h"
#include "platform_config.h"
#include "rot_memory_map.h"
#include "sp_boot.h"
#include "common/array_size.h"
#include "common/unused.h"
#include "drivers/hsp_dmb.h"
#include "firmware/graceful_shutdown.h"
#include "logging/code_path_integrity.h"
#include "splibs/hsprt/riscvcpu.h"

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
#include "crypto/ecdh.h"
#include "crypto/ecdsa.h"
#include "drivers/ccs_ksu_interface.h"
#endif


#if !(defined BUILD_FOR_FPGA || defined BUILD_FOR_HAPS)
/**
 * The current clock frequency of the HSP.
 */
SECTION (".sprtro.hsp_clock_freq")
uint32_t hsp_clock_freq;
#endif

/**
 * Data populated by ROM.
 */
struct manticore_rom_shared_sram *const rom_shared =
	(struct manticore_rom_shared_sram*) HSP_ADDR_MAP_SHAREDRAM_ADDRESS;

/**
 * HSP AEB registers.
 */
struct Creg_regs_aeb_regs *const aeb_regs =
	(struct Creg_regs_aeb_regs*) HSP_ADDR_MAP_CREG_AEB_INTERFACE_ADDRESS;

/**
 * Misc SW registers.
 */
struct Creg_regs_misc_creg_sw_regs *const sw_regs =
	(struct Creg_regs_misc_creg_sw_regs*) HSP_ADDR_MAP_CREG_MISC_REGS_SW_REGS_ADDRESS;

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
/* Necessary for CMVP testing.  Defined in the specific application context. */
extern const struct hsp_dmb dmb;

/**
 * Test case to execute for CMVP certification testing.
 */
uint32_t cmvp_test = 0;

/**
 * Flag indicating that the CMVP test case requires a simulated SoC reset flow.
 */
bool cmvp_mimic_por = false;
#endif

/**
 * Configuration for the HSP GPIOs.
 */
static const struct hsp_gpio_config gpio_config[] = {
	/* Input GPIOs. */
	{
		.gpio_num = PORT1_RESET_IRQ,
		.is_output = false,
		.init_value = false,
		.init_value_por = false,
		.pull = HSP_GPIO_INTERNAL_PULL_NONE
	},
	{
		.gpio_num = PORT1_AUTH_IRQ,
		.is_output = false,
		.init_value = false,
		.init_value_por = false,
		.pull = HSP_GPIO_INTERNAL_PULL_NONE
	},
	/* Output GPIOs.  Only assert resets and take flash on POR.  Reset controls are active low. */
	{
		.gpio_num = PORT1_RESET_CTRL,
		.is_output = true,
		.init_value = true,
		.init_value_por = false,
		.pull = HSP_GPIO_INTERNAL_PULL_NONE
	},
	{
		.gpio_num = PORT1_SPI_FILTER_MUX,
		.is_output = true,
		.init_value = true,
		.init_value_por = false,
		.pull = HSP_GPIO_INTERNAL_PULL_NONE
	},
	{
		.gpio_num = HEARTBEAT_LED,
		.is_output = true,
		.init_value = true,
		.init_value_por = true,
		.pull = HSP_GPIO_INTERNAL_PULL_NONE
	}
};


/**
 * Determine the current HSP clock frequency based on the state of A0 bypass.  This must be called
 * before anything is derived based on the HSP clock frequency.
 */
void determine_hsp_clock_frequency ()
{
#if !(defined BUILD_FOR_FPGA || defined BUILD_FOR_HAPS)
	if (is_a0_bypass ()) {
		hsp_clock_freq = HSP_CLOCK_REF_CLK_FREQUENCY_HZ;
	}
	else {
		hsp_clock_freq = HSP_CLOCK_PLL_OUT_FREQUENCY_HZ;
	}
#endif
}

/**
 * Apply work-arounds for silicon bugs.
 *
 * @param aeb The HSP aeb driver to configure.
 * @param socid First word of the SOCID.
 * @param chkpt_start Checkpoint to trigger before applying any workarounds.  This checkpoint needs
 * to enable AEB access.  Null if there is no checkpoint needed.
 * @param chkpt_end Checkpoint to trigger after applying workarounds.  This can revoke AEB access.
 * Null if there is no checkpoint needed.
 */
void handle_silicon_errata (const struct hsp_aeb *aeb, uint32_t socid,
	const HSP_CHKPT_CONFIG *chkpt_start, const HSP_CHKPT_CONFIG *chkpt_end)
{
	struct Creg_regs_spi_filter_regs *filter0 =
		(struct Creg_regs_spi_filter_regs*) HSP_ADDR_MAP_CREG_SPI_FILTER0_REGS_ADDRESS;

	code_path_integrity_checkpoint (chkpt_start);

#ifdef MANTICORE_ENABLE_A0_SUPPORT
	/* Check the first byte of the SOCID to determine if the device is A0. */
	if (MANTICORE_IS_A0 (socid)) {
		/* There is a HW bug in A0 that requires AEB65 to be enabled before internal flash can be
		 * accessed. */
		aeb->enable_aeb (aeb, 65);

		/* An A0, the SPI filter is disabled by default.  Update filter 0 to be in bypass mode.
		 * Filter 1 will be handled by normal workflows.
		 *
		 * Apply the default value for the control register that is used in B0. */
		filter0->spi_filter_ctrl = 0x00000800;
	}
#else
	UNUSED (socid);
	UNUSED (aeb);
#endif

	/* The SPI filter defaults interrupts to be enabled in both A0 and B0, which could cause issues
	 * when only one filter is being used.  Though, with the filter always in bypass mode, it's not
	 * clear that any interrupts could actually get generated, anyway.  Disable them just to be
	 * safe. */
	filter0->spi_filter_inten = 0;

	code_path_integrity_checkpoint (chkpt_end);
}

/**
 * Indicate if A0 Bypass has been asserted.
 *
 * @return true if A0 Bypass was asserted at boot or false if not.
 */
bool is_a0_bypass ()
{
	return !!(aeb_regs->AEB_GROUP_3_STATUS &
		CREG_REGS_AEB_REGS_AEB_GROUP_3_STATUS_EXT_AEB_127_A0BYPASS_FIELD_MASK);
}

/**
 * Determine if the current boot should be treated like a POR.
 *
 * @return true for a POR or false for a warm reset.
 */
bool is_por ()
{
	/* Consider the boot to be POR until system initialization has completed for the first time. */
	return !is_sys_init_done ();
}

/**
 * Indicate if the last HSP reset was triggered by software.
 *
 * @return true if a SW triggered reset or false if HW triggered.
 */
bool is_sw_reset ()
{
	/* Due to the work-around for DICE key derivation, there will always be a SW triggered reset,
	 * even in the case of a HW failure.  It's only a SW reset if that's the only bit set. */
	return (sw_regs->SW_STICKY_RW[MANTICORE_ROM_FATAL_ERRORS] ==
		CREG_REGS_STICKY_REGS_HSP_FATAL_ERR_LOG_SW_FATAL_ERR_STS_FIELD_MASK);
}

/**
 * Indicate if the last HSP reset was triggered after a graceful shutdown of the other SoC cores.
 *
 * @return true if a graceful shutdown was performed.
 */
bool is_graceful_reset ()
{
	return (sw_regs->SW_STICKY_RW[MANTICORE_SHUTDOWN_INDICATOR] ==
		GRACEFUL_SHUTDOWN_INDICATOR_MAGIC_NUMBER);
}

/**
 * Indicate if there is a valid owner key for the device, as reported by ROM.
 *
 * @return true if the device has a valid owner key.
 */
bool has_owner_key ()
{
	bool has_owner = false;
	size_t i = 0;

	/* TODO:  There is no verification done on the memory.  If run from 1SP, that may not be
	 * necessary, but it could always be validated against the HW PCR state. */
	while (!has_owner && (i < SP_ECDSA_P384_PUBLIC_KEY_SIZE)) {
		if (rom_shared->firmware.pcr_log[1].owner_public_key.data.key.AsBytes[i] != 0) {
			has_owner = true;
		}

		i++;
	}

	return has_owner;
}

/**
 * Indicate if there is a valid secondary signing key for the 1SP firmware.  This only checks for a
 * secondary owner signing key.  A tenancy grant key is not considered a secondary signing key.
 *
 * @return true if the 1SP firmware was verified with a second signature.
 */
bool has_secondary_signing_key ()
{
	bool has_secondary = false;
	size_t i = 0;

	if (rom_shared->firmware.pcr_log[1].secondary_public_key.data.event.event_id ==
		BOOT_MEASUREMENTS_EVENT_SECONDARY_PUBLIC_KEY) {
		/* The measurement reports a secondary key rather than a tenancy grant key.  Check if the
		 * key data is non-zero. */
		while (!has_secondary && (i < SP_ECDSA_P384_PUBLIC_KEY_SIZE)) {
			if (rom_shared->firmware.pcr_log[1].secondary_public_key.data.key.AsBytes[i] != 0) {
				has_secondary = true;
			}

			i++;
		}
	}

	return has_secondary;
}

/**
 * Determine if the DICE Device ID key generated by ROM can be used to deterministically identify
 * the device.
 *
 * This check is a work-around for a ROM bug that includes the reset type measurement in the DICE
 * CDI. Due to this, the DICE key generated on POR is different from the key generated on resets
 * caused by HW or SW fatal errors.  This check is to ensure only the DICE key generated after a SW
 * triggered reset is considered valid, ensuring a consistent identity for use at run-time.
 *
 * @return true if the DICE Device ID is valid or false if the key needs to be regenerated through a
 * SW reset.
 */
bool is_dice_valid ()
{
	/* If the measured reset type does not indicate only a SW fatal error, the DICE key is not
	 * valid. */
	return (rom_shared->firmware.pcr_log[1].security_state.data.reset_type ==
		CREG_REGS_STICKY_REGS_HSP_FATAL_ERR_LOG_SW_FATAL_ERR_STS_FIELD_MASK);
}

/**
 * Configure the HSP GPIOs for proper host interfacing.
 *
 * @param gpio The HSP GPIO driver to configure.
 *
 * @return 0 if the GPIOs were successfully configured or an error code.
 */
int configure_gpios (const struct hsp_gpio *gpio)
{
	return hsp_gpio_configure_multiple (gpio, gpio_config, ARRAY_SIZE (gpio_config), is_por ());
}

/**
 * Configure the boot order to use during any resets triggered by initialization error handling
 * flows.  This will also preemptively increment the boot error counter in case there is a hardware
 * error condition that triggers the reset.
 */
void configure_error_boot_order ()
{
	uint16_t error_count;
	bool flash_priority = false;

	/* Check the boot order and the error counter to determine if the flash priority has been
	 * swapped.  This needs to be done without depending on SoC register accesses so that it can
	 * work in early boot cases before things are fully initialized.
	 *
	 * If the boot order used by ROM prioritizes external flash while the error counter is below the
	 * recovery image threshold, this is a signal that the strapping pin has been set.  Similarly, a
	 * boot order prioritizing internal flash while the error counter is above the recovery image
	 * threshold indicates the same condition.
	 *
	 * ROM error handling could break these assumptions, but if there are that many errors happening
	 * during boot, it may not matter much which boot order is choosen. */
	error_count =
		MANTICORE_SPRT_BOOT_ERROR_COUNTER (sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS]);
	if (!MANTICORE_SPRT_BOOT_ERROR_BOOT_RECOVERY_IMAGE (error_count)) {
		if (MANTICORE_BOOT_ORDER >= MANTICORE_BOOT_ORDER_EXT_INTA_INTB_REC) {
			flash_priority = true;
		}
	}
	else {
		if (MANTICORE_BOOT_ORDER < MANTICORE_BOOT_ORDER_EXT_INTA_INTB_REC) {
			flash_priority = true;
		}
	}

	/* On impactless updates, only try the main image once before falling back to the recovery
	 * image.  Since the recovery image should still represent that last working firmware image,
	 * it's not expected that the recovery image will need multiple boot attempts to succeed, but
	 * this handling would be allowed if failures continue to happen. */
	if (is_impactless_update ()) {
		MANTICORE_SPRT_USE_RECOVERY_IMAGE (sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS]);
	}

	sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS]++;
	error_count =
		MANTICORE_SPRT_BOOT_ERROR_COUNTER (sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS]);

	if (MANTICORE_SPRT_BOOT_ERROR_BOOT_RECOVERY_IMAGE (error_count)) {
		if (has_updated_impactful_firmware ()) {
			/* There is an impactful update in Slot A.  Only support booting from external flash,
			 * regardless of flash priority. */
			MANTICORE_SET_BOOT_ORDER_OVERRIDE (MANTICORE_BOOT_ORDER_EXT_REC);
		}
		else {
			/* There's no need to check slot B since DC-SCM firmware only supports two boot
			 * slots. */
			if (!flash_priority) {
				MANTICORE_SET_BOOT_ORDER_OVERRIDE (MANTICORE_BOOT_ORDER_EXT_INTA_REC);
			}
			else {
				MANTICORE_SET_BOOT_ORDER_OVERRIDE (MANTICORE_BOOT_ORDER_INTA_EXT_REC);
			}
		}
	}
	else if (MANTICORE_SPRT_BOOT_ERROR_FORCE_I2C_RECOVERY (error_count)) {
		MANTICORE_SET_BOOT_ORDER_OVERRIDE (MANTICORE_BOOT_ORDER_REC);

		/* Once we reach this point, clear the boot error counter so subsequent resets try the
		 * firmware images again. */
		boot_error_clear_counter (sw_regs);
	}
	else if (has_updated_impactful_firmware ()) {
		/* Do not allow Slot A to boot since it contains an impactful update. */
		MANTICORE_SET_BOOT_ORDER_OVERRIDE (MANTICORE_BOOT_ORDER_EXT_REC);
	}
	else {
		/* Just leave the default boot order. */
	}
}

/**
 * Configure the boot order that will be used for any resets after system initialization has
 * completed.
 */
void configure_normal_boot_order ()
{
	if (has_updated_impactful_firmware ()) {
		/* There is an impactful update staged in flash, do not allow it to load. */
		MANTICORE_SET_BOOT_ORDER_OVERRIDE (MANTICORE_BOOT_ORDER_EXT_REC);
	}
	else {
		/* Clear the boot order to the default value of 0. */
		MANTICORE_SET_BOOT_ORDER_OVERRIDE (0);
	}
}

/**
 * Reset the device due to an error.  It is assumed the boot order and error counter have already
 * been updated prior to this call.
 *
 * This function should not return, since the device would have been reset.
 */
void boot_error_reset ()
{
	volatile uint32_t spin_count = 0;

	/* Spin for around 50 ms to wait for applications to complete. */
	while (spin_count < 2500000) {
		spin_count++;
	}

	/* Reset the device. */
	sw_regs->HSP_FATAL_SW_ERR = 1;

	/* Cease all operations. */
	CEASE;
}

/**
 * Clear the counter tracking device resets without a successful boot.
 */
void boot_error_clear_counter ()
{
	sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] &= ~MANTICORE_SPRT_BOOT_ERROR_COUNTER_MASK;
}

/**
 * Indicate that the device is booting in untrusted mode, using an unlock security policy.
 */
void boot_unlocked_device ()
{
	sw_regs->SW_STICKY_RW[MANTICORE_1SP_BOOT_STATUS] |= MANTICORE_1SP_UNLOCKED_BOOT_MASK;
}

/**
 * Determine if the device was ever booted using an unlock security policy.
 *
 * @return true if the device has ever been unlocked.
 */
bool has_booted_unlocked ()
{
	return !!(sw_regs->SW_STICKY_RW[MANTICORE_1SP_BOOT_STATUS] & MANTICORE_1SP_UNLOCKED_BOOT_MASK);
}

/**
 * Indicate that PCR 0 has been extended with SPRT measurements.
 */
void pcr0_extended_with_sprt ()
{
	sw_regs->SW_STICKY_RW[MANTICORE_1SP_BOOT_STATUS] |= MANTICORE_1SP_PCR0_EXTENDED_SPRT_FLAG;
}

/**
 * Determine if PCR 0 has ever been extended with SPRT measurements.
 *
 * @return true if PCR 0 has been extended.
 */
bool is_pcr0_extended_with_sprt ()
{
	return !!(sw_regs->SW_STICKY_RW[MANTICORE_1SP_BOOT_STATUS] &
		MANTICORE_1SP_PCR0_EXTENDED_SPRT_FLAG);
}

/**
 * Indicate that PCR 0 has been extended with AES state measurements.
 */
void pcr0_extended_with_aeb_state ()
{
	sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] |= MANTICORE_1SP_PCR0_EXTENDED_AEB_FLAG;
}

/**
 * Determine if PCR 0 has ever been extended with AEB state measurements.
 *
 * @return true if PCR 0 has been extended.
 */
bool is_pcr0_extended_with_aeb_state ()
{
	return !!(sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] &
		MANTICORE_1SP_PCR0_EXTENDED_AEB_FLAG);
}

/**
 * Indicate that PCR 2 has been extended with SoC firmware measurements.
 */
void pcr2_extended ()
{
	sw_regs->SW_STICKY_RW[MANTICORE_1SP_BOOT_STATUS] |= MANTICORE_1SP_PCR2_EXTENDED_FLAG;
}

/**
 * Determine if PCR 2 has ever been extended with SoC firmware measurements.
 *
 * @return true if PCR 2 has been extended.
 */
bool is_pcr2_extended ()
{
	return !!(sw_regs->SW_STICKY_RW[MANTICORE_1SP_BOOT_STATUS] &
		MANTICORE_1SP_PCR2_EXTENDED_FLAG);
}

/**
 * Indicate the system has initialized successfully and external hosts are running.  Subsequent
 * resets will be treated as warm resets until the SoC reset is asserted.
 */
void system_init_done ()
{
	sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] |= MANTICORE_SPRT_SYSTEM_INIT_FLAG;
}

/**
 * Determine if system initialization has successfully completed at least once.
 *
 * @return true if system init was successful.
 */
bool is_sys_init_done ()
{
	return !!(sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] & MANTICORE_SPRT_SYSTEM_INIT_FLAG);
}

/**
 * Indicate an impactless firmware image has been applied to boot flash.
 */
void impactless_update_applied ()
{
	sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] |= MANTICORE_FIRMWARE_UPDATE_APPLIED;
	sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] &= ~MANTICORE_FIRMWARE_IMPACTFUL_UPDATE;

	configure_normal_boot_order ();
}

/**
 * Indicate an impactful firmware image has been applied to the boot flash.
 */
void impactful_update_applied ()
{
	sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] &= ~MANTICORE_FIRMWARE_UPDATE_APPLIED;
	sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] |= MANTICORE_FIRMWARE_IMPACTFUL_UPDATE;

	configure_normal_boot_order ();
}

/**
 * Indicate the system has successfully loaded and initialized an updated firmware image.
 */
void impactless_update_done ()
{
	sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] &= ~MANTICORE_FIRMWARE_UPDATE_APPLIED;
}

/**
 * Determine if flash currently contains an updated image that has not yet been successfully loaded.
 *
 * @return true if there is an updated image on flash.
 */
bool has_updated_impactless_firmware ()
{
	return !!(sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] &
		MANTICORE_FIRMWARE_UPDATE_APPLIED);
}

/**
 * Determine if flash currently contains an updated image that requires a SoC reset before it can be
 * loaded.
 *
 * @return true if there is an impactful updated image on flash.
 */
bool has_updated_impactful_firmware ()
{
	return !!(sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] &
		MANTICORE_FIRMWARE_IMPACTFUL_UPDATE);
}

/**
 * Determine if the current boot context is part of an impactless firmware update.  This doesn't
 * indicate the current image is the updated image, just that the device is executing an impactless
 * update.  It could be that the device has rolled back to the recovery image as part of this
 * workflow.
 *
 * @return true if an impactless update is being loaded.
 */
bool is_impactless_update ()
{
	return (has_updated_impactless_firmware () && is_graceful_reset ());
}

/**
 * Clear the graceful reset indicator.  Resets will be treated as warm resets and will wipe memory
 * state.
 */
void clear_graceful_reset ()
{
	sw_regs->SW_STICKY_RW[MANTICORE_SHUTDOWN_INDICATOR] = 0;
}

#ifdef MANTICORE_ENABLE_FIPS_CMVP_TESTING
/**
 * Get the CMVP test case identifier stored in GSRAM.
 *
 * @param boot_stage The SP boot stage that is executing.  Only IDs for the current boot stage will
 * be retrieved.
 *
 * @return 0 if the test case was retrieved from GSRAM or an error code.
 */
int get_cmvp_test_case (enum cmvp_test_case_boot_stage boot_stage)
{
	uint32_t *test_id;
	int status;

	status = dmb.map_soc_address (&dmb, CMVP_TEST_CASE_ADDRESS, sizeof (uint32_t),
		HSP_DMB_ACCESS_WRITE, (void**) &test_id);
	if (status != 0) {
		return status;
	}

	cmvp_mimic_por = cmvp_test_case_mimic_por (*test_id);
	if (cmvp_mimic_por) {
		sw_regs->SW_STICKY_RW[MANTICORE_SPRT_BOOT_STATUS] = 0;
		clear_graceful_reset ();
	}

	if ((cmvp_test_case_get_core_id (*test_id) == CMVP_TEST_CASE_CORE_ID_HSP) &&
		(cmvp_test_case_get_boot_stage (*test_id) == boot_stage)) {
		/* Save the test identifier in local memory and erase the global setting. */
		cmvp_test = *test_id;

		if ((boot_stage == CMVP_TEST_CASE_BOOT_STAGE_1SP) &&
			(cmvp_test_case_get_test_type (cmvp_test) == CMVP_TEST_CASE_CAST_NEGATIVE_TEST) &&
			(cmvp_test_case_get_cast_type (cmvp_test) ==
			CMVP_TEST_CASE_CAST_TYPE_PRE_OPERATIONAL) &&
			((cmvp_test_case_get_cast_algorithm (cmvp_test) ==
			CMVP_TEST_CASE_ALGORITHM_RNG_RCT_HEALTH) ||
			(cmvp_test_case_get_cast_algorithm (cmvp_test) ==
			CMVP_TEST_CASE_ALGORITHM_RNG_APT_HEALTH))) {
			/* Do not clear the global test ID if this is a 1SP RNG test. */
		}
		else {
			*test_id = 0;
		}
	}

	dmb.unmap_soc_address (&dmb, test_id);

	return 0;
}

/**
 * Mark the CMVP test identifier has having been consumed so that it won't trigger another execution
 * of the test.
 */
void clear_cmvp_test_case ()
{
	uint32_t *test_id;
	int status;

	status = dmb.map_soc_address (&dmb, CMVP_TEST_CASE_ADDRESS, sizeof (uint32_t),
		HSP_DMB_ACCESS_WRITE, (void**) &test_id);
	if (status != 0) {
		return;
	}

	cmvp_test = 0;
	*test_id = 0;

	dmb.unmap_soc_address (&dmb, test_id);
}

/**
 * Trigger a PCT failure during key generation for CMVP testing.
 *
 * If there is no CMVP test case currently specified or if the specified test case is not for PCT
 * failure, this call will do nothing.
 */
void trigger_cmvp_pct_failure ()
{
	enum cmvp_test_case_cast_algorithm algo = CMVP_TEST_CASE_ALGORITHM_NONE;

	if (cmvp_test == 0) {
		return;
	}

	if ((cmvp_test_case_get_test_type (cmvp_test) == CMVP_TEST_CASE_CAST_NEGATIVE_TEST) &&
		(cmvp_test_case_get_cast_type (cmvp_test) == CMVP_TEST_CASE_CAST_TYPE_PCT)) {
		algo = cmvp_test_case_get_cast_algorithm (cmvp_test);
	}

	switch (algo) {
		case CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_PKA:
		case CMVP_TEST_CASE_ALGORITHM_ECDSA_SIGN_PKA:
			ecdsa_hw_fail_pct = true;
			clear_cmvp_test_case ();
			break;

		case CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_SW:
		case CMVP_TEST_CASE_ALGORITHM_ECDSA_SIGN_SW:
			ecdsa_fail_pct = true;
			clear_cmvp_test_case ();
			break;

		case CMVP_TEST_CASE_ALGORITHM_ECDSA_KEYGEN_CCS:
			ccs_ksu_interface_fail_ecdsa_pct = true;
			clear_cmvp_test_case ();
			break;

		case CMVP_TEST_CASE_ALGORITHM_ECDH_HW:
			ecdh_fail_pct = true;
			clear_cmvp_test_case ();
			break;

		default:
			/* do nothing */
			break;
	}
}
#endif	/* MANTICORE_ENABLE_FIPS_CMVP_TESTING */
