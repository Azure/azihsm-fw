// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_BOOTLOADER_STATIC_H_
#define MANTICORE_BOOTLOADER_STATIC_H_

#include "crypto/signature_verification_ecc_static.h"
#include "firmware/key_manifest_hsp_firmware_static.h"
#include "firmware/manticore_bootloader.h"


/**
 * Initialize a static instance of a bootloader for Manticore main firmware.
 *
 * There is no validation done on the arguments.
 *
 * @param boot_ptr The bootloader to initialize.
 * @param state_ptr Variable context for the firmware image.
 * @param flash_ptr The flash device that contains the firmware to load.
 * @param keys_ptr Storage for the firmware key manifest of the image.
 * @param hash_ptr Hash engine to use for firmware verification.
 * @param ecc_ptr ECC interface to use for verification of firmware signatures.
 * @param key_1sp DER encoded public key that was used to verify the 1SP firmware image.  This key
 * will be used to verify the firmware key manifest.  This can be a buffer that will be updated at
 * run-time to contain the authentication key.
 * @param key_length Length of the DER encoded 1SP authentication key.
 * @param rot_manifest_ptr RoT handler for the firmware key manifest.
 * @param security_ptr Manager for the device security policy that should be applied while loading
 * the firmware images.
 * @param sp_loader_ptr Handler for loading SPRT firmware images into SP memory.
 * @param cp_loader_ptr Handler for loading CP firmware images into CP memory.
 * @param fp0_loader_ptr Handler for loading FP core 0 firmware images into FP0 memory.
 * @param fp1_loader_ptr Handler for loading FP core 1 firmware images into FP1 memory.
 * @param fp2_loader_ptr Handler for loading FP core 2 firmware images into FP2 memory.
 * @param pcie_loader_ptr Handler for loading PCIe PHY firmware images into memory.
 */
#define	manticore_bootloader_static_init(boot_ptr, state_ptr, flash_ptr, keys_ptr, hash_ptr, \
	ecc_ptr, key_1sp, key_1sp_length, rot_manifest_ptr, security_ptr, sp_loader_ptr, \
	cp_loader_ptr, fp0_loader_ptr, fp1_loader_ptr, fp2_loader_ptr, pcie_loader_ptr)	{ \
		.state = state_ptr, \
		.flash =  flash_ptr, \
		.hash = hash_ptr, \
		.security = security_ptr, \
		.sp_loader = sp_loader_ptr, \
		.cp_loader = cp_loader_ptr, \
		.fp0_loader = fp0_loader_ptr, \
		.fp1_loader = fp1_loader_ptr, \
		.fp2_loader = fp2_loader_ptr, \
		.pcie_loader = pcie_loader_ptr, \
		.manifest = key_manifest_hsp_firmware_static_init (keys_ptr, rot_manifest_ptr, \
			security_ptr, &(boot_ptr)->verification.base, key_1sp, key_1sp_length, NULL, 0), \
		.verification = signature_verification_ecc_static_init (&(state_ptr)->verify_state, \
			ecc_ptr) \
	}


#endif	/* MANTICORE_BOOTLOADER_STATIC_H_ */
