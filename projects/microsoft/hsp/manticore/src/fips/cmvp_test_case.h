// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMVP_TEST_CASE_H_
#define CMVP_TEST_CASE_H_


/**
 * Identifiers for the core that should execute a specified CMVP test case.
 */
enum cmvp_test_case_core_id {
	CMVP_TEST_CASE_CORE_ID_HSP = 0,		/**< A CMVP test is for HSP. */
	CMVP_TEST_CASE_CORE_ID_ADMIN = 1,	/**< A CMVP test is for CP Admin. */
	CMVP_TEST_CASE_CORE_ID_HSM = 2,		/**< A CMVP test is for CP HSM. */
	CMVP_TEST_CASE_CORE_ID_FP0 = 3,		/**< A CMVP test is for FP0. */
	CMVP_TEST_CASE_CORE_ID_FP1 = 4,		/**< A CMVP test is for FP1. */
	CMVP_TEST_CASE_CORE_ID_FP2 = 5,		/**< A CMVP test is for FP2. */
	CMVP_TEST_CASE_CORE_ID_UNKNOWN,		/**< The ID does not map to a known CPU core. */
};


/**
 * Retrieve the core identifier from a CMVP test identifier.
 *
 * @param test_id The 32-bit CMVP test ID.
 *
 * @return The ID for the core that will execute the test case.
 */
#define	cmvp_test_case_get_core_id(test_id)     \
	((enum cmvp_test_case_core_id) (((test_id) >> 24) & 0x7))

/**
 * Indicate that the test case should attempt to mimic POR flows without issuing a SoC reset.
 *
 * @param test_id The 32-bit CMVP test ID.
 *
 * @return True if the device should execute workflows as if the reset was a SoC reset.
 */
#define	cmvp_test_case_mimic_por(test_id)				((test_id) & (1U << 31))


/**
 * Indication if the test case should be executed by 1SP or SPRT.
 */
enum cmvp_test_case_boot_stage {
	CMVP_TEST_CASE_BOOT_STAGE_1SP = 0,	/**< A CMVP test will execute during 1SP. */
	CMVP_TEST_CASE_BOOT_STAGE_SPRT = 1,	/**< A CMVP test will execute during SPRT. */
};


/**
 * Retrieve the boot stage targeted for a specific CMVP test.
 *
 * @param test_id The 32-bit CMVP test ID.
 *
 * @return The SP boot stage that will execute the test case.
 */
#define	cmvp_test_case_get_boot_stage(test_id)  \
	((enum cmvp_test_case_boot_stage) (!!((test_id) & (1U << 23))))


/**
 * Identifiers for the type of test case being executed. This value determines the format of
 * the other bits in the CMVP test ID used to identify a specific test case.
 */
enum cmvp_test_case_test_type {
	CMVP_TEST_CASE_NONE = 0,				/**< A test that does not belong to any test type */
	CMVP_TEST_CASE_CAST_NEGATIVE_TEST = 1,	/**< A test to trigger a CAST negative self-test */
	CMVP_TEST_CASE_RAS_ERROR_INJ_TEST = 2,	/**< A test for RAS error injection on-demand test */
	CMVP_TEST_CASE_ZEROIZATION = 3,			/**< A test for memory zeroization. */
};


/**
 * Retrieve the test type of a specific CMVP test.
 *
 * @param test_id The 32-bit CMVP test ID.
 *
 * @return The type of test being executed.
 */
#define	cmvp_test_case_get_test_type(test_id)           \
	((enum cmvp_test_case_test_type) ((test_id >> 20) & 0x7))


/**
 * Identifier for the algorithm implementation being tested.
 */
enum cmvp_test_case_cast_algorithm {
	CMVP_TEST_CASE_ALGORITHM_AES_ENCRYPT_HW = 0,		/**< AES encrypt implementation using HSP hardware. */
	CMVP_TEST_CASE_ALGORITHM_AES_DECRYPT_HW = 1,		/**< AES decrypt implementation using HSP hardware. */
	CMVP_TEST_CASE_ALGORITHM_AES_ENCRYPT_SW = 2,		/**< AES encrypt implementation using mbedTLS. */
	CMVP_TEST_CASE_ALGORITHM_AES_DECRYPT_SW = 3,		/**< AES decrypt implementation using mbedTLS. */
	CMVP_TEST_CASE_ALGORITHM_AES_KWP_WRAP_SW = 4,		/**< AES key wrap with padding, implemented in SW. */
	CMVP_TEST_CASE_ALGORITHM_AES_KWP_UNWRAP_SW = 5,		/**< AES key unwrap with padding, implemented in SW. */
	CMVP_TEST_CASE_ALGORITHM_SHA_HW = 6,				/**< SHA implementation using only HSP hardware. */
	CMVP_TEST_CASE_ALGORITHM_SHA_HW_SW = 7,				/**< SHA implementation using a combination HSP HW and SW. */
	CMVP_TEST_CASE_ALGORITHM_SHA_DRBG = 8,				/**< SHA implementation in the HSP hardware DRBG. */
	CMVP_TEST_CASE_ALGORITHM_HMAC_HW = 9,				/**< HMAC implementation in HSP CCS hardware. */
	CMVP_TEST_CASE_ALGORITHM_HMAC_SW = 10,				/**< HMAC implementation in software. */
	CMVP_TEST_CASE_ALGORITHM_HMAC_DRBG = 11,			/**< HMAC implementation in the HSP hardware DRBG. */
	CMVP_TEST_CASE_ALGORITHM_KBKDF_HW = 12,				/**< KBKDF implementation in HSP CCS hardware. */
	CMVP_TEST_CASE_ALGORITHM_KBKDF_SW = 13,				/**< KBKDF implementation in software. */
	CMVP_TEST_CASE_ALGORITHM_HKDF_SW = 14,				/**< HKDF implementation in software. */
	CMVP_TEST_CASE_ALGORITHM_DRBG_INSTANTIATE_HW = 15,	/**< DRBG instantiate using HSP hardware. */
	CMVP_TEST_CASE_ALGORITHM_DRBG_RESEED_HW = 16,		/**< DRBG reseed using HSP hardware. */
	CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_ROM = 17,		/**< ECDSA verify implementation used by ROM for 1SP. */
	CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_PKA = 18,		/**< ECDSA verify implementation directly using PKA.  */
	CMVP_TEST_CASE_ALGORITHM_ECDSA_VERIFY_SW = 19,		/**< ECDSA verify implementation using the SW API. */
	CMVP_TEST_CASE_ALGORITHM_ECDSA_SIGN_PKA = 20,		/**< ECDSA sign implementation directly using PKA. */
	CMVP_TEST_CASE_ALGORITHM_ECDSA_SIGN_SW = 21,		/**< ECDSA sign implementation using the SW API. */
	CMVP_TEST_CASE_ALGORITHM_ECDH_HW = 22,				/**< ECDH implementation using PKA hardware. */
	CMVP_TEST_CASE_ALGORITHM_RSA_SW = 23,				/**< RSA implementation using mbedTLS. */
	CMVP_TEST_CASE_ALGORITHM_ECDSA_KEYGEN_CCS = 24,		/**< ECDSA key generation using CCS. */
	CMVP_TEST_CASE_ALGORITHM_RNG_APT_HEALTH = 25,		/**< APT health test of the HSP hardware RNG. */
	CMVP_TEST_CASE_ALGORITHM_RNG_RCT_HEALTH = 26,		/**< RCT health test of the HSP hardware RNG. */
	CMVP_TEST_CASE_ALGORITHM_NONE = 32,					/**< No algorithm implementation. */
};

/**
 * Identifier for the type of algorithmic self-test being tested for CMVP verification.
 */
enum cmvp_test_case_cast_type {
	CMVP_TEST_CASE_CAST_TYPE_PRE_OPERATIONAL = 0,	/**< Test the pre-operational self-test. */
	CMVP_TEST_CASE_CAST_TYPE_PERIODIC = 1,			/**< Test the periodic self-test. */
	CMVP_TEST_CASE_CAST_TYPE_PCT = 2,				/**< Test the pairwise consistency test. */
	CMVP_TEST_CASE_CAST_TYPE_INTEGRITY_PHY0 = 3,	/**< Test the PHY0 firmware integrity test. */
	CMVP_TEST_CASE_CAST_TYPE_INTEGRITY_PHY1 = 4,	/**< Test the PHY1 firmware integrity test. */
	CMVP_TEST_CASE_CAST_TYPE_INTEGRITY_1SP = 5,		/**< Test the 1SP firmware integrity test. */
	CMVP_TEST_CASE_CAST_NONE = 16,					/**< No self-test. */
};


/**
 * Get the type of self-test being tested by the test case.
 *
 * @param test_id The 32-bit CMVP test ID.
 *
 * @return Type of self-test being tested.
 */
#define	cmvp_test_case_get_cast_type(test_id)           \
	((enum cmvp_test_case_cast_type) ((test_id) & 0xf))

/**
 * Get the algorithm implementation being tested by the CMVP test.
 *
 * @param test_id The 32-bit CMVP test ID.
 *
 * @return Algorithm implementation identifier for the test case.
 */
#define	cmvp_test_case_get_cast_algorithm(test_id)      \
	((enum cmvp_test_case_cast_algorithm) (((test_id) >> 4) & 0x1f))


/**
 * Identifier for the RAS HSP fault injection test case being tested.
 */
enum cmvp_test_case_get_ras_fault_inj_test {
	CMVP_TEST_CASE_RAS_HSP_FAULT_GSRAM_ECC_ERR = 0,		/**< RAS HSP GSRAM ECC error injection. */
	CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_BUS_ERR = 1,		/**< RAS HSP bus error injection. */
	CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_CHK_ERR = 2,		/**< RAS HSP check point error injection. */
	CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_DMB_ERR = 3,		/**< RAS HSP DMB error injection. */
	CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_MEM_ERR = 4,		/**< RAS HSP memory error injection. */
	CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_MPU_ERR = 5,		/**< RAS HSP MPU error injection. */
	CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_WDT_ERR = 6,		/**< RAS HSP watchdog timeout error injection. */
	CMVP_TEST_CASE_RAS_HSP_FAULT_INJ_AXI_WDT_ERR = 7,	/**< RAS HSP WDT error injection. */
	CMVP_TEST_CASE_RAS_HSP_FAULT_NONE = 32,				/**< No error injection. */
};


/**
 * Retrieve RAS HSP fault injection test case.
 *
 * @param test_id The 32-bit CMVP test ID.
 *
 * @return Error test ID is to inject an error type.
 */
#define cmvp_test_case_get_ras_fault_injection_test(test_id)      \
	((enum cmvp_test_case_get_ras_fault_inj_test) ((test_id) & 0x1f))


/**
 * Identifier for RAS HSP memory CREG fault injection test case
*/
enum ras_hsp_fault_injection_mem_err {
	RAS_HSP_FAULT_MEM_PKAR1_ERR_ADDR = 0,		/**< RAS HSP MEM_PKAR1 error injection register. */
	RAS_HSP_FAULT_MEM_PKAR2_ERR_ADDR = 1,		/**< RAS HSP MEM_PKAR2 error injection register. */
	RAS_HSP_FAULT_MEM_KEYSTR_ERR_ADDR = 2,		/**< RAS HSP MEM_KEYSTR error injection register. */
	RAS_HSP_FAULT_MEM_SHAREDRAM_ERR_ADDR = 3,	/**< RAS HSP MEM_SHAREDRAM error injection register. */
	RAS_HSP_FAULT_MEM_SPDRAM_ERR_ADDR = 4,		/**< RAS HSP MEM_SPDRAM error injection register. */
	RAS_HSP_FAULT_MEM_SPIRAM_ERR_ADDR = 5,		/**< RAS HSP MEM_SPIRAM error injection register. */
	RAS_HSP_FAULT_MEM_SPROM_ERR_ADDR = 6,		/**< RAS HSP MEM_SPROM error injection register. */
};


/**
 * Retrieve RAS HSP fault injection hardware register bit.
 *
 * @param test_id The 32-bit CMVP test ID.
 *
 * @return RAS HSP error injection hardware register bit to inject an error.
 */
#define cmvp_test_case_get_ras_fault_inj_hw_reg_bit_pos(test_id)  (((test_id) >> 5) & 0x7)


/**
 * Identifier for the zeroization function being tested for CMVP verification.
 */
enum cmvp_test_case_zeroization_type {
	CMVP_TEST_CASE_ZEROIZATION_TYPE_MBEDTLS_PLATFORM = 0,	/**< Test mbedtls_platform_zeroize. */
	CMVP_TEST_CASE_ZEROIZATION_NONE = 16,					/**< No zeroization test. */
};


/**
 * Get the zeroization function being tested by the test case.
 *
 * @param test_id The 32-bit CMVP test ID.
 *
 * @return Identifier for the zeroization function being tested.
 */
#define	cmvp_test_case_get_zeroization_type(test_id)           \
	((enum cmvp_test_case_zeroization_type) ((test_id) & 0xf))


#endif	/* CMVP_TEST_CASE_H_ */
