#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if [ $# -lt 2 ]; then
	echo "Usage: $0 <fw img> <hash algo> [auth key] [sig hash type]"
	exit 1
fi

script_dir=`dirname "$(realpath "$BASH_SOURCE")"`
if ! source "$script_dir/img_functions.sh"; then
	exit 1
fi

fw_img=$1
if [ ! -e "$fw_img" ]; then
	echo "Unknown firmware image: $fw_img"
	exit 1
fi

hash_type=$2
case "$hash_type" in
	sha256)
		hash_id="0"
	;;

	sha384)
		hash_id="1"
	;;

	sha512)
		hash_id="2"
	;;

	*)
		echo "Unknown hash algorithm: $hash_type"
		exit 1
	;;
esac

auth_key=$3
if [ -n "$auth_key" ] && [ ! -e "$auth_key" ]; then
	echo "Unknown authorizing key: $auth_key"
	exit 1
fi

sig_hash=$4
if [ -n "$sig_hash" ]; then
	case "$sig_hash" in
		sha256|sha384|sha512)
			:
		;;

		*)
			echo "Unknown signature algorithm: $sig_hash"
			exit 1
		;;
	esac
fi

if [ -z "$OUTPUT_DIR" ]; then
	out_dir="build"
else
	out_dir="$OUTPUT_DIR"
fi

auth_img=$out_dir/auth_fw_token.fw
auth_data=$out_dir/auth_fw_token.dat
auth_sig=$out_dir/auth_fw_token.sig
auth_token=$out_dir/auth_fw_token.bin

fw_img_length=`stat -c %s $fw_img`

empty_file "$auth_img"
output_binary_word "0x46575550" "$auth_img"
output_binary_word "$fw_img_length" "$auth_img"
output_binary_byte "$hash_id" "$auth_img"
generate_digest "$fw_img" "$hash_type" "$auth_img"

auth_data_length=`stat -c %s $auth_img`

empty_file "$auth_data"
output_binary_short "0" "$auth_data"
output_binary_short "$auth_data_length" "$auth_data"
cat $auth_img >> $auth_data

if [ -n "$auth_key" ]; then
	generate_signature "$auth_data" "$auth_sig" "$auth_key" "$sig_hash"
else
	empty_file "$auth_sig"
fi

cat $auth_data $auth_sig > $auth_token
echo $auth_token
