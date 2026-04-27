// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdbool.h>
#include <string.h>
#include "ide_driver_manticore.h"
#include "common/buffer_util.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "mmio/mmio_util.h"

/**
 * Get the stream type using the stream ID provided in IDE KM messages
 *
 * @param ide An instance of manticore IDE driver
 * @param stream_id Stream ID as supplied in IDE_KM messages
 *
 *  @return enum ide_driver_manticore_stream_id.
 */
static enum ide_driver_manticore_stream_id get_stream_type (const struct ide_driver_manticore *ide,
	uint8_t stream_id)
{
	int status;

	PcieIde_t *reg;
	IdeLinkStreamCtrl0_t lnk_strm_ctrl_reg = {0};
	IdeSlctIdeStreamCtrl0_t slct_strm_ctrl_reg = {0};
	enum ide_driver_manticore_stream_id stream_type = IDE_DRIVER_MANTICORE_STREAM_ID_MAX;

	status = ide->dmb->map_soc_address (ide->dmb, ide->reg_base_addr, sizeof (PcieIde_t),
		HSP_DMB_ACCESS_WRITE, (void**) &reg);
	if (status != 0) {
		return stream_type;
	}

	lnk_strm_ctrl_reg.all =
		mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideLinkStreamCtrl0.all);
	slct_strm_ctrl_reg.all =
		mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideSlctIdeStreamCtrl0.all);

	if (stream_id == lnk_strm_ctrl_reg.b.STREAM_ID) {
		stream_type = IDE_DRIVER_MANTICORE_STREAM_ID_LINK;
	}
	else if (stream_id == slct_strm_ctrl_reg.b.STREAM_ID) {
		stream_type = IDE_DRIVER_MANTICORE_STREAM_ID_SELECTIVE;
	}

	ide->dmb->unmap_soc_address (ide->dmb, reg);

	return stream_type;
}

/**
 * Helper function to wait for key write operation to complete.
 *
 * @param stat_reg Pointer to status register used for checking busy bit
 * @param retry_count Number of retries to check busy bit. Each retry uses 1ms sleep
 *
 * @return true if operation successful, false if times out.
 */
static bool ide_driver_manticore_wait_key_write_ready (PcieIdeAesStat_t *stat_reg,
	uint32_t timeout_in_ms)
{
	PcieIdeAesStat_t reg = {0};
	platform_clock timeout;

	platform_init_timeout (timeout_in_ms, &timeout);
	while (platform_has_timeout_expired (&timeout) == 0) {
		reg.all = mmio_register_read32 (&stat_reg->all);
		if (reg.b.BUSY == 0) {
			return true;
		}
	}

	return false;
}

/**
 * Clear the IDE Rx keys from the hardware as well as in local store.
 *
 * @param reg The IDE register memory region to program the keys.
 * @param key_context Local IDE key context to read the keys from.
 * @param key_set Key Set to use. (Active or Backup)
 * @param stream_id The stream ID to program the key. (Link or Selective IDE stream)
 * @param substream The key substream to program the key. (PCIe Posted, Non-Posted or Completion)
 *
 */
static void ide_driver_manticore_clear_rx_keys (PcieIdeAes_t *reg,
	struct ide_driver_manticore_key_context *key_context, enum ide_driver_manticore_key_set key_set,
	enum ide_driver_manticore_stream_id stream_id, uint8_t substream)
{
	uint32_t key_buffer[IDE_AES_256_KEY_LENGTH_IN_DWORDS] = {0};

	// Zero-out the key in the local context to avoid leaking them.
	buffer_zeroize (&key_context->rx_keys[key_set][stream_id][substream],
		sizeof (key_context->rx_keys[key_set][stream_id][substream]));

	/* Zero-out the key and IV in hardware to avoid leaking them.
	 * Since we are writing into hardware registers, we make sure the access are DWORD aligned
	 * using buffer_reverse_copy_dwords method. Here there is no side by reversing the bytes as
	 * the key and IV are already zeroed out. */
	mmio_register_block_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.key0Key, key_buffer,
		IDE_AES_256_KEY_LENGTH_IN_DWORDS);

	mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.initialIvLsw, 0);
	mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.initialIvMsw, 0);
}

/**
 * Program the IDE Rx keys to the hardware.
 *
 * @param reg The IDE register memory region to program the keys.
 * @param key_context Local IDE key context to read the keys from.
 * @param key_set Key Set to use. (Active or Backup)
 * @param stream_id The stream ID to program the key. (Link or Selective IDE stream)
 * @param substream The key substream to program the key. (PCIe Posted, Non-Posted or Completion)
 *
 * @return 0 if the IDE Rx keys are programmed in the hardware or an error code.
 */
static int ide_driver_manticore_set_rx_keys (PcieIdeAes_t *reg,
	struct ide_driver_manticore_key_context *key_context, enum ide_driver_manticore_key_set key_set,
	enum ide_driver_manticore_stream_id stream_id, uint8_t substream)
{
	uint32_t key_context_id = IDE_KEY_CTRL_CTX_ID (key_set, stream_id, substream);
	PcieIdeAesCtrl_t ctrl_reg = {.all = 0};
	uint32_t key_buffer[IDE_AES_256_KEY_LENGTH_IN_DWORDS] = {0};

	if (key_context->rx_keys[key_set][stream_id][substream].valid == false) {
		return IDE_DRIVER_KEY_SET_GO_RX_KEY_FAILED;
	}

	/* Key dwords have been reversed in the KEY_PROG message, so this is exact order we need */
	memcpy (key_buffer, key_context->rx_keys[key_set][stream_id][substream].aes.key,
		sizeof (key_buffer));

	mmio_register_block_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.key0Key, key_buffer,
		IDE_AES_256_KEY_LENGTH_IN_DWORDS);

	// Write the IV.
	mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.initialIvLsw,
		key_context->rx_keys[key_set][stream_id][substream].aes.iv[1]);
	mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.initialIvMsw,
		key_context->rx_keys[key_set][stream_id][substream].aes.iv[0]);

	// Write the key context ID, BIT-16 - Key Length
	ctrl_reg.all = mmio_register_read32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.ctrl.all);
	ctrl_reg.b.CTX_IDX = key_context_id;
	ctrl_reg.b.KEY_SZ = 1;
	mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.ctrl.all, ctrl_reg.all);

	if (!ide_driver_manticore_wait_key_write_ready (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.stat,
		IDE_DRIVER_MANTICORE_KEY_WRITE_TIMEOUT_MS)) {
		// Clear keys from HW registers, but keep them in the key_context for retry attempts.
		buffer_zeroize (key_buffer, sizeof (key_buffer));
		mmio_register_block_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.key0Key, key_buffer,
			IDE_AES_256_KEY_LENGTH_IN_DWORDS);

		return IDE_DRIVER_KEY_SET_GO_RX_KEY_FAILED;
	}

	// If successful, clear key_context and HW registers
	ide_driver_manticore_clear_rx_keys (reg, key_context, key_set, stream_id, substream);

	buffer_zeroize (key_buffer, sizeof (key_buffer));

	return 0;
}

/**
 * Disable the Rx keys in the hardware as well as in local store.
 *
 * @param reg The IDE register memory region to program the keys.
 * @param key_context Local IDE key context to read the keys from.
 * @param key_set Key Set to use. (Active or Backup)
 *
 * @return 0 if the IDE Rx keys are disabled in the hardware or an error code.
 */
static int ide_driver_manticore_disable_rx_keys (PcieIdeAes_t *reg,
	struct ide_driver_manticore_key_context *key_context, enum ide_driver_manticore_key_set key_set)
{
	enum ide_driver_manticore_stream_id stream_id;
	uint8_t substream;
	uint32_t key_context_id;
	PcieIdeAesCtrl_t ctrl_reg = {.all = 0};
	int status = 0;

	/*
	 * According to:
	 * Integrity and Data Encryption (IDE) – Revision A,
	 * Introduced: August 2019 Updated: 19 Oct 2021
	 *
	 * Upon receipt of the KEY_STOP command, for the indicated Key Set and direction, all keys
	 * must be invalidated and rendered unreadable. */
	for (stream_id = 0; stream_id < IDE_DRIVER_MANTICORE_STREAM_ID_MAX; stream_id++) {
		for (substream = 0; substream < IDE_DRIVER_MANTICORE_SUB_STREAM_ID_MAX; substream++) {
			key_context_id = IDE_KEY_CTRL_CTX_ID (key_set, stream_id, substream);

			// Clear the keys both locally and in hw.
			// Ignore result, to make sure we iterate through all keys in the set
			ide_driver_manticore_clear_rx_keys (reg, key_context, key_set, stream_id, substream);

			// Write the key context ID, BIT-16 - Key Length
			ctrl_reg.all = mmio_register_read32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.ctrl.all);
			ctrl_reg.b.CTX_IDX = key_context_id;
			ctrl_reg.b.KEY_SZ = 1;
			mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.ctrl.all, ctrl_reg.all);

			if (!ide_driver_manticore_wait_key_write_ready (
				&reg->dwcPcieIdeUaesGcmpApb.idAesRxHost.stat,
				IDE_DRIVER_MANTICORE_KEY_WRITE_TIMEOUT_MS)) {
				// Remeber the failure, but keep going
				status = IDE_DRIVER_KEY_SET_STOP_FAILED;
			}
		}
	}

	return status;
}

/**
 * Clear the IDE Tx keys from the hardware as well as in local store.
 *
 * @param reg The IDE register memory region to program the keys.
 * @param key_context Local IDE key context to read the keys from.
 * @param key_set Key Set to use. (Active or Backup)
 * @param stream_id The stream ID to program the key. (Link or Selective IDE stream)
 * @param substream The key substream to program the key. (PCIe Posted, Non-Posted or Completion)
 *
 */
static void ide_driver_manticore_clear_tx_keys (PcieIdeAes_t *reg,
	struct ide_driver_manticore_key_context *key_context, enum ide_driver_manticore_key_set key_set,
	enum ide_driver_manticore_stream_id stream_id, uint8_t substream)
{
	uint32_t key_buffer[IDE_AES_256_KEY_LENGTH_IN_DWORDS] = {0};

	// Zero-out the key in the local context to avoid leaking them.
	buffer_zeroize (&key_context->tx_keys[key_set][stream_id][substream],
		sizeof (key_context->tx_keys[key_set][stream_id][substream]));

	/* Zero-out the key and IV in hardware to avoid leaking them.
	 * Since we are writing into hardware registers, we make sure the access are DWORD aligned
	 * using buffer_reverse_copy_dwords method. Here there is no side by reversing the bytes as
	 * the key and IV are already zeroed out. */
	mmio_register_block_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.key0Key, key_buffer,
		IDE_AES_256_KEY_LENGTH_IN_DWORDS);

	mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.initialIvLsw, 0);
	mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.initialIvMsw, 0);
}

/**
 * Program the IDE Tx keys to the hardware.
 *
 * @param reg The IDE register memory region to program the keys.
 * @param key_context Local IDE key context to read the keys from.
 * @param key_set Key Set to use. (Active or Backup)
 * @param stream_id The stream ID to program the key. (Link or Selective IDE stream)
 * @param substream The key substream to program the key. (PCIe Posted, Non-Posted or Completion)
 *
 * @return 0 if the Tx key was programmed successfully or an error code.
 */
static int ide_driver_manticore_set_tx_keys (PcieIdeAes_t *reg,
	struct ide_driver_manticore_key_context *key_context, enum ide_driver_manticore_key_set key_set,
	enum ide_driver_manticore_stream_id stream_id, uint8_t substream)
{
	uint32_t key_context_id = IDE_KEY_CTRL_CTX_ID (key_set, stream_id, substream);
	PcieIdeAesCtrl_t ctrl_reg = {.all = 0};
	uint32_t key_buffer[IDE_AES_256_KEY_LENGTH_IN_DWORDS] = {0};

	if (key_context->tx_keys[key_set][stream_id][substream].valid == false) {
		return IDE_DRIVER_KEY_SET_GO_TX_KEY_FAILED;
	}

	/* Key dwords have been reversed in the KEY_PROG message, so this is exact order we need */
	memcpy (key_buffer, key_context->tx_keys[key_set][stream_id][substream].aes.key,
		sizeof (key_buffer));

	mmio_register_block_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.key0Key, key_buffer,
		IDE_AES_256_KEY_LENGTH_IN_DWORDS);

	// Write the IV.
	mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.initialIvLsw,
		key_context->tx_keys[key_set][stream_id][substream].aes.iv[1]);
	mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.initialIvMsw,
		key_context->tx_keys[key_set][stream_id][substream].aes.iv[0]);

	// Write the key context ID, BIT-16 - Key Length & BIT-18 - For Encrypt in Tx direction
	ctrl_reg.all = mmio_register_read32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.ctrl.all);
	ctrl_reg.b.CTX_IDX = key_context_id;
	ctrl_reg.b.KEY_SZ = 1;
	ctrl_reg.b.ENCRYPT = 1;
	mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.ctrl.all, ctrl_reg.all);

	if (!ide_driver_manticore_wait_key_write_ready (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.stat,
		IDE_DRIVER_MANTICORE_KEY_WRITE_TIMEOUT_MS)) {
		// Clear keys from HW registers, but keep them in the key_context for retry attempts.
		buffer_zeroize (key_buffer, sizeof (key_buffer));
		mmio_register_block_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.key0Key, key_buffer,
			IDE_AES_256_KEY_LENGTH_IN_DWORDS);

		return IDE_DRIVER_KEY_SET_GO_TX_KEY_FAILED;
	}

	// If successful, clear key_context and HW registers
	ide_driver_manticore_clear_tx_keys (reg, key_context, key_set, stream_id, substream);

	buffer_zeroize (key_buffer, sizeof (key_buffer));

	return 0;
}

/**
 * Disable the Tx keys in the hardware as well as in local store.
 *
 * @param reg The IDE register memory region to program the keys.
 * @param key_context Local IDE key context to read the keys from.
 * @param key_set Key Set to use. (Active or Backup)
 *
 * @return 0 if the IDE Tx keys are disabled in the hardware or an error code.
 */
static int ide_driver_manticore_disable_tx_keys (PcieIdeAes_t *reg,
	struct ide_driver_manticore_key_context *key_context, enum ide_driver_manticore_key_set key_set)
{
	enum ide_driver_manticore_stream_id stream_id;
	uint8_t substream;
	uint32_t key_context_id;
	PcieIdeAesCtrl_t ctrl_reg = {.all = 0};
	int status = 0;

	/*
	 * According to:
	 * Integrity and Data Encryption (IDE) – Revision A,
	 * Introduced: August 2019 Updated: 19 Oct 2021
	 *
	 * Upon receipt of the KEY_STOP command, for the indicated Key Set and direction, all keys
	 * must be invalidated and rendered unreadable.
	 */
	for (stream_id = 0; stream_id < IDE_DRIVER_MANTICORE_STREAM_ID_MAX; stream_id++) {
		for (substream = 0; substream < IDE_DRIVER_MANTICORE_SUB_STREAM_ID_MAX; substream++) {
			key_context_id = IDE_KEY_CTRL_CTX_ID (key_set, stream_id, substream);

			// Clear the keys both locally and in hw.
			ide_driver_manticore_clear_tx_keys (reg, key_context, key_set, stream_id, substream);

			// Write the key context ID, BIT-16 - Key Length & BIT-18 - For Encrypt in Tx direction
			ctrl_reg.all = mmio_register_read32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.ctrl.all);
			ctrl_reg.b.CTX_IDX = key_context_id;
			ctrl_reg.b.KEY_SZ = 1;
			ctrl_reg.b.ENCRYPT = 1;
			mmio_register_write32 (&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.ctrl.all, ctrl_reg.all);

			if (!ide_driver_manticore_wait_key_write_ready (
				&reg->dwcPcieIdeUaesGcmpApb.idAesTxHost.stat,
				IDE_DRIVER_MANTICORE_KEY_WRITE_TIMEOUT_MS)) {
				// Remeber the failure, but keep going
				status = IDE_DRIVER_KEY_SET_STOP_FAILED;
			}
		}
	}

	return status;
}

int ide_driver_manticore_get_bus_device_segment_info (const struct ide_driver *ide_driver,
	uint8_t port_index, uint8_t *bus_num, uint8_t *device_func_num, uint8_t *segment,
	uint8_t *max_port_index)
{
	const struct ide_driver_manticore *ide = TO_DERIVED_TYPE (ide_driver,
		const struct ide_driver_manticore, base);
	PcieAssist_t *reg;
	PcieCoreGeneralCfg_t cfg_reg = {0};
	int status;

	if ((ide_driver == NULL) || (bus_num == NULL) || (device_func_num == NULL) ||
		(segment == NULL) || (max_port_index == NULL) || (port_index != 0)) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	status = ide->dmb->map_soc_address (ide->dmb, ide->assist_reg_base_addr, sizeof (PcieAssist_t),
		HSP_DMB_ACCESS_WRITE, (void**) &reg);
	if (status != 0) {
		return status;
	}

	cfg_reg.all = mmio_register_read32 (&reg->pcieCoreGeneralCfg.all);

	*bus_num = cfg_reg.b.CFG_PBUS_NUM;
	*device_func_num = cfg_reg.b.CFG_PBUS_DEV_NUM << 3;
	*segment = 0;
	*max_port_index = 0;

	ide->dmb->unmap_soc_address (ide->dmb, reg);

	return status;
}

int ide_driver_manticore_get_capability_register (const struct ide_driver *ide_driver,
	uint8_t port_index, struct ide_capability_register *capability_register)
{
	const struct ide_driver_manticore *ide = TO_DERIVED_TYPE (ide_driver,
		const struct ide_driver_manticore, base);
	PcieIde_t *reg = 0;
	int status;

	if ((ide_driver == NULL) || (capability_register == NULL) || (port_index != 0)) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	status = ide->dmb->map_soc_address (ide->dmb, ide->reg_base_addr, sizeof (PcieIde_t),
		HSP_DMB_ACCESS_WRITE, (void**) &reg);
	if (status != 0) {
		return status;
	}

	capability_register->value = mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideCap.all);

	ide->dmb->unmap_soc_address (ide->dmb, reg);

	return 0;
}

int ide_driver_manticore_get_control_register (const struct ide_driver *ide_driver,
	uint8_t port_index, struct ide_control_register *control_register)
{
	const struct ide_driver_manticore *ide = TO_DERIVED_TYPE (ide_driver,
		const struct ide_driver_manticore, base);
	PcieIde_t *reg;
	int status;

	if ((ide_driver == NULL) || (control_register == NULL) || (port_index != 0)) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	status = ide->dmb->map_soc_address (ide->dmb, ide->reg_base_addr, sizeof (PcieIde_t),
		HSP_DMB_ACCESS_WRITE, (void**) &reg);
	if (status != 0) {
		return status;
	}

	control_register->value = mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideCtrl.all);

	ide->dmb->unmap_soc_address (ide->dmb, reg);

	return status;
}

int ide_driver_manticore_get_link_ide_register_block (const struct ide_driver *ide_driver,
	uint8_t port_index, uint8_t block_idx,
	struct ide_link_ide_stream_register_block *register_block)
{
	const struct ide_driver_manticore *ide = TO_DERIVED_TYPE (ide_driver,
		const struct ide_driver_manticore, base);
	PcieIde_t *reg;
	IdeLinkStreamCtrl0_t ctrl_reg = {0};
	IdeLinkStatus0_t lnk_status_reg = {0};
	int status;

	if ((ide_driver == NULL) || (register_block == NULL) || (block_idx > 0) || (port_index != 0)) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	status = ide->dmb->map_soc_address (ide->dmb, ide->reg_base_addr, sizeof (PcieIde_t),
		HSP_DMB_ACCESS_WRITE, (void**) &reg);
	if (status != 0) {
		return status;
	}

	ctrl_reg.all = mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideLinkStreamCtrl0.all);
	lnk_status_reg.all = mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideLinkStatus0.all);

	register_block->stream_control_register.link_ide_stream_enable =
		ctrl_reg.b.LINK_IDE_STREAM_ENABLED;
	register_block->stream_control_register.tx_aggregation_mode_npr = 0;
	register_block->stream_control_register.tx_aggregation_mode_pr = 0;
	register_block->stream_control_register.tx_aggregation_mode_cpl = 0;
	register_block->stream_control_register.pcrc_enable = ctrl_reg.b.PCRC_ENABLE;
	register_block->stream_control_register.selected_algorithm = ctrl_reg.b.SELECTED_ALGORITHM;
	register_block->stream_control_register.tc = ctrl_reg.b.TC;
	register_block->stream_control_register.stream_id = ctrl_reg.b.STREAM_ID;

	register_block->stream_status_register.value = lnk_status_reg.all;

	ide->dmb->unmap_soc_address (ide->dmb, reg);

	return status;
}

int ide_driver_manticore_get_selective_ide_stream_register_block (
	const struct ide_driver *ide_driver, uint8_t port_index, uint8_t block_idx,
	struct ide_selective_ide_stream_register_block *register_block)
{
	const struct ide_driver_manticore *ide = TO_DERIVED_TYPE (ide_driver,
		const struct ide_driver_manticore, base);
	PcieIde_t *reg;
	IdeSlctIdeStreamCtrl0_t ctrl_reg = {0};
	int status;

	if ((ide_driver == NULL) || (register_block == NULL) || (block_idx > 0) || (port_index != 0)) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	status = ide->dmb->map_soc_address (ide->dmb, ide->reg_base_addr, sizeof (PcieIde_t),
		HSP_DMB_ACCESS_WRITE, (void**) &reg);
	if (status != 0) {
		return status;
	}

	ctrl_reg.all = mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideSlctIdeStreamCtrl0.all);

	register_block->sel_ide_stream_cap_reg.value =
		mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideSlctIdeStreamCap0.all);

	register_block->sel_ide_stream_control_reg.selective_ide_stream_enable =
		ctrl_reg.b.SLCT_IDE_STREAM_ENABLED;
	register_block->sel_ide_stream_control_reg.tx_aggregation_mode_npr = 0;
	register_block->sel_ide_stream_control_reg.tx_aggregation_mode_pr = 0;
	register_block->sel_ide_stream_control_reg.tx_aggregation_mode_cpl = 0;
	register_block->sel_ide_stream_control_reg.pcrc_enable = ctrl_reg.b.PCRC_ENABLE;
	register_block->sel_ide_stream_control_reg.selective_ide_for_configuration_requests_enable =
		ctrl_reg.b.SLCR_IDE_CFG_REQ_ENABLE;
	register_block->sel_ide_stream_control_reg.selected_algorithm = ctrl_reg.b.SELECTED_ALGORITHM;
	register_block->sel_ide_stream_control_reg.tc = ctrl_reg.b.TC;
	register_block->sel_ide_stream_control_reg.default_stream = ctrl_reg.b.DEFAULT_STREAM;
	register_block->sel_ide_stream_control_reg.stream_id = ctrl_reg.b.STREAM_ID;

	register_block->sel_ide_stream_status_reg.value =
		mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideSlctIdeStreamStatus0.all);

	register_block->ide_rid_assoc_reg_1.value =
		mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideRidAssosReg10.all);

	register_block->ide_rid_assoc_reg_2.value =
		mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideRidAssosReg20.all);

	register_block->addr_assoc_reg_block[0].register_1.value =
		mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideAddrAssosReg100.all);
	register_block->addr_assoc_reg_block[0].register_2 =
		mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideAddrAssosReg200MemLimitUpper);
	register_block->addr_assoc_reg_block[0].register_3 =
		mmio_register_read32 (&reg->dwcPcieIdeApb.idIdeCaps.ideAddrAssosReg300MemBaseUpper);

	ide->dmb->unmap_soc_address (ide->dmb, reg);

	return status;
}

int ide_driver_manticore_key_prog (const struct ide_driver *ide_driver,	uint8_t port_index,
	uint8_t stream_id, uint8_t key_set, bool tx_key, uint8_t key_substream,	const uint32_t *key,
	uint32_t key_size, const uint32_t *iv, uint32_t iv_size)
{
	const struct ide_driver_manticore *ide = TO_DERIVED_TYPE (ide_driver,
		const struct ide_driver_manticore, base);
	struct ide_driver_manticore_key_context *key_context = NULL;
	enum ide_driver_manticore_stream_id stream_type;
	enum ide_driver_manticore_key_set key_set_idx;
	int status = 0;

	if ((ide_driver == NULL) || (key == NULL) || (iv == NULL) ||
		(key_set >= IDE_DRIVER_MANTICORE_KEY_SET_MAX) ||
		(key_substream >= IDE_DRIVER_MANTICORE_SUB_STREAM_ID_MAX) ||
		(key_size != IDE_AES_256_KEY_LENGTH) || (iv_size != IDE_AES_256_IV_LENGTH)) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	/*
	 * IFV, indicating the initial value for the invocation field of the IV, which must be 64
	 * bits in size, and must initially set to the value 0000_0001h upon establishment of the
	 * Stream and when performing a key refresh.
	 */
	if (((buffer_unaligned_read32 (&iv[0]) != 0) || (buffer_unaligned_read32 (&iv[1]) != 1))) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	if (port_index != 0) {
		return IDE_DRIVER_UNSUPPORTED_PORT_INDEX;
	}

	stream_type = get_stream_type (ide, stream_id);
	if (stream_type == IDE_DRIVER_MANTICORE_STREAM_ID_MAX) {
		return IDE_DRIVER_INVALID_STREAM_ID;
	}

	key_set_idx = (key_set == 0) ? IDE_DRIVER_MANTICORE_KEY_SET_ACTIVE :
			IDE_DRIVER_MANTICORE_KEY_SET_BACKUP;

	status = ide->dmb->map_soc_address (ide->dmb, ide->key_context_addr, sizeof (*key_context),
		HSP_DMB_ACCESS_WRITE, (void**) &key_context);
	if (status != 0) {
		return status;
	}

	if (tx_key) {
		memcpy (&key_context->tx_keys[key_set_idx][stream_type][key_substream].aes.key, key,
			key_size);
		memcpy (&key_context->tx_keys[key_set_idx][stream_type][key_substream].aes.iv, iv, iv_size);
		key_context->tx_keys[key_set_idx][stream_type][key_substream].valid = true;
	}
	else {
		memcpy (&key_context->rx_keys[key_set_idx][stream_type][key_substream].aes.key, key,
			key_size);
		memcpy (&key_context->rx_keys[key_set_idx][stream_type][key_substream].aes.iv, iv, iv_size);
		key_context->rx_keys[key_set_idx][stream_type][key_substream].valid = true;
	}

	ide->dmb->unmap_soc_address (ide->dmb, key_context);

	return 0;
}

int ide_driver_manticore_key_set_go (const struct ide_driver *ide_driver, uint8_t port_index,
	uint8_t stream_id, uint8_t key_set, bool tx_key, uint8_t key_substream)
{
	const struct ide_driver_manticore *ide = TO_DERIVED_TYPE (ide_driver,
		const struct ide_driver_manticore, base);
	struct ide_driver_manticore_key_context *key_context = NULL;
	PcieIdeAes_t *reg = NULL;
	enum ide_driver_manticore_stream_id stream_type;
	enum ide_driver_manticore_key_set key_set_idx;
	int status = 0;

	if ((ide_driver == NULL) || (key_set >= IDE_DRIVER_MANTICORE_KEY_SET_MAX) ||
		(key_substream >= IDE_DRIVER_MANTICORE_SUB_STREAM_ID_MAX) || (port_index != 0)) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	stream_type = get_stream_type (ide, stream_id);
	if (stream_type == IDE_DRIVER_MANTICORE_STREAM_ID_MAX) {
		return IDE_DRIVER_INVALID_STREAM_ID;
	}

	key_set_idx = (key_set == 0) ? IDE_DRIVER_MANTICORE_KEY_SET_ACTIVE :
			IDE_DRIVER_MANTICORE_KEY_SET_BACKUP;

	status = ide->dmb->map_soc_address (ide->dmb, ide->aes_reg_base_addr, sizeof (PcieIdeAes_t),
		HSP_DMB_ACCESS_WRITE, (void**) &reg);
	if (status != 0) {
		return status;
	}

	status = ide->dmb->map_soc_address (ide->dmb, ide->key_context_addr, sizeof (*key_context),
		HSP_DMB_ACCESS_WRITE, (void**) &key_context);
	if (status != 0) {
		ide->dmb->unmap_soc_address (ide->dmb, reg);

		return status;
	}

	if (tx_key) {
		status = ide_driver_manticore_set_tx_keys (reg, key_context, key_set_idx, stream_type,
			key_substream);
	}
	else {
		status = ide_driver_manticore_set_rx_keys (reg,	key_context, key_set_idx, stream_type,
			key_substream);
	}

	ide->dmb->unmap_soc_address (ide->dmb, reg);
	ide->dmb->unmap_soc_address (ide->dmb, key_context);

	return status;
}

int ide_driver_manticore_key_set_stop (const struct ide_driver *ide_driver, uint8_t port_index,
	uint8_t stream_id, uint8_t key_set, bool tx_key, uint8_t key_substream)
{
	const struct ide_driver_manticore *ide = TO_DERIVED_TYPE (ide_driver,
		const struct ide_driver_manticore, base);
	struct ide_driver_manticore_key_context *key_context = NULL;
	PcieIdeAes_t *reg = NULL;
	enum ide_driver_manticore_key_set key_set_idx = (key_set == 0) ?
			IDE_DRIVER_MANTICORE_KEY_SET_ACTIVE : IDE_DRIVER_MANTICORE_KEY_SET_BACKUP;
	struct ide_driver_observer_key_set observer_key_set = {
		.key_set = key_set_idx,
		.stream_id = stream_id,
		.substream_id = key_substream,
		.tx_key = tx_key,
	};
	int status = 0;

	UNUSED (stream_id);
	UNUSED (key_substream);

	if ((ide_driver == NULL) || (key_set >= IDE_DRIVER_MANTICORE_KEY_SET_MAX) ||
		(port_index != 0)) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	status = ide->dmb->map_soc_address (ide->dmb, ide->aes_reg_base_addr, sizeof (PcieIdeAes_t),
		HSP_DMB_ACCESS_WRITE, (void**) &reg);
	if (status != 0) {
		return status;
	}

	status = ide->dmb->map_soc_address (ide->dmb, ide->key_context_addr, sizeof (*key_context),
		HSP_DMB_ACCESS_WRITE, (void**) &key_context);
	if (status != 0) {
		ide->dmb->unmap_soc_address (ide->dmb, reg);

		return status;
	}

	if (tx_key) {
		status = ide_driver_manticore_disable_tx_keys (reg, key_context, key_set_idx);
	}
	else {
		status = ide_driver_manticore_disable_rx_keys (reg, key_context, key_set_idx);
	}

	ide->dmb->unmap_soc_address (ide->dmb, reg);
	ide->dmb->unmap_soc_address (ide->dmb, key_context);

	observable_notify_observers_with_ptr (&ide->state->observable,
		offsetof (struct ide_driver_observer, on_set_stop), &observer_key_set);

	return status;
}

/**
 * Initialize the IDE driver.
 *
 * @param ide The IDE driver to initialize.
 * @param assist_reg_base_addr The base address of the PCIE ASSIST registers.
 * @param ide_reg_base_addr The base address of the IDE registers.
 * @param ide_aes_reg_base_addr The base address of the IDE AES registers.
 * @param dmb The DMB instance for SoC address translation from HSP.
 * @param state The variable context for the IDE driver.
 *
 * @return 0 if the IDE driver was successfully initialized or an error code.
 */
int ide_driver_manticore_init (struct ide_driver_manticore *ide, uint64_t assist_reg_base_addr,
	uint64_t ide_reg_base_addr, uint64_t ide_aes_reg_base_addr, uint64_t ide_key_context_addr,
	const struct hsp_dmb *dmb, struct ide_driver_manticore_state *state)
{
	if ((ide == NULL) || (assist_reg_base_addr == 0) || (ide_reg_base_addr == 0) ||
		(ide_aes_reg_base_addr == 0) || (ide_key_context_addr == 0) || (dmb == NULL) ||
		(state == NULL)) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	memset (ide, 0, sizeof (*ide));

	ide->assist_reg_base_addr = assist_reg_base_addr;
	ide->reg_base_addr = ide_reg_base_addr;
	ide->aes_reg_base_addr = ide_aes_reg_base_addr;
	ide->key_context_addr = ide_key_context_addr;
	ide->dmb = dmb;

	ide->state = state;

	ide->base.get_bus_device_segment_info = ide_driver_manticore_get_bus_device_segment_info;
	ide->base.get_capability_register = ide_driver_manticore_get_capability_register;
	ide->base.get_control_register = ide_driver_manticore_get_control_register;
	ide->base.get_link_ide_register_block = ide_driver_manticore_get_link_ide_register_block;
	ide->base.get_selective_ide_stream_register_block =
		ide_driver_manticore_get_selective_ide_stream_register_block;
	ide->base.key_prog = ide_driver_manticore_key_prog;
	ide->base.key_set_go = ide_driver_manticore_key_set_go;
	ide->base.key_set_stop = ide_driver_manticore_key_set_stop;

	return ide_driver_manticore_init_state (ide);
}

/**
 * Initialize only the variable state for the IDE driver.  The rest of the driver is assumed to have
 * been initialized.
 *
 * @param ide The IDE driver to initialize.
 *
 * @return 0 if the IDE driver state was successfully initialized or an error code.
 */
int ide_driver_manticore_init_state (const struct ide_driver_manticore *ide)
{
	struct ide_driver_manticore_key_context *key_context;
	int status;

	if ((ide == NULL) || (ide->dmb == NULL) || (ide->assist_reg_base_addr == 0) ||
		(ide->reg_base_addr == 0) || (ide->aes_reg_base_addr == 0) ||
		(ide->key_context_addr == 0) || (ide->state == NULL)) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	status = ide->dmb->map_soc_address (ide->dmb, ide->key_context_addr, sizeof (*key_context),
		HSP_DMB_ACCESS_WRITE, (void**) &key_context);
	if (status != 0) {
		return status;
	}

	// Zero out the key context to start with a known state.
	memset (key_context, 0, sizeof (*key_context));

	ide->dmb->unmap_soc_address (ide->dmb, key_context);

	return observable_init (&ide->state->observable);
}

/**
 * Release the resources used by the IDE driver.
 *
 * @param ide The IDE driver to release.
 */
void ide_driver_manticore_release (const struct ide_driver_manticore *ide)
{
	struct ide_driver_manticore_key_context *key_context;
	int status = 0;

	if (ide) {
		status = ide->dmb->map_soc_address (ide->dmb, ide->key_context_addr, sizeof (*key_context),
			HSP_DMB_ACCESS_WRITE, (void**) &key_context);
		if (status == 0) {
			memset (key_context, 0, sizeof (*key_context));
			ide->dmb->unmap_soc_address (ide->dmb, key_context);
		}

		observable_release (&ide->state->observable);
	}
}

/**
 * Add an observer for IDE driver notifications.
 *
 * @param ide The IDE driver instance to register with.
 * @param observer The observer to add.
 *
 * @return 0 if the observer was successfully added or an error code.
 */
int ide_driver_manticore_add_ide_driver_observer (const struct ide_driver_manticore *ide,
	const struct ide_driver_observer *observer)
{
	if (ide == NULL) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	return observable_add_observer (&ide->state->observable, (void*) observer);
}

/**
 * Remove an observer from IDE driver notifications.
 *
 * @param ide The IDE driver instance to deregister from.
 * @param observer The observer to remove.
 *
 * @return 0 if the observer was successfully removed or an error code.
 */
int ide_driver_manticore_remove_ide_driver_observer (const struct ide_driver_manticore *ide,
	const struct ide_driver_observer *observer)
{
	if (ide == NULL) {
		return IDE_DRIVER_INVALID_ARGUMENT;
	}

	return observable_remove_observer (&ide->state->observable, (void*) observer);
}
