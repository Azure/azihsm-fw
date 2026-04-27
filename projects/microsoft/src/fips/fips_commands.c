// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "fips_commands.h"
#include "common/buffer_util.h"
#include "msft_protocol/cmd_interface_msft.h"


/**
 * Initialize an ACVP (Automated Cryptographic Validation Program) test.
 *
 * @param acvp_proto The ACVP Proto interface for initializing the test.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if the test was successfully initialized or an error code.
 */
int fips_init_acvp_test (const struct acvp_proto_interface *acvp_proto,
	struct cmd_interface_msg *request)
{
	struct fips_init_acvp_test_request *req;
	struct fips_init_acvp_test_response *resp;
	int status;

	if ((acvp_proto == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (struct fips_init_acvp_test_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != FIPS_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct fips_init_acvp_test_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = acvp_proto->init_test (acvp_proto, buffer_unaligned_read32 (&req->total_size));
	if (status != 0) {
		return status;
	}

	resp = (struct fips_init_acvp_test_response*) request->payload;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));

	resp->header.completion_code = 0;

	return 0;
}

/**
 * Write ACVP (Automated Cryptographic Validation Program) test data and execute the ACVP test once
 * all test data has been written.
 *
 * @param acvp_proto The ACVP Proto interface for executing the test.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int fips_acvp_test (const struct acvp_proto_interface *acvp_proto,
	struct cmd_interface_msg *request)
{
	struct fips_acvp_test_request *req;
	struct fips_acvp_test_response *resp;
	size_t result_length;
	int status;

	if ((acvp_proto == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (struct fips_acvp_test_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != FIPS_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length < sizeof (struct fips_acvp_test_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	resp = (struct fips_acvp_test_response*) request->payload;

	status = acvp_proto->add_test_data (acvp_proto, buffer_unaligned_read32 (&req->offset),
		&req->test_data, fips_acvp_test_get_test_data_length (request->payload_length));
	if (status != 0) {
		return status;
	}
	else if (req->is_last_data) {
		/* All test data has been sent; execute the test. */
		status = acvp_proto->execute_test (acvp_proto, &result_length);
		if (status != 0) {
			return status;
		}

		buffer_unaligned_write32 (&resp->test_status, FIPS_ACVP_TEST_COMPLETE);
		buffer_unaligned_write32 (&resp->test_result_length, result_length);
	}
	else {
		/* More test data is to be sent. */
		buffer_unaligned_write32 (&resp->test_status, FIPS_ACVP_TEST_PENDING_DATA);
		buffer_unaligned_write32 (&resp->test_result_length, 0);
	}

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));

	resp->header.completion_code = 0;

	return 0;
}

/**
 * Get the results of an ACVP (Automated Cryptographic Validation Program) test.
 *
 * @param acvp_proto The ACVP Proto interface for getting the test results.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if the test results were successfully retrieved or an error code.
 */
int fips_get_acvp_test_results (const struct acvp_proto_interface *acvp_proto,
	struct cmd_interface_msg *request)
{
	struct fips_get_acvp_test_results_request *req;
	struct fips_get_acvp_test_results_response *resp;
	size_t max_response;
	size_t out_length;
	int status;

	if ((acvp_proto == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (struct fips_get_acvp_test_results_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != FIPS_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length < sizeof (struct fips_get_acvp_test_results_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	resp = (struct fips_get_acvp_test_results_response*) request->payload;

	max_response = cmd_interface_msg_get_max_response (request);
	if (max_response < sizeof (struct fips_get_acvp_test_results_response)) {
		return CMD_HANDLER_MSFT_BAD_MAX_RESPONSE;
	}

	status = acvp_proto->get_test_results (acvp_proto, buffer_unaligned_read32 (&req->offset),
		&resp->test_result, fips_get_acvp_test_results_max_data_length (max_response), &out_length);
	if (status != 0) {
		return status;
	}

	cmd_interface_msg_set_message_payload_length (request,
		fips_get_acvp_test_results_response_length (out_length));

	resp->header.completion_code = 0;

	return 0;
}

/**
 * Execute a specific test case for CMVP certification testing.
 *
 * @param cmvp The CMVP test interface that will handle test execution.
 * @param background Background task to use for device resets.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int fips_cmvp_test_case (const struct cmvp_test_interface *cmvp,
	const struct cmd_background *background, struct cmd_interface_msg *request)
{
	struct fips_cmvp_test_case_request *req;
	struct fips_cmvp_test_case_response *resp;
	int status;

	if ((cmvp == NULL) || (background == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (struct fips_cmvp_test_case_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != FIPS_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct fips_cmvp_test_case_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = cmvp->trigger_test_case (cmvp, buffer_unaligned_read32 (&req->test_id));
	if (status != 0) {
		return status;
	}

	if (req->reset_device) {
		status = background->reboot_device (background);
		if (status != 0) {
			return status;
		}
	}

	resp = (struct fips_cmvp_test_case_response*) request->payload;

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Initiate a FIPS self-test of all cryptographic implementations used by the module.  Test
 * execution may occur asynchronously in a separate context.
 *
 * @param fips The handler for FIPS self-tests.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int fips_on_demand_self_test (const struct fips_self_test_interface *fips,
	struct cmd_interface_msg *request)
{
	struct fips_on_demand_self_test_request *req;
	struct fips_on_demand_self_test_response *resp;
	uint32_t execution_id;
	uint16_t execution_time;
	int status;

	if ((fips == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (struct fips_on_demand_self_test_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != FIPS_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct fips_on_demand_self_test_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	status = fips->execute_on_demand_self_test (fips, &execution_id, &execution_time);
	if (status != 0) {
		return status;
	}

	resp = (struct fips_on_demand_self_test_response*) request->payload;

	buffer_unaligned_write32 (&resp->execution_id, execution_id);
	buffer_unaligned_write16 (&resp->execution_time, execution_time);

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}

/**
 * Retrieve the result of an on-demand FIPS self-test execution.
 *
 * @param fips The handler for FIPS self-tests.
 * @param request The message containing the request.  This will be updated with the necessary
 * response.
 *
 * @return 0 if a response was successfully generated or an error code.  A valid response could be
 * an error message.
 */
int fips_on_demand_self_test_result (const struct fips_self_test_interface *fips,
	struct cmd_interface_msg *request)
{
	struct fips_on_demand_self_test_result_request *req;
	struct fips_on_demand_self_test_result_response *resp;
	uint32_t execution_id;
	uint8_t valid_result = FIPS_ON_DEMAND_SELF_TEST_RESULT_VALID;
	uint32_t result;
	int status;

	if ((fips == NULL) || (request == NULL)) {
		return CMD_HANDLER_MSFT_INVALID_ARGUMENT;
	}

	req = (struct fips_on_demand_self_test_result_request*) request->payload;

	if (buffer_unaligned_read16 (&req->header.protocol_version) != FIPS_PROTOCOL_VERSION) {
		return CMD_HANDLER_MSFT_INCOMPATIBLE;
	}

	if (request->payload_length != sizeof (struct fips_on_demand_self_test_result_request)) {
		return CMD_HANDLER_MSFT_BAD_LENGTH;
	}

	execution_id = buffer_unaligned_read32 (&req->execution_id);

	status = fips->get_on_demand_self_test_result (fips, execution_id, &result);
	if (status == 0) {
		/* After retrieving the self-test result, optionally free the execution ID. */
		if (req->clear_on_read != 0) {
			status = fips->clear_on_demand_self_test_result (fips, execution_id);
			if (status != 0) {
				return status;
			}
		}
	}
	else {
		/* If there was an error getting the result, check for conditions that can be reported
		 * through the standard response structure. */
		switch (status) {
			case FIPS_SELF_TEST_UNKNOWN_ID:
				valid_result = FIPS_ON_DEMAND_SELF_TEST_RESULT_NOT_RUN;
				break;

			case FIPS_SELF_TEST_RESULT_NOT_READY:
				valid_result = FIPS_ON_DEMAND_SELF_TEST_RESULT_NOT_READY;
				break;

			default:
				return status;
		}

		result = 0xffffffff;
	}

	resp = (struct fips_on_demand_self_test_result_response*) request->payload;

	resp->valid_result = valid_result;
	buffer_unaligned_write32 (&resp->result, result);

	cmd_interface_msg_set_message_payload_length (request, sizeof (*resp));
	resp->header.completion_code = 0;

	return 0;
}
