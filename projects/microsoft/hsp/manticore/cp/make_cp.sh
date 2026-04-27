#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

set -e
top=`dirname "$(realpath "$BASH_SOURCE")"`
cp_dir=$top/hsm
cp_target_dir=$cp_dir/target
cp_build_dir=$cp_target_dir/thumbv7em-none-eabihf

# Flag to clean everything and rebuild the image
rebuild=0

# Flag to build the app-debug image
build_type_debug=0

# Additional features to provide for CP build.  See cp/hsm/app/Cargo.toml for available features.
cp_features=

##
# Parse input arguments to adjust build properties.
##
ARGS=`getopt --unquoted -o "" -l "rebuild,cp_features:" -- "$@"`
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

		--cp_features)
			if [[ "$2" =~ ^-- ]] || [ -z "$2" ]; then
				echo "Error: --cp_features requires a comma-delimited list of features"
				exit 1
			fi
			cp_features="--features=$2"
			shift 2
		;;

		--)
			shift
		;;
	esac
done

##
# Build the CP firmware.
##

if [ $rebuild -ne 0 ]; then
	echo "Removing $cp_target_dir"
	rm -rf $cp_target_dir
fi

pushd $cp_dir >/dev/null

echo "CP - Release"
cp_build_dir=$cp_build_dir/firmware
cargo xtask app-release $cp_features

pushd $cp_build_dir >/dev/null

TARGET_NAME=mcr-admin
TARGET_IMG=$TARGET_NAME
arm-none-eabi-objdump -xdhlSC ${TARGET_NAME} > ${TARGET_IMG}.dis
# Note: The following sections have been derived by identifying sections in the *.dis file generated above. .bss, .uninit and .debug_* have been omitted intentionally.
arm-none-eabi-objcopy -O binary --set-section-flags .bss=alloc,load,contents -j .vector_table -j .text -j .rodata -j .gnu.sgstubs ${TARGET_NAME} ${TARGET_IMG}.text.bin
arm-none-eabi-objcopy -O binary --set-section-flags .bss=alloc,load,contents -j .data ${TARGET_NAME} ${TARGET_IMG}.data.bin

TARGET_NAME=mcr-hsm
TARGET_IMG=$TARGET_NAME
arm-none-eabi-objdump -xdhlSC ${TARGET_NAME} > ${TARGET_IMG}.dis
# Note: The following sections have been derived by identifying sections in the *.dis file generated above. .bss, .uninit and .debug_* have been omitted intentionally.
arm-none-eabi-objcopy -O binary --set-section-flags .bss=alloc,load,contents -j .vector_table -j .text -j .rodata -j .gnu.sgstubs ${TARGET_NAME} ${TARGET_IMG}.text.bin
arm-none-eabi-objcopy -O binary --set-section-flags .bss=alloc,load,contents -j .data ${TARGET_NAME} ${TARGET_IMG}.data.bin

popd >/dev/null

cp_list=$cp_build_dir/cp.list
cp0_data_bin=$cp_build_dir/mcr-admin.data.bin
cp0_text_bin=$cp_build_dir/mcr-admin.text.bin
cp1_data_bin=$cp_build_dir/mcr-hsm.data.bin
cp1_text_bin=$cp_build_dir/mcr-hsm.text.bin

if [[ -f $cp1_text_bin && -f $cp0_text_bin && -f $cp0_data_bin && -f $cp1_data_bin ]]; then
	echo "$cp0_text_bin,0x60220000 $cp1_text_bin,0x60000000 $cp0_data_bin,0x60200000 $cp1_data_bin,0x60600000" > $cp_list
else
	exit 1;
fi

popd >/dev/null
