// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/unused.h"
#include "host_fw/host_logging.h"
#include "host_fw/host_processor_omc_fw.h"


/**
 * Run SoC flash verification and recovery using the current PFMs.
 *
 * @param omc The host processor instance for the OMC SoC.
 * @param hash The hash engine to use for validation.
 * @param rsa The RSA engine to use for signature verification.
 * @param verify Flag indicating if flash verification should be run.
 * @param recover Flag indicating if flash recovery should be run.
 * @param force_active Flag indicating if active validation should ignore the dirty flag.
 * @param bypass_fail Flag indicating if first activation failure should be returned.
 * @param pfm_state Flag indicating if the pending PFM dirty state should be updated.
 * @param active_notify Flag indicating if the active mode notification should be triggered.
 * @param bypass_notify Flag indicating if the bypass mode notification should be triggered.
 * @param status Value to return if no operation is performed.
 *
 * @return 0 if the operation completed successfully or an error code.
 */
static int host_processor_omc_fw_validate_flash (const struct host_processor_omc_fw *omc,
	const struct hash_engine *hash, const struct rsa_engine *rsa, bool verify, bool recover,
	bool force_active, bool bypass_fail, bool pfm_state, bool active_notify, bool bypass_notify,
	int status)
{
	const struct pfm *active_pfm;
	const struct pfm *pending_pfm;
	bool pending_fail = false;
	bool dirty = host_state_manager_is_inactive_dirty (omc->host_state);
	bool pfm_dirty = host_state_manager_is_pfm_dirty (omc->host_state);

	active_pfm = omc->pfm->get_active_pfm (omc->pfm);
	pending_pfm = omc->pfm->get_pending_pfm (omc->pfm);

	if (pending_pfm && (verify || active_pfm) && (!active_pfm || dirty || pfm_dirty)) {
		/* Check to see if there is an empty pending PFM.  If there is, clear the active PFM and
		* force bypass mode during validation flows. */
		int empty_status = pending_pfm->base.is_empty (&pending_pfm->base);

		if (empty_status == 1) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_HOST_FW,
				HOST_LOGGING_CLEAR_PFMS, host_processor_get_port (&omc->base), 0);

			if (active_pfm) {
				omc->pfm->free_pfm (omc->pfm, active_pfm);
				active_pfm = NULL;
			}

			omc->pfm->free_pfm (omc->pfm, pending_pfm);
			pending_pfm = NULL;

			if (verify) {
				status = omc->pfm->base.clear_all_manifests (&omc->pfm->base);
				if (status != 0) {
					return status;
				}
			}
		}
		else if (empty_status != 0) {
			/* We could not determine the state of the pending PFM.  Remove it from any validation
			* flows. */
			omc->pfm->free_pfm (omc->pfm, pending_pfm);
			pending_pfm = NULL;
			pfm_state = false;

			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HOST_FW,
				HOST_LOGGING_CHECK_PENDING_FAILED, host_processor_get_port (&omc->base), status);

			if (!force_active) {
				status = empty_status;

				/* If we don't need to use an active PFM for validation, just return the error. */
				if (verify && !dirty) {
					goto exit;
				}
			}
		}
	}

	if (verify) {
		if (!pending_pfm && !active_pfm && bypass_notify) {
			observable_notify_observers (&omc->base.state->observable,
				offsetof (struct host_processor_observer, on_bypass_mode));
		}

		if (pending_pfm && (!active_pfm || dirty || pfm_dirty)) {
			status = omc->flash->base.validate_flash (&omc->flash->base, pending_pfm, NULL, hash,
				rsa);
			if (status == 0) {
				omc->pfm->base.activate_pending_manifest (&omc->pfm->base);

				if (active_notify) {
					observable_notify_observers (&omc->base.state->observable,
						offsetof (struct host_processor_observer, on_active_mode));
					active_notify = false;
				}
			}
			else if (IS_VALIDATION_FAILURE (status)) {
				pending_fail = true;
			}

			debug_log_create_entry ((status ==
				0) ? DEBUG_LOG_SEVERITY_INFO : DEBUG_LOG_SEVERITY_WARNING,
				DEBUG_LOG_COMPONENT_HOST_FW, (dirty) ?
					HOST_LOGGING_PENDING_VERIFY_FW_UPDATE :
					HOST_LOGGING_PENDING_VERIFY_CURRENT, host_processor_get_port (&omc->base),
				status);

			if (!active_pfm) {
				if (!pending_fail) {
					goto exit;
				}
				else if ((!bypass_fail) && (status != 0)) {
					observable_notify_observers (&omc->base.state->observable,
						offsetof (struct host_processor_observer, on_bypass_mode));

					status = 0;
				}
			}
		}

		if (active_pfm && (!pending_pfm || (status != 0))) {
			if (force_active || dirty) {
				status = omc->flash->base.validate_flash (&omc->flash->base, active_pfm, NULL, hash,
					rsa);
				if ((status == 0) && active_notify) {
					observable_notify_observers (&omc->base.state->observable,
						offsetof (struct host_processor_observer, on_active_mode));
					active_notify = false;
				}

				debug_log_create_entry ((status == 0) ?
						DEBUG_LOG_SEVERITY_INFO :
						(dirty) ? DEBUG_LOG_SEVERITY_WARNING : DEBUG_LOG_SEVERITY_ERROR,
					DEBUG_LOG_COMPONENT_HOST_FW, (dirty) ?
						HOST_LOGGING_ACTIVE_VERIFY_FW_UPDATE :
						HOST_LOGGING_ACTIVE_VERIFY_CURRENT, host_processor_get_port (&omc->base),
					status);
			}
			else if (recover) {
				status = 0;
			}
		}

		/* With an active PFM and flash that isn't dirty, we will always be in active mode. */
		if (active_pfm && !dirty && !force_active && active_notify) {
			observable_notify_observers (&omc->base.state->observable,
				offsetof (struct host_processor_observer, on_active_mode));
		}
	}

	if (recover && active_pfm && (status != 0)) {
		debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_HOST_FW,
			HOST_LOGGING_ROLLBACK_STARTED, host_processor_get_port (&omc->base), 0);

		if (pending_pfm) {
			status = omc->flash->base.recover_flash (&omc->flash->base, pending_pfm, hash, rsa);
			if (status == 0) {
				pending_fail = false;
				omc->pfm->base.activate_pending_manifest (&omc->pfm->base);
			}
			else {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HOST_FW,
					HOST_LOGGING_PENDING_ROLLBACK_FAILED, status,
					host_processor_get_port (&omc->base));
			}
		}

		if (status != 0) {
			status = omc->flash->base.recover_flash (&omc->flash->base, active_pfm, hash, rsa);
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_HOST_FW,
					HOST_LOGGING_ROLLBACK_FAILED, status, host_processor_get_port (&omc->base));
			}
		}

		if (status == 0) {
			if (active_notify) {
				observable_notify_observers (&omc->base.state->observable,
					offsetof (struct host_processor_observer, on_active_mode));
			}

			debug_log_create_entry (DEBUG_LOG_SEVERITY_INFO, DEBUG_LOG_COMPONENT_HOST_FW,
				HOST_LOGGING_ROLLBACK_COMPLETED, host_processor_get_port (&omc->base), 0);
		}
	}

	if (pfm_state && (!pending_pfm || pending_fail)) {
		host_state_manager_set_pfm_dirty (omc->host_state, false);
	}

exit:
	if (active_pfm) {
		omc->pfm->free_pfm (omc->pfm, active_pfm);
	}
	if (pending_pfm) {
		omc->pfm->free_pfm (omc->pfm, pending_pfm);
	}

	return status;
}

static int host_processor_omc_fw_power_on_reset (const struct host_processor *host,
	const struct hash_engine *hash, const struct rsa_engine *rsa)
{
	const struct host_processor_omc_fw *omc = (const struct host_processor_omc_fw*) host;

	if ((omc == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	host_state_manager_set_pfm_dirty (omc->host_state, true);

	return host_processor_omc_fw_validate_flash (omc, hash, rsa, true, false, true, false, true,
		true, true, 0);
}

static int host_processor_omc_fw_soft_reset (const struct host_processor *host,
	const struct hash_engine *hash, const struct rsa_engine *rsa)
{
	const struct host_processor_omc_fw *omc = (const struct host_processor_omc_fw*) host;

	if ((omc == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	return host_processor_omc_fw_validate_flash (omc, hash, rsa, true, true, false, false, true,
		true, true, 0);
}

static int host_processor_omc_fw_run_time_verification (const struct host_processor *host,
	const struct hash_engine *hash, const struct rsa_engine *rsa)
{
	const struct host_processor_omc_fw *omc = (const struct host_processor_omc_fw*) host;
	int status;

	if ((omc == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	status = host_processor_omc_fw_validate_flash (omc, hash, rsa, true, false, false, true, true,
		false, false, HOST_PROCESSOR_NOTHING_TO_VERIFY);

	if ((status != 0) && (status != HOST_PROCESSOR_NOTHING_TO_VERIFY)) {
		omc->flash->base.recover_flash (&omc->flash->base, NULL, NULL, NULL);
	}

	return status;
}

static int host_processor_omc_fw_flash_rollback (const struct host_processor *host,
	const struct hash_engine *hash, const struct rsa_engine *rsa, bool disable_bypass,
	bool no_reset)
{
	const struct host_processor_omc_fw *omc = (const struct host_processor_omc_fw*) host;

	if ((omc == NULL) || (hash == NULL) || (rsa == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	/* no_reset and disable_bypass will be true as part of the POR flow, when we want the active
	* mode notifications.  In other contexts, we want to suppress this notification.
	*
	* Generally, assumptions like this are not a good design, but this implementation is for a
	* specific scenario.  Rather than burden the general API with an additional parameter to
	* disable notifications, it seems the better approach to use general knowledge of the OMC
	* flows. */
	return host_processor_omc_fw_validate_flash (omc, hash, rsa, false, true, false, true, false,
		(no_reset && disable_bypass), false, HOST_PROCESSOR_NO_ROLLBACK);
}

static int host_processor_omc_fw_recover_active_read_write_data (const struct host_processor *host)
{
	return HOST_PROCESSOR_RW_RECOVERY_UNSUPPORTED;
}

static int host_processor_omc_fw_get_next_reset_verification_actions (
	const struct host_processor *host)
{
	const struct host_processor_omc_fw *omc = (const struct host_processor_omc_fw*) host;
	const struct pfm *active_pfm;
	const struct pfm *pending_pfm;
	int status = HOST_PROCESSOR_ACTION_NONE;

	if (omc == NULL) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	active_pfm = omc->pfm->get_active_pfm (omc->pfm);
	pending_pfm = omc->pfm->get_pending_pfm (omc->pfm);

	if (pending_pfm) {
		if (!active_pfm) {
			status = HOST_PROCESSOR_ACTION_VERIFY_PFM;
		}
		else if (host_state_manager_is_inactive_dirty (omc->host_state)) {
			status = HOST_PROCESSOR_ACTION_VERIFY_PFM_AND_UPDATE;
		}
		else if (host_state_manager_is_pfm_dirty (omc->host_state)) {
			status = HOST_PROCESSOR_ACTION_VERIFY_PFM;
		}
	}
	else if (active_pfm && host_state_manager_is_inactive_dirty (omc->host_state)) {
		status = HOST_PROCESSOR_ACTION_VERIFY_UPDATE;
	}

	if (active_pfm) {
		omc->pfm->free_pfm (omc->pfm, active_pfm);
	}
	if (pending_pfm) {
		omc->pfm->free_pfm (omc->pfm, pending_pfm);
	}

	return status;
}

static int host_processor_omc_fw_needs_config_recovery (const struct host_processor *host)
{
	const struct host_processor_omc_fw *omc = (const struct host_processor_omc_fw*) host;

	if (omc == NULL) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	/* TODO: This is currently only used after run-time verification, which will never leave SoC
	* flash in a bad state.  If this gets used in other situations, this check may need to be
	* updated. */

	return 0;
}

static int host_processor_omc_fw_apply_recovery_image (const struct host_processor *host,
	bool no_reset)
{
	const struct host_processor_omc_fw *omc = (const struct host_processor_omc_fw*) host;

	UNUSED (no_reset);

	if (omc == NULL) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	return HOST_PROCESSOR_RECOVERY_UNSUPPORTED;
}

static int host_processor_omc_fw_bypass_mode (const struct host_processor *host, bool swap_flash)
{
	const struct host_processor_omc_fw *omc = (const struct host_processor_omc_fw*) host;

	UNUSED (swap_flash);

	if (omc == NULL) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	observable_notify_observers (&omc->base.state->observable,
		offsetof (struct host_processor_observer, on_bypass_mode));

	return 0;
}

int host_processor_omc_fw_get_flash_config (const struct host_processor *host,
	spi_filter_flash_mode *mode, spi_filter_cs *current_ro, spi_filter_cs *next_ro,
	enum host_read_only_activation *apply_next_ro)
{
	if ((host == NULL) || (mode == NULL) || (current_ro == NULL) || (next_ro == NULL) ||
		(apply_next_ro == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	return HOST_PROCESSOR_FLASH_CONFIG_UNSUPPORTED;
}

int host_processor_omc_fw_config_read_only_flash (const struct host_processor *host,
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
 * Initialize the interface for executing host actions on OMC firmware.
 *
 * @param host The host processor instance to initialize.
 * @param state Variable context for host processor handling.  This must be uninitialized.
 * @param flash The manager for the protected firmware image on flash.
 * @param host_state The state information for the host.
 * @param pfm The manager for PFMs for the host processor.
 *
 * @return 0 if the host processor interface was successfully initialized or an error code.
 */
int host_processor_omc_fw_init (struct host_processor_omc_fw *host,
	struct host_processor_state *state, struct omc_soc_firmware *flash,
	const struct host_state_manager *host_state, const struct pfm_manager *pfm)
{
	int status;

	if ((host == NULL) || (flash == NULL) || (host_state == NULL) || (pfm == NULL)) {
		return HOST_PROCESSOR_INVALID_ARGUMENT;
	}

	memset (host, 0, sizeof (struct host_processor_omc_fw));

	status = host_processor_init (&host->base, state);
	if (status != 0) {
		return status;
	}

	host->base.power_on_reset = host_processor_omc_fw_power_on_reset;
	host->base.soft_reset = host_processor_omc_fw_soft_reset;
	host->base.run_time_verification = host_processor_omc_fw_run_time_verification;
	host->base.flash_rollback = host_processor_omc_fw_flash_rollback;
	host->base.recover_active_read_write_data =
		host_processor_omc_fw_recover_active_read_write_data;
	host->base.get_next_reset_verification_actions =
		host_processor_omc_fw_get_next_reset_verification_actions;
	host->base.needs_config_recovery = host_processor_omc_fw_needs_config_recovery;
	host->base.apply_recovery_image = host_processor_omc_fw_apply_recovery_image;
	host->base.bypass_mode = host_processor_omc_fw_bypass_mode;
	host->base.get_flash_config = host_processor_omc_fw_get_flash_config;
	host->base.config_read_only_flash = host_processor_omc_fw_config_read_only_flash;

	host->flash = flash;
	host->host_state = host_state;
	host->pfm = pfm;

	return 0;
}

/**
 * Release the OMC firmware protection interface.
 *
 * @param host The host processor instance to release.
 */
void host_processor_omc_fw_release (const struct host_processor_omc_fw *host)
{
	if (host) {
		host_processor_release (&host->base);
	}
}
