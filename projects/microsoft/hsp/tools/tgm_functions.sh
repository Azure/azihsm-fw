#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/boot_functions.sh"; then
	exit 1
fi

declare -ri \
	TGM_MAGIC=0x746d616e \
	TGM_SIZE=300

#ARGS: <fw key> <token type> <token> [grant key]
validate_tgm_args() {
	local fw_key=$1; shift
	local type=$1; shift
	local token=$1; shift
	local grant_key=$1; shift
	
	if [ ! -e "$fw_key" ]; then
		echo "Unknown firmware key: $fw_key"
		exit 1
	fi

	case "$type" in
		file)
			if [ ! -e "$token" ]; then
				echo "Unknown grant token: $token"
				exit 1
			fi
		;;

		hex)
			token_len=`echo -n "$token" | wc --bytes`
			((token_len /= 2))
			if ((token_len != 48)); then
				echo "Expected a 48 byte token: len=$token_len"
				exit 1
			fi
		;;

		*)
			echo "Unknown token type: $type"
			exit 1
		;;
	esac

	if [ -n "$grant_key" ] && [ ! -e "$grant_key" ]; then
		echo "Unknown root key: $grant_key"
		exit 1
	fi
}

#EXPORTS: out_dir, keys_out, hdr_out, hdr_sig, unsigned_out, manifest_img
generate_tgm_filenames() {
	local postfix=
	
	init_filenames "$@"
	
	keys_out=$out_dir/grant_manifest_keys$postfix.out
	hdr_out=$out_dir/grant_manifest_header$postfix.out
	hdr_sig=$out_dir/grant_manifest_header$postfix.sig
	unsigned_out=$out_dir/grant_manifest_unsigned$postfix.out
	manifest_img=$out_dir/grant_manifest$postfix.img
}

#ARGS: <token type> <grant token> <fw key> <keys output file>
output_tgm_keys() {
	local type=$1; shift
	local token=$1; shift
	local fw_key=$1; shift
	local keys_out=$1; shift
	
	# Keys contained in the manifest
	if [ "$type" = file ]; then
		cat "$token" > "$keys_out"
	else
		empty_file "$keys_out"
		output_binary_array "$token" "$keys_out"
	fi
	output_ecc_public_key "$fw_key" "$keys_out" P-384
}

#ARGS: <keys size> <keys hash> <header output file>
output_tgm_signed_header() {
	local size=$1; shift
	local hash=$1; shift
	local hdr_out=$1; shift
	
	empty_file "$hdr_out"
	output_binary_word 0 "$hdr_out"
	output_binary_word "$size" "$hdr_out"
	output_binary_array "$hash" "$hdr_out"
}

#ARGS: <magic> <header output file>
output_tgm_unsigned_header() {
	local magic=$1; shift
	local unsigned_out=$1; shift
	
	empty_file "$unsigned_out"
	output_binary_word "$magic" "$unsigned_out"
}

#ARGS: <image file>
validate_tgm_image_size() {
	validate_image_size $TGM_SIZE $*
}
