// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "cmvp_test_case.h"
#include "cmvp_test_manticore.h"
#include "common/common_math.h"
#include "common/unused.h"
#include "mbedtls/platform_util.h"


int cmvp_test_manticore_trigger_test_case (const struct cmvp_test_interface *cmvp, uint32_t test_id)
{
	const struct cmvp_test_manticore *manticore = (const struct cmvp_test_manticore*) cmvp;
	uint32_t *soc_test_id;
	int status;

	if (cmvp == NULL) {
		return CMVP_TESTING_INVALID_ARGUMENT;
	}

	if ((cmvp_test_case_get_core_id (test_id) == CMVP_TEST_CASE_CORE_ID_HSP) &&
		(cmvp_test_case_get_boot_stage (test_id) == CMVP_TEST_CASE_BOOT_STAGE_SPRT) &&
		(cmvp_test_case_get_test_type (test_id) == CMVP_TEST_CASE_ZEROIZATION)) {
		/* Explicitly handle mbedtls zeroization test cases. */
		switch (cmvp_test_case_get_zeroization_type (test_id)) {
			case CMVP_TEST_CASE_ZEROIZATION_TYPE_MBEDTLS_PLATFORM: {
				uint8_t test_data[32];

				memset (test_data, 0xff, sizeof (test_data));

				if (common_math_is_array_zero (test_data, sizeof (test_data))) {
					return CMVP_TESTING_EXECUTION_FAILED;
				}

				mbedtls_platform_zeroize (test_data, sizeof (test_data));

				if (!common_math_is_array_zero (test_data, sizeof (test_data))) {
					return CMVP_TESTING_EXECUTION_FAILED;
				}

				break;
			}

			default:
				break;
		}
	}
	else {
		/* Otherwise, save the test case to be triggered later. */
		status = manticore->dmb->map_soc_address (manticore->dmb, manticore->soc_test_id_addr,
			sizeof (uint32_t), HSP_DMB_ACCESS_WRITE, (void**) &soc_test_id);
		if (status != 0) {
			return status;
		}

		/* Only store the test ID in local memory if the test applies to SPRT.  Otherwise, it will
		 * get loaded from SoC memory when appropriate. */
		if ((cmvp_test_case_get_core_id (test_id) == CMVP_TEST_CASE_CORE_ID_HSP) &&
			(cmvp_test_case_get_boot_stage (test_id) == CMVP_TEST_CASE_BOOT_STAGE_SPRT)) {
			*manticore->test_id = test_id;
		}

		*soc_test_id = test_id;

		manticore->dmb->unmap_soc_address (manticore->dmb, soc_test_id);
	}

	return 0;
}

/**
 * Initialize a CMVP test handler for Manticore.
 *
 * @param cmvp The CMVP test handler to initialize.
 * @param dmb DMB to use for accessing SoC addresses.
 * @param hsp_test_id Buffer to use for storing CMVP test identifiers.
 * @param soc_test_id SoC address to use for storing CMVP test identifiers.
 *
 * @return 0 if the CMVP test handler was initialized successfully or an error code.
 */
int cmvp_test_manticore_init (struct cmvp_test_manticore *cmvp, const struct hsp_dmb *dmb,
	uint32_t *hsp_test_id, uint64_t soc_test_id)
{
	if ((cmvp == NULL) || (dmb == NULL) || (hsp_test_id == NULL) || (soc_test_id == 0)) {
		return CMVP_TESTING_INVALID_ARGUMENT;
	}

	memset (cmvp, 0, sizeof (*cmvp));

	cmvp->base.trigger_test_case = cmvp_test_manticore_trigger_test_case;

	cmvp->dmb = dmb;
	cmvp->test_id = hsp_test_id;
	cmvp->soc_test_id_addr = soc_test_id;

	return 0;
}

/**
 * Release the resources used for CMVP test handling.
 *
 * @param cmvp The CMVP test handler to release.
 */
void cmvp_test_manticore_release (const struct cmvp_test_manticore *cmvp)
{
	UNUSED (cmvp);
}
