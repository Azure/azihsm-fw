// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef FIPS_COMMANDS_H_
#define FIPS_COMMANDS_H_

#include "acvp/acvp_proto_interface.h"
#include "cmd_interface/cmd_background.h"
#include "fips/cmvp_test_interface.h"
#include "fips/fips_self_test_interface.h"
#include "msft_protocol/msft_mctp_protocol.h"


/**
 * Version of the FIPS protocol provided by this implementation.
 */
#define	FIPS_PROTOCOL_VERSION			0


/**
 * Command codes for the FIPS MSFT command set.
 */
enum {
	FIPS_CMD_INIT_ACVP_TEST = 0x00,				/**< Initialize ACVP (Automated Cryptographic Validation Protocol) test. */
	FIPS_CMD_ACVP_TEST = 0x01,					/**< Send ACVP test data and get length of test results. */
	FIPS_CMD_GET_ACVP_TEST_RESULTS = 0x02,		/**< Get ACVP test results. */
	FIPS_CMD_CMVP_TEST_CASE = 0x03,				/**< Trigger a test case for CMVP certification testing. */
	FIPS_CMD_ON_DEMAND_SELF_TEST = 0x04,		/**< Execute an on-demand self-test of all cryptographic implementations. */
	FIPS_CMD_ON_DEMAND_SELF_TEST_RESULT = 0x05,	/**< Retrieve the result of an on-demand self-test. */
};

/**
 * ACVP test status codes.
 */
enum {
	FIPS_ACVP_TEST_PENDING_DATA = 0,	/**< ACVP test execution is pending additional test data. */
	FIPS_ACVP_TEST_COMPLETE = 1,		/**< Device has completed the ACVP test. */
};

/**
 * Values to use for reporting validity of an on-demand self-test result.
 */
enum {
	FIPS_ON_DEMAND_SELF_TEST_RESULT_NOT_RUN = 0,	/**< No self-test has been requested with the ID. */
	FIPS_ON_DEMAND_SELF_TEST_RESULT_NOT_READY = 1,	/**< The self-test has not completed execution yet. */
	FIPS_ON_DEMAND_SELF_TEST_RESULT_VALID = 2,		/**< The self-test has completed with the provided result. */
};


#pragma pack(push, 1)

/**
 * A request to retrieve the ACVP test status.
 */
struct fips_init_acvp_test_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint32_t total_size;						/**< Total ACVP test data size. */
};

/**
 * The response containing the ACVP test initialization status.
 */
struct fips_init_acvp_test_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
};

/**
 * A request to write ACVP test data to the device.
 */
struct fips_acvp_test_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint8_t is_last_data;						/**< Indication if this is the last chunk of test input data. */
	uint32_t offset;							/**< Offset to write the test data to. */
	uint8_t test_data;							/**< First byte of the ACVP test data. */
};

/**
 * The ACVP test response containing the test status and length of the generated test results.
 */
struct fips_acvp_test_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint32_t test_status;								/**< ACVP test status. */
	uint32_t test_result_length;						/**< Total length of the ACVP test result data.  This value is only valid when test_status == FIPS_ACVP_TEST_COMPLETE. */
};


/**
 * Get the length of the ACVP test data contained in the request.
 *
 * @param len Total length of the ACVP test request message.
 *
 * @return Length of the ACVP test data within the message.
 */
#define	fips_acvp_test_get_test_data_length(len)      \
	(len - (sizeof (struct fips_acvp_test_request) - sizeof (uint8_t)))

/**
 * Get the total length of an ACVP test request.
 *
 * @param len Length of the test data in the request.
 *
 * @return Total length of the request.
 */
#define	fips_acvp_test_request_length(len)       \
	((sizeof (struct fips_acvp_test_request) - sizeof (uint8_t)) + len)

/**
 * A request to retrieve ACVP test results.
 */
struct fips_get_acvp_test_results_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint32_t offset;							/**< Offset to read the test results data from. */
};

/**
 * The response containing ACVP test results.
 */
struct fips_get_acvp_test_results_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint8_t test_result;								/**< First byte of the ACVP test result data. */
};


/**
 * Get the length of the ACVP test result data contained in the response.
 *
 * @param len Total length of the get ACVP test results response message.
 *
 * @return Length of the ACVP test result data within the message.
 */
#define	fips_get_acvp_test_results_data_length(len)      \
	(len - (sizeof (struct fips_get_acvp_test_results_response) - sizeof (uint8_t)))

/**
 * Get the total length of a get ACVP test results response.
 *
 * @param len Length of the ACVP test result data in the response.
 *
 * @return Total length of the response.
 */
#define	fips_get_acvp_test_results_response_length(len)       \
	((sizeof (struct fips_get_acvp_test_results_response) - sizeof (uint8_t)) + len)

/**
 * Get the maximum length allowed for the ACVP test result data.
 *
 * @param max The maximum response length allowed by the message.
 *
 * @return Maximum ACVP test result data length that will fit in the response.
 */
#define	fips_get_acvp_test_results_max_data_length(max)	fips_get_acvp_test_results_data_length (max)

/**
 * A request to trigger a test case for CMVP certification testing.
 */
struct fips_cmvp_test_case_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint32_t test_id;							/**< Identifier for the test case to execute. */
	uint8_t reset_device;						/**< Reset the device after handling the command. */
};

/**
 * The response when triggering a test case for CMVP certification testing.
 */
struct fips_cmvp_test_case_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
};

/**
 * A request to execution on-demand self-tests of all cryptographic implementations used by the
 * module.
 */
struct fips_on_demand_self_test_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
};

/**
 * The response for on-demand self-test execution.  The self-tests will be run asynchronously, so
 * this response does not mean that test execution has completed.
 */
struct fips_on_demand_self_test_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint16_t execution_time;							/**< Amount of time to wait before asking for the result. */
	uint32_t execution_id;								/**< Identifier for this self-test execution. */
};

/**
 * A request for the result of a scheduled on-demand self-test execution.
 */
struct fips_on_demand_self_test_result_request {
	struct msft_mctp_protocol_header header;	/**< MSFT protocol header. */
	uint32_t execution_id;						/**< Identifier for the self-test execution to query. */
	uint8_t clear_on_read;						/**< Flag to clear the self-test result. */
};

/**
 * The response containing the result of an on-demand self-test execution.
 */
struct fips_on_demand_self_test_result_response {
	struct msft_mctp_protocol_response_header header;	/**< MSFT protocol header. */
	uint8_t valid_result;								/**< Indication if the result value is valid or not. */
	uint32_t result;									/**< Self-test result value. */
};

#pragma pack(pop)


int fips_init_acvp_test (const struct acvp_proto_interface *acvp_proto,
	struct cmd_interface_msg *request);
int fips_acvp_test (const struct acvp_proto_interface *acvp_proto,
	struct cmd_interface_msg *request);
int fips_get_acvp_test_results (const struct acvp_proto_interface *acvp_proto,
	struct cmd_interface_msg *request);

int fips_cmvp_test_case (const struct cmvp_test_interface *cmvp,
	const struct cmd_background *background, struct cmd_interface_msg *request);

int fips_on_demand_self_test (const struct fips_self_test_interface *fips,
	struct cmd_interface_msg *request);
int fips_on_demand_self_test_result (const struct fips_self_test_interface *fips,
	struct cmd_interface_msg *request);


#endif	/* FIPS_COMMANDS_H_ */
