// Copyright (c) Microsoft Corporation. All rights reserved.

#include "hsp_top.h"
#include "init_crypto.h"
#include "init_ephemeral_key.h"
#include "init_flash.h"
#include "init_system.h"
#include "periodic_task_freertos_static.h"
#include "rot_memory_map.h"
#include "task_log_id.h"
#include "task_priority.h"
#include "task_stack_size.h"
#include "common/array_size.h"
#include "crypto/aes_ecb_hsp_static.h"
#include "crypto/aes_key_wrap_with_padding_static.h"
#include "crypto/ephemeral_key_generation_rsa_static.h"
#include "drivers/ccs_ksu_interface.h"
#include "firmware/manticore_device_keys.h"
#include "flash/flash_store_contiguous_blocks_key_wrap_static.h"
#include "keystore/ephemeral_key_manager_static.h"
#include "keystore/ephemeral_key_monitor.h"
#include "keystore/key_cache_flash_static.h"
#include "splibs/inc/spcryptotypes.h"



/**
 * Maximum number of requestors for RSA key cache
 */
#define RSA_KEY_CACHE_FLASH_MAX_REQUESTOR			65

/**
 * Maximum number of credits per requestor for RSA key cache
 */
#define RSA_KEY_CACHE_FLASH_MAX_CREDIT				2

/**
 * Ephemeral key manager task execution interval in ms while the key cache is full
 */
#define EPHEMERAL_KEY_MANAGER_TASK_INTERVAL_MS		10000

/**
 * Ephemeral key manager delay in ms before the next execution while the key cache is not full.  A
 * short, non-zero delay (instead of rescheduling immediately) leaves an idle window between key
 * generations for other operations to run.
 */
#define EPHEMERAL_KEY_MANAGER_KEY_GEN_DELAY_MS		1000

/**
 * Supported RSA Ephemeral key bits length
 */
#define RSA_EPHEMERAL_KEY_SIZE						2048

/**
 * @brief The maximum size of a 2K RSA key in DER format.
 */
#define RSA_2K_DER_KEY_MAX_SIZE						1270

/**
 * RSA Ephemeral Key Generation
 */
static const struct ephemeral_key_generation_rsa rsa_key_gen =
	ephemeral_key_generation_rsa_static_init (&shared_rsa.base);

/**
 * Context for the KDF to derive the AES wrapping key for stored RSA key pairs.  This is the SHA-384
 * hash of "RSAKeyWrap".
 */
static const SP_MSG_384 KEY_STORE_KDF_CONTEXT = {
	.AsBytes = {
		0xe3, 0x24, 0x00, 0xbf, 0x1c, 0x66, 0x65, 0xb3,
		0xc3, 0xdf, 0x73, 0x02, 0x18, 0x9d, 0x6e, 0xeb,
		0xfe, 0x92, 0x39, 0x9b, 0x88, 0x30, 0x5d, 0x3f,
		0x2c, 0xd8, 0x13, 0xad, 0x29, 0xe4, 0x61, 0x9c,
		0x85, 0xa7, 0xae, 0xb5, 0x82, 0xbc, 0xee, 0x99,
		0xbc, 0x6c, 0x1a, 0xb3, 0x40, 0x86, 0xb1, 0xec
	}
};

/**
 * AES-ECB interface encrypting and decrypting pre-generated RSA key pairs.
 */
static const struct aes_ecb_engine_hsp rsa_ecb =
	aes_ecb_hsp_static_init (&aes_hw, &ccs.base, MANTICORE_DEVICE_KEYS_RSA_KEY_FLASH_KEY);

/**
 * AES key wrap handler to use with stored RSA key pairs.
 */
static const struct aes_key_wrap_with_padding rsa_kwp =
	aes_key_wrap_with_padding_static_init (&rsa_ecb.base);

/**
 * Flash store state for RSA key cache
 */
static struct flash_store_contiguous_blocks_state key_cache_flash_store_state;

/**
 * Flash store used for RSA key cache
 */
static const struct flash_store_contiguous_blocks_key_wrap key_cache_flash_store =
	flash_store_contiguous_blocks_key_wrap_static_init_variable_storage (
	&key_cache_flash_store_state, &flash_external.base, RSA_KEY_CACHE_FLASH_ADDR,
	RSA_KEY_CACHE_FLASH_SECTORS, &rsa_kwp.base.base);

/**
 * Key cache flash key information
 */
static struct key_cache_flash_key_info key_cache_flash_key_info[RSA_KEY_CACHE_FLASH_SECTORS];

/**
 * Array storing the available credits for each requester.
 */
static uint8_t key_cache_flash_requestor_credit[RSA_KEY_CACHE_FLASH_MAX_REQUESTOR];

/**
 * Key cache flash state
 */
static struct key_cache_flash_state rsa_key_cache_flash_state;

/**
 * Working buffer for the producer (`ephemeral_key_manager`) to receive a freshly generated
 * DER-encoded RSA-2K private key from `mbedtls`.  Sized via the wrapped-length macro so the
 * buffer has room for the DER blob plus any AES-key-wrap overhead the producer reuses it for.
 * The DER blob is persisted as-is; the runtime DER -> PKA little-endian conversion happens later
 * in the `ephemeral_key_monitor` when a slot is refilled.
 */
static uint8_t key_buffer[AES_KEY_WRAP_INTERFACE_WRAPPED_LENGTH (RSA_2K_DER_KEY_MAX_SIZE)];

/**
 * Flash-backed key cache.  Exposed (non-static) so the ephemeral key monitor can refill GSRAM
 * slots directly without going through the producer.
 */
const struct key_cache_flash rsa_key_cache_flash =
	key_cache_flash_static_init (&rsa_key_cache_flash_state, &key_cache_flash_store.base.base,
	key_cache_flash_key_info, ARRAY_SIZE (key_cache_flash_key_info),
	key_cache_flash_requestor_credit, ARRAY_SIZE (key_cache_flash_requestor_credit),
	RSA_KEY_CACHE_FLASH_MAX_CREDIT);

/**
 * Ephemeral key manager state
 */
static struct ephemeral_key_manager_state rsa_ephemeral_key_manager_state;

/**
 *  RSA Ephemeral Key Generation
 */
const struct ephemeral_key_manager rsa_ephemeral_key_manager =
	ephemeral_key_manager_static_init (&rsa_ephemeral_key_manager_state, &rsa_key_cache_flash.base,
	&rsa_key_gen.base, EPHEMERAL_KEY_MANAGER_TASK_INTERVAL_MS,
	EPHEMERAL_KEY_MANAGER_KEY_GEN_DELAY_MS, RSA_EPHEMERAL_KEY_SIZE, key_buffer,
	ARRAY_SIZE (key_buffer));

/**
 * Periodic task state for RSA ephemeral key generation
 */
static struct periodic_task_freertos_state rsa_ephemeral_key_manager_task_context;

/**
 * Periodic task handlers array for Ephemeral Key Generation
 */
static const struct periodic_task_handler *const ephemeral_key_manager_handlers[] = {
	&rsa_ephemeral_key_manager.base,
};

/**
 * Task to generate RSA ephemeral keys
 */
static const struct periodic_task_freertos ephemeral_key_manager_task =
	periodic_task_freertos_static_init (&rsa_ephemeral_key_manager_task_context,
	ephemeral_key_manager_handlers, ARRAY_SIZE (ephemeral_key_manager_handlers),
	EPHEMERAL_KEY_GEN_TASK_LOG_ID);

/**
 * Statically allocated task control block for the IPC handler task from the Admin core.
 */
static StaticTask_t ephemeral_key_manager_task_tcb;

/**
 * Statically allocated stack for the IPC handler task from the Admin core
 */
static StackType_t ephemeral_key_manager_task_stack[EPHEMERAL_KEY_MANAGER_TASK_STACK_WORDS];


/**
 * Initialize Ephemeral key generation infrastructure.
 *
 * @return 0 if successful, otherwise error code.
 */
int initialize_ephemeral_key_handler ()
{
	int status;

	/* Derive the KEK for key wrapping.  It will be present in the KSU and does not need to be set
	 * using the key wrap interface. */
	status = ccs_ksu_derive_key (&ccs.base, MANTICORE_DEVICE_KEYS_SPRT_DEVICE_KEY,
		&KEY_STORE_KDF_CONTEXT, MANTICORE_DEVICE_KEYS_RSA_KEY_FLASH_KEY,
		CCS_KSU_ATTR_AES_ENCRYPT_ALLOWED | CCS_KSU_ATTR_AES_DECRYPT_ALLOWED);
	if (status != 0) {
		return status;
	}

	/* Initialize encrypted contiguous flash store states.  The cache persists DER-encoded keys, so
	 * the store is sized to the maximum DER blob length. */
	status = flash_store_contiguous_blocks_key_wrap_init_state (&key_cache_flash_store,
		RSA_2K_DER_KEY_MAX_SIZE);
	if (status != 0) {
		return status;
	}

	/* Initialize key cache flash state */
	status = key_cache_flash_init_state (&rsa_key_cache_flash);
	if (status != 0) {
		return status;
	}

	/* Initialize the ephemeral key manager state */
	return ephemeral_key_manager_init_state (&rsa_ephemeral_key_manager);
}

/**
 * Start the task handlers for the ephemeral key manager
 *
 * @return 0 if the task handlers have been started or an error code.
 */
int start_ephemeral_key_manager ()
{
	int status;

	/* Task to handle the ephemeral key generation */
	status = periodic_task_freertos_init_state (&ephemeral_key_manager_task);
	if (status != 0) {
		return status;
	}

	status = periodic_task_freertos_allocate_static (&ephemeral_key_manager_task,
		&ephemeral_key_manager_task_tcb, ephemeral_key_manager_task_stack,
		EPHEMERAL_KEY_MANAGER_TASK_STACK_WORDS, "Key Gen", CERBERUS_PRIORITY_BACKGROUND);
	if (status != 0) {
		return status;
	}

	periodic_task_freertos_start (&ephemeral_key_manager_task);

	return 0;
}
