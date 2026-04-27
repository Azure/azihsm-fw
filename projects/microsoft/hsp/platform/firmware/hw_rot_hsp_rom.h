// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HW_ROT_HSP_ROM_H_
#define HW_ROT_HSP_ROM_H_

#include <stdbool.h>
#include "drivers/ccs_ksu_interface.h"
#include "drivers/fuse_controller_interface.h"
#include "drivers/hsp_fuses.h"
#include "firmware/hw_rot.h"
#include "splibs/inc/spcryptotypes.h"


/* Configurable RoT parameters.  Defaults can be overridden in platform_config.h. */
#include "platform_config.h"
#ifndef HW_ROT_TENANCY_COUNTER_LENGTH
#define	HW_ROT_TENANCY_COUNTER_LENGTH		HSP_FUSES_LENGTH (RSVD0)
#endif

#if HW_ROT_TENANCY_COUNTER_LENGTH & 0x3
#error "The tenancy counter length must align to 32-bit words."
#endif


/**
 * Container for easy management of the tenancy counter.
 */
union hw_rot_hsp_rom_tenancy_counter {
	uint8_t bytes[HW_ROT_TENANCY_COUNTER_LENGTH];				/**< Byte access to the tenancy counter. */
	uint32_t dwords[IN_DWORDS (HW_ROT_TENANCY_COUNTER_LENGTH)];	/**< Fuse word access to the tenancy counter. */
};

/**
 * Buffer that will be used for managing tenancies and generating grant tokens.
 *
 * This buffer must reside in HSP shared SRAM.
 */
struct hw_rot_hsp_rom_tenancy_buffer {
	SP_MSG_384 owner_key_hash;								/**< Hash of the current owner public key. */
	union hw_rot_hsp_rom_tenancy_counter tenancy_counter;	/**< The tenancy counter value. */
	SP_ECDSA_P384_PUBLIC tenant_signing_key;				/**< The public key being granted tenancy. */
};

/**
 * Variable context for the HSP ROM RoT.
 */
struct hw_rot_hsp_rom_state {
	const volatile uint32_t *root_key;	/**< The current root key used for secure boot. */
	int current_slot;					/**< The active root key slot. */
	bool active_tenancy;				/**< Flag if the tenancy counter value indicates an active tenancy. */
	size_t tenancy_word;				/**< Current dword in the tenancy counter that is active. */
	uint32_t tenancy_mask;				/**< Mask for the next modification to the tenancy counter. */
};

/**
 * RoT implementation for HSP ROM using HSP fuses for security configuration.
 */
struct hw_rot_hsp_rom {
	struct hw_rot base;								/**< Base RoT instance. */
	struct hw_rot_hsp_rom_state *state;				/**< Variable context for the RoT. */
	const struct fuse_controller_interface *fuses;	/**< Interface to the HSP fuses. */
	struct Gfc_regs *fuse_regs;						/**< Register interface for the HSP fuses. */
	const struct ccs_ksu_interface *ccs;			/**< Driver for crypto operations using HW keys. */
	const SP_MSG_384 *mfg_key;						/**< The root key to use when no keys have been fused yet. */
	uint8_t root_slots;								/**< The number of SW_ECC slots that are used for root keys. */
	struct hw_rot_hsp_rom_tenancy_buffer *tenancy;	/**< Buffer to store the current tenancy counter value. */
	uint16_t tenancy_addr;							/**< Base fuse address of the tenancy counter. */
};


int hw_rot_hsp_rom_init (struct hw_rot_hsp_rom *rot, struct hw_rot_hsp_rom_state *state,
	const struct fuse_controller_interface *fuses, struct Gfc_regs *fuse_regs,
	const struct ccs_ksu_interface *ccs, const SP_MSG_384 *mfg_root_key, uint8_t root_slots,
	struct hw_rot_hsp_rom_tenancy_buffer *tenancy_buffer, uint16_t tenancy_addr);
int hw_rot_hsp_rom_init_state (const struct hw_rot_hsp_rom *rot);
void hw_rot_hsp_rom_release (const struct hw_rot_hsp_rom *rot);

int hw_rot_hsp_rom_get_root_key_slot (const struct hw_rot_hsp_rom *rot);

/* Internal functions for use by derived types. */
void hw_rot_hsp_rom_init_tenancy_state (const struct hw_rot_hsp_rom *rot);


#endif	/* HW_ROT_HSP_ROM_H_ */
