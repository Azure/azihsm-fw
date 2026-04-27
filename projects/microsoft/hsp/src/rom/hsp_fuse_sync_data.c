// Copyright (c) Microsoft Corporation. All rights reserved.

#include "hsp_fuse_sync_data.h"

/**
 * Internal function to calculate number of sw length
 *
 * @param fuse_map fuse map reference from fuse controller.
 * @param tenancy_counter_length length of tenancy counter.
 *
 * @return number of sw fuses in bytes.
 */
static size_t hsp_fuse_sync_data_calculate_sw_fuses_length (
	const struct fuse_controller_fuse_map *fuse_map, size_t tenancy_counter_length)
{
	uint32_t fuse_cnt_idx = 0;
	size_t sw_fuses_length = 0;

	/* SW fuses. */
	for (fuse_cnt_idx = 0; fuse_cnt_idx < FUSE_CONTROLLER_SW_FUSES_MAX_CNT; fuse_cnt_idx++) {
		sw_fuses_length += fuse_map->sw[fuse_cnt_idx].fuse_length;
	}

	/* SW_ECC fuses. */
	for (fuse_cnt_idx = 0; fuse_cnt_idx < FUSE_CONTROLLER_SW_ECC_FUSES_MAX_CNT; fuse_cnt_idx++) {
		sw_fuses_length += fuse_map->sw_ecc[fuse_cnt_idx].fuse_length;
	}

	return (sw_fuses_length + tenancy_counter_length);
}

/**
 * Prepare the fuse data and sign the fuse data. Append the data with
 * hmac digest.
 *
 * @param state The current security state of the device.
 * @param ccs CCS and KSU driver to use for key derivation and storage.
 * @param key_slot key slot in KSU used for hmac signing.
 * @param fuses  The fuse controller for security state management.
 * @param ccs_buffer buffer to hold the token and the fuse data.
 * @param ccs_buffer_length length of the ccs buffer containing the fuse data.
 * @param tenancy_counter Current value of the tenancy counter if send by caller
 * will be added to the fuse data. This can be null if the tenancy counter doesn't
 * need to be part of the fuse data.
 * @param tenancy_counter_length Length of the tenancy counter.
 *
 * @return 0 if fuse data preparation is successful or an error code.
 */
int hsp_fuse_sync_data_prepare (enum hsp_security_state state, const struct ccs_ksu_interface *ccs,
	uint8_t key_slot, const struct fuse_controller_interface *fuses, uint8_t *ccs_buffer,
	size_t ccs_buffer_length, uint8_t *tenancy_counter, size_t tenancy_counter_length)
{
	int status = 0;
	size_t sw_fuses_words = 0;
	uint32_t fuse_cnt_idx = 0;
	uint32_t offset = SP_MSG_384_SIZE;
	const struct fuse_controller_fuse_map *fuse_map;

	if ((state != HSP_SECURITY_STATE_PRODUCTION) && (state != HSP_SECURITY_STATE_SECURE)) {
		return FUSE_SYNC_DATA_UNSUPPORTED_SECURITY_STATE;
	}

	if ((fuses == NULL) || (ccs == NULL) || (ccs_buffer == NULL)) {
		return FUSE_SYNC_DATA_INVALID_ARGUMENT;
	}

	if ((tenancy_counter_length != 0) && (tenancy_counter == NULL)) {
		return FUSE_SYNC_DATA_INVALID_ARGUMENT;
	}

	fuse_map = fuses->get_fuse_map (fuses);
	if (fuse_map == NULL) {
		return FUSE_SYNC_DATA_FUSE_MAP_NULL;
	}

	if (ccs_buffer_length < hsp_fuse_sync_data_calculate_sw_fuses_length (fuse_map,
		tenancy_counter_length)) {
		return FUSE_SYNC_DATA_BUFFER_SMALL;
	}

	/* Read SW fuses */
	for (fuse_cnt_idx = 0; fuse_cnt_idx < FUSE_CONTROLLER_SW_FUSES_MAX_CNT; fuse_cnt_idx++) {
		if (fuse_map->sw[fuse_cnt_idx].fuse_length != 0) {
			status = fuses->read_registered_sw_fuses (fuses, fuse_map->sw[fuse_cnt_idx].fuse_addr,
				(uint8_t*) (ccs_buffer + offset), fuse_map->sw[fuse_cnt_idx].fuse_length);
			if (status != 0) {
				return status;
			}

			offset += fuse_map->sw[fuse_cnt_idx].fuse_length;
			sw_fuses_words += fuse_map->sw[fuse_cnt_idx].fuse_length;
		}
	}

	/* Read SW_ECC fuses */
	for (fuse_cnt_idx = 0; fuse_cnt_idx < FUSE_CONTROLLER_SW_ECC_FUSES_MAX_CNT; fuse_cnt_idx++) {
		if (fuse_map->sw_ecc[fuse_cnt_idx].fuse_length != 0) {
			status = fuses->read_registered_sw_fuses (fuses,
				fuse_map->sw_ecc[fuse_cnt_idx].fuse_addr, (uint8_t*) (ccs_buffer + offset),
				fuse_map->sw_ecc[fuse_cnt_idx].fuse_length);
			if (status != 0) {
				return status;
			}

			offset += fuse_map->sw_ecc[fuse_cnt_idx].fuse_length;
			sw_fuses_words += fuse_map->sw_ecc[fuse_cnt_idx].fuse_length;
		}
	}

	/* Copy tenancy counter. */
	if (tenancy_counter != NULL) {
		memcpy (ccs_buffer + offset, tenancy_counter, tenancy_counter_length);
	}

	/* calculate hmac */
	status = ccs->hmac (ccs, key_slot, (uint8_t*) (ccs_buffer + SP_MSG_384_SIZE),
		(sw_fuses_words + tenancy_counter_length), (SP_MSG_384*) ccs_buffer, NULL);

	return status;
}

/**
 * Update the fuse data and verify the fuse data using the
 * token information.
 *
 * @param state The current security state of the device.
 * @param ccs CCS and KSU driver to use for key derivation and storage.
 * @param key_slot key slot in KSU used for hmac verification.
 * @param fuses  The fuse controller for security state management.
 * @param ccs_buffer buffer to hold the token and the fuse data.
 * @param ccs_buffer_length length of the ccs buffer containing the fuse data.
 * @param tenancy_counter Current value of the tenancy counter if send by caller
 * will be added to the fuse data. This can be null if the tenancy counter doesn't
 * need to be part of the fuse data.
 * @param tenancy_counter_length Length of the tenancy counter.
 *
 * @return 0 if fuse data updated successful or an error code.
 */
int hsp_fuse_sync_data_update (enum hsp_security_state state, const struct ccs_ksu_interface *ccs,
	uint8_t key_slot, const struct fuse_controller_interface *fuses, uint8_t *ccs_buffer,
	size_t ccs_buffer_length, uint8_t *tenancy_counter, size_t tenancy_counter_length)
{
	int status = 0;
	uint32_t fuse_cnt_idx = 0;
	SP_MSG_384 token = {0};
	size_t sw_fuses_length = 0;
	uint32_t offset = SP_MSG_384_SIZE;
	const struct fuse_controller_fuse_map *fuse_map;

	if ((state != HSP_SECURITY_STATE_PRODUCTION) && (state != HSP_SECURITY_STATE_SECURE)) {
		return FUSE_SYNC_DATA_UNSUPPORTED_SECURITY_STATE;
	}

	if ((fuses == NULL) || (ccs == NULL) || (ccs_buffer == NULL)) {
		return FUSE_SYNC_DATA_INVALID_ARGUMENT;
	}

	if ((tenancy_counter_length != 0) && (tenancy_counter == NULL)) {
		return FUSE_SYNC_DATA_INVALID_ARGUMENT;
	}

	fuse_map = fuses->get_fuse_map (fuses);
	if (fuse_map == NULL) {
		return FUSE_SYNC_DATA_FUSE_MAP_NULL;
	}

	/* calculate hmac */
	sw_fuses_length = hsp_fuse_sync_data_calculate_sw_fuses_length (fuse_map,
		tenancy_counter_length);

	if (ccs_buffer_length < sw_fuses_length) {
		return FUSE_SYNC_DATA_BUFFER_SMALL;
	}

	status = ccs->hmac (ccs, key_slot, (uint8_t*) (ccs_buffer + SP_MSG_384_SIZE), sw_fuses_length,
		&token, NULL);
	if (status != 0) {
		return status;
	}

	/* hmac token compare */
	status = buffer_compare (token.AsBytes, (uint8_t*) ccs_buffer, SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	/* Program SW fuses to GFC registers. */
	for (fuse_cnt_idx = 0; fuse_cnt_idx < FUSE_CONTROLLER_SW_FUSES_MAX_CNT; fuse_cnt_idx++) {
		if (fuse_map->sw[fuse_cnt_idx].fuse_length != 0) {
			status = fuses->program_registered_sw_fuses (fuses,
				fuse_map->sw[fuse_cnt_idx].fuse_addr, (uint32_t*) (ccs_buffer + offset),
				IN_DWORDS (fuse_map->sw[fuse_cnt_idx].fuse_length));
			if (status != 0) {
				return status;
			}

			offset += fuse_map->sw[fuse_cnt_idx].fuse_length;
		}
	}

	/* Program SW_ECC fuses to GFC registers. */
	for (fuse_cnt_idx = 0; fuse_cnt_idx < FUSE_CONTROLLER_SW_ECC_FUSES_MAX_CNT; fuse_cnt_idx++) {
		if (fuse_map->sw_ecc[fuse_cnt_idx].fuse_length != 0) {
			status = fuses->program_registered_sw_fuses (fuses,
				fuse_map->sw_ecc[fuse_cnt_idx].fuse_addr, (uint32_t*) (ccs_buffer + offset),
				IN_DWORDS (fuse_map->sw_ecc[fuse_cnt_idx].fuse_length));
			if (status != 0) {
				return status;
			}

			offset += fuse_map->sw_ecc[fuse_cnt_idx].fuse_length;
		}
	}

	/* Copy tenancy counter. */
	if (tenancy_counter != NULL) {
		memcpy (tenancy_counter, ccs_buffer + offset, tenancy_counter_length);
	}

	return status;
}
