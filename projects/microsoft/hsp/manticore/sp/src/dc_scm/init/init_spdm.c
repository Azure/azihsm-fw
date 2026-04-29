// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdint.h>
#include "init_attestation.h"
#include "init_crypto.h"
#include "init_spdm.h"
#include "init_system.h"
#include "init_tdisp.h"
#include "rot_memory_map.h"
#include "sp_boot.h"
#include "cmd_interface/cerberus_protocol.h"
#include "cmd_interface/cmd_interface_multi_handler_static.h"
#include "common/array_size.h"
#include "common/msft_device_id.h"
#include "crypto/aes_gcm_mbedtls_static.h"
#include "crypto/hash_hs_sha_soc_static.h"
#include "crypto/hkdf_static.h"
#include "pcie/tdisp_manticore_gsram_map.h"
#include "pcisig/doe/doe_interface.h"
#include "spdm/cmd_interface_protocol_spdm_vdm_static.h"
#include "spdm/spdm_certificate_chain_dice_static.h"
#include "spdm/spdm_discovery_static.h"
#include "spdm/spdm_measurements_discovery_static.h"
#include "spdm/spdm_measurements_static.h"
#include "spdm/spdm_persistent_context_manticore_gsram_static.h"
#include "spdm/spdm_persistent_context_no_secure_session_static.h"
#include "spdm/spdm_secure_session_manager_static.h"
#include "spdm/spdm_transcript_manager_static.h"

/**
 * The CT exponent value to use in SPDM capabilities messages.  This represents a power of 2
 * exponent, equaling roughly 1 second.
 *
 * This derives from the MCTP_BASE_PROTOCOL_MAX_CRYPTO_TIMEOUT_MS setting, but that value is
 * represented in milliseconds.
 */
#define	MANTICORE_SPDM_CT_EXPONENT		20


/**
 * SPDM certificate chain manager for the device DICE certificates.
 */
static const struct spdm_certificate_chain_dice spdm_dice_certs =
	spdm_certificate_chain_dice_static_init (&dice_key_manager);

/**
 * List of certificate chains available to the the SPDM responder.
 *
 * Both MCTP and DoE will use the same certificate chains.
 */
static const struct spdm_certificate_chain *spdm_cert_chains[] = {
	&spdm_dice_certs.base
};

/**
 * Variable context for MCTP SPDM transcript hash engines.
 */
static struct hash_engine_hs_sha_state mctp_spdm_transcript_hash_context[2];

/**
 * Hash engines to use with the MCTP SPDM transcript manager.
 */
static const struct hash_engine_hs_sha mctp_spdm_transcript_hash[] = {
	hash_hs_sha_static_init (&mctp_spdm_transcript_hash_context[0], &hash_hw),
	hash_hs_sha_static_init (&mctp_spdm_transcript_hash_context[1], &hash_hw),
};

static const struct hash_engine *const mctp_spdm_transcript_hash_list[] = {
	&mctp_spdm_transcript_hash[0].base, &mctp_spdm_transcript_hash[1].base
};

/**
 * Variable context for the MCTP SPDM transcript manager.
 */
static struct spdm_transcript_manager_state mctp_spdm_transcript_context;

/**
 * Manager for SPDM transcript hashes on the MCTP SPDM connection.
 */
static const struct spdm_transcript_manager mctp_spdm_transcript =
	spdm_transcript_manager_static_init (&mctp_spdm_transcript_context,
	mctp_spdm_transcript_hash_list, ARRAY_SIZE (mctp_spdm_transcript_hash_list));

/**
 * Variable context for the MCTP SPDM responder hash engine.
 *
 * [TODO] Once SPDM responder splitted into secure/non-secure implementation
 * make sure it doesn't require additional hash engine
 */
static struct hash_engine_hs_sha_state mctp_spdm_hash_context[2];

/**
 * Hash engine to use while processing MCTP SPDM requests.
 */
static const struct hash_engine_hs_sha mctp_spdm_hash[] = {
	hash_hs_sha_static_init (&mctp_spdm_hash_context[0], &hash_hw),
	hash_hs_sha_static_init (&mctp_spdm_hash_context[1], &hash_hw),
};

/**
 * List of hash engine pointer for MCTP SPDM responder
 */
static const struct hash_engine *const mctp_spdm_hash_list[] = {
	&mctp_spdm_hash[0].base, &mctp_spdm_hash[1].base
};

/**
 * Device type identifiers in SPDM measurement block format.
 */
static const struct spdm_discovery_device_id mctp_spdm_device_id =
	spdm_discovery_device_id_static_init (CERBERUS_PROTOCOL_MSFT_PCI_VID, MSFT_DEVICE_ID_MANTICORE,
	CERBERUS_PROTOCOL_MSFT_PCI_VID, MSFT_SUBSYSTEM_DEVICE_ID_DC_SCM);

/**
 * Measurements handler for MCTP SPDM requests.
 */
static const struct spdm_measurements_discovery mctp_spdm_measurements =
	spdm_measurements_discovery_static_init (&pcr_storage, &mctp_spdm_device_id);

/**
 * List of SPDM protocol versions supported by the MCTP SPDM responder.
 */
static const struct spdm_version_num_entry mctp_spdm_supported_versions[] = {
	{
		.major_version = 1,
		.minor_version = 2
	}
};

/**
 * SPDM capabilities of the MCTP SPDM responder.
 */
static const struct spdm_device_capability mctp_spdm_capabilities = {
	.ct_exponent = MANTICORE_SPDM_CT_EXPONENT,
	.flags = {
		.cache_cap = 0,
		.cert_cap = 1,
		.chal_cap = 1,
		.meas_cap = SPDM_MEASUREMENT_RSP_CAP_MEASUREMENTS_WITH_SIG,
		.meas_fresh_cap = 0,
		.encrypt_cap = 0,
		.mac_cap = 0,
		.mut_auth_cap = 0,
		.key_ex_cap = 0,
		.psk_cap = SPDM_PSK_NOT_SUPPORTED,
		.encap_cap = 0,
		.hbeat_cap = 0,
		.key_upd_cap = 0,
		.handshake_in_the_clear_cap = 0,
		.pub_key_id_cap = 0,
		.chunk_cap = 0,
		.alias_cert_cap = 1,
	},
	.data_transfer_size = SPDM_PROTOCOL_MAX_MCTP_PAYLOAD_PER_MSG,
	.max_spdm_msg_size = SPDM_PROTOCOL_MAX_MCTP_PAYLOAD_PER_MSG,
};

/**
 * Prioritized list of hash algorithms to used during SPDM algorithms negotiation.
 */
static const uint32_t mctp_spdm_hash_priority_table[] = {
	SPDM_TPM_ALG_SHA_384, SPDM_TPM_ALG_SHA_512, SPDM_TPM_ALG_SHA_256
};

/**
 * Algorithms supported by the MCTP SPDM responder.
 */
static const struct spdm_local_device_algorithms mctp_spdm_algorithms = {
	.device_algorithms = {
		.measurement_spec = SPDM_MEASUREMENTS_DMTF_MEASUREMENT_SPEC_FORMAT,
		.other_params_support = {
			.opaque_data_format = 0,
		},
		.measurement_hash_algo = SPDM_MEAS_RSP_TPM_ALG_SHA_384,
		.base_asym_algo = SPDM_TPM_ALG_ECDSA_ECC_NIST_P384,
		.base_hash_algo = SPDM_TPM_ALG_SHA_256 | SPDM_TPM_ALG_SHA_384 | SPDM_TPM_ALG_SHA_512,
		.dhe_named_group = 0,
		.aead_cipher_suite = 0,
		.req_base_asym_alg = 0,
		.key_schedule = 0
	},
	.algorithms_priority_table = {
		.hash_priority_table = mctp_spdm_hash_priority_table,
		.hash_priority_table_count = ARRAY_SIZE (mctp_spdm_hash_priority_table),
	},
};

/**
 * Variable context for the MCTP SPDM responder persistent context.
 */
struct spdm_persistent_context_no_secure_session_state mctp_spdm_context_state;

/**
 * Persistent context for the MCTP SPDM responder.
 */
const struct spdm_persistent_context_no_secure_session mctp_spdm_persistent_context =
	spdm_persistent_context_no_secure_session_static_init (&mctp_spdm_context_state);

/**
 * Handler for SPDM requests received over MCTP.
 */
const struct cmd_interface_spdm_responder mctp_spdm_handler =
	cmd_interface_spdm_responder_static_init (&mctp_spdm_transcript, mctp_spdm_hash_list,
	ARRAY_SIZE (mctp_spdm_hash_list), mctp_spdm_supported_versions,
	ARRAY_SIZE (mctp_spdm_supported_versions), NULL, 0, &mctp_spdm_capabilities,
	&mctp_spdm_algorithms, spdm_cert_chains, ARRAY_SIZE (spdm_cert_chains),
	&mctp_spdm_measurements.base, &shared_ecc.base, &shared_rng.base, NULL, NULL,
	&mctp_spdm_persistent_context.base);


/**
 * Initialize the SPDM responder used for processing I2C MCTP SPDM requests.
 *
 * @return 0 if the SPDM responder was successfully initialized or an error code.
 */
int initialize_mctp_spdm_responder ()
{
	size_t i;
	int status;

	/* Initialize and self-test each hash engine instance being used for SPDM.  MCTP does not
	 * support secure sessions, so these only needs to be tested for SHA. */
	for (i = 0; i < ARRAY_SIZE (mctp_spdm_transcript_hash); i++) {
		status = hash_hs_sha_init_state (&mctp_spdm_transcript_hash[i]);
		if (status != 0) {
			return status;
		}
	}

	for (i = 0; i < ARRAY_SIZE (mctp_spdm_hash); i++) {
		status = hash_hs_sha_init_state (&mctp_spdm_hash[i]);
		if (status != 0) {
			return status;
		}
	}

	status = spdm_transcript_manager_init_state (&mctp_spdm_transcript);
	if (status != 0) {
		return status;
	}

	status = cmd_interface_spdm_responder_init_state (&mctp_spdm_handler);
	if (status != 0) {
		return status;
	}

	status = cmd_interface_spdm_responder_init_persistent_state (&mctp_spdm_handler);
	if (status != 0) {
		return status;
	}

	return 0;
}

/**
 * Hash engines to use with the DOE SPDM transcript manager.
 */
static const struct hash_engine_hs_sha_soc doe_spdm_transcript_hash[] = {
	hash_hs_sha_soc_static_init (SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS0, &dmb, &hash_hw),
	hash_hs_sha_soc_static_init (SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS1, &dmb, &hash_hw),
	hash_hs_sha_soc_static_init (SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS2, &dmb, &hash_hw),
	hash_hs_sha_soc_static_init (SPDM_TRANSCRIPT_HASH_STATE_GSRAM_ADDRESS3, &dmb, &hash_hw),
};

/**
 * Hash engines list to be used DOE SPDM transcript manager initialization
 */
static const struct hash_engine *const doe_spdm_transcript_hash_list[] = {
	&doe_spdm_transcript_hash[0].base, &doe_spdm_transcript_hash[1].base,
	&doe_spdm_transcript_hash[2].base, &doe_spdm_transcript_hash[3].base,
};

/**
 * Variable context for the DOE SPDM transcript manager.
 */
static struct spdm_transcript_manager_state doe_spdm_transcript_state;

/**
 * Manager for SPDM transcript hashes on the DOE SPDM responder.
 */
static const struct spdm_transcript_manager doe_spdm_transcript =
	spdm_transcript_manager_static_init (&doe_spdm_transcript_state, doe_spdm_transcript_hash_list,
	ARRAY_SIZE (doe_spdm_transcript_hash_list));

/**
 * Variable context for the DOE SPDM responder hash engine.
 */
static struct hash_engine_hs_sha_state doe_spdm_hash_state[2];

/**
 * Hash engine to use while processing DOE SPDM requests.
 */
static const struct hash_engine_hs_sha doe_spdm_hash[] = {
	hash_hs_sha_static_init (&doe_spdm_hash_state[0], &hash_hw),
	hash_hs_sha_static_init (&doe_spdm_hash_state[1], &hash_hw),
};

/**
 * List of hash engine pointer for DOE SPDM responder
 */
static const struct hash_engine *const doe_spdm_hash_list[] = {
	&doe_spdm_hash[0].base, &doe_spdm_hash[1].base,
};

/**
 * Measurements handler for DOE SPDM requests.
 */
static const struct spdm_measurements doe_spdm_measurements =
	spdm_measurements_static_init (&pcr_storage);

/**
 * List of DOE SPDM supported versions
 */
static const struct spdm_version_num_entry doe_spdm_version_list[] = {
	{.major_version = 1, .minor_version = 2, .alpha = 0, .update_version = 0, },
};

/**
 * List of DOE Secure SPDM supported versions
 */
static const struct spdm_version_num_entry doe_secure_spdm_version_list[] = {
	{.major_version = 1, .minor_version = 1, .alpha = 0, .update_version = 0, },
};

/**
 * SPDM capabilities of the DOE SPDM responder.
 */
static const struct spdm_device_capability doe_spdm_capabilities = {
	.ct_exponent = MANTICORE_SPDM_CT_EXPONENT,
	.flags = {
		.cache_cap = 0,
		.cert_cap = 1,
		.chal_cap = 1,
		.meas_cap = SPDM_MEASUREMENT_RSP_CAP_MEASUREMENTS_WITH_SIG,
		.meas_fresh_cap = 0,
		.encrypt_cap = 1,
		.mac_cap = 1,
		.mut_auth_cap = 0,
		.key_ex_cap = 1,
		.psk_cap = SPDM_PSK_NOT_SUPPORTED,
		.encap_cap = 0,
		.hbeat_cap = 0,
		.key_upd_cap = 0,
		.handshake_in_the_clear_cap = 0,
		.pub_key_id_cap = 0,
		.chunk_cap = 0,
		.alias_cert_cap = 1,
		.reserved = 0,
		.reserved2 = 0,
	},
	.data_transfer_size = DOE_MESSAGE_MAX_PAYLOAD_SIZE_IN_BYTES,
	.max_spdm_msg_size = DOE_MESSAGE_MAX_PAYLOAD_SIZE_IN_BYTES,
};

/**
 * DOE SPDM algorithms
 * */
static const struct spdm_local_device_algorithms doe_spdm_algorithms = {
	.device_algorithms = {
		.measurement_spec = SPDM_MEASUREMENT_SPEC_DMTF,
		.measurement_hash_algo = SPDM_MEAS_RSP_TPM_ALG_SHA_384,
		.base_asym_algo = SPDM_TPM_ALG_ECDSA_ECC_NIST_P384,
		.base_hash_algo = SPDM_TPM_ALG_SHA_256 | SPDM_TPM_ALG_SHA_384 | SPDM_TPM_ALG_SHA_512,
		.req_base_asym_alg = 0,
		.aead_cipher_suite = SPDM_ALG_AEAD_CIPHER_SUITE_AES_256_GCM,
		.dhe_named_group = SPDM_ALG_DHE_NAMED_GROUP_SECP_384_R1,
		.key_schedule = SPDM_ALG_KEY_SCHEDULE_HMAC_HASH,
		.other_params_support = {
			.opaque_data_format = SPDM_ALGORITHMS_OPAQUE_DATA_FORMAT_1,
		},
	},

	.algorithms_priority_table = {
		.aead_priority_table = NULL,
		.aead_priority_table_count = 0,
		.asym_priority_table = NULL,
		.asym_priority_table_count = 0,
		.dhe_priority_table = NULL,
		.dhe_priority_table_count = 0,
		.hash_priority_table = mctp_spdm_hash_priority_table,
		.hash_priority_table_count = ARRAY_SIZE (mctp_spdm_hash_priority_table),
		.key_schedule_priority_table = NULL,
		.key_schedule_priority_table_count = 0,
		.measurement_spec_priority_table = NULL,
		.measurement_spec_priority_table_count = 0,
		.other_params_support_priority_table = NULL,
		.other_params_support_priority_table_count = 0,
		.req_asym_priority_table = NULL,
		.req_asym_priority_table_count = 0,
	},
};

/**
 * Hash engine state for DOE SPDM secure session manager
 */
static struct hash_engine_hs_sha_state doe_spdm_secure_session_manager_hash_state;

/**
 * Has engine for DOE SPDM secure session manager
 */
static const struct hash_engine_hs_sha doe_spdm_secure_session_manager_hash =
	hash_hs_sha_static_init (&doe_spdm_secure_session_manager_hash_state, &hash_hw);

/**
 * Variable context for DOE SPDM HKDF.
 */
static struct hkdf_state doe_spdm_hkdf_context;

/**
 * HKDF handler for deriving secure session keys with DOE SPDM.
 */
const struct hkdf doe_spdm_hkdf = hkdf_static_init (&doe_spdm_hkdf_context,
	&doe_spdm_secure_session_manager_hash.base);

/**
 * AES engine state for DOE SPDM secure session manager
 */
static struct aes_gcm_engine_mbedtls_state doe_spdm_secure_session_aes_state;

/**
 * AES encryption engine for DOE SPDM secure session manager
 */
static const struct aes_gcm_engine_mbedtls doe_spdm_secure_session_manager_aes =
	aes_gcm_mbedtls_static_init (&doe_spdm_secure_session_aes_state);

/**
 * DOE SPDM secure session manager state
 */
static struct spdm_secure_session_manager_state doe_spdm_secure_session_manager_state;


/**
 * Algorithms metadata used by SPDM secure session manager
 */
#define SPDM_SECURE_SESSION_MANAGER_ALGO_INFO \
{ \
	.ecdh_instance_id = 0, \
}

/**
 * Variable context for the DOE SPDM persistent context.
 */
struct spdm_persistent_context_manticore_gsram_state doe_spdm_context_gsram_state;

/**
 * SPDM Persistent context for the DOE SPDM responder.
 */
static const struct spdm_persistent_context_manticore_gsram doe_spdm_persistent_context_gsram =
	spdm_persistent_context_manticore_gsram_static_init (&doe_spdm_context_gsram_state, &dmb,
	SPDM_PERSISTENT_CONTEXT_GSRAM_ADDRESS);

/**
 * DOE SPDM secure session manager
 */
static const struct spdm_secure_session_manager doe_spdm_secure_session_manager =
	spdm_secure_session_manager_static_init (&doe_spdm_secure_session_manager_state,
	&doe_spdm_capabilities, &doe_spdm_algorithms.device_algorithms,
	&doe_spdm_secure_session_manager_aes.base, &doe_spdm_secure_session_manager_hash.base,
	&shared_rng.base, &shared_ecc.base, &doe_spdm_transcript, &doe_spdm_hkdf.base,
	&error_state_handler.base_error_task, SPDM_SECURE_SESSION_MANAGER_ALGO_INFO,
	&doe_spdm_persistent_context_gsram.base);

/**
 * Impactful check based on SPDM policy.
 */
const struct impactful_check_spdm spdm_impactful =
	impactful_check_spdm_static_init (&doe_spdm_secure_session_manager);

/**
 * SPDM VDM protocol instance
 */
static const struct cmd_interface_protocol_spdm_vdm doe_spdm_vdm_protocol =
	cmd_interface_protocol_spdm_vdm_static_init ();

/**
 * SPDM VDM message handlers
 */
static const struct cmd_interface_multi_handler_msg_type spdm_vdm_handlers[] = {
	cmd_interface_multi_handler_msg_type_static_init (SPDM_REGISTRY_ID_PCISIG,
		&spdm_pcisig_handler.base),
};

/**
 * Multi handler instance for SPDM VDM
 */
static const struct cmd_interface_multi_handler spdm_vdm_handler =
	cmd_interface_multi_handler_static_init (&doe_spdm_vdm_protocol.base, spdm_vdm_handlers,
	ARRAY_SIZE (spdm_vdm_handlers));

/**
 * Handler for SPDM requests received over DOE interface
 */
const struct cmd_interface_spdm_responder doe_spdm_handler =
	cmd_interface_spdm_responder_static_init (&doe_spdm_transcript, doe_spdm_hash_list,
	ARRAY_SIZE (doe_spdm_hash_list), doe_spdm_version_list, ARRAY_SIZE (doe_spdm_version_list),
	doe_secure_spdm_version_list, ARRAY_SIZE (doe_secure_spdm_version_list), &doe_spdm_capabilities,
	&doe_spdm_algorithms, spdm_cert_chains, ARRAY_SIZE (spdm_cert_chains), &doe_spdm_measurements,
	&shared_ecc.base, &shared_rng.base, &doe_spdm_secure_session_manager, &spdm_vdm_handler.base,
	&doe_spdm_persistent_context_gsram.base);


/**
 * Initialize the SPDM responder used for processing PCIe DOE SPDM requests.
 *
 * @return 0 if the SPDM responder was successfully initialized or an error code.
 */
int initialize_doe_spdm_responder ()
{
	size_t i;
	int status;

	if (is_por ()) {
		for (i = 0; i < ARRAY_SIZE (doe_spdm_transcript_hash); i++) {
			status = hash_hs_sha_soc_init_state (&doe_spdm_transcript_hash[i]);
			if (status != 0) {
				return status;
			}
		}
	}

	for (i = 0; i < ARRAY_SIZE (doe_spdm_hash); i++) {
		status = hash_hs_sha_init_state (&doe_spdm_hash[i]);
		if (status != 0) {
			return status;
		}
	}

	status = spdm_transcript_manager_init_state (&doe_spdm_transcript);
	if (status != 0) {
		return status;
	}

	status = hash_hs_sha_init_state (&doe_spdm_secure_session_manager_hash);
	if (status != 0) {
		return status;
	}

	status = hkdf_init_state (&doe_spdm_hkdf);
	if (status != 0) {
		return status;
	}

	status = aes_gcm_mbedtls_init_state (&doe_spdm_secure_session_manager_aes);
	if (status != 0) {
		return status;
	}

	status =
		spdm_persistent_context_manticore_gsram_init_state (&doe_spdm_persistent_context_gsram);
	if (status != 0) {
		return status;
	}

	status = spdm_secure_session_manager_init_state (&doe_spdm_secure_session_manager);
	if (status != 0) {
		return status;
	}

	if (is_por ()) {
		/* Initialize persistent state for secure session manager on POR */
		status =
			spdm_secure_session_manager_init_persistent_state (&doe_spdm_secure_session_manager);
		if (status != 0) {
			return status;
		}
	}

	status =
		spdm_secure_session_manager_add_spdm_protocol_session_observer (
		&doe_spdm_secure_session_manager, &tdisp_event_policy.spdm_observer);
	if (status != 0) {
		return status;
	}

	status = cmd_interface_spdm_responder_init_state (&doe_spdm_handler);
	if (status != 0) {
		return status;
	}

	if (is_por ()) {
		status = cmd_interface_spdm_responder_init_persistent_state (&doe_spdm_handler);
		if (status != 0) {
			return status;
		}
	}

	return 0;
}
