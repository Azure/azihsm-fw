// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hsp_top.h"
#include "hw_rot_hsp_rom.h"
#include "common/buffer_util.h"
#include "common/unused.h"
#include "drivers/sram.h"
#include "logging/code_path_integrity.h"
#include "rom/device_keys.h"
#include "rom/rom_logging.h"
#include "splibs/inc/spcryptotypes.h"


/**
 * Determine the offset of the current redundant copy of the SVN.
 *
 * @param fuse Pointer to the fuse structure holding the SVN values.
 * @param x Index for the desired redundant copy.
 */
#define	HW_ROT_HSP_ROM_SVN_OFFSET(fuse, x)	((uint8_t*) &fuse->data[x].svn_1sp - (uint8_t*) fuse)


int hw_rot_hsp_rom_has_root_key (const struct hw_rot *rot)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;
	enum hsp_security_state sec_state;

	if (rom == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	sec_state = rom->fuses->get_security_state (rom->fuses);
	if ((sec_state == HSP_SECURITY_STATE_SECURE) ||
		((sec_state == HSP_SECURITY_STATE_PRODUCTION) && rom->state->root_key)) {
		return (rom->state->root_key) ? 0 : HW_ROT_NO_ROOT_KEY;
	}
	else {
		return HW_ROT_UNSUPPORTED;
	}
}

int hw_rot_hsp_rom_get_root_key_hash (const struct hw_rot *rot, const struct hash_engine *hash,
	enum hash_type type, uint8_t *digest, size_t length)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;
	size_t i;
	int status;

	if ((rom == NULL) || (hash == NULL) || (digest == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	if (type != HASH_TYPE_SHA384) {
		/* Only support reading the hash from fuses rather than calculating different hash types. */
		return HW_ROT_INVALID_ARGUMENT;
	}

	if (length < SHA384_HASH_LENGTH) {
		return HW_ROT_ROOT_HASH_TOO_LONG;
	}

	status = hw_rot_hsp_rom_has_root_key (rot);
	if (status != 0) {
		return status;
	}

	for (i = 0; i < IN_DWORDS (SHA384_HASH_LENGTH); i++) {
		((uint32_t*) digest)[i] = rom->state->root_key[i];
	}

	return 0;
}

int hw_rot_hsp_rom_verify_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;
	SP_MSG_384 root_key_hash;
	int status;

	if ((rom == NULL) || (root_key == NULL) || (hash == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	status = hw_rot_hsp_rom_has_root_key (rot);
	if (status != 0) {
		return status;
	}

	/* We are secure with a root key, so compare the hash. */
	status = hash->calculate_sha384 (hash, root_key, length, root_key_hash.AsBytes,
		SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	status = buffer_compare_dwords ((uint32_t*) rom->state->root_key, root_key_hash.AsUINT32s,
		IN_DWORDS (SP_MSG_384_SIZE));

	return (status == 0) ? 0 : HW_ROT_BAD_ROOT_KEY;
}

/**
 * Determine the address offset of the next root key slot.
 *
 * @param rom The ROM RoT to query.
 * @param next_slot Output for the next slot address offset.
 *
 * @return 0 if there is a root key slot available or an error code.
 */
static int hw_rot_hsp_rom_get_next_root_slot_offset (const struct hw_rot_hsp_rom *rom,
	uint16_t *next_slot)
{
	int status;

	/* If root keys aren't supported, there are no root key slots. */
	status = hw_rot_hsp_rom_has_root_key (&rom->base);
	if (status == HW_ROT_UNSUPPORTED) {
		return status;
	}

	/* If we are currently using the the last slot, there are no free slots available. */
	if (rom->state->current_slot == (rom->root_slots - 1)) {
		return HW_ROT_ROOT_KEYS_EXHAUSTED;
	}

	*next_slot = HSP_FUSES_SLOT_LENGTH (SW0_ECC) * (rom->state->current_slot + 1);

	return 0;
}

int hw_rot_hsp_rom_has_free_root_key_slots (const struct hw_rot *rot)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;
	uint16_t next_slot;
	int status;

	if (rom == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	status = hw_rot_hsp_rom_get_next_root_slot_offset (rom, &next_slot);
	if (status != 0) {
		return status;
	}

	/* Make sure the next slot is completely blank. */
	return rom->fuses->blank_check (rom->fuses, HSP_FUSES_ADDRESS (SW0_ECC) + next_slot,
		HSP_FUSES_LAST_WORD_ADDRESS (SW0_ECC) + next_slot, NULL);
}

int hw_rot_hsp_rom_update_root_key (const struct hw_rot *rot, const uint8_t *root_key,
	size_t length, const struct hash_engine *hash)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;
	uint16_t next_slot;
	SP_MSG_384 root_key_hash;
	int status;

	if ((rot == NULL) || (root_key == NULL) || (hash == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	status = hw_rot_hsp_rom_get_next_root_slot_offset (rom, &next_slot);
	if (status != 0) {
		return status;
	}

	/* Now that we know where the next slot is, program the root key hash into that slot. */
	status = hash->calculate_sha384 (hash, root_key, length, root_key_hash.AsBytes,
		SP_MSG_384_SIZE);
	if (status != 0) {
		return status;
	}

	status = rom->fuses->program_sw_fuses (rom->fuses, HSP_FUSES_ADDRESS (SW0_ECC) + next_slot,
		root_key_hash.AsUINT32s, IN_DWORDS (SP_MSG_384_SIZE));
	if (status != 0) {
		return status;
	}

	rom->state->current_slot++;
	if (!rom->state->root_key) {
		rom->state->root_key = rom->fuse_regs->SW0_ecc_fuse.SW0_ecc_fuse;
	}
	else {
		rom->state->root_key += IN_DWORDS (sizeof (rom->fuse_regs->SW0_ecc_fuse));
	}

	return 0;
}

int hw_rot_hsp_rom_get_svn (const struct hw_rot *rot, uint64_t *svn)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;
	struct hsp_fuses_sw0 *sw0;
	size_t i;

	if ((rom == NULL) || (svn == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	sw0 = (struct hsp_fuses_sw0*) rom->fuse_regs->SW0_fuse.SW0_fuse;
	*svn = 0;

	/* Read all redundant copies of the SVN data and set all bits. */
	for (i = 0; i < (sizeof (sw0->data) / sizeof (sw0->data[0])); i++) {
		*svn |= sw0->data[i].svn_1sp;
	}

	return 0;
}

int hw_rot_hsp_rom_get_svn_bits (const struct hw_rot *rot)
{
	if (rot) {
#ifndef BUILD_FOR_SIMULATION

		return sizeof (((struct hsp_fuses_sw0_data*) (0))->svn_1sp) * 8;
#else

		/* Reduce the number of valid SVN bits in simulation to reduce the number of KDFs executed
		 * on the hash stick key. */
		return 5;
#endif
	}
	else {
		return 0;
	}
}

int hw_rot_hsp_rom_update_svn (const struct hw_rot *rot, uint64_t svn)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;
	struct hsp_fuses_sw0 *sw0;
	size_t i;
	int status;

	if (rom == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	status = hw_rot_hsp_rom_has_root_key (rot);
	if (status != 0) {
		return status;
	}

	sw0 = (struct hsp_fuses_sw0*) rom->fuse_regs->SW0_fuse.SW0_fuse;

	/* Program all redundant copies of the SVN.  Do not fail on an error for a single copy. */
	status = -1;
	for (i = 0; i < (sizeof (sw0->data) / sizeof (sw0->data[0])); i++) {
		int prog_status = rom->fuses->program_sw_fuses (rom->fuses,
			HSP_FUSES_ADDRESS (SW0) + HW_ROT_HSP_ROM_SVN_OFFSET (sw0, i), (uint32_t*) &svn, 1);

		if (status != 0) {
			/* Latch successful program operations, or report the last failure. */
			status = prog_status;
		}
	}

	return status;
}

int hw_rot_hsp_rom_refresh_svn (const struct hw_rot *rot, uint64_t svn)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;
	struct hsp_fuses_sw0 *sw0;
	size_t i;
	int status = 0;

	if (rom == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	status = hw_rot_hsp_rom_has_root_key (rot);
	if (status != 0) {
		return (svn == 0) ? 0 : status;
	}

	sw0 = (struct hsp_fuses_sw0*) rom->fuse_regs->SW0_fuse.SW0_fuse;

	for (i = 0; i < (sizeof (sw0->data) / sizeof (sw0->data[0])); i++) {
		if (sw0->data[i].svn_1sp != svn) {
			int prog_status = rom->fuses->program_sw_fuses (rom->fuses,
				HSP_FUSES_ADDRESS (SW0) + HW_ROT_HSP_ROM_SVN_OFFSET (sw0, i), (uint32_t*) &svn, 1);

			if (prog_status != 0) {
				status = prog_status;
			}
		}
	}

	return status;
}

int hw_rot_hsp_rom_has_active_tenancy (const struct hw_rot *rot)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;

	if (rom == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	return rom->state->active_tenancy;
}

int hw_rot_hsp_rom_get_tenancy_counter (const struct hw_rot *rot, uint8_t *counter, size_t length)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;

	if ((rom == NULL) || (counter == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	if (length < HW_ROT_TENANCY_COUNTER_LENGTH) {
		return HW_ROT_TENANCY_COUNTER_TOO_LONG;
	}

	memcpy (counter, rom->tenancy->tenancy_counter.bytes, HW_ROT_TENANCY_COUNTER_LENGTH);

	return HW_ROT_TENANCY_COUNTER_LENGTH;
}

int hw_rot_hsp_rom_get_tenancy_grant_token (const struct hw_rot *rot, const uint8_t *tenant_key,
	size_t key_length, uint8_t *token, size_t length)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;
	size_t i;
	int status;

	if ((rom == NULL) || (tenant_key == NULL) || (token == NULL)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	if (key_length != SP_ECDSA_P384_PUBLIC_KEY_SIZE) {
		return HW_ROT_UNSUPPORTED_TENANT_KEY;
	}

	if (length < SP_MSG_384_SIZE) {
		return HW_ROT_GRANT_TOKEN_TOO_LONG;
	}

	status = hw_rot_hsp_rom_has_root_key (rot);
	if (status != 0) {
		return status;
	}

	for (i = 0; i < IN_DWORDS (sizeof (rom->fuse_regs->SW0_ecc_fuse)); i++) {
		rom->tenancy->owner_key_hash.AsUINT32s[i] = rom->state->root_key[i];
	}
	memcpy (rom->tenancy->tenant_signing_key.AsBytes, tenant_key, SP_ECDSA_P384_PUBLIC_KEY_SIZE);

	if (!rom->state->active_tenancy) {
		/* When there is no active tenancy, we need update the counter as if there was before we
		 * we can generate the token. */
		if (rom->state->tenancy_mask == 0) {
			/* If the mask is 0, that means we've run out of bits to change.  Once that happens, we
			 * can no longer support issuing tenancy grants. */
			return HW_ROT_TENANCY_EXHAUSTED;
		}

		rom->tenancy->tenancy_counter.dwords[rom->state->tenancy_word] |= rom->state->tenancy_mask;
	}

	status = rom->ccs->hmac (rom->ccs, DEVICE_KEYS_TENANCY_KEY, (uint8_t*) rom->tenancy,
		sizeof (struct hw_rot_hsp_rom_tenancy_buffer), (SP_MSG_384*) token, NULL);

	/* Revert any temporary change to the tenancy counter. */
	rom->tenancy->tenancy_counter.dwords[rom->state->tenancy_word] &= rom->state->tenancy_mask - 1;

	return status;
}

int hw_rot_hsp_rom_get_tenancy_grant_token_code_tracing (const struct hw_rot *rot,
	const uint8_t *tenant_key, size_t key_length, uint8_t *token, size_t length)
{
	code_path_integrity_message_trace (ROM_LOGGING_TRACE_TENANCY_GRANT);

	return hw_rot_hsp_rom_get_tenancy_grant_token (rot, tenant_key, key_length, token, length);
}

int hw_rot_hsp_rom_tenancy_transfer (const struct hw_rot *rot, bool grant)
{
	const struct hw_rot_hsp_rom *rom = (const struct hw_rot_hsp_rom*) rot;
	uint32_t updated;
	int status;

	if (rot == NULL) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	status = hw_rot_hsp_rom_has_root_key (rot);
	if (status != 0) {
		return status;
	}

	if (grant == rom->state->active_tenancy) {
		/* The expected tenancy state is already represented in the counter.  Don't need to do
		 * anything. */
		return 0;
	}

	if (rom->state->tenancy_mask == 0) {
		return HW_ROT_TENANCY_EXHAUSTED;
	}

	updated = rom->tenancy->tenancy_counter.dwords[rom->state->tenancy_word];
	updated |= rom->state->tenancy_mask;

	status = rom->fuses->program_sw_fuses (rom->fuses,
		rom->tenancy_addr + (rom->state->tenancy_word * sizeof (uint32_t)), &updated, 1);
	if (status == 0) {
		rom->tenancy->tenancy_counter.dwords[rom->state->tenancy_word] = updated;
		rom->state->active_tenancy = !rom->state->active_tenancy;

		rom->state->tenancy_mask <<= 1;
		if (rom->state->tenancy_mask == 0) {
			/* We've reach the end of the current dword. */
			rom->state->tenancy_word++;
			if (rom->state->tenancy_word < IN_DWORDS (HW_ROT_TENANCY_COUNTER_LENGTH)) {
				/* We have another word available in the counter, so set the mask for the first
				 * bit. */
				rom->state->tenancy_mask = 0x01;
			}
		}
	}

	return status;
}

/**
 * Initialize the RoT state in the SECURE security state.  This will determine the current root key
 * for firmware verification and the tenancy counter.
 *
 * @param rot The RoT state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
static int hw_rot_hsp_rom_init_secure_state (const struct hw_rot_hsp_rom *rot)
{
	uint64_t load_status = rot->fuse_regs->Fuse_Load_Status.Fuse_Load_Status[0] |
		(((uint64_t) rot->fuse_regs->Fuse_Load_Status.Fuse_Load_Status[1]) << 32);
	int i = rot->root_slots;
	int status;

	/* Find the active root key slot by looking for the largest slot number that has been
	 * programmed with a key. */
	while ((rot->state->root_key == NULL) && (i > 0)) {
		uint32_t last_addr =
			HSP_FUSES_ADDRESS (SW0_ECC) + (HSP_FUSES_SLOT_LENGTH (SW0_ECC) * i) - 4;

		/* Check the last address of the slot to see if has been programmed.  If it has not, the
		 * slot is not valid.  This makes the assumption that fuse words are programmed sequentially
		 * from the first address of the slot.  We don't want to blank check the entire slot because
		 * a partially programmed slot would then be viewed as containing a valid root key. */
		status = rot->fuses->blank_check (rot->fuses, last_addr, last_addr, NULL);
		if (status == FUSE_CONTROLLER_NOT_BLANK) {
			rot->state->current_slot = i - 1;
			rot->state->root_key = rot->fuse_regs->SW0_ecc_fuse.SW0_ecc_fuse +
				IN_DWORDS (sizeof (rot->fuse_regs->SW0_ecc_fuse) * rot->state->current_slot);

			/* Two bits per slot status.  Bit 0 -> Busy, Bit 1 -> Pass.
			 * SW0_ECC load status is at bit 26.
			 *
			 * Check to make sure the active root key slot loaded successfully. */
			if ((load_status & (0x3ull << (26 + (2 * rot->state->current_slot)))) !=
					(0x2ull << (26 + (2 * rot->state->current_slot)))) {
				return HW_ROT_ROOT_KEY_LOAD_FAILURE;
			}
		}
		else if (status != 0) {
			return status;
		}

		i--;
	}

	/* No root key has been programmed into fuses.  Use the manufacturing root key. */
	if (rot->state->root_key == NULL) {
		rot->state->root_key = rot->mfg_key->AsUINT32s;
	}

	/* Load the current tenancy counter from fuses and determine the tenancy state. */
	status = rot->fuses->read_sw_fuses (rot->fuses, rot->tenancy_addr,
		rot->tenancy->tenancy_counter.bytes, HW_ROT_TENANCY_COUNTER_LENGTH);
	if (status != 0) {
		return status;
	}

	hw_rot_hsp_rom_init_tenancy_state (rot);

	return 0;
}

/**
 * Initialize the tenancy state based on the current tenancy counter.  The tenancy counter must
 * already be present in the tenancy buffer.
 *
 * @param rot The RoT instance to initialize.
 */
void hw_rot_hsp_rom_init_tenancy_state (const struct hw_rot_hsp_rom *rot)
{
	uint32_t counter_dword;

	/* We need to loop through the counter looking for the first unset bit.  If this is found in an
	 * odd numbered bit position, there is an active tenancy. */
	do {
		counter_dword = rot->tenancy->tenancy_counter.dwords[rot->state->tenancy_word];
		rot->state->tenancy_mask = 0x01;

		while (((counter_dword & rot->state->tenancy_mask) != 0) &&
			(rot->state->tenancy_mask != 0)) {
			rot->state->active_tenancy = !rot->state->active_tenancy;
			rot->state->tenancy_mask <<= 1;
		}

		if (rot->state->tenancy_mask == 0) {
			rot->state->tenancy_word++;
		}
	} while ((rot->state->tenancy_word < IN_DWORDS (HW_ROT_TENANCY_COUNTER_LENGTH)) &&
		(rot->state->tenancy_mask == 0));
}

/**
 * Initialize the RoT interface for HSP ROM.
 *
 * @param rot The RoT instance to initialize.
 * @param state Variable context for the RoT.  This must not already be initialized.
 * @param fuses Driver interface to the HSP fuses where security information is stored.
 * @param fuse_regs Register interface to the fuse controller.  This contains cached information
 * for most fuse slots.
 * @param ccs Driver interface for executing crypto operations with HW managed keys.
 * @param mfg_root_key Optional hash of the root key that should be used for firmware verification
 * when there are no other keys programmed into fuses.  If supplied, this will get used in both
 * Production and Secure states.  Setting this to null will disable root key operations until a key
 * is fused.
 * @param root_slots The number of SW ECC fuse slots that are used to store root keys.  The first
 * slot is assumed to be SW0_ECC.
 * @param tenancy_buffer Buffer to use for storage of the tenancy counter.
 * @param tenancy_addr Starting fuse address of the tenancy counter.
 *
 * @return 0 if the RoT interface was initialized successfully or an error code.
 */
int hw_rot_hsp_rom_init (struct hw_rot_hsp_rom *rot, struct hw_rot_hsp_rom_state *state,
	const struct fuse_controller_interface *fuses, struct Gfc_regs *fuse_regs,
	const struct ccs_ksu_interface *ccs, const SP_MSG_384 *mfg_root_key, uint8_t root_slots,
	struct hw_rot_hsp_rom_tenancy_buffer *tenancy_buffer, uint16_t tenancy_addr)
{
	if ((rot == NULL) || (state == NULL) || (fuses == NULL) || (fuse_regs == NULL) ||
		(ccs == NULL) || (root_slots == 0)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	memset (rot, 0, sizeof (struct hw_rot_hsp_rom));

	rot->base.has_root_key = hw_rot_hsp_rom_has_root_key;
	rot->base.get_root_key_hash = hw_rot_hsp_rom_get_root_key_hash;
	rot->base.verify_root_key = hw_rot_hsp_rom_verify_root_key;
	rot->base.has_free_root_key_slots = hw_rot_hsp_rom_has_free_root_key_slots;
	rot->base.update_root_key = hw_rot_hsp_rom_update_root_key;
	rot->base.get_svn = hw_rot_hsp_rom_get_svn;
	rot->base.get_svn_bits = hw_rot_hsp_rom_get_svn_bits;
	rot->base.update_svn = hw_rot_hsp_rom_update_svn;
	rot->base.refresh_svn = hw_rot_hsp_rom_refresh_svn;
	rot->base.has_active_tenancy = hw_rot_hsp_rom_has_active_tenancy;
	rot->base.get_tenancy_counter = hw_rot_hsp_rom_get_tenancy_counter;
	rot->base.get_tenancy_grant_token = hw_rot_hsp_rom_get_tenancy_grant_token;
	rot->base.tenancy_transfer = hw_rot_hsp_rom_tenancy_transfer;

	rot->state = state;
	rot->fuses = fuses;
	rot->fuse_regs = fuse_regs;
	rot->ccs = ccs;
	rot->mfg_key = mfg_root_key;
	rot->root_slots = root_slots;
	rot->tenancy = tenancy_buffer;
	rot->tenancy_addr = tenancy_addr;

	return hw_rot_hsp_rom_init_state (rot);
}

/**
 * Initialize only the variable state for an HSP ROM RoT instance.  The rest of the RoT instance is
 * assumed to have already been initialized.
 *
 * This would generally be used with a statically initialized instance.
 *
 * @param rot The RoT instance that contains the state to initialize.
 *
 * @return 0 if the state was successfully initialized or an error code.
 */
int hw_rot_hsp_rom_init_state (const struct hw_rot_hsp_rom *rot)
{
	enum hsp_security_state sec_state;

	if ((rot == NULL) || (rot->state == NULL) || (rot->fuses == NULL) || (rot->fuse_regs == NULL) ||
		(rot->ccs == NULL) || (rot->root_slots == 0)) {
		return HW_ROT_INVALID_ARGUMENT;
	}

	if (rot->tenancy_addr & 0x3) {
		/* The tenancy flash address needs to be 32-bit aligned for fuse programming operations. */
		return HW_ROT_ADDRESS_NOT_SUPPORTED;
	}

	if (!sram_is_buffer_in_shared_sram (rot->tenancy, sizeof (*rot->tenancy))) {
		return HW_ROT_ADDRESS_NOT_SUPPORTED;
	}

	memset (rot->state, 0, sizeof (struct hw_rot_hsp_rom_state));
	memset (rot->tenancy, 0, sizeof (*rot->tenancy));

	rot->state->current_slot = -1;

	sec_state = rot->fuses->get_security_state (rot->fuses);
	if ((sec_state == HSP_SECURITY_STATE_SECURE) ||
		((sec_state == HSP_SECURITY_STATE_PRODUCTION) && rot->mfg_key)) {
		/* The device is in the Secure state or uses a default manufacturing key in Production
		 * state.  Full security features are available and the current configuration needs to be
		 * determined. */
		return hw_rot_hsp_rom_init_secure_state (rot);
	}
	else {
		return 0;
	}
}

/**
 * Release the resources used by the HSP ROM RoT.
 *
 * @param rot The RoT instance to release.
 */
void hw_rot_hsp_rom_release (const struct hw_rot_hsp_rom *rot)
{
	UNUSED (rot);
}

/**
 * Get the slot index for the current owner key.
 *
 * @param rot The RoT instance to query.
 *
 * @return An index for the owner key slot currently being used.  If there is no owner key, this
 * will be -1.
 */
int hw_rot_hsp_rom_get_root_key_slot (const struct hw_rot_hsp_rom *rot)
{
	if (rot && rot->state->root_key) {
		return rot->state->current_slot;
	}

	return -1;
}
