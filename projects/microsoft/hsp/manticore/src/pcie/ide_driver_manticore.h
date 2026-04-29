// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef IDE_DRIVER_MANTICORE_H_
#define IDE_DRIVER_MANTICORE_H_

#include <stdbool.h>
#include "common/observable.h"
#include "drivers/hsp_dmb.h"
#include "marvell/RegPcieAssist.h"
#include "marvell/RegPcieIde.h"
#include "marvell/RegPcieIdeAes.h"
#include "pcisig/ide/ide_driver.h"
#include "pcisig/ide/ide_driver_observer.h"


/**
 * IDE Key context ID - Calculate Key context from Key Set, Key Stream and Key Sub-Stream
 */
#define IDE_KEY_CTRL_CTX_ID(set, stream, sub_stream)    \
	(((set) * IDE_DRIVER_MANTICORE_SUB_STREAM_ID_MAX * IDE_DRIVER_MANTICORE_STREAM_ID_MAX) +    \
	 ((stream) * IDE_DRIVER_MANTICORE_SUB_STREAM_ID_MAX) + (sub_stream))

/**
 * Stream ID index to identify Link or Selective IDE Stream AES key slots.
 */
enum ide_driver_manticore_stream_id {
	IDE_DRIVER_MANTICORE_STREAM_ID_SELECTIVE = 0,	/**< Selective IDE stream */
	IDE_DRIVER_MANTICORE_STREAM_ID_LINK = 1,		/**< Link IDE stream */
	IDE_DRIVER_MANTICORE_STREAM_ID_MAX = 2,			/**< Max IDE stream */
};

/**
 * Sub Stream ID index to identify if a key belong to Posted, Non-Posted or Completion PCIe
 * transaction AES key slots.
 */
enum ide_driver_manticore_sub_stream_id {
	IDE_DRIVER_MANTICORE_SUB_STREAM_ID_POSTED = 0,		/**< IDE key for Posted PCIe transaction */
	IDE_DRIVER_MANTICORE_SUB_STREAM_ID_NON_POSTED = 1,	/**< IDE key for Non-posted PCIe transaction */
	IDE_DRIVER_MANTICORE_SUB_STREAM_ID_COMPLETION = 2,	/**< IDE key for Completion PCIe transaction */
	IDE_DRIVER_MANTICORE_SUB_STREAM_ID_MAX = 3,			/**< Max IDE sub-stream */
};

/**
 * IDE Key set index to identify if a key belong to Active or Backup IDE AES key slots.
 */
enum ide_driver_manticore_key_set {
	IDE_DRIVER_MANTICORE_KEY_SET_ACTIVE = 0,	/**< Active IDE key set */
	IDE_DRIVER_MANTICORE_KEY_SET_BACKUP = 1,	/**< Backup IDE key set */
	IDE_DRIVER_MANTICORE_KEY_SET_MAX = 2,		/**< Max IDE key set */
};

/**
 * IDE Key direction index to identify if a key belong to Rx or Tx direction AES key slots.
 */
enum ide_driver_manticore_key_direction {
	IDE_DRIVER_MANTICORE_KEY_DIRECTION_RX = 0,	/**< IDE key for Rx */
	IDE_DRIVER_MANTICORE_KEY_DIRECTION_TX = 1,	/**< IDE key for Tx */
};

/**
 * Keys write timeout in ms.
 */
#define IDE_DRIVER_MANTICORE_KEY_WRITE_TIMEOUT_MS	10

/**
 * A Single IDE Key Unit comprises of an AES-256 Key and an AES-256 IV.
 * The key unit is valid if the valid flag is set to true.
 */
struct ide_driver_manticore_key_unit {
	struct ide_km_aes_256_gcm_key_buffer aes;	/**< AES-256 key and IV */
	uint32_t valid;								/**< Key unit is valid flag */
};


_Static_assert (offsetof (struct ide_driver_manticore_key_unit, aes) == 0,
	"Unexpected struct member offset");
_Static_assert (offsetof (struct ide_driver_manticore_key_unit, valid) == 40,
	"Unexpected struct member offset");
_Static_assert (sizeof (struct ide_driver_manticore_key_unit) == 44,
	"Unexpected struct member offset");

/**
 * Type to represent Active and Backup IDE Key sets for all the stream and sub-streams.
 */
typedef struct ide_driver_manticore_key_unit ide_key_sets_t[IDE_DRIVER_MANTICORE_KEY_SET_MAX]
	[IDE_DRIVER_MANTICORE_STREAM_ID_MAX][IDE_DRIVER_MANTICORE_SUB_STREAM_ID_MAX];

/**
 * IDE Driver Interface state for Manticore.
 *
 * this is supposed to be variable context
 */
struct ide_driver_manticore_key_context {
	ide_key_sets_t rx_keys;	/**< IDE Rx Keys */
	ide_key_sets_t tx_keys;	/**< IDE Tx Keys */
};


_Static_assert (offsetof (struct ide_driver_manticore_key_context, rx_keys) == 0,
	"Unexpected struct member offset");
/* 44 * 2 * 2 * 3 = 528 */
_Static_assert (offsetof (struct ide_driver_manticore_key_context, tx_keys) == 528,
	"Unexpected struct member offset");
_Static_assert (sizeof (struct ide_driver_manticore_key_context) == 1056,
	"Unexpected struct member offset");

/**
 * Variable context for the IDE driver.
 */
struct ide_driver_manticore_state {
	struct observable observable;	/**< Observer manager for IDE events. */
};

/**
 * IDE hardware driver object implements PCISIG IDE driver ineterface backed with Manticore IDE
 * hardware.
 */
struct ide_driver_manticore {
	struct ide_driver base;						/**< The base PCI-SIG IDE driver interface. */
	const struct hsp_dmb *dmb;					/**< DMB for SoC address translation from SP. */
	uint64_t assist_reg_base_addr;				/**< Base address of the PCIE assist registers. */
	uint64_t reg_base_addr;						/**< Base address of the IDE registers. */
	uint64_t aes_reg_base_addr;					/**< Base address of the IDE AES registers. */
	uint64_t key_context_addr;					/**< Base address of the IDE key context in SoC memory. */
	struct ide_driver_manticore_state *state;	/**< Variable context for the IDE driver. */
};


int ide_driver_manticore_init (struct ide_driver_manticore *ide, uint64_t assist_reg_base_addr,
	uint64_t ide_reg_base_addr, uint64_t ide_aes_reg_base_addr, uint64_t ide_key_context_addr,
	const struct hsp_dmb *dmb, struct ide_driver_manticore_state *state);
int ide_driver_manticore_init_state (const struct ide_driver_manticore *ide);
void ide_driver_manticore_release (const struct ide_driver_manticore *ide);

int ide_driver_manticore_add_ide_driver_observer (const struct ide_driver_manticore *ide,
	const struct ide_driver_observer *observer);
int ide_driver_manticore_remove_ide_driver_observer (const struct ide_driver_manticore *ide,
	const struct ide_driver_observer *observer);


#endif	/* IDE_DRIVER_MANTICORE_H_ */
