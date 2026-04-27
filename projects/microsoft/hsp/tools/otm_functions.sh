#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/boot_functions.sh"; then
	exit 1
fi

declare -ri \
	OTM_MAGIC=0x6f776e74 \
	OTM_MAGIC_DUAL_ROOT=0x6f746d32 \
	OTM_SIZE=344
	OTM_SIZE_DUAL_ROOT=632

#ARGS: <root key file> <new key file>
validate_otm_args() {
	local root_key=$1; shift
	local new_key=$1; shift
	
	if [ ! -e "$root_key" ]; then
		echo "Unknown owner key: $root_key"
		exit 1
	fi

	if [ ! -e "$new_key" ]; then
		echo "Unknown new owner key: $new_key"
		exit 1
	fi
}

#ARGS: <fw_key file> <fw_key2 file> <new fw_key file> <new fw_key2 file>
validate_otm_args_dual_root() {
	local fw_key=$1; shift
	local fw_key2=$1; shift
	local new_fw_key=$1; shift
	local new_fw_key2=$1; shift
	
	if [ ! -e "$fw_key" ]; then
		echo "Unknown fw_key: $fw_key"
		exit 1
	fi

	if [ -n "$fw_key2" ] && [ ! -e "$fw_key2" ]; then
		echo "Unknown fw_key2: $fw_key2"
		exit 1
	fi

	if [ ! -e "$new_fw_key" ]; then
		echo "Unknown new_fw_key: $new_fw_key"
		exit 1
	fi

	if [ -n "$new_fw_key2" ] && [ ! -e "$new_fw_key2" ]; then
		echo "Unknown new_fw_key2: $new_fw_key2"
		exit 1
	fi
}

#EXPORTS: out_dir, keys_out, hdr_out, hdr_sig, unsigned_out, manifest_img
generate_otm_filenames() {
	local postfix=
	
	init_filenames "$@"
	
	keys_out=$out_dir/owner_trans_manifest_keys$postfix.out
	hdr_out=$out_dir/owner_trans_manifest_header$postfix.out
	hdr_sig=$out_dir/owner_trans_manifest_header$postfix.sig
	unsigned_out=$out_dir/owner_trans_manifest_unsigned$postfix.out
	manifest_img=$out_dir/owner_trans_manifest$postfix.img
}

#ARGS: <new_fw_key> <new_fw_key2> <keys file>
output_otm_keys_file() {
	local new_fw_key=$1; shift
	local new_fw_key2=$1; shift
	local keys_out=$1; shift

	empty_file "$keys_out"
	output_ecc_public_key "$new_fw_key" "$keys_out" P-384
	if [ -n "$new_fw_key2" ]; then
		output_ecc_public_key "$new_fw_key2" "$keys_out" P-384
	else
		add_padding "$keys_out" 192 "00"
	fi
}

#ARGS: <keys size> <keys hash> <header output file>
output_otm_signed_header() {
	local size=$1; shift
	local hash=$1; shift
	local hdr_out=$1; shift
	
	empty_file "$hdr_out"
	output_binary_word "$size" "$hdr_out"
	output_binary_array "$hash" "$hdr_out"
}

#ARGS: <manifest image file>
validate_otm_image_size() {
	validate_image_size $OTM_SIZE $*
}

#ARGS: <manifest image file>
validate_otm_image_size_dual_root() {
	validate_image_size $OTM_SIZE_DUAL_ROOT $*
}
