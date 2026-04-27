// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "common/buffer_util.h"
#include "drivers/hsp_aeb.h"


/**
 * Control and status registers for a single AEB group.
 */
struct hsp_aeb_group {
	volatile uint32_t *status;		/**< Accessor for the AEB_GROUP_x_STATUS register. */
	volatile uint32_t *enable_w1s;	/**< Accessor for the AEB_GROUP_x_ENABLE_RW1S register. */
	volatile uint32_t *enable_w1c;	/**< Accessor for the AEB_GROUP_x_ENABLE_RW1C register. */
	volatile uint32_t *disable_w1s;	/**< Accessor for the AEB_GROUP_x_DISABLE_RW1S register. */
	volatile uint32_t *disable_w1c;	/**< Accessor for the AEB_GROUP_x_DISABLE_RW1C register. */
	volatile uint32_t *wlock_w1s;	/**< Accessor for the AEB_GROUP_x_WLOCK_RW1S register. */
};


/**
 * Get the set of registers for a specific group of AEBs.  This provides an abstracted way of
 * dealing with the AEB registers, without needing to understand how they are addressed in hardware.
 *
 * It also provides a central location for all error checking for AEB requests.
 *
 * @param aeb The AEB driver that contains the required group registers.
 * @param group_num The AEB group number.
 * @param group_regs Output for the registers to control the group.
 *
 * @return 0 if the group registers were retrieved successfully or an error code.
 */
static int hsp_aeb_get_group_regs (const struct hsp_aeb *aeb, unsigned int group_num,
	struct hsp_aeb_group *group_regs)
{
	if (aeb == NULL) {
		return HSP_AEB_INVALID_ARGUMENT;
	}

	switch (group_num) {
		case 0:
			group_regs->status = &aeb->regs->AEB_GROUP_0_STATUS;
			group_regs->enable_w1s = &aeb->regs->AEB_GROUP_0_ENABLE_RW1S;
			group_regs->enable_w1c = &aeb->regs->AEB_GROUP_0_ENABLE_RW1C;
			group_regs->disable_w1s = &aeb->regs->AEB_GROUP_0_DISABLE_RW1S;
			group_regs->disable_w1c = &aeb->regs->AEB_GROUP_0_DISABLE_RW1C;
			group_regs->wlock_w1s = &aeb->regs->AEB_GROUP_0_WLOCK_RW1S;
			break;

		case 1:
			group_regs->status = &aeb->regs->AEB_GROUP_1_STATUS;
			group_regs->enable_w1s = &aeb->regs->AEB_GROUP_1_ENABLE_RW1S;
			group_regs->enable_w1c = &aeb->regs->AEB_GROUP_1_ENABLE_RW1C;
			group_regs->disable_w1s = &aeb->regs->AEB_GROUP_1_DISABLE_RW1S;
			group_regs->disable_w1c = &aeb->regs->AEB_GROUP_1_DISABLE_RW1C;
			group_regs->wlock_w1s = &aeb->regs->AEB_GROUP_1_WLOCK_RW1S;
			break;

		case 2:
			group_regs->status = &aeb->regs->AEB_GROUP_2_STATUS;
			group_regs->enable_w1s = &aeb->regs->AEB_GROUP_2_ENABLE_RW1S;
			group_regs->enable_w1c = &aeb->regs->AEB_GROUP_2_ENABLE_RW1C;
			group_regs->disable_w1s = &aeb->regs->AEB_GROUP_2_DISABLE_RW1S;
			group_regs->disable_w1c = &aeb->regs->AEB_GROUP_2_DISABLE_RW1C;
			group_regs->wlock_w1s = &aeb->regs->AEB_GROUP_2_WLOCK_RW1S;
			break;

		case 3:
			group_regs->status = &aeb->regs->AEB_GROUP_3_STATUS;
			group_regs->enable_w1s = &aeb->regs->AEB_GROUP_3_ENABLE_RW1S;
			group_regs->enable_w1c = &aeb->regs->AEB_GROUP_3_ENABLE_RW1C;
			group_regs->disable_w1s = &aeb->regs->AEB_GROUP_3_DISABLE_RW1S;
			group_regs->disable_w1c = &aeb->regs->AEB_GROUP_3_DISABLE_RW1C;
			group_regs->wlock_w1s = &aeb->regs->AEB_GROUP_3_WLOCK_RW1S;
			break;

		default:
			return HSP_AEB_OUT_OF_RANGE;
	}

	return 0;
}

/**
 * Get the set of registers for a specific AEB.
 *
 * This call will acquire the driver mutex on success.
 *
 * @param aeb The AEB driver that contains the required AEB registers.
 * @param num The AEB number.
 * @param group_regs Output for the registers to control the group containing the requested AEB.
 * @param mask Output for the bit mask that control the specified AEB.  Only one bit will be set.
 *
 * @return 0 if the AEB registers were retrieved successfully or an error code.
 */
static int hsp_aeb_get_group_regs_for_aeb (const struct hsp_aeb *aeb, unsigned int num,
	struct hsp_aeb_group *group_regs, uint32_t *mask)
{
	int status;

	*mask = HSP_AEB_GET_BIT_MASK (num);

	status = hsp_aeb_get_group_regs (aeb, HSP_AEB_GET_GROUP (num), group_regs);
	if (status == 0) {
		platform_mutex_lock (&aeb->state->lock);
	}

	return status;
}

int hsp_aeb_get_aeb_state (const struct hsp_aeb *aeb, unsigned int num)
{
	struct hsp_aeb_group group;
	uint32_t mask;
	int status;

	status = hsp_aeb_get_group_regs_for_aeb (aeb, num, &group, &mask);
	if (status != 0) {
		return status;
	}

	status = !!(*group.status & mask);

	platform_mutex_unlock (&aeb->state->lock);

	return status;
}

int hsp_aeb_enable_aeb (const struct hsp_aeb *aeb, unsigned int num)
{
	struct hsp_aeb_group group;
	uint32_t mask;
	int status;

	status = hsp_aeb_get_group_regs_for_aeb (aeb, num, &group, &mask);
	if (status != 0) {
		return status;
	}

	if (*group.wlock_w1s & mask) {
		status = HSP_AEB_LOCKED;
		goto exit;
	}

	*group.enable_w1s = mask;

exit:
	platform_mutex_unlock (&aeb->state->lock);

	return status;
}

int hsp_aeb_is_enabled (const struct hsp_aeb *aeb, unsigned int num)
{
	struct hsp_aeb_group group;
	uint32_t mask;
	int status;

	status = hsp_aeb_get_group_regs_for_aeb (aeb, num, &group, &mask);
	if (status != 0) {
		return status;
	}

	status = !!(*group.enable_w1s & mask);

	platform_mutex_unlock (&aeb->state->lock);

	return status;
}

int hsp_aeb_disable_aeb (const struct hsp_aeb *aeb, unsigned int num)
{
	struct hsp_aeb_group group;
	uint32_t mask;
	int status;

	status = hsp_aeb_get_group_regs_for_aeb (aeb, num, &group, &mask);
	if (status != 0) {
		return status;
	}

	*group.disable_w1s = mask;

	platform_mutex_unlock (&aeb->state->lock);

	return status;
}

int hsp_aeb_disable_and_lock_aeb (const struct hsp_aeb *aeb, unsigned int num)
{
	struct hsp_aeb_group group;
	uint32_t mask;
	int status;

	status = hsp_aeb_get_group_regs_for_aeb (aeb, num, &group, &mask);
	if (status != 0) {
		return status;
	}

	*group.wlock_w1s = mask;

	platform_mutex_unlock (&aeb->state->lock);

	return status;
}

int hsp_aeb_is_disabled (const struct hsp_aeb *aeb, unsigned int num)
{
	struct hsp_aeb_group group;
	uint32_t mask;
	int status;

	status = hsp_aeb_get_group_regs_for_aeb (aeb, num, &group, &mask);
	if (status != 0) {
		return status;
	}

	status = !!((*group.disable_w1s & mask) | (*group.wlock_w1s & mask));

	platform_mutex_unlock (&aeb->state->lock);

	return status;
}

int hsp_aeb_is_locked (const struct hsp_aeb *aeb, unsigned int num)
{
	struct hsp_aeb_group group;
	uint32_t mask;
	int status;

	status = hsp_aeb_get_group_regs_for_aeb (aeb, num, &group, &mask);
	if (status != 0) {
		return status;
	}

	status = !!(*group.wlock_w1s & mask);

	platform_mutex_unlock (&aeb->state->lock);

	return status;
}

int hsp_aeb_restore_default (const struct hsp_aeb *aeb, unsigned int num)
{
	struct hsp_aeb_group group;
	uint32_t mask;
	int status;

	status = hsp_aeb_get_group_regs_for_aeb (aeb, num, &group, &mask);
	if (status != 0) {
		return status;
	}

	if (*group.wlock_w1s & mask) {
		status = HSP_AEB_LOCKED;
		goto exit;
	}

	/* Clear the enable and disable bits to let hardware determine the state of the AEB. */
	*group.enable_w1c = mask;
	*group.disable_w1c = mask;

exit:
	platform_mutex_unlock (&aeb->state->lock);

	return status;
}

int hsp_aeb_get_multiple_aeb_state (const struct hsp_aeb *aeb, uint32_t *state, size_t word_count)
{
	struct hsp_aeb_group group;
	size_t i;
	uint32_t state_val;
	int status;

	if ((aeb == NULL) || (state == NULL)) {
		return HSP_AEB_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&aeb->state->lock);

	for (i = 0; i < word_count; i++) {
		status = hsp_aeb_get_group_regs (aeb, i, &group);
		if (status == 0) {
			state_val = *group.status;
		}
		else {
			state_val = 0;
		}

		/* Store the value in an intermediate and use memcpy to handle misaligned addresses. */
		memcpy (state, &state_val, sizeof (state_val));
		state++;
	}

	platform_mutex_unlock (&aeb->state->lock);

	return 0;
}

int hsp_aeb_get_multiple_aeb_locked (const struct hsp_aeb *aeb, uint32_t *locked, size_t word_count)
{
	struct hsp_aeb_group group;
	size_t i;
	uint32_t locked_val;
	int status;

	if ((aeb == NULL) || (locked == NULL)) {
		return HSP_AEB_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&aeb->state->lock);

	for (i = 0; i < word_count; i++) {
		status = hsp_aeb_get_group_regs (aeb, i, &group);
		if (status == 0) {
			locked_val = *group.wlock_w1s;
		}
		else {
			locked_val = 0;
		}

		/* Store the value in an intermediate and use memcpy to handle misaligned addresses. */
		memcpy (locked, &locked_val, sizeof (locked_val));
		locked++;
	}

	platform_mutex_unlock (&aeb->state->lock);

	return 0;
}

int hsp_aeb_configure_multiple_aeb (const struct hsp_aeb *aeb, const uint32_t *enable,
	const uint32_t *disable, const uint32_t *lock, size_t word_count)
{
	struct hsp_aeb_group group;
	size_t i;

	if (aeb == NULL) {
		return HSP_AEB_INVALID_ARGUMENT;
	}

	if (word_count > HSP_AEB_MAX_GROUPS) {
		return HSP_AEB_OUT_OF_RANGE;
	}

	platform_mutex_lock (&aeb->state->lock);

	/* AEB configuration should be applied in order from the most restrictive to least restrictive.
	 * This ensures that permissive AEB settings that may allow restrictive settings to be bypassed
	 * should be set last.  A clear example of this is JTAG enable.  If JTAG is enabled first, then
	 * it can interrupt the process before other AEBs have been locked. */
	if (lock) {
		for (i = 0; i < word_count; i++) {
			hsp_aeb_get_group_regs (aeb, i, &group);
			*group.wlock_w1s = lock[i];
		}
	}

	if (disable) {
		for (i = 0; i < word_count; i++) {
			hsp_aeb_get_group_regs (aeb, i, &group);
			*group.disable_w1s = disable[i];
		}
	}

	if (enable) {
		for (i = 0; i < word_count; i++) {
			hsp_aeb_get_group_regs (aeb, i, &group);
			*group.enable_w1s = enable[i];
		}
	}

	platform_mutex_unlock (&aeb->state->lock);

	return 0;
}

/**
 * Initialize a driver for the HSP Access Enablement Bits (AEB) to set the device security
 * configuration.
 *
 * @param aeb The AEB driver to initialize.
 * @param state Variable context for the AEB driver.  This must be uninitialized.
 * @param regs Register interface for the AEB hardware.
 *
 * @return 0 if the AEB driver was successfully initialized or an error code.
 */
int hsp_aeb_init (struct hsp_aeb *aeb, struct hsp_aeb_state *state,	struct Creg_regs_aeb_regs *regs)
{
	if ((aeb == NULL) || (state == NULL) || (regs == NULL)) {
		return HSP_AEB_INVALID_ARGUMENT;
	}

	memset (aeb, 0, sizeof (struct hsp_aeb));

	aeb->get_aeb_state = hsp_aeb_get_aeb_state;
	aeb->enable_aeb = hsp_aeb_enable_aeb;
	aeb->is_enabled = hsp_aeb_is_enabled;
	aeb->disable_aeb = hsp_aeb_disable_aeb;
	aeb->disable_and_lock_aeb = hsp_aeb_disable_and_lock_aeb;
	aeb->is_disabled = hsp_aeb_is_disabled;
	aeb->is_locked = hsp_aeb_is_locked;
	aeb->restore_default = hsp_aeb_restore_default;
	aeb->get_multiple_aeb_state = hsp_aeb_get_multiple_aeb_state;
	aeb->get_multiple_aeb_locked = hsp_aeb_get_multiple_aeb_locked;
	aeb->configure_multiple_aeb = hsp_aeb_configure_multiple_aeb;

	aeb->state = state;
	aeb->regs = regs;

	return hsp_aeb_init_state (aeb);
}

/**
 * Initialize only the variable state for an AEB driver.  The rest of the driver is assumed to have
 * already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param aeb The AEB driver that contains the state to initialize.
 *
 * @return 0 if the driver state was successfully initialized or an error code.
 */
int hsp_aeb_init_state (const struct hsp_aeb *aeb)
{
	if ((aeb == NULL) || (aeb->state == NULL) || (aeb->regs == NULL)) {
		return HSP_AEB_INVALID_ARGUMENT;
	}

	memset (aeb->state, 0, sizeof (struct hsp_aeb_state));

	return platform_mutex_init (&aeb->state->lock);
}

/**
 * Release the resources used by an AEB driver.
 *
 * @param aeb The AEB driver to release.
 */
void hsp_aeb_release (const struct hsp_aeb *aeb)
{
	if (aeb) {
		platform_mutex_free (&aeb->state->lock);
	}
}
