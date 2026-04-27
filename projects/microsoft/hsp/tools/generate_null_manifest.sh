#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/boot_functions.sh"; then
	exit 1
fi

declare -i NUL_MAGIC=0x4e554c4c

out_dir=`get_output_dir`
manifest_img=$out_dir/rom_null_manifest.img

empty_file "$manifest_img"
output_binary_word $NUL_MAGIC "$manifest_img"

echo $manifest_img
