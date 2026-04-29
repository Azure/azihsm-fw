#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/boot_functions.sh"; then
	exit 1
fi

declare -ri SP1_MAGIC=0x53504657

#ARGS: <fw binary file> <fw key file> <aes key file> <fw key 2>
validate_1sp_args() {
	local fw_binary=$1; shift
	local fw_key=$1; shift
	local aes_key=$1; shift
	local fw_key2=$1; shift

	if [ ! -e "$fw_binary" ]; then
		echo "Unknown firmware image: $fw_binary"
		exit 1
	fi

	if [ ! -e "$fw_key" ]; then
		echo "Unknown firmware signing key: $fw_key"
		exit 1
	fi

	if [ -n "$aes_key" ] && [ ! -e "$aes_key" ]; then
		echo "Unknown AES key file: $aes_key"
		exit 1
	fi

	if [ -n "$fw_key2" ] && [ ! -e "$fw_key2" ]; then
		echo "Unknown second firmware signing key: $fw_key2"
		exit 1
	fi
}

#ARGS: <postfix>
#EXPORTS: out_dir, fw_enc, hdr_out, hdr_sig, hdr_sig2 unsigned_out, fw_img
generate_1sp_filenames() {
	local postfix=

	init_filenames "$@"

	fw_enc=$out_dir/fw_1sp$postfix.enc
	hdr_out=$out_dir/fw_1sp_header$postfix.out
	hdr_sig=$out_dir/fw_1sp_header$postfix.sig
	hdr_sig2=$out_dir/fw_1sp_header$postfix.sig2
	unsigned_out=$out_dir/fw_1sp_unsigned$postfix.out
	fw_img=$out_dir/fw_1sp$postfix.img
}

#ARGS: <fw binary file>
align_1sp_fw_and_get_digest() {
	align_file "$*" 16 "FF"
	generate_sha384_digest "$*"
}

#ARGS: <fw binary file> <aes key> <output file>
#EXPORTS: key, iv
try_1sp_encrypt_img() {
	local fw_binary=$1; shift
	local aes_key=$1; shift
	local fw_enc=$1; shift

	if [ -n "$aes_key" ]; then
		local key=`cat "$aes_key" | xxd -p | tr -d '\n'`
		iv=`head --bytes=16 /dev/random | xxd -p`
		if ! openssl enc -aes-256-cbc -e -nosalt -nopad -in "$fw_binary" -out "$fw_enc" -K $key -iv $iv; then
			exit 1
		fi

		return 0
	else
		iv=00000000000000000000000000000000
		return 1
	fi
}

#ARGS: <svn> <version> <addr> <fw size> <fw hash> <header output file>
create_1sp_signed_header() {
	local svn=$1; shift
	local version=$1; shift
	local addr=$1; shift
	local size=$1; shift
	local hash=$1; shift
	local hdr_out=$1; shift

	empty_file "$hdr_out"
	output_binary_word "$(printf 0x%08x $svn)" "$hdr_out"
	output_binary_array "$(printf %016x $version)" "$hdr_out"
	output_binary_word "$(printf 0x%08x $addr)" "$hdr_out"
	output_binary_word "$size" "$hdr_out"
	output_binary_array "$hash" "$hdr_out"
}

#ARGS: <magic value> <encryption key> <encryption IV> <unsigned output file>
create_1sp_unsigned_header() {
	local magic=$1; shift
	local fw_key=$1; shift
	local iv=$1; shift
	local unsigned_out=$1; shift

	local flags=0
	if [ -n "$fw_key" ]; then
		flags=0x80
	fi

	empty_file "$unsigned_out"
	output_binary_word $magic "$unsigned_out"
	output_binary_byte $flags "$unsigned_out"
	output_binary_array '000000' "$unsigned_out"
	output_binary_array "$iv" "$unsigned_out"
}

#ARGS: <unsigned hdr>
pad_1sp_for_nul_key2() {
	add_padding "$*" 216 '00'
}
