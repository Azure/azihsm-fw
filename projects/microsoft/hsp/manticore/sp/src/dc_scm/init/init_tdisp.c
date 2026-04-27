// Copyright (c) Microsoft Corporation. All rights reserved.

#include "init_crypto.h"
#include "init_firmware.h"
#include "init_ipc.h"
#include "init_system.h"
#include "init_tdisp.h"
#include "rot_memory_map.h"
#include "sp_boot.h"
#include "common/array_size.h"
#include "logging/init_logging.h"
#include "pcie/cmd_interface_tdisp_event_policy_static.h"
#include "pcie/ide_driver_manticore_static.h"
#include "pcie/tdisp_driver_manticore_static.h"
#include "pcie/tdisp_manticore_gsram_map.h"
#include "pcie/tdisp_tdi_context_manager_manticore_static.h"
#include "pcisig/ide/cmd_interface_ide_responder_static.h"
#include "pcisig/tdisp/cmd_interface_tdisp_responder_static.h"
#include "pcisig/tdisp/firmware_update_observer_tdisp_static.h"
#include "pcisig/tdisp/tdisp_commands.h"
#include "spdm/cmd_interface_protocol_spdm_pcisig_static.h"

/**
 * Address of PCIE top registers block
 */
#define PCIE_TOP_REGISTERS_ADDR			0xB0160000

/**
 * Address of PCIE assist registers block
 */
#define PCIE_ASSIST_REGISTERS_ADDR		0xB01C0000

/**
 * IDE registers block address
 */
#define PCIE_IDE_REGISTERS_ADDR			0xB01E0000

/**
 * IDE keys control registers block
 */
#define PCIE_IDE_AES_REGISTERS_ADDR		0xB01F0000


/**
 * IDE driver state
 */
static struct ide_driver_manticore_state ide_state = {};

/**
 * IDE driver instance for Manticore device
 */
static const struct ide_driver_manticore ide_driver =
	ide_driver_manticore_static_init (PCIE_ASSIST_REGISTERS_ADDR, PCIE_IDE_REGISTERS_ADDR,
	PCIE_IDE_AES_REGISTERS_ADDR, IDE_KEY_CONTEXT_ADDRESS, &dmb, &ide_state);

/**
 * Handler for IDE messages
 */
static const struct cmd_interface_ide_responder ide_responder =
	cmd_interface_ide_responder_static_init (&ide_driver.base);

/**
 * Manticore TDISP TDI context manager
 */
static struct tdisp_tdi_context_manager_manticore tdi_context_manager =
	tdisp_tdi_context_manager_manticore_static_init (TDISP_TDI_CONTEXTS_GSRAM_ADDRESS,
	TDISP_TDI_CONTEXT_MAX_COUNT, &dmb);

/**
 * List of supported TDISP versions
 */
static const uint8_t tdisp_version_num[] = {
	TDISP_VERSION_1_0,
};

/**
 * PCIE registers block, which covers all PCIE and TDISP blocks. Physcial address usage is
 * preffered.
 */
static struct mmio_register_block_soc_state tdisp_pcie_registers_state = {};
static const struct mmio_register_block_soc tdisp_pcie_registers =
	mmio_register_block_soc_static_init (&tdisp_pcie_registers_state, &dmb, PCIE_TOP_REGISTERS_ADDR,
	0xA0000);

/**
 * Variable context for the TDISP driver
 */
static struct tdisp_driver_manticore_state tdisp_driver_manticore_state = {};

/**
 * Manticore TDISP driver
 */
static const struct tdisp_driver_manticore tdisp_driver =
	tdisp_driver_manticore_static_init (&tdi_context_manager.base, &tdisp_pcie_registers.base,
	&ide_driver.base, &tdisp_driver_manticore_state);

/**
 * Handler for TDISP messages
 */
static const struct cmd_interface_tdisp_responder tdisp_responder =
	cmd_interface_tdisp_responder_static_init (&tdi_context_manager.base, &tdisp_driver.base,
	tdisp_version_num, ARRAY_SIZE (tdisp_version_num), &shared_rng.base);

/**
 * SPDM PCISIG protocol instance
 */
static const struct cmd_interface_protocol_spdm_pcisig spdm_pcisig_protocol =
	cmd_interface_protocol_spdm_pcisig_static_init ();

/**
 * PCISIG message handlers (IDE/TDISP)
 */
static const struct cmd_interface_multi_handler_msg_type spdm_pcisig_handlers[] = {
	cmd_interface_multi_handler_msg_type_static_init (SPDM_PROTOCOL_PCISIG_IDE,
		&ide_responder.base),
	cmd_interface_multi_handler_msg_type_static_init (SPDM_PROTOCOL_PCISIG_TDISP,
		&tdisp_responder.base),
};

/**
 * Multi handler instance for PCISIG
 */
const struct cmd_interface_multi_handler spdm_pcisig_handler =
	cmd_interface_multi_handler_static_init (&spdm_pcisig_protocol.base, spdm_pcisig_handlers,
	ARRAY_SIZE (spdm_pcisig_handlers));

/**
 * Default IPC request timeout per function for the TDISP event policy, in milliseconds.
 */
#define TDISP_EVENT_POLICY_DEFAULT_IPC_TIMEOUT_PER_FN_MS		1200

/**
 * TDISP event policy runtime state
 */
static struct cmd_interface_tdisp_event_policy_state tdisp_event_policy_state;

/**
 * Command interface for handling TDISP events.
 */
const struct cmd_interface_tdisp_event_policy tdisp_event_policy =
	cmd_interface_tdisp_event_policy_static_init (&tdisp_event_policy_state,
	&ipc_hsp_to_admin_stop_intf_channel, &ide_driver.base, &tdisp_driver.base,
	TDISP_EVENT_POLICY_DEFAULT_IPC_TIMEOUT_PER_FN_MS);

/**
 * Observer for firmware updates, enabling TDISP policy to block firmware updates.
 */
static const struct firmware_update_observer_tdisp tdisp_fw_update_observer =
	firmware_update_observer_tdisp_static_init (&tdisp_driver.base, &tdi_context_manager.base,
	TDISP_TDI_CONTEXT_MAX_COUNT);


/**
 * Initialize TDISP infrastructure
 *
 * @param is_graceful_reset Indicates if the reset is graceful.
 *
 * @return 0 if successful, otherwise error code
 */
int initialize_tdisp (bool is_graceful_reset)
{
	int status;

	status = ide_driver_manticore_init_state (&ide_driver);
	if (status != 0) {
		return status;
	}

	status = ide_driver_manticore_add_ide_driver_observer (&ide_driver,
		&tdisp_event_policy.ide_observer);
	if (status != 0) {
		return status;
	}

	status = tdisp_driver_manticore_init_state (&tdisp_driver);
	if (status != 0) {
		return status;
	}

	status = cmd_interface_tdisp_event_policy_init_state (&tdisp_event_policy);
	if (status != 0) {
		return status;
	}

	if ((reset_source != RESET_POR) && !is_graceful_reset) {
		/* If the reset was non-POR and non-graceful, then there was a crash.  Ensure that any
		 * active TDIs are pushed into ERROR state. */
		status = tdisp_driver_manticore_set_all_error_state (&tdisp_driver,
			&tdisp_pcie_registers.base);
		if (status != 0) {
			return status;
		}
	}

	status = tdisp_driver_manticore_add_tdisp_driver_observer (&tdisp_driver,
		&tdisp_event_policy.tdisp_observer);
	if (status != 0) {
		return status;
	}

	status = firmware_update_add_observer (&fw_updater, &tdisp_fw_update_observer.base);
	if (status != 0) {
		return status;
	}

	return 0;
}
