// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef INIT_HOST_H_
#define INIT_HOST_H_

#include <stdint.h>
#include "drivers/hsp_gpio_static.h"
#include "host_fw/host_cmd_handler_static.h"
#include "host_fw/host_control_hsp_gpio_static.h"
#include "host_fw/host_gpio_irq_event_manager.h"
#include "host_fw/host_irq_handler.h"
#include "host_fw/host_processor_filtered.h"
#include "host_fw/host_state_manager_static.h"
#include "spi_filter/spi_filter_hsp_static.h"


extern const struct hsp_gpio gpio;
extern const struct host_control_hsp_gpio host_gpio;
extern const struct host_state_manager host_state;
extern const struct spi_filter_hsp host_filter;
extern const struct host_cmd_handler host_handler;
extern struct host_processor_filtered host_manager;
extern const struct host_irq_handler *host_irq;
extern struct host_gpio_irq_event_manager gpio_irq;


int initialize_host_gpios ();
int initialize_host_firmware ();
int enable_spi_filter_interrupts ();


#endif	/* INIT_HOST_H_ */
