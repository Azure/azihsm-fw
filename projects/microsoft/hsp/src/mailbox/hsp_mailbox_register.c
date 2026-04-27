// Copyright (c) Microsoft Corporation. All rights reserved.

#include <string.h>
#include "hsp_mailbox_register.h"

/**
 * Reads the status information from the INSTS register to the supplied variables
 *
 * @param reg The register bank to be used to get the status variables
 * @param is_valid Optional pointer to variable in which to store the valid bit. Can be set to NULL if not needed.
 * @param is_err Optional pointer to variable in which to store the err bit. Can be set to NULL if not needed.
 * @param count Optional pointer to variable in which to store the FIFO count. Can be set to NULL if not needed.
 */
void hsp_mailbox_register_get_status (const struct hsp_mailbox_register_bank *reg, bool *is_valid,
	bool *is_err, uint32_t *count)
{
	uint32_t reg_value = reg->INSTS;

	if (is_valid != NULL) {
		*is_valid = HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_VALID_GET (reg_value);
	}

	if (is_err != NULL) {
		*is_err = HSP_MAILBOX_REGISTER_BANK_INSTS_ERR_BIT_GET (reg_value);
	}

	if (count != NULL) {
		*count = HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_CNT_GET (reg_value);
	}
}

int hsp_mailbox_register_send_fifo_push (const struct hsp_mailbox_interface *mailbox, uint32_t data)
{
	struct hsp_mailbox_register *mb_reg = (struct hsp_mailbox_register*) mailbox;

	if (mb_reg == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	mb_reg->send_reg->FIFO_PUSH = data;

	return 0;
}

int hsp_mailbox_register_send_set_valid (const struct hsp_mailbox_interface *mailbox)
{
	struct hsp_mailbox_register *mb_reg = (struct hsp_mailbox_register*) mailbox;

	if (mb_reg == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	mb_reg->send_reg->INSTS = HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_VALID_SET (1);

	return 0;
}

int hsp_mailbox_register_send_set_err (const struct hsp_mailbox_interface *mailbox)
{
	struct hsp_mailbox_register *mb_reg = (struct hsp_mailbox_register*) mailbox;

	if (mb_reg == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	mb_reg->send_reg->INSTS = HSP_MAILBOX_REGISTER_BANK_INSTS_ERR_BIT_SET (1);

	return 0;
}

int hsp_mailbox_register_send_get_status (const struct hsp_mailbox_interface *mailbox,
	bool *is_valid, bool *is_err, uint32_t *count)
{
	struct hsp_mailbox_register *mb_reg = (struct hsp_mailbox_register*) mailbox;

	if (mb_reg == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	hsp_mailbox_register_get_status (mb_reg->send_reg, is_valid, is_err, count);

	return 0;
}

int hsp_mailbox_register_recv_fifo_pop (const struct hsp_mailbox_interface *mailbox, uint32_t *data)
{
	struct hsp_mailbox_register *mb_reg = (struct hsp_mailbox_register*) mailbox;

	if ((mb_reg == NULL) || (data == NULL)) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	*data = mb_reg->recv_reg->FIFO_POP;

	return 0;
}

int hsp_mailbox_register_recv_get_status (const struct hsp_mailbox_interface *mailbox,
	bool *is_valid, bool *is_err, uint32_t *count)
{
	struct hsp_mailbox_register *mb_reg = (struct hsp_mailbox_register*) mailbox;

	if (mb_reg == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	hsp_mailbox_register_get_status (mb_reg->recv_reg, is_valid, is_err, count);

	return 0;
}

int hsp_mailbox_register_recv_clear_valid (const struct hsp_mailbox_interface *mailbox)
{
	struct hsp_mailbox_register *mb_reg = (struct hsp_mailbox_register*) mailbox;

	if (mb_reg == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	mb_reg->recv_reg->INSTS = HSP_MAILBOX_REGISTER_BANK_INSTS_FIFO_VALID_SET (1);

	return 0;
}

int hsp_mailbox_register_recv_clear_err (const struct hsp_mailbox_interface *mailbox)
{
	struct hsp_mailbox_register *mb_reg = (struct hsp_mailbox_register*) mailbox;

	if (mb_reg == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	mb_reg->recv_reg->INSTS = HSP_MAILBOX_REGISTER_BANK_INSTS_ERR_BIT_SET (1);

	return 0;
}

int hsp_mailbox_register_recv_enable_fifo_valid_interrupt (
	const struct hsp_mailbox_interface *mailbox, bool enabled)
{
	struct hsp_mailbox_register *mb_reg = (struct hsp_mailbox_register*) mailbox;

	if (mb_reg == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	mb_reg->recv_reg->CTRL =
		HSP_MAILBOX_REGISTER_BANK_CTRL_FIFO_VALID_INT_EN_MODIFY (mb_reg->recv_reg->CTRL, enabled);

	return 0;
}

int hsp_mailbox_register_recv_enable_err_interrupt (const struct hsp_mailbox_interface *mailbox,
	bool enabled)
{
	struct hsp_mailbox_register *mb_reg = (struct hsp_mailbox_register*) mailbox;

	if (mb_reg == NULL) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	mb_reg->recv_reg->CTRL =
		HSP_MAILBOX_REGISTER_BANK_CTRL_ERR_INT_EN_MODIFY (mb_reg->recv_reg->CTRL, enabled);

	return 0;
}

/**
 * Initialize an HSP mailbox register instance.
 *
 * @param mailbox The mailbox to initialize.
 * @param send_reg The address of the outgoing mailbox registers.
 * @param recv_reg The address of the incoming mailbox registers.
 *
 * @return 0 if the mailbox was initialized successfully or an error code.
 */
int hsp_mailbox_register_init (struct hsp_mailbox_register *mailbox,
	struct hsp_mailbox_register_bank *send_reg, struct hsp_mailbox_register_bank *recv_reg)
{
	if ((mailbox == NULL) || (send_reg == NULL) || (recv_reg == NULL)) {
		return HSP_MAILBOX_INVALID_ARGUMENT;
	}

	memset (mailbox, 0, sizeof (struct hsp_mailbox_register));

	mailbox->base.send_fifo_push = hsp_mailbox_register_send_fifo_push;
	mailbox->base.send_set_valid = hsp_mailbox_register_send_set_valid;
	mailbox->base.send_set_err = hsp_mailbox_register_send_set_err;
	mailbox->base.send_get_status = hsp_mailbox_register_send_get_status;

	mailbox->base.recv_fifo_pop = hsp_mailbox_register_recv_fifo_pop;
	mailbox->base.recv_get_status = hsp_mailbox_register_recv_get_status;
	mailbox->base.recv_clear_valid = hsp_mailbox_register_recv_clear_valid;
	mailbox->base.recv_clear_err = hsp_mailbox_register_recv_clear_err;
	mailbox->base.recv_enable_fifo_valid_interrupt =
		hsp_mailbox_register_recv_enable_fifo_valid_interrupt;
	mailbox->base.recv_enable_err_interrupt = hsp_mailbox_register_recv_enable_err_interrupt;

	mailbox->send_reg = send_reg;
	mailbox->recv_reg = recv_reg;

	return 0;
}

/**
 * Release the resources used by an HSP mailbox register instance.
 *
 * @param mailbox The mailbox to release.
 */
void hsp_mailbox_register_release (const struct hsp_mailbox_register *mailbox)
{
	UNUSED (mailbox);
}
