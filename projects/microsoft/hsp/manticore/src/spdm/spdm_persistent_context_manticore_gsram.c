// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "spdm_persistent_context_manticore_gsram.h"
#include "common/type_cast.h"

//#define SPDM_PERMANENT_CONTEXT_DEBUG_SPEW
#ifdef SPDM_PERMANENT_CONTEXT_DEBUG_SPEW
#include "platform_io_api.h"
#endif


int spdm_persistent_context_manticore_gsram_get_responder_state (
	const struct spdm_persistent_context_interface *context, struct spdm_responder_state **state)
{
	int status;
	const struct spdm_persistent_context_manticore_gsram *manticore =
		TO_DERIVED_TYPE (context, struct spdm_persistent_context_manticore_gsram, base);

	if ((context == NULL) || (state == NULL)) {
		return SPDM_PERSISTENT_CONTEXT_INVALID_ARGUMENT;
	}

	*state = NULL;

	platform_mutex_lock (&manticore->state->lock);
	if (manticore->state->context_data == NULL) {
		status = manticore->dmb->map_soc_address (manticore->dmb, manticore->context_soc_address,
			sizeof (struct spdm_persistent_context_manticore_gsram_data), HSP_DMB_ACCESS_WRITE,
			(void**) &manticore->state->context_data);
		if (status != 0) {
			platform_mutex_unlock (&manticore->state->lock);

			return status;
		}
#ifdef SPDM_PERMANENT_CONTEXT_DEBUG_SPEW
		platform_printf (
			"spdm_persistent_context_manticore_gsram(%d): manticore->state->ref_count=%d\n",
			__LINE__, manticore->state->ref_count);
#endif
	}

	manticore->state->ref_count++;
	*state = &manticore->state->context_data->responder_state;

#ifdef SPDM_PERMANENT_CONTEXT_DEBUG_SPEW
	platform_printf (
		"spdm_persistent_context_manticore_gsram(%d): manticore->state->ref_count=%d\n", __LINE__,
		manticore->state->ref_count);
#endif

	platform_mutex_unlock (&manticore->state->lock);

	return 0;
}

int spdm_persistent_context_manticore_gsram_get_secure_session_manager_state (
	const struct spdm_persistent_context_interface *context,
	struct spdm_secure_session_manager_persistent_state **state)
{
	int status;
	const struct spdm_persistent_context_manticore_gsram *manticore =
		TO_DERIVED_TYPE (context, struct spdm_persistent_context_manticore_gsram, base);

	if ((context == NULL) || (state == NULL)) {
		return SPDM_PERSISTENT_CONTEXT_INVALID_ARGUMENT;
	}

	*state = NULL;

	platform_mutex_lock (&manticore->state->lock);
	if (manticore->state->context_data == NULL) {
		status = manticore->dmb->map_soc_address (manticore->dmb, manticore->context_soc_address,
			sizeof (struct spdm_persistent_context_manticore_gsram_data), HSP_DMB_ACCESS_WRITE,
			(void**) &manticore->state->context_data);
		if (status != 0) {
			platform_mutex_unlock (&manticore->state->lock);

			return status;
		}
#ifdef SPDM_PERMANENT_CONTEXT_DEBUG_SPEW
		platform_printf (
			"spdm_persistent_context_manticore_gsram(%d): manticore->state->ref_count=%d\n",
			__LINE__, manticore->state->ref_count);
#endif
	}

	manticore->state->ref_count++;
	*state = &manticore->state->context_data->ssm_state;

#ifdef SPDM_PERMANENT_CONTEXT_DEBUG_SPEW
	platform_printf (
		"spdm_persistent_context_manticore_gsram(%d): manticore->state->ref_count=%d\n", __LINE__,
		manticore->state->ref_count);
#endif

	platform_mutex_unlock (&manticore->state->lock);

	return 0;
}

void spdm_persistent_context_manticore_gsram_unlock (
	const struct spdm_persistent_context_interface *context)
{
	const struct spdm_persistent_context_manticore_gsram *manticore =
		TO_DERIVED_TYPE (context, struct spdm_persistent_context_manticore_gsram, base);

	if (context == NULL) {
		return;
	}

	platform_mutex_lock (&manticore->state->lock);
	if (manticore->state->context_data != NULL) {
		if (manticore->state->ref_count > 0) {
			manticore->state->ref_count--;
		}

		if (manticore->state->ref_count == 0) {
			manticore->dmb->unmap_soc_address (manticore->dmb,
				(void*) manticore->state->context_data);
			manticore->state->context_data = NULL;
		}
#ifdef SPDM_PERMANENT_CONTEXT_DEBUG_SPEW
		platform_printf (
			"spdm_persistent_context_manticore_gsram(%d): manticore->state->ref_count=%d\n",
			__LINE__, manticore->state->ref_count);
#endif
	}
	platform_mutex_unlock (&manticore->state->lock);
}

/**
 * Initialize a Manticore SPDM persistent context instance.
 *
 * @param context The context to initialize.
 * @param state The internal state for the context.
 * @param dmb The DMB interface to use for accessing the persistent state.
 * @param context_soc_address The SoC address of the SPDM context.
 *
 * @return 0 if the context was successfully initialized or an error code.
 */
int spdm_persistent_context_manticore_gsram_init (
	struct spdm_persistent_context_manticore_gsram *context,
	struct spdm_persistent_context_manticore_gsram_state *state, const struct hsp_dmb *dmb,
	uint64_t context_soc_address)
{
	if ((context == NULL) || (state == NULL) || (dmb == NULL)) {
		return SPDM_PERSISTENT_CONTEXT_INVALID_ARGUMENT;
	}

	memset (context, 0, sizeof (struct spdm_persistent_context_manticore_gsram));
	context->state = state;
	context->dmb = dmb;
	context->context_soc_address = context_soc_address;

	context->base.get_responder_state = spdm_persistent_context_manticore_gsram_get_responder_state;
	context->base.get_secure_session_manager_state =
		spdm_persistent_context_manticore_gsram_get_secure_session_manager_state;
	context->base.unlock = spdm_persistent_context_manticore_gsram_unlock;

	return spdm_persistent_context_manticore_gsram_init_state (context);
}

/**
 * Releases any resources for SPDM persistent context
 *
 * @param context SPDM persistent context
 */
void spdm_persistent_context_manticore_gsram_release (
	struct spdm_persistent_context_manticore_gsram *context)
{
	if ((context != NULL) && (context->state != NULL)) {
		platform_mutex_free (&context->state->lock);
	}
}

/**
 * Initializes SPDM persistent context state
 *
 * @param context SPDM persistent context
 *
 * @return 0 if successful, error code otherwise
 */
int spdm_persistent_context_manticore_gsram_init_state (
	const struct spdm_persistent_context_manticore_gsram *context)
{
	if ((context == NULL) || (context->state == NULL) || (context->dmb == NULL)) {
		return SPDM_PERSISTENT_CONTEXT_INVALID_ARGUMENT;
	}

	memset (context->state, 0, sizeof (struct spdm_persistent_context_manticore_gsram_state));

	return platform_mutex_init (&context->state->lock);
}
