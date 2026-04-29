// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SELF_TEST_MANTICORE_STATIC_H_
#define SELF_TEST_MANTICORE_STATIC_H_

#include "self_test_manticore.h"


/* Internal functions declared to allow for static initialization. */
int self_test_manticore_run_self_test_first (const struct self_test_interface *self_test,
	struct debug_log_entry_info *error_info);
int self_test_manticore_run_self_test_second (const struct self_test_interface *self_test,
	struct debug_log_entry_info *error_info);


/**
 * Constant initializer for the self-test API for the first group of tests.
 */
#define	SELF_TEST_MANTICORE_FIRST_API_INIT  { \
		.run_self_test = self_test_manticore_run_self_test_first, \
	}

/**
 * Constant initializer for the self-test API for the second group of tests.
 */
#define	SELF_TEST_MANTICORE_SECOND_API_INIT  { \
		.run_self_test = self_test_manticore_run_self_test_second, \
	}


/**
 * Initialize a static instance for self-testing all Manticore cryptographic algorithms.  This can
 * be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param gcm_ptr An instance of the AES-GCM implementation to use for testing.  This must be a
 * separate instance from any other AES-GCM usage.
 * @param aes_kwp_ptr An instance of the AES key wrap with padding implementation to use for
 * testing.  This must be a separate instance from any other AES key wrap usage.
 * @param ccs_ptr Driver for the CCS to use for hardware algorithm testing.
 * @param key_slot_arg The KSU key slot to use for CCS algorithm testing.  This must not be used for
 * any other key.
 * @param hkdf_ptr An instance of the HKDF implementation to use for testing.  This must be a
 * separate instance from any other HKDF usage.
 * @param pka_ptr Driver for the PKA hardware to use for ROM ECDSA testing.
 * @param ecc_ptr An instance of the ECC implementation to use for ECDSA and ECDH testing.
 * @param hash_ptr An instance of the hash implementation to use for ECDSA testing.
 */
#define	self_test_manticore_static_init(gcm_ptr, aes_kwp_ptr, ccs_ptr, key_slot_arg, hkdf_ptr, \
	pka_ptr, ecc_ptr, hash_ptr)	{ \
		.base_first = SELF_TEST_MANTICORE_FIRST_API_INIT, \
		.base_second = SELF_TEST_MANTICORE_SECOND_API_INIT, \
		.gcm = gcm_ptr, \
		.aes_kwp = aes_kwp_ptr, \
		.ccs = ccs_ptr, \
		.key_slot = key_slot_arg, \
		.hkdf = hkdf_ptr, \
		.pka = pka_ptr, \
		.ecc = ecc_ptr, \
		.hash = hash_ptr, \
	}


#endif	/* SELF_TEST_MANTICORE_STATIC_H_ */
