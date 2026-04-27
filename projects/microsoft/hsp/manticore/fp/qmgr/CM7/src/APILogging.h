// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Marvell

//=============================================================================
//
//! @file
//! @brief Header for logging producer module API
//!
//=============================================================================

#ifndef APILOGGING_H_
#define APILOGGING_H_

#include <stdint.h>

/**
 * API for logging producer Initialization
 */
void API_LoggingProducerOneTimeInit();

/**
 * Api to send a log entry to the ring buffer via the logging_producer module
 *
 * @param tokenIdx the index token used to decode the debug log during detokenization
 * @param severity Severity level of the log entry.
 * @param arg1 First argument for the log message.
 * @param arg2 Second argument for the log message.
 *
 */
void API_AddDebugLog(uint8_t tokenIdx, uint8_t severity, uint32_t arg1, uint32_t arg2);

#endif	/* APILOGGING_H_ */
