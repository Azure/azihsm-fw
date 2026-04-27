// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdio.h>
#include "manticore_fp_log_parser.h"
#include "manticore_fp_log_tokens.h"

extern uint8_t telemetry_fp0_log_split_args[FP0_COMMON_MAX][MANTICORE_FP_LOG_ARGS_SPLITTER_MAX];
extern uint8_t telemetry_fp1_log_split_args[FP1_COMMON_MAX][MANTICORE_FP_LOG_ARGS_SPLITTER_MAX];
extern uint8_t telemetry_fp2_log_split_args[FP2_COMMON_MAX][MANTICORE_FP_LOG_ARGS_SPLITTER_MAX];


/**
 * Parses the arguments based on the splitter array.
 *
 * @param arg The argument to be parsed.
 * @param arg_cnt The count of arguments.
 * @param arg_spliter The splitter array.
 * @param shift The shift value.
 * @param arg_parser The array to store parsed arguments.
 * @param idx The index to store parsed arguments.
 *
 * @return 0 on success, -1 on failure.
 */
static int8_t manticore_fp_log_parse_args (uint32_t arg, uint8_t arg_cnt,
	const uint8_t *arg_spliter, uint8_t *shift, uint32_t *arg_parser, uint8_t *idx)
{
	uint8_t i;

	if (arg_cnt == 1) {
		arg_parser[(*idx)++] = arg;
	}
	else {
		for (i = 0; i < arg_cnt; i++) {
			*shift -= arg_spliter[i];
			arg_parser[(*idx)++] = (arg >> *shift) & ((1 << arg_spliter[i]) - 1);
		}
	}

	return 0;
}

/**
 * Splits the parser arguments based on the splitter array.
 *
 * @param parser The parser structure containing arguments and splitter array.
 *
 */
static void manticore_fp_log_split_parser (struct telemetry_fp_token_parser *parser)
{
	uint8_t idx = 0;
	uint8_t shift = 32;

	manticore_fp_log_parse_args (parser->arg1, parser->arg1_cnt, parser->arg_spliter, &shift,
		parser->arg_parser, &idx);
	manticore_fp_log_parse_args (parser->arg2, parser->arg2_cnt, parser->arg_spliter + 4, &shift,
		parser->arg_parser, &idx);
}

/**
 * Counts the number of arguments based on the splitter array.
 *
 * @param parser The parser structure containing the splitter array.
 * @param split_cnt The count of splitters.
 *
 * @return 0 on success, -1 on failure.
 */
static int8_t manticore_fp_log_args_counter (struct telemetry_fp_token_parser *parser,
	uint8_t split_cnt)
{
	uint8_t idx = 0;

	if ((parser->arg_spliter == NULL) || (split_cnt > MANTICORE_FP_LOG_ARGS_SPLITTER_MAX)) {
		return -1;
	}

	for (idx = 0; idx < split_cnt; idx++) {
		if ((idx < 4) && (parser->arg_spliter[idx] != 0)) {
			parser->arg1_cnt++;
		}
		else if ((idx < 8) && (parser->arg_spliter[idx] != 0)) {
			parser->arg2_cnt++;
		}
	}

	return 0;
}

/**
 * Initializes the parser and splits the arguments.
 *
 * @param parser The parser structure to be initialized.
 * @param debug_arg1 The first debug argument.
 * @param debug_arg2 The second debug argument.
 * @param arg_parser The array to store parsed arguments.
 * @param split_array The splitter array.
 *
 * @return 0 on success, -1 on failure.
 */
int8_t manticore_fp_log_args_parser_init (struct telemetry_fp_token_parser *parser,
	const uint32_t *debug_arg1, const uint32_t *debug_arg2, uint32_t *arg_parser,
	uint8_t *split_array, uint8_t max_elements)
{
	int8_t status = 0;

	if ((parser == NULL) || (arg_parser == NULL) || (split_array == NULL) ||
		(debug_arg1 == NULL) || (debug_arg2 == NULL)) {
		return -1;
	}

	parser->arg1 = *debug_arg1;
	parser->arg2 = *debug_arg2;
	parser->arg_parser = arg_parser;
	parser->arg_spliter = split_array;
	parser->arg1_cnt = 0;
	parser->arg2_cnt = 0;

	status = manticore_fp_log_args_counter (parser, max_elements);
	if (status != 0) {
		//argument counter failed
		return -1;
	}

	manticore_fp_log_split_parser (parser);

	return 0;
}

/**
 * Parses the telemetry fp log message.
 *
 * @param rcv_entry The received log entry.
 * @param message The message to be parsed.
 * @param component The component string.
 *
 * @return 0 on success, -1 on failure.
 */
int8_t manticore_fp_log_parser_init (struct manticore_logging_record *rcv_entry, char *message,
	size_t message_size)
{
	uint32_t parser_array[MANTICORE_FP_LOG_ARGS_SPLITTER_MAX] = {0};
	uint8_t split_array_size = 0;

	struct telemetry_fp_parser fp_parser;
	struct telemetry_fp_token_parser fp_token_parser;

	if (rcv_entry->msg_index == 0) {
		return -1;
	}

	//Initialize the parser
	if (rcv_entry->component == LOGGING_COMPONENT_MANTICORE_FP0) {
		fp_parser.msg_str = telemetry_fp0_logging_component_str[rcv_entry->msg_index];
		fp_parser.split_args = telemetry_fp0_log_split_args[rcv_entry->msg_index];
		fp_token_parser.arg_spliter = telemetry_fp0_log_split_args[rcv_entry->msg_index];
	}
	else if (rcv_entry->component == LOGGING_COMPONENT_MANTICORE_FP1) {
		fp_parser.msg_str = telemetry_fp1_logging_component_str[rcv_entry->msg_index];
		fp_parser.split_args = telemetry_fp1_log_split_args[rcv_entry->msg_index];
		fp_token_parser.arg_spliter = telemetry_fp1_log_split_args[rcv_entry->msg_index];
	}
	else if (rcv_entry->component == LOGGING_COMPONENT_MANTICORE_FP2) {
		fp_parser.msg_str = telemetry_fp2_logging_component_str[rcv_entry->msg_index];
		fp_parser.split_args = telemetry_fp2_log_split_args[rcv_entry->msg_index];
		fp_token_parser.arg_spliter = telemetry_fp2_log_split_args[rcv_entry->msg_index];
	}
	else {
		return -1;
	}

	//Initialize the token parser
	fp_parser.msg_index = rcv_entry->msg_index;
	fp_token_parser.arg1 = rcv_entry->arg1;
	fp_token_parser.arg2 = rcv_entry->arg2;
	fp_token_parser.arg1_cnt = 0;
	fp_token_parser.arg2_cnt = 0;
	fp_token_parser.arg_parser = parser_array;
	split_array_size = MANTICORE_FP_LOG_ARGS_SPLITTER_MAX;

	if (manticore_fp_log_args_parser_init (&fp_token_parser, &fp_token_parser.arg1,
		&fp_token_parser.arg2, parser_array, fp_parser.split_args, split_array_size) != 0) {
		return -1;
	}

	snprintf (message, message_size, fp_parser.msg_str, parser_array[0], parser_array[1],
		parser_array[2], parser_array[3], parser_array[4], parser_array[5], parser_array[6],
		parser_array[7]);

	return 0;
}
