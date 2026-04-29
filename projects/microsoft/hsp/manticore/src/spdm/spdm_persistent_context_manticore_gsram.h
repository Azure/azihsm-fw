// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SPDM_PERSISTENT_CONTEXT_MANTICORE_GSRAM_H_
#define SPDM_PERSISTENT_CONTEXT_MANTICORE_GSRAM_H_

#include "drivers/hsp_dmb.h"
#include "spdm/spdm_persistent_context_interface.h"
#include "spdm/spdm_secure_session_manager.h"
#include "spdm/spdm_state.h"

/**
 * SPDM context struct (containing both SPDM responder state and SPDM secure session manager) and
 * stored in GSRAM memory
 */
struct spdm_persistent_context_manticore_gsram_data {
	struct spdm_responder_state responder_state;					/**< SPDM responder state. */
	struct spdm_secure_session_manager_persistent_state ssm_state;	/**< SPDM secure session manager persistent state. */
};


_Static_assert (offsetof (struct spdm_persistent_context_manticore_gsram_data,
	responder_state) == 0, "Unexpected struct member offset");
_Static_assert (offsetof (struct spdm_persistent_context_manticore_gsram_data, ssm_state) == 88,
	"Unexpected struct member offset");
_Static_assert (sizeof (struct spdm_persistent_context_manticore_gsram_data) == 800,
	"Unexpected size of struct spdm_secure_session_manager_persistent_state");

/**
 * Internal state for the Manticore SPDM persistent context.
 */
struct spdm_persistent_context_manticore_gsram_state {
	platform_mutex lock;												/**< Mutex for synchronizing access to the context. */
	struct spdm_persistent_context_manticore_gsram_data *context_data;	/**< SPDM responder state. */
	uint32_t ref_count;													/**< Reference count for the context. */
};

/**
 * Manticore implementation of the SPDM persistent context.
 */
struct spdm_persistent_context_manticore_gsram {
	struct spdm_persistent_context_interface base;					/**< SPDM persistent context interface */
	struct spdm_persistent_context_manticore_gsram_state *state;	/**< Internal state */
	const struct hsp_dmb *dmb;										/**< DMB interface */
	uint64_t context_soc_address;									/**< SoC address of the responder state. */
};


int spdm_persistent_context_manticore_gsram_init (
	struct spdm_persistent_context_manticore_gsram *context,
	struct spdm_persistent_context_manticore_gsram_state *state, const struct hsp_dmb *dmb,
	uint64_t context_soc_address);
void spdm_persistent_context_manticore_gsram_release (
	struct spdm_persistent_context_manticore_gsram *context);
int spdm_persistent_context_manticore_gsram_init_state (
	const struct spdm_persistent_context_manticore_gsram *context);


#endif	/* SPDM_PERSISTENT_CONTEXT_MANTICORE_GSRAM_H_ */
