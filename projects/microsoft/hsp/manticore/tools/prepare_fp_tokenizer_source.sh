#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

top=`dirname "$(realpath "cd ..")"`

##
# Require variables.
##
fp_dir=$top/fp/qmgr
fp_tokens_dat_file=$fp_dir/_out/tokenize/Tokens.dat
fp_tokenizer_src_path=$top/src/logging
python_script_path=$top/tools/telemetry_tokenizer

if [ ! -f $fp_tokens_dat_file ];then
	echo "$fp_tokens_dat_file not found."
	exit 1
fi

echo -e "\e[1;36mGenerating FP log tokenizer source files\e[0m"

revert_and_exit()
{
	# Exit and Revert required changes
	git checkout $python_script_path > /dev/null 2>&1 && git checkout $fp_tokenizer_src_path > /dev/null 2>&1
	exit 1
}

# collect sha256sum of present source files
curr_src_sha=$(sha256sum $fp_tokenizer_src_path/manticore_fp_log_tokens.c | awk '{print $1}')
curr_hdr_sha=$(sha256sum $fp_tokenizer_src_path/manticore_fp_log_tokens.h | awk '{print $1}')

##
# Run python script to prepare FP logging tokenizer source files
##
python $python_script_path/log_tokenizer.py $fp_tokens_dat_file $fp_tokenizer_src_path
if [ $? -ne 0 ]; then
	echo "ERROR: log_tokenizer.py failed."
	revert_and_exit
fi

# collect sha256sum of updated source files
derived_src_sha=$(sha256sum $fp_tokenizer_src_path/manticore_fp_log_tokens.c | awk '{print $1}')
derived_hdr_sha=$(sha256sum $fp_tokenizer_src_path/manticore_fp_log_tokens.h | awk '{print $1}')

if [ $curr_src_sha != $derived_src_sha ] || [ $curr_hdr_sha != $derived_hdr_sha ]; then
	# Information to update require in cerberus utility
	echo -e "\e[1;31mBuild causes FP logging tokenizer source update.\e[0m"
	echo -e "\e[1;31mFP logging component update detected which could fail to parse logs in Cerberus Utility.\e[0m"

	echo -e "\e[1;31mIt is advisable to sync $fp_tokenizer_src_path files changes in the Cerberus Utility.\e[0m"
else
	echo -e "No update require for Cerberus Utility FP logging tokenizer."
fi

