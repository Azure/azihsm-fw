// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CRYPTO_HW_H_
#define CRYPTO_HW_H_

#include <stdbool.h>
#include <stdint.h>
#include "platform_api.h"


struct Creg_regs_creg_crypto_group;	/* Defined in HSP register definition. */


/**
 * Helper macro to get the bit mask for crypto IRQ registers.
 *
 * @param reg The register name.
 * @param bit The value name in the register.
 */
#define	CRYPTO_HW_IRQ_BIT_MASK(reg, bit)    \
	CREG_REGS_CREG_CRYPTO_GROUP_ ## reg ## _ ## bit ## _FIELD_MASK


/**
 * Callback prototype for converting hardware status bits to a status code.
 *
 * @param status The raw status bits from the hardware register.
 * @param error_code Error to return when the the command failure bit is set.
 *
 * @return 0 if the command completed successfully or the error code for command execution.
 */
typedef int (*crypto_hw_parse_status_callback) (uint32_t status, int error_code);


bool crypto_hw_handle_interrupt (struct Creg_regs_creg_crypto_group *irq, uint32_t irq_mask,
	platform_semaphore *done);
int crypto_hw_submit_command_interrupt (struct Creg_regs_creg_crypto_group *irq, uint32_t irq_mask,
	platform_semaphore *done, void *cmd_ptr, volatile uint32_t *cmd_reg,
	volatile uint32_t *status_reg, uint32_t busy_mask,
	crypto_hw_parse_status_callback parse_command_status, int error_code, int timeout_code);

int crypto_hw_submit_command_polling (void *cmd_ptr, volatile uint32_t *cmd_reg,
	volatile uint32_t *status_reg, uint32_t busy_mask,
	crypto_hw_parse_status_callback parse_command_status, int error_code, int timeout_code);


#endif	/* CRYPTO_HW_H_ */
