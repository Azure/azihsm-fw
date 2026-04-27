// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_AEB_STATIC_H_
#define HSP_AEB_STATIC_H_

#include "drivers/hsp_aeb.h"


/* Internal functions declared to allow for static initialization. */
int hsp_aeb_get_aeb_state (const struct hsp_aeb *aeb, unsigned int num);
int hsp_aeb_enable_aeb (const struct hsp_aeb *aeb, unsigned int num);
int hsp_aeb_is_enabled (const struct hsp_aeb *aeb, unsigned int num);
int hsp_aeb_disable_aeb (const struct hsp_aeb *aeb, unsigned int num);
int hsp_aeb_disable_and_lock_aeb (const struct hsp_aeb *aeb, unsigned int num);
int hsp_aeb_is_disabled (const struct hsp_aeb *aeb, unsigned int num);
int hsp_aeb_is_locked (const struct hsp_aeb *aeb, unsigned int num);
int hsp_aeb_restore_default (const struct hsp_aeb *aeb, unsigned int num);
int hsp_aeb_get_multiple_aeb_state (const struct hsp_aeb *aeb, uint32_t *state, size_t word_count);
int hsp_aeb_get_multiple_aeb_locked (const struct hsp_aeb *aeb, uint32_t *locked,
	size_t word_count);
int hsp_aeb_configure_multiple_aeb (const struct hsp_aeb *aeb, const uint32_t *enable,
	const uint32_t *disable, const uint32_t *lock, size_t word_count);


/**
 * Constant initializer for the AEB driver API.
 */
#define	HSP_AEB_API_INIT    \
	.get_aeb_state = hsp_aeb_get_aeb_state, \
	.enable_aeb = hsp_aeb_enable_aeb, \
	.is_enabled = hsp_aeb_is_enabled, \
	.disable_aeb = hsp_aeb_disable_aeb, \
	.disable_and_lock_aeb = hsp_aeb_disable_and_lock_aeb, \
	.is_disabled = hsp_aeb_is_disabled, \
	.is_locked = hsp_aeb_is_locked, \
	.restore_default = hsp_aeb_restore_default, \
	.get_multiple_aeb_state = hsp_aeb_get_multiple_aeb_state, \
	.get_multiple_aeb_locked = hsp_aeb_get_multiple_aeb_locked, \
	.configure_multiple_aeb = hsp_aeb_configure_multiple_aeb \


/**
 * Initialize a static AEB driver instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the AEB driver.
 * @param regs_ptr Register interface for the AEB hardware.
 */
#define	hsp_aeb_static_init(state_ptr, regs_ptr)	{ \
		HSP_AEB_API_INIT, \
		.state = state_ptr, \
		.regs = regs_ptr, \
	}


#endif	/* HSP_AEB_STATIC_H_ */
