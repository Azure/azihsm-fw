// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MANTICORE_FP_LOG_PARSER_H_
#define MANTICORE_FP_LOG_PARSER_H_

#include <stddef.h>
#include <stdint.h>
#include "manticore_fp_log_tokens.h"
#include "manticore_logging_record.h"


#define LOGGING_COMPONENT_MANTICORE_FP0	246
#define LOGGING_COMPONENT_MANTICORE_FP1	247
#define LOGGING_COMPONENT_MANTICORE_FP2	248


/*
 * Telemetry FP parser contents
 */
struct telemetry_fp_token_parser {
	uint32_t arg1;			//Received debug argument-1
	uint32_t arg2;			//Received debug argument-2
	uint8_t arg1_cnt;		//Argument-1 split counter
	uint8_t arg2_cnt;		//Argument-2 split counter
	uint8_t *arg_spliter;	//Argument parser
	uint32_t *arg_parser;	//Argument parser
};

/*
 * Telemetry FP log parser
 */
struct telemetry_fp_parser {
	const char *msg_str;	//Message string
	uint8_t *split_args;	//Split arguments
	uint8_t msg_index;		//Message index
};


int8_t manticore_fp_log_args_parser_init (struct telemetry_fp_token_parser *parser,
	const uint32_t *debug_arg1, const uint32_t *debug_arg2, uint32_t *arg_parser,
	uint8_t *split_array, uint8_t total_elements);
int8_t manticore_fp_log_parser_init (struct manticore_logging_record *rcv_entry, char *message,
	size_t message_size);


#endif	// MANTICORE_FP_LOG_PARSER_H_
