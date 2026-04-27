// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CMVP_TEST_INTERFACE_H_
#define CMVP_TEST_INTERFACE_H_

#include <stdint.h>
#include "status/msft_module_id.h"


/**
 * Interface to provide test hooks for firmware to trigger specific validation scenarios that are
 * necessary to support FIPS CMVP certification.
 */
struct cmvp_test_interface {
	/**
	 * Trigger execution of a specific test case within the module.  The tests that are supported
	 * and how they identified are device-specific.
	 *
	 * @param cmvp The CMVP test manager.
	 * @param test_id Identifier for the test case to execute.
	 *
	 * @return 0 if the test case was successfully initiated or an error code.  Successful return
	 * from this function does not mean the test case has executed.  Behavior induced by the test
	 * case would typically be inspected through other means.
	 */
	int (*trigger_test_case) (const struct cmvp_test_interface *cmvp, uint32_t test_id);
};


#define	CMVP_TESTING_ERROR(code)		ROT_ERROR (MSFT_MODULE_CMVP_TESTING, code)

/**
 * Error codes that can be generated during CMVP test handling.
 */
enum {
	CMVP_TESTING_INVALID_ARGUMENT = CMVP_TESTING_ERROR (0x00),		/**< Input parameter is null or not valid. */
	CMVP_TESTING_NO_MEMORY = CMVP_TESTING_ERROR (0x01),				/**< Memory allocation failed. */
	CMVP_TESTING_TRIGGER_TEST_FAILED = CMVP_TESTING_ERROR (0x02),	/**< Failed to trigger a CMVP test case. */
	CMVP_TESTING_EXECUTION_FAILED = CMVP_TESTING_ERROR (0x03),		/**< Execution of a test failed. */
};


#endif	/* CMVP_TEST_INTERFACE_H_ */
