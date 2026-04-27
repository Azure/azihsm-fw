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
	MANTICORE_MODULE_CIRCULAR_QUEUE = 0x2000,			/**< The circular queue interface. */
};


#endif	/* MANTICORE_MODULE_ID_H_ */
