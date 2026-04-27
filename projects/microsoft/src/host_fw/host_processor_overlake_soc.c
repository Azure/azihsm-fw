// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/unused.h"
#include "host_fw/host_logging.h"
#include "host_fw/host_processor_observer.h"
#include "host_fw/host_processor_overlake_soc.h"


/**
 * Give the SPI flash back to the SoC.  This function will spin indefinitely until this operation
 * is successful, unless an update is in progress.  Until it succeeds, the SoC will never be able to
 * boot.
 *
 * @param host The host processor instance.
 *
 * @return 0 if the was successfully given to the SoC or an error code if an update is in progress.
 */
int host_processor_overlake_soc_set_soc_flash_access (
	const struct host_processor_overlake_soc *host)
{
	int status;
	int log_status = 0;
	uint32_t retries = 0;

	do {
		retries++;
		status = host->flash->set_flash_for_soc_access (host->flash);
		if (status == OVERLAKE_FLASH_MGR_UPDATE_IN_PROGRESS) {
			/* There is an update in progress, so the SoC cannot have flash right now. */
			return status;
		}

		if (status != log_status) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HOST_FW,
				HOST_LOGGING_HOST_FLASH_ACCESS_ERROR, host_processor_get_port (&host->base),
				status);
			log_status = status;
		}
	} while (status != 0);

	if (log_status != 0) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_HOST_FW,
			HOST_LOGGING_HOST_FLASH_ACCESS_RETRIES, host_processor_get_port (&host->base), retries);
	}

	return 0;
}

static int host_processor_overlake_soc_power_on_reset (const struct host_processor *host,
	const struct hash_engine *hash, const struct rsa_engine *rsa)
{
	const struct host_processor_overlake_soc *soc =
		(const struct host_processor_overlake_soc*) host;
	int status;
	int boot_status;

	if ((soc == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	status = soc->flash->set_flash_for_rot_access (soc->flash);
	if (status != 0) {
		return status;
	}

	boot_status = soc->boot->power_on_reset (soc->boot, hash, rsa);
	if (soc->board_type != OVERLAKE_GLACIER_PEAK) {
		status = soc->nitro->power_on_reset (soc->nitro, hash, rsa);
	}

	if (boot_status != 0) {
		return boot_status;
	}
	else if (status != 0) {
		return status;
	}

	return host_processor_overlake_soc_set_soc_flash_access (soc);
}

static int host_processor_overlake_soc_soft_reset (const struct host_processor *host,
	const struct hash_engine *hash, const struct rsa_engine *rsa)
{
	const struct host_processor_overlake_soc *soc =
		(const struct host_processor_overlake_soc*) host;
	int status;
	int boot_status;
	int flash_status;

	if ((soc == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	if ((soc->board_type == OVERLAKE_CELESTIAL_PEAK) ||
		(soc->board_type == OVERLAKE_CASTLE_PEAK)) {
		soc->control->hold_processor_in_reset (soc->control, true);
	}

	/* We don't  need to validate flash for OMC. Just give a reset pulse and come back
	 * while ensuring SoC has flash access*/
	if (soc->board_type == OVERLAKE_CASTLE_PEAK) {
		observable_notify_observers (&host->state->observable,
			offsetof (struct host_processor_observer, on_soft_reset));

		flash_status = host_processor_overlake_soc_set_soc_flash_access (soc);
		if (flash_status == 0) {
			platform_msleep (10);
			soc->control->hold_processor_in_reset (soc->control, false);
		}

		return flash_status;
	}

	status = soc->flash->set_flash_for_rot_access (soc->flash);
	if (status != 0) {
		soc->boot->bypass_mode (soc->boot, false);
		if (soc->board_type != OVERLAKE_GLACIER_PEAK) {
			soc->nitro->bypass_mode (soc->nitro, false);
		}
		goto return_flash;
	}

	observable_notify_observers (&host->state->observable,
		offsetof (struct host_processor_observer, on_soft_reset));

	status = soc->flash->has_blocking_update (soc->flash);
	if (status != 0) {
		return status;
	}

	boot_status = soc->boot->soft_reset (soc->boot, hash, rsa);
	if (boot_status != 0) {
		soc->boot->bypass_mode (soc->boot, false);
	}

	if (soc->board_type != OVERLAKE_GLACIER_PEAK) {
		status = soc->nitro->soft_reset (soc->nitro, hash, rsa);
		if (status != 0) {
			soc->nitro->bypass_mode (soc->nitro, false);
		}
	}

	if (boot_status != 0) {
		status = boot_status;
	}

return_flash:
	flash_status = host_processor_overlake_soc_set_soc_flash_access (soc);
	if (flash_status == 0) {
		/* On Non CP boards, hold SoC in reset after flash access is set to SoC. */
		if (soc->board_type != OVERLAKE_CELESTIAL_PEAK) {
			soc->control->hold_processor_in_reset (soc->control, true);
			/* TODO: Investigate why 1ms delay here ends up with a reset pulse less
			 * than 1ms wide. */
			platform_msleep (10);
		}
		soc->control->hold_processor_in_reset (soc->control, false);
	}

	return (status != 0) ? status : flash_status;
}

static int host_processor_overlake_soc_run_time_verification (const struct host_processor *host,
	const struct hash_engine *hash, const struct rsa_engine *rsa)
{
	const struct host_processor_overlake_soc *soc =
		(const struct host_processor_overlake_soc*) host;
	int boot_status;
	int nitro_status = HOST_PROCESSOR_NOTHING_TO_VERIFY;

	if ((soc == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	boot_status = soc->boot->run_time_verification (soc->boot, hash, rsa);
	if (soc->board_type != OVERLAKE_GLACIER_PEAK) {
		nitro_status = soc->nitro->run_time_verification (soc->nitro, hash, rsa);
	}

	if ((boot_status != 0) && (boot_status != HOST_PROCESSOR_NOTHING_TO_VERIFY)) {
		return boot_status;
	}
	else if ((nitro_status != 0) && (nitro_status != HOST_PROCESSOR_NOTHING_TO_VERIFY)) {
		return nitro_status;
	}

	return ((boot_status == 0) || (nitro_status == 0)) ? 0 : HOST_PROCESSOR_NOTHING_TO_VERIFY;
}

static int host_processor_overlake_soc_flash_rollback (const struct host_processor *host,
	const struct hash_engine *hash, const struct rsa_engine *rsa, bool disable_bypass,
	bool no_reset)
{
	const struct host_processor_overlake_soc *soc =
		(const struct host_processor_overlake_soc*) host;
	int boot_status;
	int nitro_status = HOST_PROCESSOR_NO_ROLLBACK;

	if ((soc == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	boot_status = soc->boot->flash_rollback (soc->boot, hash, rsa, disable_bypass, no_reset);
	if (soc->board_type != OVERLAKE_GLACIER_PEAK) {
		nitro_status = soc->nitro->flash_rollback (soc->nitro, hash, rsa, disable_bypass, no_reset);
	}

	if ((boot_status != 0) && (boot_status != HOST_PROCESSOR_NO_ROLLBACK)) {
		return boot_status;
	}
	else if ((nitro_status != 0) && (nitro_status != HOST_PROCESSOR_NO_ROLLBACK)) {
		return nitro_status;
	}

	return ((boot_status == 0) || (nitro_status == 0)) ? 0 : HOST_PROCESSOR_NO_ROLLBACK;
}

static int host_processor_overlake_soc_recover_active_read_write_data (
	const struct host_processor *host)
{
	return HOST_PROCESSOR_RW_RECOVERY_UNSUPPORTED;
}

static int host_processor_overlake_soc_get_next_reset_verification_actions (
	const struct host_processor *host)
{
	const struct host_processor_overlake_soc *soc =
		(const struct host_processor_overlake_soc*) host;
	int boot_action;
	int nitro_action;

	if (soc == NULL) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	boot_action = soc->boot->get_next_reset_verification_actions (soc->boot);
	if (soc->board_type != OVERLAKE_GLACIER_PEAK) {
		nitro_action = soc->nitro->get_next_reset_verification_actions (soc->nitro);

		if (ROT_IS_ERROR (nitro_action)) {
			boot_action = nitro_action;
		}
		else if (!ROT_IS_ERROR (boot_action) && (nitro_action != HOST_PROCESSOR_ACTION_NONE)) {
			if (boot_action == HOST_PROCESSOR_ACTION_NONE) {
				boot_action = nitro_action;
			}
			else if (boot_action != nitro_action) {
				boot_action = HOST_PROCESSOR_ACTION_VERIFY_PFM_AND_UPDATE;
			}
		}
	}

	return boot_action;
}

static int host_processor_overlake_soc_needs_config_recovery (const struct host_processor *host)
{
	const struct host_processor_overlake_soc *soc =
		(const struct host_processor_overlake_soc*) host;
	int boot_flag;
	int nitro_flag = 0;

	if (soc == NULL) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	boot_flag = soc->boot->needs_config_recovery (soc->boot);
	if (soc->board_type != OVERLAKE_GLACIER_PEAK) {
		nitro_flag = soc->nitro->needs_config_recovery (soc->nitro);
	}

	if (ROT_IS_ERROR (boot_flag)) {
		return boot_flag;
	}
	else if (ROT_IS_ERROR (nitro_flag)) {
		return nitro_flag;
	}

	return (boot_flag || nitro_flag);
}

static int host_processor_overlake_soc_apply_recovery_image (const struct host_processor *host,
	bool no_reset)
{
	const struct host_processor_overlake_soc *soc =
		(const struct host_processor_overlake_soc*) host;

	UNUSED (no_reset);

	if (soc == NULL) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	return HOST_PROCESSOR_RECOVERY_UNSUPPORTED;
}

static int host_processor_overlake_soc_bypass_mode (const struct host_processor *host,
	bool swap_flash)
{
	const struct host_processor_overlake_soc *soc =
		(const struct host_processor_overlake_soc*) host;
	int boot_status;
	int nitro_status = 0;

	if (soc == NULL) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	boot_status = soc->boot->bypass_mode (soc->boot, swap_flash);
	if (soc->board_type != OVERLAKE_GLACIER_PEAK) {
		nitro_status = soc->nitro->bypass_mode (soc->nitro, swap_flash);
	}

	if (boot_status != 0) {
		return boot_status;
	}
	else if (nitro_status != 0) {
		return nitro_status;
	}

	return host_processor_overlake_soc_set_soc_flash_access (soc);
}

int host_processor_overlake_soc_get_flash_config (const struct host_processor *host,
	spi_filter_flash_mode *mode, spi_filter_cs *current_ro, spi_filter_cs *next_ro,
	enum host_read_only_activation *apply_next_ro)
{
	if ((host == NULL) || (mode == NULL) || (current_ro == NULL) || (next_ro == NULL) ||
		(apply_next_ro == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	return HOST_PROCESSOR_FLASH_CONFIG_UNSUPPORTED;
}

int host_processor_overlake_soc_config_read_only_flash (const struct host_processor *host,
	const spi_filter_cs *current_ro, const spi_filter_cs *next_ro,
	const enum host_read_only_activation *apply_next_ro)
{
	if (host == NULL) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	UNUSED (current_ro);
	UNUSED (next_ro);
	UNUSED (apply_next_ro);

	return HOST_PROCESSOR_FLASH_CONFIG_UNSUPPORTED;
}

/**
 * Run flash verification and recovery after SoC boot has completed.
 *
 * @param host The host instance to run validation for.
 * @param hash The hash engine to use for firmware validation.
 * @param rsa The RSA engine to use for signature verification.
 * @param fw The firmware being verified.
 *
 * @return 0 if validation complete successfully or an error code.
 */
static int host_processor_overlake_soc_boot_complete_verification (
	const struct host_processor *host, const struct hash_engine *hash, const struct rsa_engine *rsa,
	struct overlake_soc_firmware *fw)
{
	int count = 1;
	int status;

	do {
		status = host->run_time_verification (host, hash, rsa);
	} while ((status != 0) && count--);

	if (status == 0) {
		fw->base.backup_flash (&fw->base);
	}

	count = 2;
	while ((status != 0) && count--) {
		int rollback;

		rollback = host->flash_rollback (host, hash, rsa, false, false);
		if (rollback == HOST_PROCESSOR_NO_ROLLBACK) {
			break;
		}

		status = rollback;
	}

	return status;
}

static int host_processor_overlake_soc_soc_boot_complete (
	const struct host_processor_overlake_soc *host, const struct hash_engine *hash,
	const struct rsa_engine *rsa)
{
	int boot_status;
	int nitro_status = 0;

	if ((host == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	boot_status = host->flash->set_flash_for_rot_access (host->flash);
	if (boot_status != 0) {
		return boot_status;
	}

	boot_status = host_processor_overlake_soc_boot_complete_verification (host->boot, hash, rsa,
		host->flash->get_boot_image (host->flash));
	if (host->board_type != OVERLAKE_GLACIER_PEAK) {
		nitro_status = host_processor_overlake_soc_boot_complete_verification (host->nitro, hash,
			rsa, host->flash->get_nitro_image (host->flash));
	}

	if ((nitro_status != 0) &&
		((boot_status == 0) || IS_VALIDATION_FAILURE (boot_status))) {
		return nitro_status;
	}
	else {
		return boot_status;
	}
}

/**
 * Initialize firmware protection for the Overlake SoC.
 *
 * @param host The SoC manager to initialize.
 * @param state Variable context for host processor handling.  This must be uninitialized.
 * @param control The interface for controlling the host processor.
 * @param flash The manager for the flash device for the host processor.
 * @param boot_fw Firmware protection manager for the boot firmware image.
 * @param nitro_fw Firmware protection manager for the Nitro firmware image.
 * @param board_id The Overlake board ID.
 *
 * @return 0 if the SoC manager was successfully initialized or an error code.
 */
int host_processor_overlake_soc_init (struct host_processor_overlake_soc *host,
	struct host_processor_state *state, const struct host_control *control,
	struct overlake_flash_manager *flash, struct host_processor *boot_fw,
	struct host_processor *nitro_fw, enum overlake_board_id board_id)
{
	int status;

	if ((host == NULL) || (control == NULL) || (flash == NULL) || (boot_fw == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	if ((board_id == OVERLAKE_BOARD_ID_CELESTIAL_PEAK) && (nitro_fw == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	memset (host, 0, sizeof (struct host_processor_overlake_soc));

	status = host_processor_init (&host->base, state);
	if (status != 0) {
		return status;
	}

	host->base.power_on_reset = host_processor_overlake_soc_power_on_reset;
	host->base.soft_reset = host_processor_overlake_soc_soft_reset;
	host->base.run_time_verification = host_processor_overlake_soc_run_time_verification;
	host->base.flash_rollback = host_processor_overlake_soc_flash_rollback;
	host->base.recover_active_read_write_data =
		host_processor_overlake_soc_recover_active_read_write_data;
	host->base.get_next_reset_verification_actions =
		host_processor_overlake_soc_get_next_reset_verification_actions;
	host->base.needs_config_recovery = host_processor_overlake_soc_needs_config_recovery;
	host->base.apply_recovery_image = host_processor_overlake_soc_apply_recovery_image;
	host->base.bypass_mode = host_processor_overlake_soc_bypass_mode;
	host->base.get_flash_config = host_processor_overlake_soc_get_flash_config;
	host->base.config_read_only_flash = host_processor_overlake_soc_config_read_only_flash;

	host->soc_boot_complete = host_processor_overlake_soc_soc_boot_complete;

	host->control = control;
	host->flash = flash;
	host->boot = boot_fw;
	host->nitro = nitro_fw;
	host->board_type = overlake_get_board_type (board_id);

	return 0;
}

/**
 * Release the resources used for SoC protection.
 *
 * @param host The manager to release.
 */
void host_processor_overlake_soc_release (const struct host_processor_overlake_soc *host)
{
	if (host) {
		host_processor_release (&host->base);
	}
}
