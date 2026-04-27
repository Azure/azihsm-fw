// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CCS_KSU_H_
#define CCS_KSU_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "platform_api.h"
#include "crypto/ecc_hw_pka.h"
#include "drivers/ccs_ksu_interface.h"
#include "drivers/hs_sha.h"
#include "drivers/hsp_aes.h"
#include "drivers/hsp_rng_hw.h"
#include "splibs/inc/spcryptotypes.h"
#include "status/msft_module_id.h"
#include "trap/hsp_interrupt_handler.h"


/* Defined in HSP register definition. */
struct Ccs_regs;
struct Creg_regs_creg_crypto_group;

/**
 * The command structure to use for issuing CCS commands.  This structure must be stored in HSP
 * shared RAM for CCS to be able to access it and should be allocated at a 32-bit aligned address.
 */
struct ccs_command {
	uint32_t command_code;		/**< CCS command to execute. */
	uint32_t address[4];		/**< Address arguments for the command. */
	union {
		uint32_t address4;		/**< Extra address argument. */
		uint32_t attributes;	/**< Key attributes. */
	};
};

/**
 * Memory structure in HSP shared SRAM that is required to be able to execute CCS commands.  This
 * should be 32-bit aligned.
 */
struct ccs_cmd_buffer {
	struct ccs_command cmd;												/**< The command and arguments passed to the CCS. */
	union {
		SP_MSG_384 plain_key;											/**< The input is a plaintext key. */
		SP_MSG_512 wrapped_key;											/**< The input is a wrapped key. */
		struct {
			SP_ECDSA_P384_PUBLIC key;									/**< The peer public key. */
			SP_MSG_384 hash;											/**< The input is Hash */
		} key_exchange_data;											/**< The input is a key exchange data structure. */
		uint8_t bytes[SP_ECDSA_P384_PUBLIC_KEY_SIZE + SP_MSG_384_SIZE];	/**< Access to the raw bytes for the input. */
	} input;															/**< Input data to the CCS command. */
	union {
		SP_MSG_384 hmac;												/**< The output is an HMAC. */
		SP_MSG_384 plain_key;											/**< The output is a plaintext key. */
		SP_MSG_512 wrapped_key;											/**< The output is a wrapped key. */
		SP_ECDSA_P384_PUBLIC public_key;								/**< The output is a public key. */
		SP_ECDSA_P384_SIGNATURE signature;								/**< The output is an ECDSA signature. */
		struct {
			SP_ECDSA_P384_PUBLIC key;									/**< The certified key. */
			SP_ECDSA_P384_SIGNATURE sig;								/**< Signature over the public key, PCR, and attributes. */
		} certified_key;												/**< The output is a certified ECC public key. */
	} output;															/**< Output data from the CCS command. */
};


/**
 * Variable context for the CCS/KSU driver instance.
 */
struct ccs_ksu_state {
	platform_mutex lock;		/**< Lock for synchronization. */
	platform_semaphore done;	/**< Semaphore indicating when a HW operation has completed. */
};

/**
 * Implementation of the CCS/KSU driver interface to execute commands against hardware blocks.
 * Depending on how it's configured, it can support both SHACK1 and SHACK2 hardware.
 */
struct ccs_ksu {
	struct ccs_ksu_interface base;				/**< Base driver interface for CCS commands. */
	struct hsp_interrupt_handler base_irq;		/**< Base interface for handling HW interrupts. */
	struct ccs_ksu_state *state;				/**< Variable context for the driver. */
	struct Ccs_regs *regs;						/**< Register interface to the CCS. */
	struct Creg_regs_creg_crypto_group *irq;	/**< Register interface to CCS interrupts. */
	const struct hs_sha *sha;					/**< Interface to the HS-SHA used by CCS. */
	const struct hsp_aes *aes;					/**< Interface to the AES used by CCS. */
	const struct ecc_hw_pka *pka;				/**< Interface to the PKA used by CCS. */
	const struct hsp_rng_hw *rng;				/**< Interface to the RNG used by CCS. */
	struct ccs_cmd_buffer *buffer;				/**< Memory location for the CCS command structure. */
	const struct ksu_key_slot *keys;			/**< Memory location where KSU keys are stored. */
	size_t key_slots;							/**< The number of key slots supported by the KSU. */
	const struct ksu_pcr_slot *pcrs;			/**< Memory location for the KSU PCR storage. */
	size_t pcr_slots;							/**< The number of PCR slots supported by the KSU. */

	/**
	 * Submit a command to the hardware for execution and block until the hardware has completed the
	 * request.
	 *
	 * @param ccs The CCS executing the command.
	 * @param error_code Error to return when the the command failure bit is set.
	 *
	 * @returns 0 if the command completed successfully or an error code.
	 */
	int (*submit_command) (const struct ccs_ksu *ccs, int error_code);
};


int ccs_ksu_init_polling (struct ccs_ksu *ccs, struct ccs_ksu_state *state, struct Ccs_regs *regs,
	const struct hs_sha *sha, const struct hsp_aes *aes, const struct ecc_hw_pka *pka,
	const struct hsp_rng_hw *rng, struct ccs_cmd_buffer *cmd_buffer,
	const struct ksu_key_slot *keys, size_t num_keys, const struct ksu_pcr_slot *pcrs,
	size_t num_pcrs);
int ccs_ksu_init_interrupt (struct ccs_ksu *ccs, struct ccs_ksu_state *state, struct Ccs_regs *regs,
	struct Creg_regs_creg_crypto_group *irq_regs, const struct hs_sha *sha,
	const struct hsp_aes *aes, const struct ecc_hw_pka *pka, const struct hsp_rng_hw *rng,
	struct ccs_cmd_buffer *cmd_buffer, const struct ksu_key_slot *keys, size_t num_keys,
	const struct ksu_pcr_slot *pcrs, size_t num_pcrs);
int ccs_ksu_init_state (const struct ccs_ksu *ccs);
void ccs_ksu_release (const struct ccs_ksu *ccs);

/* Internal functions for use by derived types. */
bool ccs_ksu_get_pcr_slot_address (const struct ccs_ksu *ccs, uint8_t pcr_slot, uint32_t *address);
int ccs_ksu_execute_command (const struct ccs_ksu *ccs, struct ccs_command *cmd, size_t input_len,
	uint8_t *output, size_t output_len, int error_code, uint8_t hw_flags);

int ccs_ksu_is_key_slot_valid (const struct ccs_ksu_interface *ccs, uint8_t key_slot);
int ccs_ksu_get_key_attributes (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t *key_attributes);
int ccs_ksu_set_key (const struct ccs_ksu_interface *ccs, const SP_MSG_384 *key, uint8_t key_slot,
	uint32_t key_attributes);
#ifdef CCS_KSU_ENABLE_SEND_KEY
int ccs_ksu_send_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot, uint32_t dest_addr);
#endif
int ccs_ksu_generate_random_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	uint32_t key_attributes);
int ccs_ksu_export_fw_ecc_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	SP_MSG_384 *key, uint32_t *key_attributes);
int ccs_ksu_get_ecc_public_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot,
	SP_ECDSA_P384_PUBLIC *public_key, uint32_t *key_attributes);
int ccs_ksu_ecdh_key_exchange (const struct ccs_ksu_interface *ccs, uint8_t key_in,	uint8_t key_out,
	const uint8_t *partner_public_key_and_hash, size_t input_len, uint32_t key_attributes);
int ccs_ksu_ecc_sign (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const SP_MSG_384 *digest, const struct rng_engine *rng, SP_ECDSA_P384_SIGNATURE *signature,
	uint32_t *key_attributes);
int ccs_ksu_ecdsa_sign_message (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const uint8_t *message, size_t length, const struct hash_engine *hash, enum hash_type hash_algo,
	SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes);
int ccs_ksu_ecdsa_sign_hash (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const struct hash_engine *hash, SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes);
int ccs_ksu_ecdsa_sign_hash_and_finish (const struct ccs_ksu_interface *ccs, uint8_t signing_key,
	const struct hash_engine *hash, SP_ECDSA_P384_SIGNATURE *signature, uint32_t *key_attributes);
int ccs_ksu_burn_key (const struct ccs_ksu_interface *ccs, uint8_t key_slot);

int ccs_ksu_submit_command_interrupt (const struct ccs_ksu *ccs, int error_code);
int ccs_ksu_submit_command_polling (const struct ccs_ksu *ccs, int error_code);

bool ccs_ksu_handle_interrupt (const struct hsp_interrupt_handler *handler, uintptr_t param);


/* Internal definitions for use by derived types. */

/**
 * Identifiers to use to execute CCS commands.
 *
 * Refer to the CCS documentation for details about each command.
 */
enum ccs_cmd_code {
	CCS_CMD_CODE_SET_KEY = 0x60000000,					/**< SetKey */
	CCS_CMD_CODE_GEN_RANDOM_KEY = 0x60000001,			/**< GenRandomKey */
	CCS_CMD_CODE_SEND_KEY = 0x60000002,					/**< SendKey */
	CCS_CMD_CODE_LOAD_KEY = 0x60000003,					/**< LoadKey */
	CCS_CMD_CODE_DECRYPT_LEGACY_KEY = 0x60000004,		/**< DecryptLegacyKey */
	CCS_CMD_CODE_STORE_KEY = 0x60000005,				/**< StoreKey */
	CCS_CMD_CODE_SAVE_KEY = 0x60000006,					/**< SaveKey */
	CCS_CMD_CODE_KDF_KEY = 0x60000007,					/**< KDFKey (deprecated) */
	CCS_CMD_CODE_KDF_PCR = 0x60000008,					/**< KDFPCR (deprecated) */
	CCS_CMD_CODE_KDF_AS_PCR = 0x60000009,				/**< KDFAsPCR (deprecated) */
	CCS_CMD_CODE_DERIVE_ECC_PUBLIC = 0x6000000a,		/**< DerivecECCPublic */
	CCS_CMD_CODE_ECC_PCR_SIGN = 0x6000000b,				/**< ECCPCRSign */
	CCS_CMD_CODE_ECC_SIGN = 0x6000000c,					/**< ECCSign */
	CCS_CMD_CODE_ECDH_PCR_KEY_EXCHANGE = 0x6000000d,	/**< ECDHPCRKeyExchange (deprecated) */
	CCS_CMD_CODE_ECDH_KEY_EXCHANGE = 0x6000000e,		/**< ECDHKeyExchange (deprecated) */
	CCS_CMD_CODE_EXTEND_PCR = 0x6000000f,				/**< ExtendPCR */
	CCS_CMD_CODE_BURN_KEY = 0x60000010,					/**< BurnKey */
	CCS_CMD_CODE_GEN_RANDOM_ECC_KEY = 0x60000011,		/**< GenRandomECCKey */
	CCS_CMD_CODE_UNWRAP_KEY = 0x60000012,				/**< UnwrapKey */
	CCS_CMD_CODE_WRAP_INPUT = 0x60000013,				/**< WrapInput */
	CCS_CMD_CODE_WRAP_KEY = 0x60000014,					/**< WrapKey */
	CCS_CMD_CODE_KDF_KEY2 = 0x60000015,					/**< KDFKey2 */
	CCS_CMD_CODE_KDF_PCR2 = 0x60000016,					/**< KDFPCR2 */
	CCS_CMD_CODE_KDF_AS_PCR2 = 0x60000017,				/**< KDFAsPCR2 */
	CCS_CMD_CODE_ECDH_PCR_KEY_EXCHANGE2 = 0x60000018,	/**< ECDHPCRKeyExchange2 */
	CCS_CMD_CODE_ECDH_KEY_EXCHANGE2 = 0x60000019,		/**< ECDHKeyExchange2 */
	CCS_CMD_CODE_EXTEND_PCR_384 = 0x6000001a,			/**< ExtendPCR384 */
	CCS_CMD_CODE_REINIT_PCR = 0x6000001b,				/**< ReInitPCR */
	CCS_CMD_CODE_CERTIFY_ECC_PUBLIC_KEY = 0x6000001c,	/**< CertifyECCPublicKey */
	CCS_CMD_CODE_KDF_TWO_KEYS = 0x6000001d,				/**< KDFTwoKeys */
	CCS_CMD_CODE_HMAC = 0x6000001e,						/**< HMAC */
	CCS_CMD_CODE_DERIVE_ECC_KEY = 0x6000001f,			/**< DeriveECCKey */
};

/**
 * Flags that indicate how a specific CCS command uses external hardware blocks.
 */
enum ccs_ksu_cmd_hw {
	CCS_KSU_CMD_HW_NONE = 0,					/**< The CCS command does not access any external HW. */
	CCS_KSU_CMD_HW_LOCK_CCS = (1U << 0),		/**< Take the CCS HW lock as part of command execution. */
	CCS_KSU_CMD_HW_RNG_NORMAL_MODE = (1U << 1),	/**< The CCS command accesses the RNG in normal mode. */
	CCS_KSU_CMD_HW_RNG_FW_MODE = (1U << 2),		/**< The CCS command accesses the RNG in FW mode. */
	CCS_KSU_CMD_HW_SHA = (1U << 3),				/**< The CCS command accesses the HS-SHA. */
	CCS_KSU_CMD_HW_AES = (1U << 4),				/**< The CCS command accesses the AES. */
	CCS_KSU_CMD_HW_PKA = (1U << 5),				/**< The CCS command accesses the PKA. */

	/**
	 * This is not a flag to set but an easy way to check if a command uses the RNG HW.
	 */
	CCS_KSU_CMD_HW_RNG = (CCS_KSU_CMD_HW_RNG_NORMAL_MODE | CCS_KSU_CMD_HW_RNG_FW_MODE),
};

/* Constants to use when identifying KSU key/PCR slots during command execution. */
#define	CCS_KSU_KEY_SLOT			0x8eff1000
#define	CCS_KSU_PCR_SLOT			0x8eff2000

/**
 * Flag to set during command executing indicating input data doesn't need to be reversed.
 */
#define	CCS_KSU_INPUT_REVERSE		(1U << 31)

/**
 * Flag to indicate input data should not be processed in any way.  KSU and PCR slots will still be
 * interpreted.
 */
#define	CCS_KSU_INPUT_NO_PROCESSING	(1U << 30)

/**
 * Determine if a key should be reversed as part of a CCS operation.  ECC keys need to be reversed
 * to be compatible with the PKA interface.
 */
#define	ccs_ksu_reverse_key(attr)   \
	((attr & (CCS_KSU_ATTR_ECC_SIGN_ALLOWED | CCS_KSU_ATTR_ECDH_ALLOWED)) ? \
		CCS_KSU_INPUT_REVERSE : 0)


#endif	/* CCS_KSU_H_ */
