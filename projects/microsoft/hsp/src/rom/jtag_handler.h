// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef JTAG_HANDLER_H_
#define JTAG_HANDLER_H_

#include <stddef.h>
#include <stdint.h>
#include "crypto/rng.h"
#include "drivers/ccs_ksu_interface.h"
#include "drivers/fuse_controller_interface.h"
#include "drivers/jtag_mailbox.h"
#include "status/msft_module_id.h"


/**
 * Command codes that can be sent over JTAG for ROM processing.
 */
enum jtag_handler_cmd {
	JTAG_HANDLER_CMD_NULL_MESSAGE = 0x00,				/**< No message is in the mailbox. */
	JTAG_HANDLER_CMD_TRANSITION_TO_TEST = 0x01,			/**< Transition the device security state to Test. */
	JTAG_HANDLER_CMD_TRANSITION_TO_PROD = 0x02,			/**< Transition the device security state to Production. */
	JTAG_HANDLER_CMD_PROVISION = 0x03,					/**< Provision device keys and transition the security state to Secure. */
	JTAG_HANDLER_CMD_TRANSITION_TO_RETEST = 0x04,		/**< Transition the device security state to Retest. */
	JTAG_HANDLER_CMD_REQUEST_PUBLIC_KEYS = 0x05,		/**< Extract public keys from the device. */
	JTAG_HANDLER_CMD_HALT_ROM = 0x06,					/**< Stop all ROM execution and wait for reset. */
	JTAG_HANDLER_CMD_EMC_FUSE_PROGRAM = 0x07,			/**< Program an EMC fuse register. */
	JTAG_HANDLER_CMD_EMC_FUSE_READ = 0x08,				/**< Read an EMC fuse register. */
	JTAG_HANDLER_CMD_FUSE_BLANK_CHECK = 0x09,			/**< Check a range for any unprogrammed fuses. */
	JTAG_HANDLER_CMD_RNG_CALIBRATION_PROGRAM = 0x0a,	/**< Program RNG calibration data into fuses. */
	JTAG_HANDLER_CMD_RNG_CALIBRATION_READ = 0x0b,		/**< Read the current RNG calibration data. */
};

/**
 * Buffer descriptor for a public key that can be retrieved over JTAG.
 */
struct jtag_handler_public_key {
	const uint8_t *key;	/**< Buffer containing the public key data. */
	size_t length;		/**< Length of the public key buffer. */
};

#pragma pack(push, 1)
/**
 * Base message structure used for ROM JTAG commands.
 */
union jtag_handler_msg {
	struct {
		uint8_t undefined[7];	/**< Undefined in the base structure. */
		uint8_t task_byte;		/**< Request and status information. */
	};							/**< Base request structure common for all message types. */

	uint32_t raw[2];			/**< Raw data of the mailbox registers. */
};

/**
 * Get the request type from a JTAG message.
 *
 * @param msg Pointer to a message structure.
 */
#define	jtag_handler_msg_get_request(msg)			((msg)->task_byte & 0x1f)

/**
 * Set the request type for a JTAG message.
 *
 * @param msg Pointer to a message structure.
 * @param req The request value to set.
 */
#define	jtag_handler_msg_set_request(msg, req)      \
	(msg)->task_byte &= ~0x1f;  \
	(msg)->task_byte |= ((req) & 0x1f)

/**
 * Set the request type for a JTAG message and clear all status bits.
 *
 * @param msg Pointer to a message structure.
 * @param req The request value to set.
 */
#define	jtag_handler_msg_set_new_request(msg, req)	(msg)->task_byte = req

/**
 * Check the message to see if all the status bits are cleared, indicating a new request.
 *
 * @param msg Pointer to a message structure.
 */
#define	jtag_handler_msg_is_new_request(msg)		(!((msg)->task_byte & 0xe0))

/**
 * Check the JTAG message for acknowledgement.
 *
 * @param msg Pointer to a message structure.
 */
#define	jtag_handler_msg_is_ack(msg)				(!!((msg)->task_byte & 0x20))

/**
 * Set the acknowledgement bit in the status fields.
 *
 * @param Pointer to a message structure.
 */
#define	jtag_handler_msg_set_ack(msg)				(msg)->task_byte |= 0x20

/**
 * Clear the acknowledgement bit in the status fields.
 *
 * @param Pointer to a message structure.
 */
#define	jtag_handler_msg_clear_ack(msg)				(msg)->task_byte &= ~0x20

/**
 * Check the JTAG message for completion.
 *
 * @param msg Pointer to a message structure.
 */
#define	jtag_handler_msg_is_done(msg)				(!!((msg)->task_byte & 0x40))

/**
 * Set the done bit in the status fields.
 *
 * @param Pointer to a message structure.
 */
#define	jtag_handler_msg_set_done(msg)				(msg)->task_byte |= 0x40

/**
 * Clear the done bit in the status fields.
 *
 * @param Pointer to a message structure.
 */
#define	jtag_handler_msg_clear_done(msg)			(msg)->task_byte &= ~0x40

/**
 * Check the JTAG message for success.  This is only the Pass/Fail status, and not coupled with the
 * completion status.
 *
 * @param msg Pointer to a message structure.
 */
#define	jtag_handler_msg_is_pass(msg)				(!!((msg)->task_byte & 0x80))

/**
 * Set the pass bit in the status fields.
 *
 * @param Pointer to a message structure.
 */
#define	jtag_handler_msg_set_pass(msg)				(msg)->task_byte |= 0x80

/**
 * Clear the pass bit in the status fields.
 *
 * @param Pointer to a message structure.
 */
#define	jtag_handler_msg_clear_pass(msg)			(msg)->task_byte &= ~0x80

/**
 * Set all bits in the status fields.
 *
 * @param Pointer to a message structure.
 */
#define	jtag_handler_msg_set_all_status(msg)		(msg)->task_byte |= 0xe0

/**
 * Clear all bits in the status fields.
 *
 * @param Pointer to a message structure.
 */
#define	jtag_handler_msg_clear_all_status(msg)		(msg)->task_byte &= ~0xe0


/**
 * Message structure to transition the device security state.
 */
struct jtag_handler_msg_ss_transition {
	uint8_t unused[5];	/**< Unused in this message. */
	uint8_t last;		/**< Last transaction flag. */
	uint8_t ext_status;	/**< Extended status for failure details. */
	uint8_t task_byte;	/**< Base request and status information. */
};

/**
 * Indicate if the message is the last transaction expected
 *
 * @param msg Pointer to a security state transition message structure.
 */
#define	jtag_handler_msg_ss_transition_is_last(msg)					(!!((msg)->last & 0x80))

/**
 * Set the last transaction bit in the security state transition message.
 *
 * @param msg Pointer to a security state transition message structure.
 */
#define	jtag_handler_msg_ss_transition_set_last_transaction(msg)	(msg)->last |= 0x80

/**
 * Clear the last transaction bit in the security state transition message.
 *
 * @param msg Pointer to a security state transition message structure.
 */
#define	jtag_handler_msg_ss_transition_clear_last_transaction(msg)	(msg)->last &= ~0x80


/**
 * Message structure to provision device keys and transition to the Secure state.
 */
struct jtag_handler_msg_provision {
	union {
		uint8_t seed_data[6];		/**< Global seed data chunk for provisioning. */
		struct {
			uint32_t ext_status;	/**< Extended status information for failures. */
			uint16_t ext_unused;	/**< Unused for status reporting. */
		};
	};

	uint8_t seed_offset_flags;	/**< Offset of the seed data and status flags. */
	uint8_t task_byte;			/**< Base request and status information. */
};

/**
 * Get the offset for the current seed data.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_get_seed_offset(msg)			((msg)->seed_offset_flags & 0x0f)

/**
 * Set the offset for the current seed data.
 *
 * @param msg Pointer to a provision message structure.
 * @param off Seed offset to set.
 */
#define	jtag_handler_msg_provision_set_seed_offset(msg, off)    \
	(msg)->seed_offset_flags &= ~0x0f;  \
	(msg)->seed_offset_flags |= ((off) & 0x0f)

/**
 * Check if the global seed has been received and verified.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_is_seed_received(\
	msg)		(!!((msg)->seed_offset_flags & 0x10))

/**
 * Set the seed received bit in the provision message.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_set_seed_received(msg)		(msg)->seed_offset_flags |= 0x10

/**
 * Clear the seed received bit in the provision message.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_clear_seed_received(msg)		(msg)->seed_offset_flags &= ~0x10

/**
 * Check if the global seed has been programmed to fuses.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_is_seed_programmed(\
	msg)	  (!!((msg)->seed_offset_flags & 0x20))

/**
 * Set the seed programmed bit in the provision message.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_set_seed_programmed(msg)		(msg)->seed_offset_flags |= 0x20

/**
 * Clear the seed programmed bit in the provision message.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_clear_seed_programmed(msg)	(msg)->seed_offset_flags &= ~0x20

/**
 * Check if the SOCID and device keys have been programmed.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_are_keys_programmed(\
	msg)	 (!!((msg)->seed_offset_flags & 0x40))

/**
 * Set the keys programmed bit in the provision message.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_set_keys_programmed(msg)		(msg)->seed_offset_flags |= 0x40

/**
 * Clear the keys programmed bit in the provision message.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_clear_keys_programmed(msg)	(msg)->seed_offset_flags &= ~0x40

/**
 * Check if device provisioning has completed.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_is_complete(\
	msg)			 (!!((msg)->seed_offset_flags & 0x80))

/**
 * Set the provisioning complete bit in the provision message.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_set_complete(msg)			(msg)->seed_offset_flags |= 0x80

/**
 * Clear the provisioning complete bit in the provision message.
 *
 * @param msg Pointer to a provision message structure.
 */
#define	jtag_handler_msg_provision_clear_complete(msg)			(msg)->seed_offset_flags &= ~0x80


/**
 * Message structure to extract public keys from the device.
 */
struct jtag_handler_msg_public_key {
	uint8_t key_data[6];	/**< Data for the public key. */
	uint8_t key_info;		/**< Key index and data offset. */
	uint8_t task_byte;		/**< Base request and status information. */
};

/**
 * Get the offset for the current key data.
 *
 * @param msg Pointer to a public key message structure.
 */
#define	jtag_handler_msg_public_key_get_key_offset(msg)			((msg)->key_info & 0x3f)

/**
 * Set the offset for the current key data.
 *
 * @param msg Pointer to a public key message structure.
 * @param off Key offset to set.
 */
#define	jtag_handler_msg_public_key_set_key_offset(msg, off)    \
	(msg)->key_info &= ~0x3f;   \
	(msg)->key_info |= ((off) & 0x3f)

/**
 * Get the index for the current key data.
 *
 * @param msg Pointer to a public key message structure.
 */
#define	jtag_handler_msg_public_key_get_key_index(msg)			(((msg)->key_info >> 6) & 0x03)

/**
 * Set the index for the current key data.
 *
 * @param msg Pointer to a public key message structure.
 * @param id Key index to set.
 */
#define	jtag_handler_msg_public_key_set_key_index(msg, id)      \
	(msg)->key_info &= ~0xc0;   \
	(msg)->key_info |= (((id) & 0x03) << 6)


/**
 * Message structure to program or verify EMC fuse values.
 */
struct jtag_handler_msg_emc_fuse {
	uint32_t fuse_data;		/**< Data of the EMC fuse register. */
	uint16_t fuse_address;	/**< Lower bits of the EMC fuse register to access. */
	uint8_t ext_status;		/**< Extended status, includes the last transaction bit. */
	uint8_t task_byte;		/**< Base request and status information. */
};

/**
 * Indicate if the message is the last transaction expected
 *
 * @param msg Pointer to a EMC fuse message structure.
 */
#define	jtag_handler_msg_emc_fuse_is_last(msg)					((msg)->ext_status & 0x01)

/**
 * Set the last transaction bit in the EMP fuse message.
 *
 * @param msg Pointer to a EMC fuse message structure.
 */
#define	jtag_handler_msg_emc_fuse_set_last_transaction(msg)		(msg)->ext_status |= 0x01

/**
 * Clear the last transaction bit in the EMC fuse message.
 *
 * @param msg Pointer to a EMC fuse message structure.
 */
#define	jtag_handler_msg_emc_fuse_clear_last_transaction(msg)	(msg)->ext_status &= ~0x01

/**
 * Get the value of the extended status for the message.
 *
 * @param msg Pointer to a EMC fuse message structure.
 */
#define	jtag_handler_msg_emc_fuse_get_ext_status(msg)			(((msg)->ext_status >> 1) & 0x7f)

/**
 * Set the value of the extended status for the message.
 *
 * @param msg Pointer to a EMC fuse message structure.
 * @param val The extended status value to set.
 */
#define	jtag_handler_msg_emc_fuse_set_ext_status(msg, val)      \
	(msg)->ext_status &= ~(0x7f << 1);  \
	(msg)->ext_status |= (((val) & 0x7f) << 1)


/**
 * Message structure to run blank checking on HSP fuses.
 */
struct jtag_handler_msg_fuse_blank_check {
	uint16_t first_not_blank_addr;	/**< Address for the first fuse register that is not blank. */
	uint8_t last;					/**< Last transaction flag. */
	uint16_t end_address;			/**< Last fuse register to blank check. */
	uint16_t start_address;			/**< First fuse register to blank check. */
	uint8_t task_byte;				/**< Base request and status information. */
};

/**
 * Indicate if the message is the last transaction expected
 *
 * @param msg Pointer to a fuse blank check message structure.
 */
#define	jtag_handler_msg_fuse_blank_check_is_last(msg)					((msg)->last & 0x01)

/**
 * Set the last transaction bit in the fuse blank check message.
 *
 * @param msg Pointer to a fuse blank check message structure.
 */
#define	jtag_handler_msg_fuse_blank_check_set_last_transaction(msg)		(msg)->last |= 0x01

/**
 * Clear the last transaction bit in the fuse blank check message.
 *
 * @param msg Pointer to a fuse blank check message structure.
 */
#define	jtag_handler_msg_fuse_blank_check_clear_last_transaction(msg)	(msg)->last &= ~0x01


/**
 * Message structure to provide RNG calibration information.
 */
struct jtag_handler_msg_rng_calibration {
	uint8_t calibration[5];	/**< Calibration data for the RNG. */
	uint8_t last;			/**< Last transaction flag. */
	uint8_t ext_status;		/**< Extended status for failure details. */
	uint8_t task_byte;		/**< Base request and status information. */
};

/**
 * Indicate if the message is the last transaction expected
 *
 * @param msg Pointer to an RNG calibration message structure.
 */
#define	jtag_handler_msg_rng_calibration_is_last(msg)					(!!((msg)->last & 0x80))

/**
 * Set the last transaction bit in the security state transition message.
 *
 * @param msg Pointer to an RNG calibration message structure.
 */
#define	jtag_handler_msg_rng_calibration_set_last_transaction(msg)		(msg)->last |= 0x80

/**
 * Clear the last transaction bit in the security state transition message.
 *
 * @param msg Pointer to an RNG calibration message structure.
 */
#define	jtag_handler_msg_rng_calibration_clear_last_transaction(msg)	(msg)->last &= ~0x80

/**
 * Maximum number of key slots.
 */
#define JTAG_HANDLER_MAX_KEY_SLOTS 3
#pragma pack(pop)

/**
 * ROM handler for messages present during boot in the HSP JTAG mailbox.
 */
struct jtag_handler {
	/**
	 * Handle a message from the JTAG mailbox.
	 *
	 * @param jtag The handler to execute.
	 *
	 * @return 0 if a message was present and handled successfully or an error code.
	 */
	int (*handle_msg) (const struct jtag_handler *jtag);

	const struct jtag_mailbox *mailbox;				/**< Interface to the JTAG mailbox. */
	const struct fuse_controller_interface *fuses;	/**< Interface to the HSP fuses. */
	const struct ccs_ksu_interface *ccs;			/**< Interface to the HSP secure keys. */
	const struct rng_engine *rng;					/**< RNG for SOCID generation. */
	const struct jtag_handler_public_key *keys;		/**< List of public keys available over JTAG. */
};


/* Internal functions for use by derived types. */
int jtag_handler_init (struct jtag_handler *jtag, const struct jtag_mailbox *mailbox,
	const struct fuse_controller_interface *fuses, const struct ccs_ksu_interface *ccs,
	const struct rng_engine *rng, const struct jtag_handler_public_key public_keys[4]);
void jtag_handler_release (const struct jtag_handler *jtag);
int jtag_handler_message_done (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	int status, bool is_last);
void jtag_handler_msg_provision_fail (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov, int status);
void jtag_handler_msg_provision_complete (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov);
int jtag_handler_receive_global_seed (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov, SP_MSG_512 *global_seed);
int jtag_handler_unwrap_global_seed (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov, SP_MSG_512 *global_seed);
int jtag_handler_program_global_seed (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov);
int jtag_handler_unwrap_program_global_seed (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov, SP_MSG_512 *global_seed);
int jtag_handler_generate_socid (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov, SP_MSG_512 *socid, const uint8_t *socid_prefix,
	size_t prefix_length);
int jtag_handler_program_socid (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	struct jtag_handler_msg_provision *prov, SP_MSG_512 *socid);
int jtag_handler_generate_and_program_socid (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov, SP_MSG_512 *socid,
	const uint8_t *socid_prefix, size_t prefix_length);
int jtag_handler_generate_device_unique_key (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov, size_t num_slots);
int jtag_handler_program_device_unique_key (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov, size_t num_slots);
int jtag_handler_transition_to_secure_state (const struct jtag_handler *jtag,
	union jtag_handler_msg *msg, struct jtag_handler_msg_provision *prov);
int jtag_handler_read_mailbox_msg (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	enum hsp_security_state *current_state);
int jtag_handler_handle_common_msg (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	enum hsp_security_state state, int *fail_id);
int jtag_handler_finish_msg (const struct jtag_handler *jtag, union jtag_handler_msg *msg,
	int status, uint8_t fail_id);

#define	JTAG_HANDLER_ERROR(code)		ROT_ERROR (MSFT_MODULE_JTAG_HANDLER, code)

/**
 * Error codes that can be generated by a JTAG message handler.
 */
enum {
	JTAG_HANDLER_INVALID_ARGUMENT = JTAG_HANDLER_ERROR (0x00),		/**< Input parameter is null or not valid. */
	JTAG_HANDLER_NO_MEMORY = JTAG_HANDLER_ERROR (0x01),				/**< Memory allocation failed. */
	JTAG_HANDLER_HANDLE_MSG_FAIL = JTAG_HANDLER_ERROR (0x02),		/**< Failed to handle a JTAG message. */
	JTAG_HANDLER_NO_MESSAGE = JTAG_HANDLER_ERROR (0x03),			/**< There was no message available to process. */
	JTAG_HANDLER_MORE = JTAG_HANDLER_ERROR (0x04),					/**< There are additional messages to process. */
	JTAG_HANDLER_HALT = JTAG_HANDLER_ERROR (0x05),					/**< Any further execution should be halted. */
	JTAG_HANDLER_SHORT_SEED_DATA = JTAG_HANDLER_ERROR (0x06),		/**< Not enough seed data was sent with the message. */
	JTAG_HANDLER_UNSUPPORTED_MSG = JTAG_HANDLER_ERROR (0x07),		/**< The received message is not supported in the current context. */
	JTAG_HANDLER_TOO_MANY_KEY_SLOTS = JTAG_HANDLER_ERROR (0x08),	/**< Number of key slots exceeds the maximum allowed. */
};


#endif	/* JTAG_HANDLER_H_ */
