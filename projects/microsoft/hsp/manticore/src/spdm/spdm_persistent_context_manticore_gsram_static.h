#ifndef SPDM_PERSISTENT_CONTEXT_MANTICORE_GSRAM_STATIC_H_
#define SPDM_PERSISTENT_CONTEXT_MANTICORE_GSRAM_STATIC_H_

#include "spdm_persistent_context_manticore_gsram.h"

int spdm_persistent_context_manticore_gsram_get_responder_state (
	const struct spdm_persistent_context_interface *context, struct spdm_responder_state **state);
int spdm_persistent_context_manticore_gsram_get_secure_session_manager_state (
	const struct spdm_persistent_context_interface *context,
	struct spdm_secure_session_manager_persistent_state **state);
void spdm_persistent_context_manticore_gsram_unlock (
	const struct spdm_persistent_context_interface *context);

/**
 * Constant initializer for the Manticore GSRAM SPDM persistent context interface.
 */
#define SPDM_PERSISTENT_CONTEXT_MANTICORE_GSRAM_API_INIT { \
	.get_responder_state = spdm_persistent_context_manticore_gsram_get_responder_state, \
	.get_secure_session_manager_state = spdm_persistent_context_manticore_gsram_get_secure_session_manager_state, \
	.unlock = spdm_persistent_context_manticore_gsram_unlock, \
}

/**
 * Initialize a static instance of a Manticore SPDM persistent context located in GSRAM.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Internal state for the context.
 * @param dmb_ptr The DMB interface to use for accessing the persistent state.
 * @param context_soc_address_arg The SoC address of the SPDM context.
 */
#define spdm_persistent_context_manticore_gsram_static_init(state_ptr, dmb_ptr, context_soc_address_arg) { \
	.base = SPDM_PERSISTENT_CONTEXT_MANTICORE_GSRAM_API_INIT, \
	.state = state_ptr, \
	.dmb = dmb_ptr, \
	.context_soc_address = context_soc_address_arg, \
}


#endif	/* SPDM_PERSISTENT_CONTEXT_MANTICORE_GSRAM_STATIC_H_ */
