// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_MODULE_ID_H_
#define MANTICORE_MODULE_ID_H_

#include "status/rot_status.h"


/**
 * The IDs for Manticore modules that can generate errors.
 *
 * Commented module IDs have been deprecated or promoted to a different layer.  These IDs should not
 * be reused.
 */
enum {
	MANTICORE_MODULE_CIRCULAR_QUEUE = 0x2000,					/**< The circular queue interface. */
	MANTICORE_MODULE_LOGGING_COLLECTOR = 0x2001,				/**< Manticore remote core logging collector interface. */
	MANTICORE_MODULE_FENCE = 0x2002,							/**< Memory fence interface */
	MANTICORE_MODULE_MANTICORE_FW_DESCRIPTOR = 0x2003,			/**< Parser for the firmware descriptor on images. */
	MANTICORE_MODULE_MANTICORE_FW_PACKAGE = 0x2004,				/**< Handler for the firmware package. */
	MANTICORE_MODULE_MANTICORE_BOOTLOADER = 0x2005,				/**< Bootloader for main Manticore firmware. */
	MANTICORE_MODULE_NVME_DMA = 0x2006,							/**< The NVME DMA module interface. */
	MANTICORE_MODULE_IPC_CHANNEL = 0x2007,						/**< The IPC module interface. */
	MANTICORE_MODULE_UID_EFUSE = 0x2008,						/**< The UID eFuse interface. */
	MANTICORE_MODULE_GDMA = 0x2009,								/**< The GDMA driver interface. */
	MANTICORE_MODULE_MANTICORE_MEASUREMENTS = 0x200a,			/**< Handler for firmware measurements. */
	MANTICORE_MODULE_SOC_RESET_CONTROL = 0x200b,				/**< Driver for SoC hardware reset control. */
	MANTICORE_MODULE_IPC_MESSAGE_HANDLER = 0x200c,				/**< The IPC Message Handler */
	MANTICORE_MODULE_CMD_INTERFACE_IPC_ADMIN = 0x200d,			/**< The Command Interface IPC Admin Module */
	MANTICORE_MODULE_CMD_INTERFACE_IPC_HSM = 0x200e,			/**< The Command Interface IPC HSM Module */
	MANTICORE_MODULE_GRACEFUL_SHUTDOWN = 0x200f,				/**< Handler for managing graceful shutdown of the cores. */
	MANTICORE_MODULE_DEVICE_KEYS = 0x2010,						/**< Generation of device specific keys. */
	MANTICORE_MODULE_CRASHDUMP_SOC_INTERFACE = 0x2012,			/**< Crashdump SoC Interface */
	MANTICORE_MODULE_TELEMETRY_TEMPERATURE = 0x2013,			/**< Telemetry Temperature handler interface */
	MANTICORE_MODULE_TELEMETRY_PCIE = 0x2014,					/**< Telemetry PCIe handler interface */
	MANTICORE_MODULE_SELF_TEST = 0x2015,						/**< Periodic self-test execution context. */
	MANTICORE_MODULE_CMD_INTERFACE_TDISP_EVENT_POLICY = 0x2016,	/**< Handler for the TDISP event policy. */
};


#endif	/* MANTICORE_MODULE_ID_H_ */
