// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CHECKPOINT_STATIC_H_
#define CHECKPOINT_STATIC_H_

#include "drivers/checkpoint.h"


/* Internal functions declared to allow for static initialization. */
void checkpoint_set_config (const struct checkpoint *chkpt, const HSP_CHKPT_CONFIG *config);
int checkpoint_check_config (const struct checkpoint *chkpt, const HSP_CHKPT_CONFIG *config);
void checkpoint_lock_hw (const struct checkpoint *chkpt);
void checkpoint_write_message (const struct checkpoint *chkpt, uint32_t message);


/**
 * Initialize a static checkpoint driver instance.
 *
 * There is no validation done on the arguments.
 *
 * @param regs_ptr Base address of the checkpoint hardware registers.
 */
#define	checkpoint_static_init(regs_ptr)	{ \
		.set_config = checkpoint_set_config, \
		.check_config = checkpoint_check_config, \
		.lock_hw = checkpoint_lock_hw, \
		.write_message = checkpoint_write_message, \
		.regs = regs_ptr, \
	}


#endif	/* CHECKPOINT_STATIC_H_ */
