// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_MAILBOX_REGISTER_BANK_H_
#define HSP_MAILBOX_REGISTER_BANK_H_

#include <stdint.h>

/**
 * Structure used to access mailbox registers
 *
 * In platform/project code, static asserts should be used to verify
 * that this generic structure matches the hardware implementation.
 */
struct __attribute__((aligned (4))) hsp_mailbox_register_bank {
	volatile uint32_t CTRL;			/**< Control register */
	volatile uint32_t INSTS;		/**< Interrupt status register */
	volatile uint32_t FIFO_PUSH;	/**< Fifo push register */
	volatile uint32_t FIFO_POP;		/**< Fifo pop register */
};

/**
 * Offsets to each member of the hsp_mailbox_register_bank struct
 */
#define HSP_MAILBOX_REGISTER_BANK_CTRL_OFFSET offsetof(struct hsp_mailbox_register_bank, CTRL)
#define HSP_MAILBOX_REGISTER_BANK_INSTS_OFFSET offsetof(struct hsp_mailbox_register_bank, INSTS)
#define HSP_MAILBOX_REGISTER_BANK_FIFO_PUSH_OFFSET \
		offsetof(struct hsp_mailbox_register_bank, FIFO_PUSH)
#define HSP_MAILBOX_REGISTER_BANK_FIFO_POP_OFFSET \
		offsetof(struct hsp_mailbox_register_bank, FIFO_POP)

/**
 * Bit manipulation macros for the mailbox register members
 */
#define HSP_MAILBOX_REGISTER_BANK_CTRL_ERR_INT_EN_MODIFY(r, x) \
	((((x) << 3) & 0x00000008ul) | ((r) & 0xfffffff7ul))
#define HSP_MAILBOX_REGISTER_BANK_CTRL_FIFO_VALID_INT_EN_MODIFY(r, x) \
	((((x) << 2) & 0x00000004ul) | ((r) & 0xfffffffbul))

#define HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_CNT_GET(x) \
	(((x) & 0x0000ff00ul) >> 8)
#define HSP_MAILBOX_REGISTER_BANK_INSTS_ERR_BIT_GET(x) \
	(((x) & 0x00000008ul) >> 3)
#define HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_VALID_GET(x) \
	(((x) & 0x00000004ul) >> 2)

#define HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_CNT_SET(x) \
	(((x) << 8) & 0x0000ff00ul)
#define HSP_MAILBOX_REGISTER_BANK_INSTS_ERR_BIT_SET(x) \
	(((x) << 3) & 0x00000008ul)
#define HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_VALID_SET(x) \
	(((x) << 2) & 0x00000004ul)


#endif	/* HSP_MAILBOX_REGISTER_BANK_H_ */
