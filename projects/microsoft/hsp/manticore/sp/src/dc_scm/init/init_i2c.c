// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdint.h>
#include "hsp_top.h"
#include "common/array_size.h"
#include "init/init_cmd.h"
#include "init/init_i2c.h"
#include "init/init_system.h"
#include "logging/init_logging.h"
#include "mctp/msft_mctp_base_protocol.h"
#include "trap/hsp_interrupt.h"

#ifdef I2C_SLAVE
#include "init_i2c_slave.h"
#else
#include "init_i2c_multimaster.h"
#endif

/**
 * The number of buffers allocated to receive I2C packets.
 */
#define	MANTICORE_NUM_I2C_RX_BUFFERS		2

/**
 * Buffers to receive packets over system I2C channel.
 */
struct cmd_packet system_rx_buffers[MANTICORE_NUM_I2C_RX_BUFFERS];

/**
 * Manager for details about known devices.
 *
 * TODO:  Create a static initializer for this type.
 */
struct device_manager device_manager;

/**
 * Variable context for the FIPS error state handling on the I2C command channel.
 */
static struct cmd_channel_error_state_state fips_i2c_context;

/**
 * I2C channel interposer used to enforce the FIPS error state.
 */
const struct cmd_channel_error_state fips_i2c =
	cmd_channel_error_state_static_init (&fips_i2c_context, &system_i2c.base.base, 0);


/**
 * Initialize the i2c interface
 *
 * @param pcd The PCD to query.
 *
 * @return 0 if initialized successfully or an error code.
 */
int initialize_i2c_interface (const struct pcd *active_pcd)
{
	struct device_manager_full_capabilities i2c_caps;
	uint8_t i_device = 2;
	int status;
	struct pcd_rot_info rot_info = {
		.is_pa_rot = DEFAULT_IS_PA_ROT,
		.port_count = 1,
		.components_count = 0,
		.i2c_slave_addr = DEFAULT_I2C_SLAVE_ADDR,
		.eid = MCTP_BASE_PROTOCOL_MANTICORE_AC_ROT_EID,
		.bridge_i2c_addr = DEFAULT_BMC_SLAVE_ADDRESS,
		.bridge_eid = MCTP_BASE_PROTOCOL_BMC_EID,
	};

	if (active_pcd) {
		status = active_pcd->get_rot_info (active_pcd, &rot_info);
		if (status != 0) {
			debug_log_create_entry (DEBUG_LOG_SEVERITY_ERROR, DEBUG_LOG_COMPONENT_INIT,
				INIT_LOGGING_ROT_CONFIG, status, 0);
		}
	}

	status = device_manager_init (&device_manager, 6, 0, 0,
		rot_info.is_pa_rot ? DEVICE_MANAGER_PA_ROT_MODE : DEVICE_MANAGER_AC_ROT_MODE,
		DEVICE_MANAGER_SLAVE_BUS_ROLE, rot_info.attestation_fail_retry,
		rot_info.attestation_success_retry, rot_info.discovery_fail_retry,
		rot_info.mctp_ctrl_timeout, rot_info.mctp_bridge_additional_timeout,
		rot_info.attestation_rsp_not_ready_max_duration,
		rot_info.attestation_rsp_not_ready_max_retry);
	if (status != 0) {
		goto done;
	}

	// Update entry for Cerberus
	status = device_manager_update_not_attestable_device_entry (&device_manager,
		DEVICE_MANAGER_SELF_DEVICE_NUM, rot_info.eid, rot_info.i2c_slave_addr,
		DEVICE_MANAGER_NOT_PCD_COMPONENT);
	if (status != 0) {
		goto done;
	}

	// Adjust the capabilities on the I2C interface.
	device_manager_get_device_capabilities (&device_manager, DEVICE_MANAGER_SELF_DEVICE_NUM,
		&i2c_caps);
	i2c_caps.request.security_mode |= DEVICE_MANAGER_SECURITY_CONFIDENTIALITY;
	i2c_caps.request.ecc_key_strength = DEVICE_MANAGER_ECC_KEY_256;
	i2c_caps.request.ecdsa = 1;
	i2c_caps.request.rsa_key_strength = DEVICE_MANAGER_RSA_KEY_2048 | DEVICE_MANAGER_RSA_KEY_3072 |
		DEVICE_MANAGER_RSA_KEY_4096;
	i2c_caps.request.rsa = 1;
	i2c_caps.request.aes_enc_key_strength = DEVICE_MANAGER_AES_KEY_256;
	i2c_caps.request.pfm_support = 1;
	i2c_caps.request.fw_protection = 1;
	device_manager_update_device_capabilities (&device_manager, DEVICE_MANAGER_SELF_DEVICE_NUM,
		&i2c_caps);

	// Update entry for BMC
	status = device_manager_update_not_attestable_device_entry (&device_manager,
		DEVICE_MANAGER_MCTP_BRIDGE_DEVICE_NUM, rot_info.bridge_eid, rot_info.bridge_i2c_addr,
		DEVICE_MANAGER_NOT_PCD_COMPONENT);
	if (status != 0) {
		goto done;
	}

	// Update entry for local BMC (using NULL EID to transmit message to local BMC)
	status = device_manager_update_not_attestable_device_entry (&device_manager, i_device,
		MCTP_BASE_PROTOCOL_NULL_EID, rot_info.bridge_i2c_addr, DEVICE_MANAGER_NOT_PCD_COMPONENT);
	if (status != 0) {
		goto done;
	}

	++i_device;

	/*
	 * Create a copy of the device manager entry for the BMC for handling Multisled
	 * scenarios for now. This will be removed once the device manager is updated to
	 * handle multisled scenarios.
	 */
	status = device_manager_update_not_attestable_device_entry (&device_manager, i_device,
		rot_info.bridge_eid, rot_info.bridge_i2c_addr, DEVICE_MANAGER_NOT_PCD_COMPONENT);
	if (status != 0) {
		goto done;
	}
	++i_device;

	// Update entry for in-band utility
	status = device_manager_update_not_attestable_device_entry (&device_manager, i_device,
		MCTP_BASE_PROTOCOL_IB_EXT_MGMT, rot_info.bridge_i2c_addr, DEVICE_MANAGER_NOT_PCD_COMPONENT);
	if (status != 0) {
		goto done;
	}

	++i_device;

	// Update entry for out-of-band utility
	status = device_manager_update_not_attestable_device_entry (&device_manager, i_device,
		MCTP_BASE_PROTOCOL_OOB_EXT_MGMT, rot_info.bridge_i2c_addr,
		DEVICE_MANAGER_NOT_PCD_COMPONENT);
	if (status != 0) {
		goto done;
	}

	status = cmd_channel_error_state_init_state (&fips_i2c);
	if (status != 0) {
		goto done;
	}

	status = initialize_i2c_driver (rot_info.i2c_slave_addr, system_rx_buffers,
		ARRAY_SIZE (system_rx_buffers));
	if (status != 0) {
		goto done;
	}

	status = hsp_interrupt_register (CREG_REGS_INT_HSP_INTSTS_I2C_INTSTS_LSB,
		&i2c_hw.i2c_base.isr_handler);
	if (status != 0) {
		goto done;
	}

done:

	return status;
}

/**
 * Start the i2c interface by enabling the hsp interrupt
 *
 * @return 0 if successfully enable the interrupt & slave mode or an error code.
 */
int start_i2c_interface ()
{
	int status;

	status = hsp_interrupt_enable (CREG_REGS_INT_HSP_IRQINTEN_I2C_INTEN_LSB,
		HSP_INTERRUPT_IRQ_LEVEL_IRQ);
	if (status != 0) {
		return status;
	}

	i2c_dw_apb_enable_slave_mode (&i2c_hw.i2c_base);

	return 0;
}
