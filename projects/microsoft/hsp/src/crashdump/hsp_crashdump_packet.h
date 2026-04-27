// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_CRASHDUMP_PACKET_H_
#define HSP_CRASHDUMP_PACKET_H_


#include "crash_dump_riscv.h"
#include "logging/crash_dump.h"


/**
 * Crash dump packet version control
 */
#define HSP_CRASHDUMP_VERSION_MINOR				0x01
#define HSP_CRASHDUMP_VERSION_MAJOR				0x00
#define HSP_CRASHDUMP_VERSION                   \
		(HSP_CRASHDUMP_VERSION_MAJOR << 8 | HSP_CRASHDUMP_VERSION_MINOR)

/**
 * Magic number used to indicate start of a crashdump packet
 */
#define HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_COMITTED					0x4D446D70
#define	HSP_CRASHDUMP_PACKET_DUMP_HEADER_MAGIC_DIRTY					0x2BB2928F

/**
 * Crashdump Core Types
 */
enum hsp_crashdump_packet_core_type {
	HSP_CRASHDUMP_PACKET_CORE_TYPE_HSP_TYPE = 0,	/**< HSP core Type. */
};

/**
 * Fault code that sorts the failures fetched core register
 * set stored on crashdump payload to different categories to help datacenter to
 * briefly to identify the issue in the first place.
 */
enum hsp_crashdump_packet_fault_code {
	HSP_CRASHDUMP_PACKET_FAULT_CODE_UNKNOWN = 0,				/**< Unknown failure code. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_NONMASKABLE_INTERRUPT = 1,	/**< Non-maskable interrupt. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_HARDFAULT = 2,				/**< Hard fault. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_MEMORY_FAULT = 3,			/**< Memory fault. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_BUS_FAULT = 4,				/**< Bus fault. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_USAGE_FAULT = 5,			/**< Usage fault. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_SECURE_FAULT = 6,			/**< Secure fault. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_SV_CALL = 7,				/**< SVCall. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_DEBUG_MONITOR = 8,			/**< Debug monitor. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_PEND_SV = 9,				/**< PendSV. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_SYS_TICK = 10,				/**< SysTick. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_PANIC = 11,					/**< Panic. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_WATCHDOG = 12,				/**< Watchdog reset as sent by HSP. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_STACK_OVER_FLOW = 13,		/**< Stack overflow detected. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_DOUBLE_FAULT = 14,			/**< Double fault. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_OTHER_CORE = 15,			/**< Triggered by other core. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_ExplicitFailure = 16,		/**< Explicitly triggered on unrecoverable failure. */

	/* 17 - 50 fault code IDs are reserved for platform-specific use. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_ACCESS_ERR = 51,			/**< Access HW error occurred. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_BUS_ERR = 52,				/**< Bus HW error occurred. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_CHK_ERR = 53,				/**< Check point HW error occurred. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_DMB_ERR = 54,				/**< DMB HW error occurred. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_MEM_ERR = 55,				/**< Memory HW error occurred. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_MPU_ERR = 56,				/**< MPU HW error occurred. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_RNG_ERR = 57,				/**< Crypto HW error occurred. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_WATCHDOG_TIMEOUT = 58,		/**< Watchdog timer timeout. */
	HSP_CRASHDUMP_PACKET_FAULT_CODE_WDT_ERR = 59,				/**< AXI watchdog HW error occurred. */
};

/**
 * Crashdump HSP Core Production Code Payload.
 */
struct hsp_crashdump_packet_hsp_production_payload {
	uint32_t fw_version[2];			/**< Version of FW that crashed */
	struct crash_dump_riscv riscv;	/**< RISC-V register set defined for crashdump */
};

/**
 * Crashdump HSP Core Production Packet.
 */
struct hsp_crashdump_packet_hsp_production_packet {
	struct crash_dump_packet_header header;						/**< Packet header */
	struct hsp_crashdump_packet_hsp_production_payload payload;	/**< Packet payload */
};


#endif	/* HSP_CRASHDUMP_PACKET_H_ */
