// Copyright (c) Microsoft Corporation. All rights reserved.

#include "hsp_rng_hw_kat.h"
#include "hsp_rng_hw_kat_vectors.h"
#include "hsp_top.h"
#include "common/buffer_util.h"
#include "drivers/hsp_rng_hw.h"


/**
 * Wait until a specific DRBG status bit is no longer asserted.
 *
 * @param regs Base register address for the RNG hardware to query.
 * @param bit Status bit to wait for deassertion.
 *
 * @return 0 if the bit was cleared or HSP_RNG_HW_SELF_TEST_FAULT if there was a fault in the DRBG.
 */
static int hsp_rng_hw_kat_wait_for_status_bit_clear (struct Rng_regs *regs, uint32_t bit)
{
	uint32_t rng_status;

	do {
		rng_status = regs->status;

		if (rng_status & RNG_REGS_STATUS_DRBG_FAULT_ERROR_FIELD_MASK) {
			return HSP_RNG_HW_SELF_TEST_FAULT;
		}
	} while (rng_status & bit);

	return 0;
}

/**
 * Wait until a specific DRBG status bit is asserted.
 *
 * @param regs Base register address for the RNG hardware to query.
 * @param bit Status bit to wait for assertion.
 *
 * @return 0 if the was set or HSP_RNG_HW_SELF_TEST_FAULT if there was a fault in the DRBG.
 */
static int hsp_rng_hw_kat_wait_for_status_bit_set (struct Rng_regs *regs, uint32_t bit)
{
	uint32_t rng_status;

	do {
		rng_status = regs->status;

		if (rng_status & RNG_REGS_STATUS_DRBG_FAULT_ERROR_FIELD_MASK) {
			return HSP_RNG_HW_SELF_TEST_FAULT;
		}
	} while (!(rng_status & bit));

	return 0;
}

/**
 * Wait until the DRBG has completed the instantiate operation in FW mode.  There must be enough
 * data in the FIFO for DRBG instantiate to complete or else this call will block indefinitely.
 *
 * @param regs Base register address for the RNG hardware to query.
 *
 * @return 0 if the instantiate operation completed or HSP_RNG_HW_SELF_TEST_FAULT if there was a
 * fault in the DRBG.
 */
static int hsp_rng_hw_kat_wait_for_drbg_instantiate (struct Rng_regs *regs)
{
	return hsp_rng_hw_kat_wait_for_status_bit_clear (regs,
		RNG_REGS_STATUS_DRBG_INST_BUSY_FIELD_MASK);
}

/**
 * Wait until the DRBG has completed the generate operation and there is data available to read from
 * the FWOUT FIFO.
 *
 * @param regs Base register address for the RNG hardware to query.
 *
 * @return 0 if there is data available to read or HSP_RNG_HW_SELF_TEST_FAULT if there was a fault
 * in the DRBG.
 */
int hsp_rng_hw_kat_wait_for_drbg_generate (struct Rng_regs *regs)
{
	return hsp_rng_hw_kat_wait_for_status_bit_clear (regs, RNG_REGS_STATUS_BUSY_FIELD_MASK);
}

/**
 * Wait until the DRBG FWOUT FIFO is empty and is waiting to reseed.
 *
 * @param regs Base register address for the RNG hardware to query.
 *
 * @return 0 if there is data available to read or HSP_RNG_HW_SELF_TEST_FAULT if there was a fault
 * in the DRBG.
 */
int hsp_rng_hw_kat_wait_for_reseed_ready (struct Rng_regs *regs)
{
	int status;

	status = hsp_rng_hw_kat_wait_for_status_bit_set (regs, RNG_REGS_STATUS_BUSY_FIELD_MASK);
	if (status != 0) {
		return status;
	}

	return hsp_rng_hw_kat_wait_for_status_bit_set (regs,
		RNG_REGS_STATUS_DRBG_RESEED_BUSY_FIELD_MASK);
}

/**
 * Wait until the DRBG has completed the reseed operation in FW mode.  There must be enough data in
 * the FIFO for DRBG reseed to complete or else this call will block indefinitely.
 *
 * @param regs Base register address for the RNG hardware to query.
 *
 * @return 0 if the reseed operation completed or HSP_RNG_HW_SELF_TEST_FAULT if there was a fault in
 * the DRBG.
 */
static int hsp_rng_hw_kat_wait_for_drbg_reseed (struct Rng_regs *regs)
{
	return hsp_rng_hw_kat_wait_for_status_bit_clear (regs,
		RNG_REGS_STATUS_DRBG_RESEED_BUSY_FIELD_MASK);
}

/**
 * Test the DRBG within the RNG hardware to ensure that it is functioning properly.  This needs to
 * execute an Instantiate-Reseed-Generate sequence of operations to satisfy FIPS requirements.
 *
 * @param rng The RNG device to test.
 *
 * @return 0 if the test was successful or an error code.
 */
int hsp_rng_hw_kat_run_self_test (const struct hsp_rng_hw *rng)
{
	uint32_t ctrl;
	uint32_t generate_interval;
	uint32_t reseed_interval;
	SP_MSG_512 output;
	size_t i;
	int status;

	if (rng == NULL) {
		return HSP_RNG_HW_INVALID_ARGUMENT;
	}

	platform_mutex_lock (&rng->state->lock);

	/* Before switching to FW mode, need to ensure the DRBG is not actively reading any entropy. */
	hsp_rng_hw_wait_for_entropy_read_done (rng);

	/* Save the current register values so they can be restored later. */
	generate_interval = rng->regs->generate_interval;
	reseed_interval = rng->regs->reseed_interval;

	/* Set the generate and reseed intervals to low values for the test. */
	rng->regs->generate_interval = 2;
	rng->regs->reseed_interval = 2;

	/* Put the DRBG into FW mode to give the test control over the generated data. */
	ctrl = rng->regs->ctrl;

	ctrl |= RNG_REGS_CTRL_ENABLE_FIELD_MASK | RNG_REGS_CTRL_FW_MODE_FIELD_MASK |
		RNG_REGS_CTRL_DRBG_INSTANTIATE_FIELD_MASK | RNG_REGS_CTRL_DRBG_GENERATE_FIELD_MASK;
	ctrl &= ~RNG_REGS_CTRL_DRBG_UNINSTANTIATE_FIELD_MASK;

	rng->regs->ctrl = ctrl;

	/* Fill the input FIFO.  Once it is full, the DRBG will start processing it to generate output.
	 * the busy status bit will indicate when that is done. */
	for (i = 0; i < IN_DWORDS (SP_MSG_512_SIZE); i++) {
		*((volatile uint32_t*) &rng->regs->FWIN_data) =
			RNG_HSP_HW_KAT_INSTANTIATE_INPUT.AsUINT32s[i];
	}

	status = hsp_rng_hw_kat_wait_for_drbg_instantiate (rng->regs);
	if (status != 0) {
		goto exit;
	}

	status = hsp_rng_hw_kat_wait_for_drbg_generate (rng->regs);
	if (status != 0) {
		goto exit;
	}

	/* Instantiate has completed, along with a Generate.  Drain the output FIFO to trigger a Reseed
	 * operation.  With generate_interval set to 2, 128 bytes of data needs to read from the output
	 * FIFO.  Check the second 64 bytes to ensure the expected value was generated. */
	for (i = 0; i < IN_DWORDS (SP_MSG_512_SIZE); i++) {
		output.AsUINT32s[i] = *((volatile uint32_t*) &rng->regs->FWOUT_data);
	}

	for (i = 0; i < IN_DWORDS (SP_MSG_512_SIZE); i++) {
		output.AsUINT32s[i] = *((volatile uint32_t*) &rng->regs->FWOUT_data);
	}

	status = buffer_compare (RNG_HSP_HW_KAT_INSTANTIATE_OUTPUT.AsBytes, output.AsBytes,
		SP_MSG_512_SIZE);
	if (status == BUFFER_UTIL_DATA_MISMATCH) {
		status = HSP_RNG_HW_SELF_TEST_FAILED;
		goto exit;
	}

	/* Wait for the HW to report that the FIFO is empty and is waiting for data to provided for the
	 * Reseed operation.  Once the HW is ready, push the reseed data into the input FIFO. */
	status = hsp_rng_hw_kat_wait_for_reseed_ready (rng->regs);
	if (status != 0) {
		goto exit;
	}

	for (i = 0; i < IN_DWORDS (SP_MSG_512_SIZE); i++) {
		*((volatile uint32_t*) &rng->regs->FWIN_data) = RNG_HSP_HW_KAT_RESEED_INPUT.AsUINT32s[i];
	}

	status = hsp_rng_hw_kat_wait_for_drbg_reseed (rng->regs);
	if (status != 0) {
		goto exit;
	}

	status = hsp_rng_hw_kat_wait_for_drbg_generate (rng->regs);
	if (status != 0) {
		goto exit;
	}

	/* Reseed has completed, along with a Generate.  Read the output FIFO and compare to the
	 * expected value. */
	for (i = 0; i < IN_DWORDS (SP_MSG_512_SIZE); i++) {
		output.AsUINT32s[i] = *((volatile uint32_t*) &rng->regs->FWOUT_data);
	}

	status = buffer_compare (RNG_HSP_HW_KAT_RESEED_OUTPUT.AsBytes, output.AsBytes, SP_MSG_512_SIZE);
	if (status == BUFFER_UTIL_DATA_MISMATCH) {
		status = HSP_RNG_HW_SELF_TEST_FAILED;
	}

exit:
	/* Restore the RNG HW to normal mode. */
	rng->regs->generate_interval = generate_interval;
	rng->regs->reseed_interval = reseed_interval;

	ctrl &= ~(RNG_REGS_CTRL_FW_MODE_FIELD_MASK | RNG_REGS_CTRL_DRBG_INSTANTIATE_FIELD_MASK |
		RNG_REGS_CTRL_DRBG_GENERATE_FIELD_MASK);
	rng->regs->ctrl = ctrl;

	platform_mutex_unlock (&rng->state->lock);

	return status;
}
