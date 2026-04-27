// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_CONTROL_HSP_GPIO_STATIC_H_
#define HOST_CONTROL_HSP_GPIO_STATIC_H_

#include "host_control_hsp_gpio.h"


/* Internal functions declared to allow for static initialization. */
int host_control_hsp_gpio_hold_processor_in_reset (const struct host_control *control, bool reset);
int host_control_hsp_gpio_is_processor_held_in_reset (const struct host_control *control);
int host_control_hsp_gpio_is_processor_in_reset (const struct host_control *control);
int host_control_hsp_gpio_enable_processor_flash_access (const struct host_control *control,
	bool enable);
int host_control_hsp_gpio_processor_has_flash_access (const struct host_control *control);


/**
 * Constant initializer for the host control API.
 */
#define	HOST_CONTROL_HSP_GPIO_API_INIT	{ \
		.hold_processor_in_reset = host_control_hsp_gpio_hold_processor_in_reset, \
		.is_processor_held_in_reset = host_control_hsp_gpio_is_processor_held_in_reset, \
		.is_processor_in_reset = host_control_hsp_gpio_is_processor_in_reset, \
		.enable_processor_flash_access = host_control_hsp_gpio_enable_processor_flash_access, \
		.processor_has_flash_access = host_control_hsp_gpio_processor_has_flash_access \
	}


/**
 * Initialize a static host control interface using HSP GPIOs.
 *
 * There is no validation done on the arguments.
 *
 * @param gpio_ptr Driver for the HSP GPIOs that provide control over host hardware.  All GPIOs must
 * be managed by the same driver.
 * @param reset_ctrl_num The GPIO number for host processor reset control.
 * @param reset_ind_num The GPIO number that indicates when the processor is in reset.
 * @param mux_sel_num The GPIO number for the SPI filter mux select.
 */
#define	host_control_hsp_gpio_static_init(gpio_ptr, reset_ctrl_num, reset_ind_num, mux_sel_num)	{ \
		.base = HOST_CONTROL_HSP_GPIO_API_INIT, \
		.gpio = gpio_ptr, \
		.reset_ctrl = reset_ctrl_num, \
		.reset_state = reset_ind_num, \
		.spi_mux = mux_sel_num \
	}


#endif	/* HOST_CONTROL_HSP_GPIO_STATIC_H_ */
