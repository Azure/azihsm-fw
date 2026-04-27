// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "hw_rot_hsp_rom_no_fuses.h"
#include "hw_rot_hsp_rom_static.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "drivers/sram.h"


int hw_rot_hsp_rom_no_fuses_has_free_root_key_slots (const struct hw_rot *rot)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;

	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	if ((rom->state->current_slot + 1) < rom->root_slots) {
		return 0;
	}
	else {
		return HW_ROT_ROOT_KEYS_EXHAUSTED;
	}
}

int hw_rot_hsp_rom_no_fuses_update_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash)
{
	UNUSED (length);

	if ((rot == NULL) || (root_key == NULL) || (hash == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	return HW_ROT_UNSUPPORTED;
}

int hw_rot_hsp_rom_no_fuses_update_svn (const struct hw_rot *rot, uint64_t svn)
{
	UNUSED (svn);

	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	return HW_ROT_UNSUPPORTED;
}

int hw_rot_hsp_rom_no_fuses_tenancy_transfer (const struct hw_rot *rot, bool grant)
{
	UNUSED (grant);

	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	return HW_ROT_UNSUPPORTED;
}

/**
 * Initialize the RoT interface for run-time code to execute the same checks as ROM.
 *
 * Read/write access to the fuses is assumed to not be available, so all checks are done against
 * information already in memory or in the fuse cache registers.
 *
 * @param rot The RoT instance to initialize.
 * @param state Variable context for the RoT.  This must not already be initialized.
 * @param fuses Driver interface to the HSP fuses where security information is stored.
 * @param fuse_regs Register interface to the fuse controller.  This contains cached information
 * for most fuse slots.
 * @param ccs Driver interface for executing crypto operations with HW managed keys.
 * @param mfg_root_key Optional hash of the root key that will be used for firmware verification
 * when there are no other keys programmed into fuses.  Set to null if there is no root key when no
 * key is programmed.
 * @param root_slots The number of SW ECC fuse slots that are used to store root keys.  The first
 * slot is assumed to be SW0_ECC.
 * @param tenancy_buffer Buffer to use for storage of the tenancy counter.
 * @param root_key The root key that was used by ROM for verifying the running image.  If this is
 * null, there was no root key used.
 * @param hash Hash engine to use for root key matching.  This must be provided if the root key is
 * not null.
 * @param tenancy_counter Current value of the tenancy counter as reported by ROM.
 *
 * @return 0 if the RoT interface was initialized successfully or an error code.
 */
int hw_rot_hsp_rom_no_fuses_init (struct hw_rot_hsp_rom_no_fuses *rot,
	struct hw_rot_hsp_rom_state *state, const struct fuse_controller_interface *fuses,
	struct Gfc_regs *fuse_regs, const struct ccs_ksu_interface *ccs, const SP_MSG_384 *mfg_root_key,
	uint8_t root_slots, struct hw_rot_hsp_rom_tenancy_buffer *tenancy_buffer,
	const SP_ECDSA_P384_PUBLIC *root_key, const struct hash_engine *hash,
	const union hw_rot_hsp_rom_tenancy_counter *tenancy_counter)
{
	if ((rot == NULL) || (state == NULL) || (fuses == NULL) || (fuse_regs == NULL) ||
		(ccs == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	memset (rot, 0, sizeof (struct hw_rot_hsp_rom));

	rot->base.base.has_root_key = hw_rot_hsp_rom_has_root_key;
	rot->base.base.get_root_key_hash = hw_rot_hsp_rom_get_root_key_hash;
	rot->base.base.verify_root_key = hw_rot_hsp_rom_verify_root_key;
	rot->base.base.has_free_root_key_slots = hw_rot_hsp_rom_no_fuses_has_free_root_key_slots;
	rot->base.base.update_root_key = hw_rot_hsp_rom_no_fuses_update_root_key;
	rot->base.base.get_svn = hw_rot_hsp_rom_get_svn;
	rot->base.base.get_svn_bits = hw_rot_hsp_rom_get_svn_bits;
	rot->base.base.update_svn = hw_rot_hsp_rom_no_fuses_update_svn;
	rot->base.base.refresh_svn = hw_rot_hsp_rom_no_fuses_update_svn;
	rot->base.base.has_active_tenancy = hw_rot_hsp_rom_has_active_tenancy;
	rot->base.base.get_tenancy_counter = hw_rot_hsp_rom_get_tenancy_counter;
	rot->base.base.get_tenancy_grant_token = hw_rot_hsp_rom_get_tenancy_grant_token;
	rot->base.base.tenancy_transfer = hw_rot_hsp_rom_no_fuses_tenancy_transfer;

	rot->base.state = state;
	rot->base.fuses = fuses;
	rot->base.fuse_regs = fuse_regs;
	rot->base.ccs = ccs;
	rot->base.mfg_key = mfg_root_key;
	rot->base.root_slots = root_slots;
	rot->base.tenancy = tenancy_buffer;

	return hw_rot_hsp_rom_no_fuses_init_state (rot, root_key, hash, tenancy_counter);
}

/**
 * Initialize only the variable state for an HSP ROM RoT instance usable by run-time code.  The rest
 * of the RoT instance is assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param rot The RoT instance that contains the state to initialize.
 * @param root_key The root key that was used by ROM for verifying the running image.  If this is
 * null, there was no root key used.
 * @param hash Hash engine to use for root key matching.  This must be provided if the root key is
 * not null.
 * @param tenancy_counter Current value of the tenancy counter as reported by ROM.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int hw_rot_hsp_rom_no_fuses_init_state (const struct hw_rot_hsp_rom_no_fuses *rot,
	const SP_ECDSA_P384_PUBLIC *root_key, const struct hash_engine *hash,
	const union hw_rot_hsp_rom_tenancy_counter *tenancy_counter)
{
	SP_MSG_384 digest;
	int status;

	if ((rot == NULL) || (tenancy_counter == NULL) || (rot->base.state == NULL) ||
		(rot->base.fuses == NULL) || (rot->base.fuse_regs == NULL) || (rot->base.ccs == NULL) ||
		(rot->base.root_slots == 0)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	if ((root_key != NULL) && (hash == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	if (!sram_is_buffer_in_shared_sram (rot->base.tenancy, sizeof (*rot->base.tenancy))) {
		return HW_ROT_ADDRESS_NOT_SUPPORTED;
	}

	memset (rot->base.state, 0, sizeof (struct hw_rot_hsp_rom_state));

	if (root_key) {
		status = hash->calculate_sha384 (hash, root_key->AsBytes, SP_ECDSA_P384_PUBLIC_KEY_SIZE,
			digest.AsBytes, SP_MSG_384_SIZE);
		if (status != 0) {
			return status;
		}

		/* Loop through each root key fuse slot to find the matching digest. */
		rot->base.state->current_slot = 0;
		do {
			/* Determine the register address of the next root key slot. */
			rot->base.state->root_key = rot->base.fuse_regs->SW0_ecc_fuse.SW0_ecc_fuse +
				IN_DWORDS (sizeof (rot->base.fuse_regs->SW0_ecc_fuse) *
				rot->base.state->current_slot);

			status = buffer_compare_dwords ((uint32_t*) rot->base.state->root_key, digest.AsUINT32s,
				IN_DWORDS (SP_MSG_384_SIZE));
		} while ((status != 0) && (++rot->base.state->current_slot < rot->base.root_slots));

		/* If there was no match in the fused keys, check the manufacturing key. */
		if ((status != 0) && rot->base.mfg_key) {
			status = buffer_compare_dwords (rot->base.mfg_key->AsUINT32s, digest.AsUINT32s,
				IN_DWORDS (SP_MSG_384_SIZE));
			if (status == 0) {
				rot->base.state->root_key = rot->base.mfg_key->AsUINT32s;
				rot->base.state->current_slot = -1;
			}
		}

		if (status != 0) {
			/* The root key could not be determined. */
			return HW_ROT_ROOT_KEY_LOAD_FAILURE;
		}
	}

	/* Copy the tenancy counter to the RoT tenancy buffer for use with grant tokens. */
	memcpy (rot->base.tenancy->tenancy_counter.bytes, tenancy_counter->bytes,
		sizeof (tenancy_counter->bytes));
	hw_rot_hsp_rom_init_tenancy_state (&rot->base);

	return 0;
}

/**
 * Release the resources used by the HSP ROM RoT.
 *
 * @param rot The RoT instance to release.
 */
void hw_rot_hsp_rom_no_fuses_release (const struct hw_rot_hsp_rom_no_fuses *rot)
{
	UNUSED (rot);
}
