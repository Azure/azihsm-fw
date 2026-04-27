// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef AES_HSP_HW_INTF_H_
#define AES_HSP_HW_INTF_H_

#include <stddef.h>
#include <stdint.h>
#include "drivers/ccs_ksu_interface.h"
#include "drivers/hsp_aes.h"


/* Common flows for HSP AES implementations.  These functions are not expected to be called
 * directly.  Rather, they will be leveraged by the different HSP AES types to interface with the
 * common hardware drivers. */

int aes_hsp_hw_intf_set_key (const struct ccs_ksu_interface *ccs, const uint8_t *key, size_t length,
	uint8_t key_slot, uint32_t extra_attributes);
int aes_hsp_hw_intf_clear_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot);
int aes_hsp_hw_intf_encrypt (const struct hsp_aes *aes, const struct ccs_ksu_interface *ccs,
	enum hsp_aes_mode mode, uint8_t key_slot, uint32_t extra_attributes, int no_key_error,
	const uint8_t *plaintext, size_t length, const SP_MSG_128 *iv, uint8_t *ciphertext,
	size_t out_length, SP_MSG_128 *out_iv);
int aes_hsp_hw_intf_decrypt (const struct hsp_aes *aes, const struct ccs_ksu_interface *ccs,
	enum hsp_aes_mode mode, uint8_t key_slot, uint32_t extra_attributes, int no_key_error,
	const uint8_t *ciphertext, size_t length, const SP_MSG_128 *iv, uint8_t *plaintext,
	size_t out_length, SP_MSG_128 *out_iv);


#endif	/* AES_HSP_HW_INTF_H_ */
