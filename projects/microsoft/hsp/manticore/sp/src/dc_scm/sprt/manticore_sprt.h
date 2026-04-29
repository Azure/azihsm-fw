// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_SPRT_H_
#define MANTICORE_SPRT_H_

#include <stdint.h>
#include "manticore_rom.h"
#include "dc_scm/1sp/manticore_1sp.h"


/**
 * The buffer size in shared SRAM for AES operations.
 *
 * TODO:  This is not used currently, so keep it minimal.
 */
#define	MANTICORE_CRYPTO_AES_BUFFER_SIZE	(SP_MSG_128_SIZE)

/**
 * The buffer size in shared SRAM for hash operations with HS-SHA hardware.
 */
#define	MANTICORE_CRYPTO_HS_SHA_BUFFER_SIZE         \
	(sizeof (union manticore_rom_shared_sram_internal) - sizeof (struct ccs_cmd_buffer) - \
		sizeof (struct ecc_hw_pka_cmd_buffer) - sizeof (struct hs_sha_cmd_buffer) - \
		sizeof (struct hsp_aes_cmd_buffer) - MANTICORE_CRYPTO_AES_BUFFER_SIZE - \
		MANTICORE_UNLOCK_HMAC_BUFFER_SIZE)

/**
 * Define the crypto command buffers that will be used during SPRT execution.  The buffers cannot
 * overlap since the application runs multiple tasks.
 */
struct manticore_sprt_shared_sram_crypto {
	struct ccs_cmd_buffer ccs;								/**< Command buffer for CCS HW. */
	struct ecc_hw_pka_cmd_buffer pka;						/**< Command buffer for PKA HW. */
	struct {
		struct hs_sha_cmd_buffer cmd;						/**< HS-SHA command buffer. */
		uint8_t data[MANTICORE_CRYPTO_HS_SHA_BUFFER_SIZE];	/**< Temp data buffer for hash operations. */
	} hs_sha;												/**< Shared memory usage for HS-SHA HW. */
	struct {
		struct hsp_aes_cmd_buffer cmd;						/**< AES command buffer. */
		uint8_t data[MANTICORE_CRYPTO_AES_BUFFER_SIZE];		/**< Temp data buffer for AES operations. */
	} aes;													/**< Shared memory usage for AES HW. */
	uint8_t unlock[MANTICORE_UNLOCK_HMAC_BUFFER_SIZE];		/**< Temp data buffer for unlock HMAC operations. */
};


/* Make the crypto buffers stay within the bounds defined by ROM so that ROM artifacts and the
 * tenancy counter buffer are not disturbed.
 *
 * TODO:  Enable larger crypto buffers by moving ROM artifacts to another location.  The tenancy
 * counter may also need to be moved to another area of shared SRAM to optimize memory usage. */
_Static_assert ((sizeof (struct manticore_sprt_shared_sram_crypto) ==
	sizeof (union manticore_rom_shared_sram_internal)),
	"SPRT crypto buffers are not sized correctly.");


#endif	/* MANTICORE_SPRT_H_ */
