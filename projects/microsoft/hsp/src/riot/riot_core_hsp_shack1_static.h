// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef RIOT_CORE_HSP_SHACK1_STATIC_H_
#define RIOT_CORE_HSP_SHACK1_STATIC_H_

#include "riot_core_hsp_static.h"


/* Internal functions declared to allow for static initialization. */
int riot_core_hsp_shack1_generate_device_id (const struct riot_core *riot, const uint8_t *cdi,
	size_t length);
int riot_core_hsp_shack1_generate_alias_key (const struct riot_core *riot, const uint8_t *fwid,
	size_t length);
int riot_core_hsp_shack1_get_alias_key (const struct riot_core *riot, uint8_t **key,
	size_t *length);


/**
 * Constant initializer for the RIoT core API.
 */
#define	RIOT_CORE_HSP_SHACK1_API_INIT  { \
		.generate_device_id = riot_core_hsp_shack1_generate_device_id, \
		.get_device_id_csr = riot_core_hsp_get_device_id_csr, \
		.get_device_id_cert = riot_core_hsp_get_device_id_cert, \
		.generate_alias_key = riot_core_hsp_shack1_generate_alias_key, \
		.get_alias_key = riot_core_hsp_shack1_get_alias_key, \
		.get_alias_key_cert = riot_core_hsp_get_alias_key_cert, \
	}


/**
 * Initialize a static instance for HSP DICE layer 0 handling.  This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for DICE handling.
 * @param ccs_ptr Driver for the CCS instance that contains the CDI and Device ID.
 * @param base64_ptr The base64 encoding engine to use with identity certificate generation.
 * @param x509_ptr The X.509 engine to use for identity certificate generation.
 * @param hash_ptr The hash engine to use for identity certificate generation.
 * @param cdi_slot The KSU slot that contains the DICE CDI.
 * @param device_id_slot The KSU slot that contains the Device ID key.
 * @param alias_slot The KSU slot that will be used for the Alias key.
 * @param device_id_ext_ptr A list of additional, custom extensions that should be added to the
 * Device ID certificate and CSR.  At minimum, this should include the DICE TcbInfo extension for
 * layer 0.
 * @param device_id_ext_cnt The number of custom extensions to add to the Device ID certificate
 * and CSR.
 * @param alias_ext_ptr A list of additional, custom extensions that should be added to the
 * Alias certificate.  At minimum, this should include the DICE TcbInfo extension for layer 1.
 * @param alias_ext_cnt The number of custom extensions to add to the Alias certificate.
 */
#define	riot_core_hsp_shack1_static_init(state_ptr, ccs_ptr, base64_ptr, x509_ptr, hash_ptr, \
	cdi_slot, device_id_slot, alias_slot, device_id_ext_ptr, device_id_ext_cnt, alias_ext_ptr, \
	alias_ext_cnt)	{ \
		.riot.base = RIOT_CORE_HSP_SHACK1_API_INIT, \
		.riot.state = state_ptr, \
		.riot.ccs = ccs_ptr, \
		.riot.base64 = base64_ptr, \
		.riot.x509 = x509_ptr, \
		.riot.dev_id_ext = device_id_ext_ptr, \
		.riot.dev_id_ext_count = device_id_ext_cnt, \
		.riot.dev_id_pathlen = 0, \
		.riot.alias_ext = alias_ext_ptr, \
		.riot.alias_ext_count = alias_ext_cnt, \
		.riot.alias_pathlen = -1, \
		.riot.cdi = cdi_slot, \
		.riot.device_id_key = device_id_slot, \
		.riot.alias_key = alias_slot, \
		.hash = hash_ptr \
	}


#endif	/* RIOT_CORE_HSP_SHACK1_STATIC_H_ */
