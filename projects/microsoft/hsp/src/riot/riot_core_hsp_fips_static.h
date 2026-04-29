// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef RIOT_CORE_HSP_FIPS_STATIC_H_
#define RIOT_CORE_HSP_FIPS_STATIC_H_

#include "riot_core_hsp_fips.h"
#include "riot_core_hsp_static.h"


/* Internal functions declared to allow for static initialization. */
int riot_core_hsp_fips_generate_alias_key (const struct riot_core *riot, const uint8_t *fwid,
	size_t length);


/**
 * Constant initializer for the RIoT core API.
 */
#define	RIOT_CORE_HSP_FIPS_API_INIT  { \
		.generate_device_id = riot_core_hsp_generate_device_id, \
		.get_device_id_csr = riot_core_hsp_get_device_id_csr, \
		.get_device_id_cert = riot_core_hsp_get_device_id_cert, \
		.generate_alias_key = riot_core_hsp_fips_generate_alias_key, \
		.get_alias_key = riot_core_hsp_get_alias_key, \
		.get_alias_key_cert = riot_core_hsp_get_alias_key_cert, \
	}


/**
 * Initialize a static instance for HSP DICE layer 0 handling that is FIPS compliant.  This
 * implementation assumes the Device ID key has already been generated, presumably by ROM, and is
 * present in the KSU along with the CDI.  This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for DICE handling.
 * @param ccs_ptr Driver for the CCS instance that contains the CDI and Device ID.  This must be a
 * FIPS compliant implementation.
 * @param ecc_ptr Driver for the PKA instance that will be used for PCT.
 * @param hash_ptr Hash engine that will be used for PCT.
 * @param base64_ptr The base64 encoding engine to use with identity certificate generation.
 * @param x509_ptr The X.509 engine to use for identity certificate generation.
 * @param cdi_slot The KSU slot that contains the DICE CDI.
 * @param device_id_slot The KSU slot that contains the Device ID key.
 * @param alias_slot The KSU slot that will be used for the Alias key.
 * @param device_id_ext_ptr A list of additional, custom extensions that should be added to the
 * Device ID certificate and CSR.  At minimum, this should include the DICE TcbInfo extension for
 * layer 0.
 * @param device_id_ext_cnt The number of custom extensions to add to the Device ID certificate
 * and CSR.
 * @param device_id_pathlen The path length to use for the Device ID CA certificate.  Use a value
 * larger than X509_CERT_MAX_PATHLEN to indicate the certificate should be generated without any
 * path length constraint.
 * @param alias_ext_ptr A list of additional, custom extensions that should be added to the
 * Alias certificate.  At minimum, this should include the DICE TcbInfo extension for layer 1.
 * @param alias_ext_cnt The number of custom extensions to add to the Alias certificate.
 */
#define	riot_core_hsp_fips_static_init(state_ptr, ccs_ptr, ecc_ptr, hash_ptr, base64_ptr, \
	x509_ptr, cdi_slot, device_id_slot, alias_slot, device_id_ext_ptr, device_id_ext_cnt, \
	device_id_pathlen, alias_ext_ptr, alias_ext_cnt)	{ \
		.base = riot_core_hsp_static_init_with_api (RIOT_CORE_HSP_FIPS_API_INIT, \
			&(state_ptr)->base, ccs_ptr, base64_ptr, x509_ptr, cdi_slot, device_id_slot, \
			alias_slot, device_id_ext_ptr, device_id_ext_cnt, device_id_pathlen, alias_ext_ptr, \
			alias_ext_cnt, -1), \
		.state = state_ptr, \
		.ecc = ecc_ptr, \
		.hash = hash_ptr, \
	}

/**
 * Initialize a static instance for HSP DICE layer 0 handling that is FIPS compliant.  This
 * implementation assumes the Device ID key has already been generated, presumably by ROM, and is
 * present in the KSU along with the CDI.  This can be a constant instance.
 *
 * The Alias Certificate that is generated will be a CA certificate.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for DICE handling.
 * @param ccs_ptr Driver for the CCS instance that contains the CDI and Device ID.  This must be a
 * FIPS compliant implementation.
 * @param ecc_ptr Driver for the PKA instance that will be used for PCT.
 * @param hash_ptr Hash engine that will be used for PCT.
 * @param base64_ptr The base64 encoding engine to use with identity certificate generation.
 * @param x509_ptr The X.509 engine to use for identity certificate generation.
 * @param cdi_slot The KSU slot that contains the DICE CDI.
 * @param device_id_slot The KSU slot that contains the Device ID key.
 * @param alias_slot The KSU slot that will be used for the Alias key.
 * @param device_id_ext_ptr A list of additional, custom extensions that should be added to the
 * Device ID certificate and CSR.  At minimum, this should include the DICE TcbInfo extension for
 * layer 0.
 * @param device_id_ext_cnt The number of custom extensions to add to the Device ID certificate
 * and CSR.
 * @param device_id_pathlen The path length to use for the Device ID CA certificate.  Use a value
 * larger than X509_CERT_MAX_PATHLEN to indicate the certificate should be generated without any
 * path length constraint.  This value must be at least 1 to account for the Alias CA certificate.
 * @param alias_ext_ptr A list of additional, custom extensions that should be added to the
 * Alias certificate.  At minimum, this should include the DICE TcbInfo extension for layer 1.
 * @param alias_ext_cnt The number of custom extensions to add to the Alias certificate.
 * @param alias_pathlen_arg The path length to use for the Alias CA certificate.  Use a value
 * larger than X509_CERT_MAX_PATHLEN to indicate the certificate should be generated without any
 * path length constraint.
 */
#define	riot_core_hsp_fips_static_init_alias_ca(state_ptr, ccs_ptr, ecc_ptr, hash_ptr, base64_ptr, \
	x509_ptr, cdi_slot, device_id_slot, alias_slot, device_id_ext_ptr, device_id_ext_cnt, \
	device_id_pathlen, alias_ext_ptr, alias_ext_cnt, alias_pathlen_arg)	{ \
		.base = riot_core_hsp_static_init_with_api (RIOT_CORE_HSP_FIPS_API_INIT, \
			&(state_ptr)->base, ccs_ptr, base64_ptr, x509_ptr, cdi_slot, device_id_slot, \
			alias_slot, device_id_ext_ptr, device_id_ext_cnt, device_id_pathlen, alias_ext_ptr, \
			alias_ext_cnt, alias_pathlen_arg), \
		.state = state_ptr, \
		.ecc = ecc_ptr, \
		.hash = hash_ptr, \
	}


#endif	/* RIOT_CORE_HSP_FIPS_STATIC_H_ */
