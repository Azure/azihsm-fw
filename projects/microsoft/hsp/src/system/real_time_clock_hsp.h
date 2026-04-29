// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef REAL_TIME_CLOCK_HSP_H_
#define REAL_TIME_CLOCK_HSP_H_

#include <stdbool.h>
#include "platform_api.h"
#include "common/clock_utils.h"
#include "system/real_time_clock.h"


/**
 * The magic number is used to set the intrusion tamper register and
 * verify the number read from the intrusion tamper register.
 */
#define REAL_TIME_CLOCK_HSP_INTRUSION_MAGIC_NUMBER 0xBEAF

/* Define bit masks for HSP RTC INTSTS and INTEN registers. */
#define REAL_TIME_CLOCK_HSP_INTSTS_INTRUSION_MASK		0x00000002ul	/**< Intrusion interrupt status register intrusion bit. */
#define REAL_TIME_CLOCK_HSP_INTSTS_ALARM_MASK			0x00000001ul	/**< Intrusion interrupt status register alarm bit. */
#define REAL_TIME_CLOCK_HSP_INTEN_INTRUSION_MASK		0x00000002ul	/**< Intrusion interrupt enable register intrusion bit. */

/* Defined in HSP register definition. */
struct Creg_regs_creg_rtc_group;

/**
 * Variable context for settable HSP real time clock.
 */
struct real_time_clock_hsp_state {
	platform_mutex lock;				/**< Synchronization for RTC operations. */
	platform_clock set_time_cooldown;	/**< Timeout for the cooldown between set_time calls. */
	bool cooldown_valid;				/**< Flag to indicate if the cooldown timeout is initialized. */
};

/**
 * Real time clock interface for HSP RTC hardware.
 */
struct real_time_clock_hsp {
	struct real_time_clock base;	/**< The base real time clock interface. */

	/**
	 * Gets intrusion count.
	 *
	 * @param rtc The real time clock HSP instance.
	 * @param intrusion_count The output of the intrusion count.
	 *
	 * @return 0 if successful, else an error code.
	 */
	int (*get_intrusion_count) (const struct real_time_clock_hsp *rtc, uint32_t *intrusion_count);

	/**
	 * Clears intrusion count.
	 *
	 * @param rtc The real time clock HSP instance.
	 *
	 * @return 0 if successful, else an error code.
	 */
	int (*clear_intrusion_count) (const struct real_time_clock_hsp *rtc);

	/**
	 * Gets intrusion state.
	 *
	 * @param rtc The real time clock HSP instance.
	 * @param intrusion_state The output of the intrusion state. It can be either 1 or 0.
	 * 0 means being intruded; 1 means normal.
	 *
	 * @return 0 if successful, else an error code.
	 */
	int (*get_intrusion_state) (const struct real_time_clock_hsp *rtc, uint32_t *intrusion_state);

	/**
	 * Resets intrusion tamper register to the given magic number.
	 *
	 * @param rtc The real time clock HSP instance.
	 * @param intrusion_magic_number The intrusion magic number to be set.
	 *
	 * @return 0 if successful, else an error code.
	 */
	int (*set_intrusion_magic_number) (const struct real_time_clock_hsp *rtc,
		uint32_t intrusion_magic_number);

	/**
	 * Gets intrusion magic number.
	 *
	 * @param rtc The real time clock HSP instance.
	 * @param intrusion_magic_number The output of intrusion magic number.
	 *
	 * @return 0 if successful, else an error code.
	 */
	int (*get_intrusion_magic_number) (const struct real_time_clock_hsp *rtc,
		uint32_t *intrusion_magic_number);

	/**
	 * Gets RTC interrupt status.
	 *
	 * @param rtc The real time clock HSP instance.
	 * @param int_status The output of interrupt status.
	 *
	 * @return 0 if successful, else an error code.
	 */
	int (*get_interrupt_status) (const struct real_time_clock_hsp *rtc, uint32_t *int_status);

	/**
	 * Clears RTC interrupt status.
	 *
	 * @param rtc The real time clock HSP instance.
	 * @param int_status The interrupt status to be cleared.
	 *
	 * @return 0 if successful, else an error code.
	 */
	int (*clear_interrupt_status) (const struct real_time_clock_hsp *rtc, uint32_t int_status);

	/**
	 * Gets RTC interrupt enable status.
	 *
	 * @param rtc The real time clock HSP instance.
	 * @param enable The output of interrupt enable status.
	 *
	 * @return 0 if successful, else an error code.
	 */
	int (*get_interrupt_enable) (const struct real_time_clock_hsp *rtc, uint32_t *enable);

	/**
	 * Sets RTC interrupt enable status.
	 *
	 * @param rtc The real time clock HSP instance.
	 * @param enable The interrupt enable status to be set.
	 *
	 * @return 0 if successful, else an error code.
	 */
	int (*set_interrupt_enable) (const struct real_time_clock_hsp *rtc, uint32_t enable);

	struct real_time_clock_hsp_state *state;	/**< The variable state context. */
	struct Creg_regs_creg_rtc_group *regs;		/**< RTC HW registers. */
	uint32_t ready_timeout_ms;					/**< The timeout in milliseconds to wait for RTC HW to be ready. */
};


int real_time_clock_hsp_init (struct real_time_clock_hsp *rtc,
	struct real_time_clock_hsp_state *state, struct Creg_regs_creg_rtc_group *regs,
	uint32_t ready_timeout_ms);
int real_time_clock_hsp_init_no_set_time (struct real_time_clock_hsp *rtc,
	struct Creg_regs_creg_rtc_group *regs, uint32_t ready_timeout_ms);
int real_time_clock_hsp_init_state (const struct real_time_clock_hsp *rtc);
void real_time_clock_hsp_release (const struct real_time_clock_hsp *rtc);


#endif	/* REAL_TIME_CLOCK_HSP_H_ */
