
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_CRASHDUMP_PACKET_H_
#define SOC_CRASHDUMP_PACKET_H_


#include "crash_dump_arm.h"
#include "dc_scm/rot_memory_map.h"


/**
 * SOC address of TCON registers
 */
#define TCON_REGISTERS_BLOCK_ADDRESS			0xB0005000


/**
 * ARM Core IDs
 */
enum soc_crashdump_arm_core_id {
	SOC_CRASHDUMP_ARM_CORE_ID_CP0_ID = 0,			/**< CP0 or CP Admin core ID. */
	SOC_CRASHDUMP_ARM_CORE_ID_CP1_ID,				/**< CP1 or CP Admin core ID. */
	SOC_CRASHDUMP_ARM_CORE_ID_FP0_ID,				/**< FP0 ID. */
	SOC_CRASHDUMP_ARM_CORE_ID_FP1_ID,				/**< FP1 ID. */
	SOC_CRASHDUMP_ARM_CORE_ID_FP2_ID,				/**< FP2 ID. */
	SOC_CRASHDUMP_ARM_CORE_ID_NUM_OF_ARM_CORE_IDS,	/**< Number of ARM cores. */
};

/**
 * Crashdump Core Types
 */
enum soc_crashdump_packet_core_type {
	SOC_CRASHDUMP_PACKET_CORE_TYPE_CP0_TYPE = 1,	/**< CP0 or CP Admin core Type. */
	SOC_CRASHDUMP_PACKET_CORE_TYPE_CP1_TYPE,		/**< CP1 or CP Admin core Type. */
	SOC_CRASHDUMP_PACKET_CORE_TYPE_FP0_TYPE,		/**< FP0 Type. */
	SOC_CRASHDUMP_PACKET_CORE_TYPE_FP1_TYPE,		/**< FP1 Type. */
	SOC_CRASHDUMP_PACKET_CORE_TYPE_FP2_TYPE,		/**< FP2 Type. */
};

/**
 * Crashdump ARM Core Production Code Payload.
 */
struct soc_crashdump_packet_arm_production_payload {
	struct crash_dump_arm common_regs;	/**< ARM core register sets defined for crashdump by Cerberus Core */
};

/**
 * Crashdump ARM Core Production Packet.
 */
struct soc_crashdump_packet_arm_production_packet {
	struct crash_dump_packet_header header;						/**< Packet header */
	struct soc_crashdump_packet_arm_production_payload payload;	/**< Packet payload */
};


#endif	/* SOC_CRASHDUMP_PACKET_H_ */
