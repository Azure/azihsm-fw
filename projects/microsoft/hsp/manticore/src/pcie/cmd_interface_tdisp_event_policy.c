// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_interface_tdisp_event_policy.h"
#include "platform_io_api.h"
#include "cmd_interface/cmd_interface.h"
#include "common/common_math.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "pcie/ide_driver_manticore.h"
#include "pcie/tdisp_driver_manticore.h"


/**
 * Stop all TDISP interfaces which are in RUN or LOCKED state.
 *
 * @param tdisp_event_policy The TDISP event policy interface to use.
 * @param pf_mask Output for the PF mask indicating whether the PF was stopped.
 * @param vf_mask Output for the VF mask indicating which VFs were stopped.
 *
 * @return 0 if successful or an error code.
 */
static int cmd_interface_tdisp_event_policy_stop_active_tdis (
	const struct cmd_interface_tdisp_event_policy *tdisp_event_policy, uint32_t *pf_mask,
	uint64_t *vf_mask)
{
	size_t function_id;
	uint8_t tdi_state;
	int status;

	if ((tdisp_event_policy == NULL) || (pf_mask == NULL) || (vf_mask == NULL)) {
		return CMD_INTERFACE_TDISP_EVENT_POLICY_INVALID_ARGUMENT;
	}

	*pf_mask = 0;
	*vf_mask = 0;

	for (function_id = 0; function_id < TDISP_TDI_MAX_COUNT; function_id++) {
		status = tdisp_event_policy->tdisp->get_device_interface_state (tdisp_event_policy->tdisp,
			function_id, &tdi_state);
		if (status == TDISP_DRIVER_INVALID_INTERFACE) {
			continue;
		}
		else if (status != 0) {
			return status;
		}

		if ((tdi_state == TDISP_TDI_STATE_RUN) || (tdi_state == TDISP_TDI_STATE_CONFIG_LOCKED)) {
			status = tdisp_event_policy->tdisp->stop_interface_request (tdisp_event_policy->tdisp,
				function_id);
			if (status != 0) {
				return status;
			}

			if (function_id == 0) {
				*pf_mask |= (1 << 0);
			}
			else {
				*vf_mask |= (1ULL << (function_id - 1));
			}
		}
	}

	return 0;
}

/**
 * Get masks representing all TDISP interfaces which are in ERROR state.
 *
 * @param tdisp_event_policy The TDISP event policy interface to use.
 * @param pf_mask Output for the PF mask indicating whether the PF is in ERROR state.
 * @param vf_mask Output for the VF mask indicating which VFs are in ERROR state.
 *
 * @return 0 if successful or an error code.
 */
static int cmd_interface_tdisp_event_policy_get_error_state_tdis (
	const struct cmd_interface_tdisp_event_policy *tdisp_event_policy, uint32_t *pf_mask,
	uint64_t *vf_mask)
{
	size_t function_id;
	uint8_t tdi_state;
	int status;

	if ((tdisp_event_policy == NULL) || (pf_mask == NULL) || (vf_mask == NULL)) {
		return CMD_INTERFACE_TDISP_EVENT_POLICY_INVALID_ARGUMENT;
	}

	*pf_mask = 0;
	*vf_mask = 0;

	for (function_id = 0; function_id < TDISP_TDI_MAX_COUNT; function_id++) {
		status = tdisp_event_policy->tdisp->get_device_interface_state (tdisp_event_policy->tdisp,
			function_id, &tdi_state);
		if (status == TDISP_DRIVER_INVALID_INTERFACE) {
			continue;
		}
		else if (status != 0) {
			return status;
		}

		if (tdi_state == TDISP_TDI_STATE_ERROR) {
			if (function_id == 0) {
				*pf_mask |= (1 << 0);
			}
			else {
				*vf_mask |= (1ULL << (function_id - 1));
			}
		}
	}

	return 0;
}

/**
 * Send STOP_INTERFACE IPC message including the given PF and VF masks.
 *
 * @param tdisp_event_policy The TDISP event policy interface to use.
 * @param pf_mask The PF mask to use in the IPC request.
 * @param vf_mask The VF mask to use in the IPC request.
 *
 * @return 0 if successful or an error code.
 */
static int cmd_interface_tdisp_event_policy_send_ipc (
	const struct cmd_interface_tdisp_event_policy *tdisp_event_policy, uint32_t pf_mask,
	uint64_t vf_mask)
{
	int pf_count;
	int vf_count;
	uint32_t ipc_timeout;
	struct ipc_message ipc_msg = {0};
	struct ipc_message_stop_interface *ipc_request =
		(struct ipc_message_stop_interface*) &ipc_msg;

	if (tdisp_event_policy == NULL) {
		return CMD_INTERFACE_TDISP_EVENT_POLICY_INVALID_ARGUMENT;
	}

	pf_count = pf_mask ? 1 : 0;
	vf_count = common_math_get_num_bits_set_in_array ((uint8_t*) &vf_mask, sizeof (vf_mask));

	if ((pf_count + vf_count) > 0) {
		ipc_request->header.opcode = IPC_MESSAGE_OPCODE_STOP_INTERFACE;
		ipc_request->header.data_length = sizeof (ipc_request->payload);
		ipc_request->payload.pf_mask = pf_mask;
		ipc_request->payload.vf_mask = vf_mask;

		ipc_timeout = tdisp_event_policy->ipc_timeout_per_fn * (pf_count + vf_count);

		return tdisp_event_policy->ipc_to_admin->send_and_receive (tdisp_event_policy->ipc_to_admin,
			&ipc_msg, ipc_timeout);
	}

	return 0;
}

int cmd_interface_tdisp_event_policy_process_request (const struct cmd_interface *intf,
	struct cmd_interface_msg *request)
{
	const struct cmd_interface_tdisp_event_policy *tdisp_event_policy =
		TO_DERIVED_TYPE (intf, const struct cmd_interface_tdisp_event_policy, base);
	const struct cmd_interface_tdisp_event_policy_payload *req;

	if ((tdisp_event_policy == NULL) || (request == NULL)) {
		return CMD_INTERFACE_TDISP_EVENT_POLICY_INVALID_ARGUMENT;
	}

	if (request->payload_length < sizeof (struct cmd_interface_tdisp_event_policy_payload)) {
		return CMD_INTERFACE_TDISP_EVENT_POLICY_INVALID_MSG_SIZE;
	}

	req = (const struct cmd_interface_tdisp_event_policy_payload*) request->payload;

	switch (req->source) {
		/* TODO: determine which functions are impacted by the event and act accordingly.
		 * Only TDISP events require the StopInterface IPC because the specific PF/VFs that are
		 * impacted can vary. */
		case CMD_INTERFACE_TDISP_EVENT_POLICY_SOURCE_TDISP:
			return cmd_interface_tdisp_event_policy_send_ipc (tdisp_event_policy, req->pf_mask,
				req->vf_mask);

		case CMD_INTERFACE_TDISP_EVENT_POLICY_SOURCE_PCIE_FLR:
		case CMD_INTERFACE_TDISP_EVENT_POLICY_SOURCE_IDE:
		case CMD_INTERFACE_TDISP_EVENT_POLICY_SOURCE_PCIE_PERST_UP:
		case CMD_INTERFACE_TDISP_EVENT_POLICY_SOURCE_PCIE_PERST_DOWN:
			return 0;

		default:
			return CMD_INTERFACE_TDISP_EVENT_POLICY_INVALID_SOURCE;
	}
}

void cmd_interface_tdisp_event_policy_on_close_session (
	const struct spdm_protocol_session_observer *spdm_observer, uint32_t *session_id)
{
	const struct cmd_interface_tdisp_event_policy *tdisp_event_policy =
		TO_DERIVED_TYPE (spdm_observer, struct cmd_interface_tdisp_event_policy, spdm_observer);
	uint64_t vf_mask = 0;
	uint32_t pf_mask = 0;
	int status;

	UNUSED (session_id);

	if (spdm_observer == NULL) {
		return;
	}

	/* Stop all TDIs in RUN or LOCKED state. */
	status = cmd_interface_tdisp_event_policy_stop_active_tdis (tdisp_event_policy, &pf_mask,
		&vf_mask);
	if (status != 0) {
		return;
	}

	/* Send IPC denoting all TDIs which were stopped. */
	status = cmd_interface_tdisp_event_policy_send_ipc (tdisp_event_policy, pf_mask, vf_mask);
	if (status != 0) {
		return;
	}

	tdisp_event_policy->state->skip_on_ide_set_stop_processing = true;
	/* TODO: Currently, the key_set_stop handling wipes both selective and link stream keys
	 * regardless of the stream_id value.  The key_set_stop handling should be cleaned up to
	 * properly account for the stream_id. */
	for (size_t stream = 0; stream < IDE_DRIVER_MANTICORE_STREAM_ID_MAX; stream++) {
		for (size_t key_set = 0; key_set < IDE_DRIVER_MANTICORE_KEY_SET_MAX; key_set++) {
			for (size_t substream = 0;
				substream < IDE_DRIVER_MANTICORE_SUB_STREAM_ID_MAX; substream++) {
				status = tdisp_event_policy->ide->key_set_stop (tdisp_event_policy->ide, 0, stream,
					key_set, true, substream);
				if (status != 0) {
					goto exit;
				}

				status = tdisp_event_policy->ide->key_set_stop (tdisp_event_policy->ide, 0, stream,
					key_set, false, substream);
				if (status != 0) {
					goto exit;
				}
			}
		}
	}
exit:
	tdisp_event_policy->state->skip_on_ide_set_stop_processing = false;

	return;
}

void cmd_interface_tdisp_event_policy_on_set_stop (
	const struct ide_driver_observer *ide_observer, struct ide_driver_observer_key_set *key_set)
{
	const struct cmd_interface_tdisp_event_policy *tdisp_event_policy =
		TO_DERIVED_TYPE (ide_observer, struct cmd_interface_tdisp_event_policy, ide_observer);
	uint64_t vf_mask = 0;
	uint32_t pf_mask = 0;
	int status;

	UNUSED (key_set);

	if (ide_observer == NULL) {
		return;
	}

	/* Skip processing if this is part of other cleanup */
	if (tdisp_event_policy->state->skip_on_ide_set_stop_processing) {
		return;
	}

	/* Stop all TDIs in RUN or LOCKED state. */
	status = cmd_interface_tdisp_event_policy_stop_active_tdis (tdisp_event_policy, &pf_mask,
		&vf_mask);
	if (status != 0) {
		return;
	}

	/* Send IPC denoting all TDIs which were stopped. */
	status = cmd_interface_tdisp_event_policy_send_ipc (tdisp_event_policy, pf_mask, vf_mask);
	if (status != 0) {
		return;
	}

	return;
}

void cmd_interface_tdisp_event_policy_on_stop_interface (
	const struct tdisp_driver_observer *tdisp_observer, uint32_t *function_index)
{
	const struct cmd_interface_tdisp_event_policy *tdisp_event_policy =
		TO_DERIVED_TYPE (tdisp_observer, struct cmd_interface_tdisp_event_policy, tdisp_observer);
	struct ipc_message ipc_msg = {0};
	struct ipc_message_stop_interface *ipc_request =
		(struct ipc_message_stop_interface*) &ipc_msg;
	uint64_t vf_mask = 0;
	uint32_t pf_mask = 0;

	if ((tdisp_observer == NULL) || (function_index == NULL)) {
		return;
	}

	/* PF is function index 0; VFs are function indices [1, TDISP_TDI_MAX_COUNT). */
	if (*function_index == 0) {
		pf_mask = 0x1;
	}
	else {
		vf_mask = (1 << (*function_index - 1));
	}

	ipc_request->header.opcode = IPC_MESSAGE_OPCODE_STOP_INTERFACE;
	ipc_request->header.data_length = sizeof (ipc_request->payload);
	ipc_request->payload.pf_mask = pf_mask;
	ipc_request->payload.vf_mask = vf_mask;

	tdisp_event_policy->ipc_to_admin->send_and_receive (tdisp_event_policy->ipc_to_admin, &ipc_msg,
		tdisp_event_policy->ipc_timeout_per_fn);

	return;
}

void cmd_interface_tdisp_event_policy_on_soft_reset (
	const struct host_processor_observer *host_observer)
{
	const struct cmd_interface_tdisp_event_policy *tdisp_event_policy =
		TO_DERIVED_TYPE (host_observer, struct cmd_interface_tdisp_event_policy, host_observer);
	uint64_t vf_mask = 0;
	uint32_t pf_mask = 0;
	int status;

	/* On soft reset, ensure that no TDIs are in ERROR state. If any are, send an IPC denoting
	 * them. */
	status = cmd_interface_tdisp_event_policy_get_error_state_tdis (tdisp_event_policy, &pf_mask,
		&vf_mask);
	if (status != 0) {
		return;
	}

	cmd_interface_tdisp_event_policy_send_ipc (tdisp_event_policy, pf_mask, vf_mask);

	return;
}

/**
 * Initialize the TDISP event policy command interface instance.
 *
 * @param tdisp_event_policy TDISP event policy command interface instance to initialize.
 * @param state TDISP event policy runtime state
 * @param ipc_to_admin IPC channel to use for IPC operations from HSP to Admin.
 * @param ide IDE driver to use for IDE operations.
 * @param tdisp TDISP driver to use for TDISP operations.
 * @param ipc_timeout_per_fn_ms The timeout to use per function when sending IPC messages to Admin,
 * in milliseconds.
 *
 * @return 0 if the TDISP event policy command interface instance was initialized successfully or
 * an error code.
 */
int cmd_interface_tdisp_event_policy_init (
	struct cmd_interface_tdisp_event_policy *tdisp_event_policy,
	struct cmd_interface_tdisp_event_policy_state *state, struct ipc_channel *ipc_to_admin,
	struct ide_driver *ide, struct tdisp_driver *tdisp, uint32_t ipc_timeout_per_fn_ms)
{
	if ((tdisp_event_policy == NULL) || (ipc_to_admin == NULL) || (ide == NULL) ||
		(tdisp == NULL) || (state == NULL)) {
		return CMD_INTERFACE_TDISP_EVENT_POLICY_INVALID_ARGUMENT;
	}

	memset (tdisp_event_policy, 0, sizeof (struct cmd_interface_tdisp_event_policy));

	tdisp_event_policy->ipc_to_admin = ipc_to_admin;
	tdisp_event_policy->ide = ide;
	tdisp_event_policy->tdisp = tdisp;
	tdisp_event_policy->ipc_timeout_per_fn = ipc_timeout_per_fn_ms;
	tdisp_event_policy->state = state;

	tdisp_event_policy->base.process_request = cmd_interface_tdisp_event_policy_process_request;

	tdisp_event_policy->spdm_observer.on_new_session = NULL;
	tdisp_event_policy->spdm_observer.on_close_session =
		cmd_interface_tdisp_event_policy_on_close_session;

	tdisp_event_policy->ide_observer.on_set_stop = cmd_interface_tdisp_event_policy_on_set_stop;

	tdisp_event_policy->tdisp_observer.on_start_interface = NULL;
	tdisp_event_policy->tdisp_observer.on_stop_interface =
		cmd_interface_tdisp_event_policy_on_stop_interface;

	tdisp_event_policy->host_observer.on_soft_reset =
		cmd_interface_tdisp_event_policy_on_soft_reset;
	tdisp_event_policy->host_observer.on_active_mode = NULL;
	tdisp_event_policy->host_observer.on_bypass_mode = NULL;
	tdisp_event_policy->host_observer.on_recovery = NULL;

	return cmd_interface_tdisp_event_policy_init_state (tdisp_event_policy);
}

/**
 * Initializes TDISP event policy runtime state
 *
 * @param policy TDISP event policy
 *
 * @return 0 if succeeded, error code otherwise
 */
int cmd_interface_tdisp_event_policy_init_state (
	const struct cmd_interface_tdisp_event_policy *policy)
{
	if ((policy == NULL) || (policy->ipc_to_admin == NULL) || (policy->ide == NULL) ||
		(policy->tdisp == NULL) || (policy->state == NULL)) {
		return CMD_INTERFACE_TDISP_EVENT_POLICY_INVALID_ARGUMENT;
	}

	memset (policy->state, 0, sizeof (*policy->state));

	return 0;
}

/**
 * Release the resources used by the TDISP event policy command interface instance.
 *
 * @param tdisp_event_policy TDISP event policy command interface instance to release.
 */
void cmd_interface_tdisp_event_policy_release (
	const struct cmd_interface_tdisp_event_policy *tdisp_event_policy)
{
	UNUSED (tdisp_event_policy);
}
