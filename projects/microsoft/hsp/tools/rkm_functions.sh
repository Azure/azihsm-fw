#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/boot_functions.sh"; then
	exit 1
fi

declare -ri \
	RKM_MAGIC=0x4d414e54 \
	RKM_MAGIC_DUAL_ROOT=0x524b4d32 \
	RKM_SIZE=448	\
	RKM_SIZE_DUAL_ROOT=640

#ARGS: <root_key> <fw_key> <2ndary key>
validate_rkm_args() {
	local root_key=$1; shift
	local fw_key=$1; shift
	local fw_key2=$1; shift

	if [ ! -e "$root_key" ]; then
		echo "Unknown root key: $root_key"
		exit 1
	fi

	if [ ! -e "$fw_key" ]; then
		echo "Unknown firmware key: $fw_key"
		exit 1
	fi

	if [ -n "$fw_key2" ] && [ ! -e "$fw_key2" ]; then
		echo "Unknown secondary key: $fw_key2"
		exit 1
	fi
}

#ARGS: <root_key> <root_2key> <fw_key> <2ndary key>
validate_rkm_args_v2() {
	local root_key=$1; shift
	local root_2key=$1; shift
	local fw_key=$1; shift
	local fw_key2=$1; shift

	if [ ! -e "$root_key" ]; then
		echo "Unknown root key: $root_key"
		exit 1
	fi

	if [ -n "$root_2key" ] && [ ! -e "$root_2key" ]; then
		echo "Unknown root2 key: $root_2key"
		exit 1
	fi

	if [ ! -e "$fw_key" ]; then
		echo "Unknown firmware key: $fw_key"
		exit 1
	fi

	if [ -n "$fw_key2" ] && [ ! -e "$fw_key2" ]; then
		echo "Unknown secondary key: $fw_key2"
		exit 1
	fi
}

#ARGS: <type> <secondary key>
get_secondary_key() {
	local -i type=$1; shift
	local fw_key2=

	case $type in
		0)
			fw_key2=$*
		;;

		1)
			fw_key2=$*
		;;

		*)
			echo "Manifest type $type is not valid (0 = Owner, 1 = Tenent)"
			exit 1
		;;
	esac

	echo "$fw_key2"
}

#EXPORTS: out_dir, keys_out, hdr_out, hdr_sig, unsigned_out, manifest_img
generate_rkm_filenames() {
	local postfix=

	init_filenames "$@"

	keys_out=$out_dir/rom_key_manifest_keys$postfix.out
	hdr_out=$out_dir/rom_key_manifest_header$postfix.out
	hdr_sig=$out_dir/rom_key_manifest_header$postfix.sig
	unsigned_out=$out_dir/rom_key_manifest_unsigned$postfix.out
	manifest_img=$out_dir/rom_key_manifest$postfix.img
}

#ARGS: <fw_key> <fw_key2> <keys file>
output_rkm_keys_file() {
	local fw_key=$1; shift
	local fw_key2=$1; shift
	local keys_out=$1; shift

	empty_file "$keys_out"
	output_ecc_public_key "$fw_key" "$keys_out" P-384
	if [ -n "$fw_key2" ]; then
		output_ecc_public_key "$fw_key2" "$keys_out" P-384
	else
		add_padding "$keys_out" 192 "00"
	fi
}

#ARGS: <type U32> <svn> <keys size> <keys hash> <2ndary key> <header output file>
output_rkm_signed_header() {
	local type=$1; shift
	local svn=$1; shift
	local size=$1; shift
	local hash=$1; shift
	local fw_key2=$1; shift
	local hdr_out=$1; shift

	local -i key_cnt=0x1
	if [ -n "$fw_key2" ]; then
		key_cnt=0x2
	fi

	empty_file "$hdr_out"
	output_binary_byte "$type" "$hdr_out"
	output_binary_byte "$key_cnt" "$hdr_out"
	output_binary_array '0000' "$hdr_out"
	output_binary_word "$svn" "$hdr_out"
	output_binary_word "$size" "$hdr_out"
	output_binary_array "$hash" "$hdr_out"
}

#ARGS: <manifest image file>
validate_rkm_image_size() {
	validate_image_size $RKM_SIZE $*
}

#ARGS: <manifest image file>
validate_rkm_image_size_dual_root() {
	validate_image_size $RKM_SIZE_DUAL_ROOT $*
}