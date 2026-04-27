#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

set -e
top=`dirname "$(realpath "$BASH_SOURCE")"`
fp_dir=$top/qmgr
fp_out_dir=$fp_dir/_out
fp_build_dir=$fp_dir/build
fp_tokenize_dir=$fp_dir/tokenize
fp_package_dir=$fp_dir/package

# Flag to clean everything and rebuild the image
rebuild=0

# Flag to fetch fp_features arguments
if [[ "$@" == *"--fp_features="* ]]; then
	fp_features=$(echo "$@" | sed -r 's/.*--fp_features=([^ ]+).*/\1/')
fi

##
# Parse input arguments to adjust build properties.
##
ARGS=`getopt --unquoted -o "" -l "rebuild,fp_features:" -- "$@"`
if [ $? -ne 0 ]; then
	exit 1
fi

set -- $ARGS
while [ $# -gt 0 ]; do
	case "$1" in
		--rebuild)
			rebuild=1
			shift
		;;

		--fp_features)
			# Build the FP image with additional features.
			if [[ "$2" =~ ^-- ]] || [ -z "$2" ]; then
				echo "Error: Invalid fp_features. Supported features: --fp_features=pre_reset,post_reset"
				exit 1
			fi
			fp_features="$1=$2"
			shift 2
		;;

		--)
			shift
		;;
	esac
done

##
# Build the FP firmware.
##

if [ $rebuild -ne 0 ]; then
	echo "Removing $fp_out_dir $fp_build_dir $fp_tokenize_dir $fp_package_dir"
	rm -rf $fp_out_dir
	rm -rf $fp_build_dir
	rm -rf $fp_tokenize_dir
	rm -rf $fp_package_dir
fi

#IDFU fault injection flags to enable in fp/qmgr/Makefile
if echo "$fp_features" | grep -q "idfu_fault_pre_reset_fps_shutdown_ipc_resp_err"; then
	fp_features=$(echo "$fp_features" | sed 's/ *--fp_features.*/PRERESETFAULTINJECTION=ENABLE/')
elif echo "$fp_features" | grep -q "idfu_fault_post_reset_fps_prepare_no_resp_err"; then
	fp_features=$(echo "$fp_features" | sed 's/ *--fp_features.*/POSTRESETFAULTINJECTION=ENABLE/')
# For parity with CP firmware image whenever the mcr_test_hooks is enabled in CP, FPs mcr test hooks need to be enabled as well
elif echo "$cp_features" | grep -q "mcr_test_hooks"; then
	fp_features=$(echo "$fp_features" | sed 's/ *--fp_features.*/MCRTESTHOOKS=ENABLE/')
fi

pushd $fp_dir >/dev/null
./mk.sh $fp_features
if [ $? -ne 0 ]; then
	exit 1;
fi

fp0_list=$fp_out_dir/fp0.list
fp1_list=$fp_out_dir/fp1.list
fp2_list=$fp_out_dir/fp2.list
fp0_text_bin=$fp_out_dir/fps_cpu0_compile/fps_cpu0.text.bin
fp1_text_bin=$fp_out_dir/fps_cpu1_compile/fps_cpu1.text.bin
fp2_text_bin=$fp_out_dir/fps_cpu2_compile/fps_cpu2.text.bin
fp0_data_bin=$fp_out_dir/fps_cpu0_compile/fps_cpu0.data.bin
fp1_data_bin=$fp_out_dir/fps_cpu1_compile/fps_cpu1.data.bin
fp2_data_bin=$fp_out_dir/fps_cpu2_compile/fps_cpu2.data.bin

if [[ -f $fp0_text_bin && -f $fp0_data_bin ]]; then
	echo "$fp0_text_bin,0xA2000000 $fp0_data_bin,0xA3000000" > $fp0_list
else
	exit 1;
fi

if [[ -f $fp1_text_bin && -f $fp1_data_bin ]]; then
	echo "$fp1_text_bin,0xA2200000 $fp1_data_bin,0xA3200000" > $fp1_list
else
	exit 1;
fi

if [[ -f $fp2_text_bin && -f $fp2_data_bin ]]; then
	echo "$fp2_text_bin,0xA2400000 $fp2_data_bin,0xA3400000" > $fp2_list
else
	exit 1;
fi
