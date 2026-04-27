// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "crypto/rng_hsp.h"
#include "drivers/ccs_ksu_shack1.h"
#include "drivers/crypto_hw.h"
#include "drivers/sram.h"
#include "splibs/inc/sptypes.h"


int ccs_ksu_shack1_set_key (const struct ccs_ksu_interface *ccs, const SP_MSG_384 *key,
	uint8_t key_slot, uint32_t key_attributes)
{
	if ((key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) != 0) {
		return CCS_KSU_UNSUPPORTED_KEY_ATTR;
	}

	return ccs_ksu_set_key (ccs, key, key_slot, key_attributes);
}

int ccs_ksu_shack1_generate_random_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t key_attributes)
{
	if ((key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) != 0) {
		return CCS_KSU_UNSUPPORTED_KEY_ATTR;
	}

	return ccs_ksu_generate_random_key (ccs, key_slot, key_attributes);
}

int ccs_ksu_shack1_derive_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	const SP_MSG_384 *context, uint8_t key_slot, uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (context == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if ((key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) != 0) {
		return CCS_KSU_UNSUPPORTED_KEY_ATTR;
	}

	cmd.command_code = CCS_CMD_CODE_KDF_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_in;
	cmd.address[1] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[2] = (uint32_t) context;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd, SP_MSG_256_SIZE, NULL, 0,
		CCS_KSU_DERIVE_KEY_FAILED, CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_SHA);
}

int ccs_ksu_shack1_derive_key_using_pcr (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	uint8_t pcr, uint8_t key_slot, uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if (ccs_hw == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if ((key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) != 0) {
		return CCS_KSU_UNSUPPORTED_KEY_ATTR;
	}

	cmd.command_code = CCS_CMD_CODE_KDF_PCR;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_in;
	cmd.address[1] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[2] = CCS_KSU_PCR_SLOT | pcr;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd, 0, NULL, 0, CCS_KSU_DERIVE_KEY_PCR_FAILED,
		CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_SHA);
}

int ccs_ksu_shack1_generate_random_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t key_attributes)
{
	UNUSED (ccs);
	UNUSED (key_slot);
	UNUSED (key_attributes);

	return CCS_KSU_UNSUPPORTED_CMD;
}

int ccs_ksu_shack1_derive_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	uint8_t key_slot, uint32_t key_attributes)
{
	UNUSED (ccs);
	UNUSED (key_in);
	UNUSED (key_slot);
	UNUSED (key_attributes);

	return CCS_KSU_UNSUPPORTED_CMD;
}

int ccs_ksu_shack1_derive_fw_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	SP_MSG_384 *key, uint32_t key_attributes)
{
	UNUSED (ccs);
	UNUSED (key_in);
	UNUSED (key);
	UNUSED (key_attributes);

	return CCS_KSU_UNSUPPORTED_CMD;
}

int ccs_ksu_shack1_certify_ecc_public_key (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	uint8_t pcr, uint8_t key_slot, const SP_MSG_384 *sign_data, SP_ECDSA_P384_PUBLIC *public_key,
	SP_ECDSA_P384_SIGNATURE *signature, uint32_t *public_key_attributes,
	uint32_t *signing_key_attributes)
{
	UNUSED (ccs);
	UNUSED (signing_key);
	UNUSED (pcr);
	UNUSED (key_slot);
	UNUSED (sign_data);
	UNUSED (public_key);
	UNUSED (signature);
	UNUSED (public_key_attributes);
	UNUSED (signing_key_attributes);

	return CCS_KSU_UNSUPPORTED_CMD;
}

int ccs_ksu_shack1_ecdh_key_exchange (const struct ccs_ksu_interface *ccs, uint8_t key_in,
	uint8_t key_out, const uint8_t *partner_public_key_and_hash, size_t input_len,
	uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (partner_public_key_and_hash == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if (input_len != (SP_ECDSA_P384_PUBLIC_KEY_SIZE + SP_MSG_384_SIZE)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_ECDH_KEY_EXCHANGE;
	cmd.address[0] = CCS_KSU_KEY_SLOT | key_in;
	cmd.address[1] = CCS_KSU_KEY_SLOT | key_out;
	cmd.address[2] = (uint32_t) partner_public_key_and_hash;
	cmd.address[3] = NULL;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd, SP_ECDSA_P384_PUBLIC_KEY_SIZE + SP_MSG_384_SIZE,
		NULL, 0, CCS_KSU_ECDH_KEY_EXCHANGE_FAILED, CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_PKA);
}

int ccs_ksu_shack1_wrap_key_buffer (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
	const SP_MSG_384 *key, SP_MSG_512 *wrapped_key, uint32_t key_attributes)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (key == NULL) || (wrapped_key == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if ((key_attributes & CCS_KSU_ATTR_KEY_SIZE_384) != 0) {
		return CCS_KSU_UNSUPPORTED_KEY_ATTR;
	}

	cmd.command_code = CCS_CMD_CODE_STORE_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | kek_slot;
	cmd.address[1] = (uint32_t) key;
	cmd.address[2] = (uint32_t) &ccs_hw->buffer->output;
	cmd.attributes = key_attributes;

	return ccs_ksu_execute_command (ccs_hw, &cmd,
		ccs_ksu_reverse_key (key_attributes) | SP_MSG_256_SIZE, (uint8_t*) wrapped_key,
		SP_MSG_384_SIZE, CCS_KSU_WRAP_BUFFER_FAILED, CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_AES);
}

int ccs_ksu_shack1_unwrap_key (const struct ccs_ksu_interface *ccs, uint8_t kek_slot,
	const SP_MSG_512 *wrapped_key, uint8_t key_slot)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (wrapped_key == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_LOAD_KEY;
	cmd.address[0] = CCS_KSU_KEY_SLOT | kek_slot;
	cmd.address[1] = CCS_KSU_KEY_SLOT | key_slot;
	cmd.address[2] = (uint32_t) wrapped_key;

	return ccs_ksu_execute_command (ccs_hw, &cmd, SP_MSG_384_SIZE, NULL, 0, CCS_KSU_UNWRAP_FAILED,
		CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_AES);
}

int ccs_ksu_shack1_reset_pcr (const struct ccs_ksu_interface *ccs, uint8_t pcr)
{
	UNUSED (ccs);
	UNUSED (pcr);

	return CCS_KSU_UNSUPPORTED_CMD;
}

int ccs_ksu_shack1_extend_pcr (const struct ccs_ksu_interface *ccs, uint8_t pcr,
	const SP_MSG_384 *digest)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	struct ccs_command cmd = {0};

	if ((ccs_hw == NULL) || (digest == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	cmd.command_code = CCS_CMD_CODE_EXTEND_PCR;
	cmd.address[0] = CCS_KSU_PCR_SLOT | pcr;
	cmd.address[1] = (uint32_t) digest;

	return ccs_ksu_execute_command (ccs_hw, &cmd, SP_MSG_256_SIZE, NULL, 0,
		CCS_KSU_EXTEND_PCR_FAILED, CCS_KSU_CMD_HW_LOCK_CCS | CCS_KSU_CMD_HW_SHA);
}

int ccs_ksu_shack1_get_pcr_value (const struct ccs_ksu_interface *ccs, uint8_t pcr,
	SP_MSG_384 *value)
{
	const struct ccs_ksu *ccs_hw = (const struct ccs_ksu*) ccs;
	uint32_t address;
	SP_MSG_256 tmp;

	if ((ccs == NULL) || (value == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	if (!ccs_ksu_get_pcr_slot_address (ccs_hw, pcr, &address)) {
		return CCS_KSU_UNSUPPORTED_PCR_SLOT;
	}

	/* The PCR memory is not byte addressable, so copy it to a temporary buffer whose alignment is
	 * known. */
	memcpy (tmp.AsBytes, (void*) (uintptr_t) address, SP_MSG_256_SIZE);

	/* Then copy to the output buffer. */
	memcpy (value->AsBytes, tmp.AsBytes, SP_MSG_256_SIZE);

	return 0;
}

int ccs_ksu_shack1_hmac (const struct ccs_ksu_interface *ccs, uint8_t key_slot, const uint8_t *data,
	size_t length, SP_MSG_384 *hmac, uint32_t *key_attributes)
{
	UNUSED (ccs);
	UNUSED (key_slot);
	UNUSED (data);
	UNUSED (length);
	UNUSED (hmac);
	UNUSED (key_attributes);

	return CCS_KSU_UNSUPPORTED_CMD;
}

/**
 * Initialize a driver for interfacing the the CCS and KSU using SHACK1.
 *
 * @param ccs The CCS driver instance to initialize.
 * @param state Variable context for the CCS driver instance.  The must be uninitialized.
 * @param regs Base address for the CCS registers.
 * @param sha Interface to the HS-SHA used by this CCS instance.
 * @param aes Interface to the AES used by this CCS instance.
 * @param pka Interface to the PKA used by this CCS instance.
 * @param rng Interface to the RNG used by this CCS instance.
 * @param cmd_buffer Location in HSP shared RAM where CCS commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param keys Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 * @param pcrs Address for PCR storage in the KSU.
 * @param num_pcrs The number of PCRs available in the KSU.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
static int ccs_ksu_shack1_init (struct ccs_ksu *ccs, struct ccs_ksu_state *state,
	struct Ccs_regs *regs, const struct hs_sha *sha, const struct hsp_aes *aes,
	const struct ecc_hw_pka *pka, const struct hsp_rng_hw *rng, struct ccs_cmd_buffer *cmd_buffer,
	const struct ksu_key_slot *keys, size_t num_keys, const struct ksu_pcr_slot *pcrs,
	size_t num_pcrs)
{
	if ((ccs == NULL) || (state == NULL) || (regs == NULL) || (sha == NULL) || (aes == NULL) ||
		(pka == NULL) || (rng == NULL) || (keys == NULL) || (pcrs == NULL)) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	memset (ccs, 0, sizeof (struct ccs_ksu));

	ccs->base.is_key_slot_valid = ccs_ksu_is_key_slot_valid;
	ccs->base.get_key_attributes = ccs_ksu_get_key_attributes;
	ccs->base.set_key = ccs_ksu_shack1_set_key;
#ifdef CCS_KSU_ENABLE_SEND_KEY
	ccs->base.send_key = ccs_ksu_send_key;
#endif
	ccs->base.generate_random_key = ccs_ksu_shack1_generate_random_key;
	ccs->base.derive_key = ccs_ksu_shack1_derive_key;
	ccs->base.derive_key_using_pcr = ccs_ksu_shack1_derive_key_using_pcr;
	ccs->base.generate_random_ecc_key = ccs_ksu_shack1_generate_random_ecc_key;
	ccs->base.derive_ecc_key = ccs_ksu_shack1_derive_ecc_key;
	ccs->base.derive_fw_ecc_key = ccs_ksu_shack1_derive_fw_ecc_key;
	ccs->base.export_fw_ecc_key = ccs_ksu_export_fw_ecc_key;
	ccs->base.get_ecc_public_key = ccs_ksu_get_ecc_public_key;
	ccs->base.certify_ecc_public_key = ccs_ksu_shack1_certify_ecc_public_key;
	ccs->base.ecdh_key_exchange = ccs_ksu_shack1_ecdh_key_exchange;
	ccs->base.ecc_sign = ccs_ksu_ecc_sign;
	ccs->base.ecdsa_sign_message = ccs_ksu_ecdsa_sign_message;
	ccs->base.ecdsa_sign_hash = ccs_ksu_ecdsa_sign_hash;
	ccs->base.ecdsa_sign_hash_and_finish = ccs_ksu_ecdsa_sign_hash_and_finish;
	ccs->base.wrap_key_buffer = ccs_ksu_shack1_wrap_key_buffer;
	ccs->base.unwrap_key = ccs_ksu_shack1_unwrap_key;
	ccs->base.burn_key = ccs_ksu_burn_key;
	ccs->base.reset_pcr = ccs_ksu_shack1_reset_pcr;
	ccs->base.extend_pcr = ccs_ksu_shack1_extend_pcr;
	ccs->base.get_pcr_value = ccs_ksu_shack1_get_pcr_value;
	ccs->base.hmac = ccs_ksu_shack1_hmac;

	ccs->state = state;
	ccs->regs = regs;
	ccs->sha = sha;
	ccs->aes = aes;
	ccs->pka = pka;
	ccs->rng = rng;
	ccs->buffer = cmd_buffer;
	ccs->keys = keys;
	ccs->key_slots = num_keys;
	ccs->pcrs = pcrs;
	ccs->pcr_slots = num_pcrs;

	return ccs_ksu_init_state (ccs);
}

/**
 * Initialize a driver for interfacing the the CCS and KSU using SHACK1.  CCS operations will block,
 * waiting for an interrupt to indicate when the hardware has finished.
 *
 * @param ccs The CCS driver instance to initialize.
 * @param state Variable context for the CCS driver instance.  The must be uninitialized.
 * @param regs Base address for the CCS registers.
 * @param sha Interface to the HS-SHA used by this CCS instance.
 * @param aes Interface to the AES used by this CCS instance.
 * @param pka Interface to the PKA used by this CCS instance.
 * @param rng Interface to the RNG used by this CCS instance.
 * @param cmd_buffer Location in HSP shared RAM where CCS commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param keys Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 * @param pcrs Address for PCR storage in the KSU.
 * @param num_pcrs The number of PCRs available in the KSU.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int ccs_ksu_shack1_init_interrupt (struct ccs_ksu *ccs, struct ccs_ksu_state *state,
	struct Ccs_regs *regs, struct Creg_regs_creg_crypto_group *irq_regs, const struct hs_sha *sha,
	const struct hsp_aes *aes, const struct ecc_hw_pka *pka, const struct hsp_rng_hw *rng,
	struct ccs_cmd_buffer *cmd_buffer, const struct ksu_key_slot *keys, size_t num_keys,
	const struct ksu_pcr_slot *pcrs, size_t num_pcrs)
{
	int status;

	if (irq_regs == NULL) {
		return CCS_KSU_INVALID_ARGUMENT;
	}

	status = ccs_ksu_shack1_init (ccs, state, regs, sha, aes, pka, rng, cmd_buffer, keys, num_keys,
		pcrs, num_pcrs);
	if (status == 0) {
		ccs->base_irq.handle_interrupt = ccs_ksu_handle_interrupt;
		ccs->submit_command = ccs_ksu_submit_command_interrupt;

		ccs->irq = irq_regs;
	}

	return status;
}

/**
 * Initialize a driver for interfacing the the CCS and KSU using SHACK1.  CCS operations will enter
 * a busy waiting loop, actively polling the hardware to determine when they have finished.
 *
 * The interrupt handler will be null for instances initialized in this way.
 *
 * @param ccs The CCS driver instance to initialize.
 * @param state Variable context for the CCS driver instance.  The must be uninitialized.
 * @param regs Base address for the CCS registers.
 * @param sha Interface to the HS-SHA used by this CCS instance.
 * @param aes Interface to the AES used by this CCS instance.
 * @param pka Interface to the PKA used by this CCS instance.
 * @param rng Interface to the RNG used by this CCS instance.
 * @param cmd_buffer Location in HSP shared RAM where CCS commands should be constructed.  This
 * must be a 32-bit aligned address.
 * @param keys Address for key storage in the KSU.
 * @param num_keys The number of key slots available in the KSU.
 * @param pcrs Address for PCR storage in the KSU.
 * @param num_pcrs The number of PCRs available in the KSU.
 *
 * @return 0 if the driver was successfully initialized or an error code.
 */
int ccs_ksu_shack1_init_polling (struct ccs_ksu *ccs, struct ccs_ksu_state *state,
	struct Ccs_regs *regs, const struct hs_sha *sha, const struct hsp_aes *aes,
	const struct ecc_hw_pka *pka, const struct hsp_rng_hw *rng, struct ccs_cmd_buffer *cmd_buffer,
	const struct ksu_key_slot *keys, size_t num_keys, const struct ksu_pcr_slot *pcrs,
	size_t num_pcrs)
{
	int status;

	status = ccs_ksu_shack1_init (ccs, state, regs, sha, aes, pka, rng, cmd_buffer, keys, num_keys,
		pcrs, num_pcrs);
	if (status == 0) {
		ccs->submit_command = ccs_ksu_submit_command_polling;
	}

	return status;
}
