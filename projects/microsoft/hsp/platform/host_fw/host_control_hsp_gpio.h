// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HOST_CONTROL_HSP_GPIO_H_
#define HOST_CONTROL_HSP_GPIO_H_

#include <stdint.h>
#include "drivers/hsp_gpio.h"
#include "host_fw/host_control.h"


/**
 * The interface for host hardware control using HSP GPIOs.
 */
struct host_control_hsp_gpio {
	struct host_control base;		/**< The base API instance. */
	const struct hsp_gpio *gpio;	/**< Driver for the HSP GPIOs. */
	uint8_t reset_ctrl;				/**< The reset control signal for the host processor. */
	uint8_t reset_state;			/**< The indication of reset state for the host processor. */
	uint8_t spi_mux;				/**< The SPI MUX select control signal. */
};


int host_control_hsp_gpio_init (struct host_control_hsp_gpio *control, const struct hsp_gpio *gpio,
	uint8_t reset_ctrl, uint8_t reset_ind, uint8_t mux_sel);
void host_control_hsp_gpio_release (const struct host_control_hsp_gpio *control);


#endif	/* HOST_CONTROL_HSP_GPIO_H_ */
