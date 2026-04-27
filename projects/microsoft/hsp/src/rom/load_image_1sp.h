// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef LOAD_IMAGE_1SP_H_
#define LOAD_IMAGE_1SP_H_

#include <stdbool.h>
#include <stdint.h>
#include "crypto/ecc_hw.h"
#include "crypto/hash.h"
#include "firmware/firmware_loader.h"
#include "firmware/hsp_fw_1sp.h"
#include "firmware/hw_rot.h"
#include "firmware/key_manifest_hsp_rom.h"
#include "flash/flash.h"
#include "splibs/inc/spchkptdefs.h"
#include "status/msft_module_id.h"


int load_image_1sp_from_flash (const struct flash *manifest_flash, const struct flash *img_flash,
	uint32_t base_addr, const struct ecc_hw *pka, const struct hash_engine *hash,
	const struct key_manifest_hsp_rom *manifest, const struct hsp_fw_1sp *image);
int load_image_1sp_from_memory (const uint8_t *base_addr, const struct ecc_hw *pka,
	const struct hash_engine *hash, const struct key_manifest_hsp_rom *manifest,
	const struct hsp_fw_1sp *image);

/**
 * Defines a pair of checkpoints that should be used when looking for an image on multiple flash
 * devices.
 *
 * When a slot fails verification, the current chain will be terminated and handed off to the chain
 * start.  On successful loads, the last active chain remains open.
 *
 * The chain will remain open for the last slot regardless of success or failure.
 */
struct load_image_1sp_chkpt {
	const HSP_CHKPT_CONFIG *start;	/**< The beginning of the checkpoint chain for validation. */
	const HSP_CHKPT_CONFIG *end;	/**< The end of the validation chain. */
};

/**
 * Defines a single flash location that can contain a 1SP firmware image.  All components that are
 * used for a firmware slot can be constant instances.
 *
 * It is possible that 3 different flash device interfaces can be involved in the firmware load
 * process:  1 for the key manifest, 1 for the 1SP image header, and 1 for the 1SP image data.
 * While it is highly unlikely that these three interfaces will map to different physical flash
 * devices, having this kind of flexibility allows for different permissions to be set in each phase
 * of the load process.
 */
struct load_image_1sp_fw_slot {
	const struct key_manifest_hsp_rom *manifest;	/**< The key manifest instance to use with this slot. */
	const struct flash *manifest_flash;				/**< The flash device for the key manifest in the image slot. */
	const struct hsp_fw_1sp *image;					/**< The 1SP image instance to use with this slot. */
	const struct flash *image_flash;				/**< Optional alternative flash for loading the 1SP image header. */
	uint32_t base_addr;								/**< Flash address where the image is located. */
	uint8_t fail_id;								/**< Failure identifier for the image slot. */
	uint32_t trace_msg;								/**< Execution tracing message to use for the slot. */
	const struct load_image_1sp_chkpt *chkpt;		/**< Optional checkpoint config to apply if the slot encounters an error. */
};


int load_image_1sp_from_multiple_flash (const struct load_image_1sp_fw_slot *slots, size_t count,
	const struct ecc_hw *pka, const struct hash_engine *hash, size_t *slot);

int load_image_1sp_update_rot (const struct key_manifest_hsp_rom *manifest,
	const struct hash_engine *hash, bool tenancy_only);


#define	LOAD_IMAGE_1SP_ERROR(code)		ROT_ERROR (MSFT_MODULE_LOAD_IMAGE_1SP, code)

/**
 * Error codes that can be generated during 1SP firmware image load and verification.
 */
enum {
	LOAD_IMAGE_1SP_INVALID_ARGUMENT = LOAD_IMAGE_1SP_ERROR (0x00),	/**< Input parameter is null or not valid. */
	LOAD_IMAGE_1SP_NO_MEMORY = LOAD_IMAGE_1SP_ERROR (0x01),			/**< Memory allocation failed. */
	LOAD_IMAGE_1SP_MANIFEST_REVOKED = LOAD_IMAGE_1SP_ERROR (0x02),	/**< The image manifest is not allowed to be used. */
	LOAD_IMAGE_1SP_NO_IMAGE_KEY = LOAD_IMAGE_1SP_ERROR (0x02),		/**< There is no key to use for image verification. */
	LOAD_IMAGE_1SP_LOAD_IMG_FAILED = LOAD_IMAGE_1SP_ERROR (0x03),	/**< Failed to load an image. */
};


#endif	/* LOAD_IMAGE_1SP_H_ */
