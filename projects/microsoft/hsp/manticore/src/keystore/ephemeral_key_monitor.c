// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <string.h>
#include "ephemeral_key_monitor.h"
#include "ephemeral_key_monitor_static.h"
#include "platform_io_api.h"
#include "common/buffer_util.h"
#include "common/type_cast.h"
#include "common/unused.h"
#include "crypto/mbedtls_compat.h"
#include "keystore/key_cache.h"
#include "logging/debug_log.h"
#include "logging/manticore_logging.h"
#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"


/**
 * RSA-2K PKA little-endian component lengths: `d || n || e` (d=256, n=256, e=4), summing to
 * `PART_PERSISTENT_STORE_KEY_SIZE`.
 */
#define	EPHEMERAL_KEY_MONITOR_RSA_2K_D_LEN		256u
#define	EPHEMERAL_KEY_MONITOR_RSA_2K_N_LEN		256u
#define	EPHEMERAL_KEY_MONITOR_RSA_2K_E_LEN		4u


/**
 * Convert a DER-encoded RSA-2K private key into the 516-byte PKA little-endian `d || n || e` layout.
 * `out` is zeroized on any error path so a partial private exponent never leaks.
 *
 * @param der DER-encoded RSA-2K private key buffer.
 * @param der_len Length of the DER buffer in bytes.
 * @param out Output buffer that receives the PKA little-endian layout.  Must hold at least
 *   `PART_PERSISTENT_STORE_KEY_SIZE` bytes.
 * @param out_size Size of the output buffer in bytes.
 * @param out_len On success, set to `PART_PERSISTENT_STORE_KEY_SIZE`.
 *
 * @return 0 on success, or one of `EPHEMERAL_KEY_MONITOR_CONVERT_*` on failure.
 */
static int ephemeral_key_monitor_convert_der_to_pka_le_2k (const uint8_t *der, size_t der_len,
	uint8_t *out, size_t out_size, size_t *out_len)
{
	mbedtls_pk_context pk;
	mbedtls_rsa_context *rsa;
	uint8_t *d;
	uint8_t *n;
	uint8_t *e;
	int status;

	if ((der == NULL) || (out == NULL) || (out_len == NULL) || (der_len == 0)) {
		return EPHEMERAL_KEY_MONITOR_INVALID_ARGUMENT;
	}

	if (out_size < PART_PERSISTENT_STORE_KEY_SIZE) {
		return EPHEMERAL_KEY_MONITOR_CONVERT_SMALL_OUTPUT;
	}

	d = &out[0];
	n = &out[EPHEMERAL_KEY_MONITOR_RSA_2K_D_LEN];
	e = &out[EPHEMERAL_KEY_MONITOR_RSA_2K_D_LEN + EPHEMERAL_KEY_MONITOR_RSA_2K_N_LEN];

	mbedtls_pk_init (&pk);

#if MBEDTLS_IS_VERSION_3
	/* Parse the private key in DER format from the buffer.  No RNG is needed for RSA keys, despite
	 * the API description saying it's required.  If one is desired (or becomes required), an
	 * rng_engine can be provided to this instance during init and used along with
	 * rng_mbedtls_rng_callback(). */
	status = mbedtls_pk_parse_key (&pk, der, der_len, NULL, 0, NULL, NULL);
#else
	status = mbedtls_pk_parse_key (&pk, der, der_len, NULL, 0);
#endif
	if (status != 0) {
		goto exit;
	}

	if (mbedtls_pk_get_type (&pk) != MBEDTLS_PK_RSA) {
		status = EPHEMERAL_KEY_MONITOR_CONVERT_KEY_NOT_RSA;
		goto exit;
	}

	rsa = mbedtls_pk_rsa (pk);
	if (rsa == NULL) {
		status = EPHEMERAL_KEY_MONITOR_CONVERT_KEY_NOT_RSA;
		goto exit;
	}

	/* mbedtls exports big-endian; PKA needs little-endian per component. */
	status = mbedtls_rsa_export_raw (rsa, n, EPHEMERAL_KEY_MONITOR_RSA_2K_N_LEN, NULL, 0, NULL, 0,
		d, EPHEMERAL_KEY_MONITOR_RSA_2K_D_LEN, e, EPHEMERAL_KEY_MONITOR_RSA_2K_E_LEN);
	if (status != 0) {
		buffer_zeroize (out, PART_PERSISTENT_STORE_KEY_SIZE);
		goto exit;
	}

	buffer_reverse (d, EPHEMERAL_KEY_MONITOR_RSA_2K_D_LEN);
	buffer_reverse (n, EPHEMERAL_KEY_MONITOR_RSA_2K_N_LEN);
	buffer_reverse (e, EPHEMERAL_KEY_MONITOR_RSA_2K_E_LEN);

	*out_len = PART_PERSISTENT_STORE_KEY_SIZE;

exit:
	mbedtls_pk_free (&pk);

	return status;
}


/**
 * Compute the SoC GSRAM address of a per-PFN slot in the HSM Part Persistent Store.
 *
 * @param gsram_base Base SoC address of the HSM Part Persistent Store region.
 * @param pfn Slot index, in the range [0, PART_PERSISTENT_STORE_SLOT_COUNT).
 *
 * @return The absolute SoC byte address of slot `pfn`.
 */
static uint64_t ephemeral_key_monitor_part_slot_address (uint64_t gsram_base, uint16_t pfn)
{
	return gsram_base + ((uint64_t) pfn * PART_PERSISTENT_STORE_SLOT_SIZE);
}

/**
 * Read both staging gates (Gate 1 `unwrapping_key_required`, Gate 2 `unwrapping_key_bk_valid`) for
 * a slot in a single GSRAM mapping.
 *
 * @param dmb DMB interface used to map the GSRAM slot window.
 * @param gsram_base Base SoC address of the HSM Part Persistent Store region.
 * @param pfn Slot index in the range [0, PART_PERSISTENT_STORE_SLOT_COUNT).
 * @param required Out: `true` if `unwrapping_key_required == 1` (the CP has armed this PFN for
 *   staging).  Set to `false` on map failure; only meaningful when the call returns 0.
 * @param valid Out: `true` if `unwrapping_key_bk_valid == 1` (the slot already advertises a valid
 *   key).  Set to `false` on map failure; only meaningful when the call returns 0.
 *
 * @return 0 on success, or `EPHEMERAL_KEY_MONITOR_INVALID_ARGUMENT` / `_DMB_MAP_FAILED`.
 */
static int ephemeral_key_monitor_get_part_slot_state (const struct hsp_dmb *dmb,
	uint64_t gsram_base, uint16_t pfn, bool *required, bool *valid)
{
	uint8_t *part_slot;
	int status;

	if ((dmb == NULL) || (required == NULL) || (valid == NULL) ||
		(pfn >= PART_PERSISTENT_STORE_SLOT_COUNT)) {
		return EPHEMERAL_KEY_MONITOR_INVALID_ARGUMENT;
	}

	status = dmb->map_soc_address (dmb, ephemeral_key_monitor_part_slot_address (gsram_base, pfn),
		PART_PERSISTENT_STORE_SLOT_SIZE, HSP_DMB_ACCESS_READ, (void**) &part_slot);
	if (status != 0) {
		*required = false;
		*valid = false;

		return EPHEMERAL_KEY_MONITOR_DMB_MAP_FAILED;
	}

	*required = (part_slot[PART_PERSISTENT_STORE_REQUIRED_OFFSET] != 0);
	*valid = (part_slot[PART_PERSISTENT_STORE_VALID_OFFSET] != 0);

	dmb->unmap_soc_address (dmb, part_slot);

	return 0;
}

/**
 * Publish a key payload to a slot in the HSM Part Persistent Store and raise the slot's valid
 * flag.  Gate 1 (`unwrapping_key_required`) is re-checked under the write mapping to close the CP
 * disarm race: if the PFN is not/no-longer armed, any staged payload is scrubbed and the slot is
 * left empty.
 *
 * @param dmb DMB interface used to map the GSRAM slot window.
 * @param gsram_base Base SoC address of the HSM Part Persistent Store region.
 * @param pfn Slot index in the range [0, PART_PERSISTENT_STORE_SLOT_COUNT).
 * @param key Buffer holding the key payload to publish.
 * @param key_length Length of `key` in bytes.  Must be exactly `PART_PERSISTENT_STORE_KEY_SIZE`;
 *   any other value is rejected.
 *
 * @return 0 if the key was published, `EPHEMERAL_KEY_MONITOR_SLOT_SKIPPED` if the PFN was
 *   not/no-longer armed by the CP (any staged payload was scrubbed), or
 *   `EPHEMERAL_KEY_MONITOR_INVALID_ARGUMENT` / `_DMB_MAP_FAILED`.
 */
static int ephemeral_key_monitor_publish_key (const struct hsp_dmb *dmb, uint64_t gsram_base,
	uint16_t pfn, const uint8_t *key, size_t key_length)
{
	uint8_t *part_slot;
	int status;
	int result = 0;

	if ((dmb == NULL) || (key == NULL) || (pfn >= PART_PERSISTENT_STORE_SLOT_COUNT) ||
		(key_length != PART_PERSISTENT_STORE_KEY_SIZE)) {
		return EPHEMERAL_KEY_MONITOR_INVALID_ARGUMENT;
	}

	status = dmb->map_soc_address (dmb, ephemeral_key_monitor_part_slot_address (gsram_base, pfn),
		PART_PERSISTENT_STORE_SLOT_SIZE, HSP_DMB_ACCESS_WRITE, (void**) &part_slot);
	if (status != 0) {
		return EPHEMERAL_KEY_MONITOR_DMB_MAP_FAILED;
	}

	/* Gate 1: skip if the CP has not armed (or has since released) this PFN. */
	if (part_slot[PART_PERSISTENT_STORE_REQUIRED_OFFSET] == 0) {
		result = EPHEMERAL_KEY_MONITOR_SLOT_SKIPPED;
		goto unmap;
	}

	/* Write payload first, then the valid flag; the unmap below orders the writes for the CP. */
	memcpy (&part_slot[PART_PERSISTENT_STORE_KEY_OFFSET], key, PART_PERSISTENT_STORE_KEY_SIZE);

	/* Release race: if the CP disarmed the PFN during the copy, scrub the payload and leave the
	 * slot empty so a released PFN never advertises a live key. */
	if (part_slot[PART_PERSISTENT_STORE_REQUIRED_OFFSET] == 0) {
		buffer_zeroize (&part_slot[PART_PERSISTENT_STORE_KEY_OFFSET],
			PART_PERSISTENT_STORE_KEY_SIZE);
		result = EPHEMERAL_KEY_MONITOR_SLOT_SKIPPED;
		goto unmap;
	}

	part_slot[PART_PERSISTENT_STORE_VALID_OFFSET] = 1;

unmap:
	dmb->unmap_soc_address (dmb, part_slot);

	return result;
}

/**
 * Re-arm the periodic clock for the next execute().
 *
 * @param monitor The ephemeral key monitor whose timeout should be rearmed.
 */
static void ephemeral_key_monitor_schedule_next (
	const struct ephemeral_key_monitor *monitor)
{
	if (platform_init_timeout (monitor->period_ms, &monitor->state->next) == 0) {
		monitor->state->next_valid = true;
	}
	else {
		monitor->state->next_valid = false;
	}
}

const platform_clock* ephemeral_key_monitor_get_next_execution (
	const struct periodic_task_handler *handler)
{
	const struct ephemeral_key_monitor *monitor = TO_DERIVED_TYPE (handler,
		const struct ephemeral_key_monitor, base);

	if (monitor->state->next_valid) {
		return &monitor->state->next;
	}

	/* If the next timeout is not valid, just indicate immediate execution. */
	return NULL;
}

void ephemeral_key_monitor_prepare (const struct periodic_task_handler *handler)
{
	const struct ephemeral_key_monitor *monitor = TO_DERIVED_TYPE (handler,
		const struct ephemeral_key_monitor, base);

	ephemeral_key_monitor_schedule_next (monitor);
}


/**
 * Periodic tick: refill every empty, armed slot from the cache.  Per-slot errors do not abort the
 * scan; QUEUE_IS_EMPTY / CREDIT_NOT_AVAILABLE are silenced as the normal "producer not ready" path.
 */
void ephemeral_key_monitor_execute (const struct periodic_task_handler *handler)
{
	const struct ephemeral_key_monitor *monitor = TO_DERIVED_TYPE (handler,
		const struct ephemeral_key_monitor, base);

	/* Wait until the producer cache has finished boot init. */
	if (monitor->key_cache->is_initialized (monitor->key_cache) == true) {
		for (uint16_t pfn = 0; pfn < PART_PERSISTENT_STORE_SLOT_COUNT; pfn++) {
			bool slot_required = false;
			bool slot_valid = false;
			size_t key_len = 0;
			size_t pka_le_len = 0;
			int status;

			/* Gate 1 skips PFNs the CP has not armed; publish_key re-checks it to close the race. */
			status = ephemeral_key_monitor_get_part_slot_state (monitor->dmb,
				monitor->part_persistent_store_base, pfn, &slot_required, &slot_valid);
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_EPHEMERAL_KEY_MONITOR, pfn, (uint32_t) status);
				continue;
			}

			if (!slot_required) {
				continue;
			}

			if (slot_valid) {
				continue;
			}

			/* Read the DER key from the cache (unwrapped in place) before converting and publishing. */
			status = monitor->key_cache->remove (monitor->key_cache, pfn, monitor->key_buffer,
				monitor->key_buffer_len, &key_len);
			if (status != 0) {
				if ((status != KEY_CACHE_QUEUE_IS_EMPTY) &&
					(status != KEY_CACHE_CREDIT_NOT_AVAILABLE)) {
					debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING,
						DEBUG_LOG_COMPONENT_MANTICORE, MANTICORE_LOGGING_EPHEMERAL_KEY_MONITOR, pfn,
						(uint32_t) status);
				}
				buffer_zeroize (monitor->key_buffer, monitor->key_buffer_len);
				continue;
			}

			/* Convert DER -> PKA-LE in place (parse consumes the DER before the payload is written). */
			status = ephemeral_key_monitor_convert_der_to_pka_le_2k (monitor->key_buffer, key_len,
				monitor->key_buffer, monitor->key_buffer_len, &pka_le_len);
			if (status != 0) {
				debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING, DEBUG_LOG_COMPONENT_MANTICORE,
					MANTICORE_LOGGING_EPHEMERAL_KEY_MONITOR, pfn, (uint32_t) status);
			}
			else {
				status = ephemeral_key_monitor_publish_key (monitor->dmb,
					monitor->part_persistent_store_base, pfn, monitor->key_buffer, pka_le_len);
				if ((status != 0) && (status != EPHEMERAL_KEY_MONITOR_SLOT_SKIPPED)) {
					debug_log_create_entry (DEBUG_LOG_SEVERITY_WARNING,
						DEBUG_LOG_COMPONENT_MANTICORE, MANTICORE_LOGGING_EPHEMERAL_KEY_MONITOR, pfn,
						(uint32_t) status);
				}
			}

			/* CSP scrub before next slot. */
			buffer_zeroize (monitor->key_buffer, monitor->key_buffer_len);
		}
	}

	ephemeral_key_monitor_schedule_next (monitor);
}

/**
 * Initialize a handler for the ephemeral key monitor task.
 *
 * @param monitor The ephemeral key monitor to initialize.
 * @param state Variable context for the handler.  This must be uninitialized.
 * @param dmb The DMB interface used to map GSRAM slot windows.
 * @param key_cache Flash-backed key cache feeding refill attempts.
 * @param key_buffer Working buffer used to read the DER key and stage the PKA-LE payload.  Must be
 *   at least `EPHEMERAL_KEY_MONITOR_KEY_BUFFER_SIZE` bytes.
 * @param key_buffer_len Length of `key_buffer`, in bytes.
 * @param period_ms The amount of time between scan ticks, in milliseconds.
 * @param part_persistent_store_base SoC base address of the HSM Part Persistent Store.
 *
 * @return 0 if the handler was successfully initialized or an error code.
 */
int ephemeral_key_monitor_init (struct ephemeral_key_monitor *monitor,
	struct ephemeral_key_monitor_state *state, const struct hsp_dmb *dmb,
	const struct key_cache *key_cache, uint8_t *key_buffer, size_t key_buffer_len,
	uint32_t period_ms, uint64_t part_persistent_store_base)
{
	if (monitor == NULL) {
		return EPHEMERAL_KEY_MONITOR_INVALID_ARGUMENT;
	}

	monitor->base.prepare = ephemeral_key_monitor_prepare;
	monitor->base.get_next_execution = ephemeral_key_monitor_get_next_execution;
	monitor->base.execute = ephemeral_key_monitor_execute;

	monitor->state = state;
	monitor->dmb = dmb;
	monitor->key_cache = key_cache;
	monitor->key_buffer = key_buffer;
	monitor->key_buffer_len = key_buffer_len;
	monitor->period_ms = period_ms;
	monitor->part_persistent_store_base = part_persistent_store_base;

	return ephemeral_key_monitor_init_state (monitor);
}

/**
 * Initialize only the variable state for an ephemeral key monitor.  The rest of the
 * handler is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param monitor The ephemeral key monitor that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int ephemeral_key_monitor_init_state (const struct ephemeral_key_monitor *monitor)
{
	if ((monitor == NULL) || (monitor->state == NULL) || (monitor->dmb == NULL) ||
		(monitor->key_cache == NULL) || (monitor->key_buffer == NULL) ||
		(monitor->key_buffer_len < EPHEMERAL_KEY_MONITOR_KEY_BUFFER_SIZE) ||
		(monitor->period_ms == 0u)) {
		return EPHEMERAL_KEY_MONITOR_INVALID_ARGUMENT;
	}

	memset (monitor->state, 0, sizeof (struct ephemeral_key_monitor_state));

	return 0;
}

/**
 * Release the resources used by an ephemeral key monitor.
 *
 * @param monitor The handler to release.
 */
void ephemeral_key_monitor_release (const struct ephemeral_key_monitor *monitor)
{
	UNUSED (monitor);
}
