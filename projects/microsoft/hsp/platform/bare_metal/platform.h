// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "platform_base.h"
#include "platform_clock_hsp.h"
#if __has_include ("platform_compiler.h")
#include "platform_compiler.h"
#endif
#ifdef PLATFORM_OVERRIDES
#include PLATFORM_OVERRIDES
#endif


/**
 * Container for timer handling.
 */
typedef struct {
	timer_callback callback;
	void *context;
} platform_timer;


#endif	/* PLATFORM_H_ */
