#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if [ $# -lt 5 ]; then
	echo "Usage: $0 <key id> <root key> <pfm key> <max bit size> <sig hash> [output dir]"
	exit 1
fi

key_id=`printf "%d" $1`
if [ $? -ne 0 ]; then
	echo "Invalid key ID: $1"
	exit 1
fi

root_key=$2
if [ ! -e "$root_key" ]; then
	echo "Unknown root key: $root_key"
	exit 1
fi

manifest_key=$3
if [ ! -e "$manifest_key" ]; then
	echo "Unknown manifest key: $manifest_key"
	exit 1
fi

max_bit=$4
sig_hash_type=$5

if [ -n "$BUILD_SOURCESDIRECTORY" ]; then
	DIR=$BUILD_SOURCESDIRECTORY
else
	DIR=`dirname ${0}`
fi

source $DIR/img_functions.sh
if [ $? -ne 0 ]; then
	exit 1
fi

out_dir=$6
if [ -z "$out_dir" ]; then
	out_dir=./build
fi

key_pub=$out_dir/manifest_key.pub
key_out=$out_dir/manifest_key.out
key_sig=$out_dir/manifest_key.sig
key_img=$out_dir/manifest_key.img

determine_key_type "$root_key"
root_key_type=$key_type

determine_key_type "$manifest_key"
manifest_key_type=$key_type

if [ "$root_key_type" != "$manifest_key_type" ]; then
	echo "Mismatched keys: root=$root_key_type, manifest=$manifest_key_type"
	exit 1
fi

if [ "$manifest_key_type" = "RSA" ]; then
	let 'max_len = max_bit / 8'
	max_pub=$max_len
	max_sig=$max_len

	get_public_key "$manifest_key"
	manifest_pub=$key_mod
else
	let 'max_len = (max_bit + 7) / 8'
	key_len=$max_len

	get_ecc_der_public_key_max_length
	max_pub=$der_len

	get_ecc_max_signature_length
	max_sig=$sig_len

	get_ecc_der_public_key "$manifest_key"
	manifest_pub=$key_pub_der
fi

if [ $key_len -gt $max_len ]; then
	echo "Unsupported key length: $key_len > $max_len"
	exit 1
fi

get_max_signature_length "$root_key"
if [ $sig_len -gt $max_sig ]; then
	echo "Unsupported signature length: $sig_len > $max_sig"
	exit 1
fi

empty_file "$key_pub"
output_binary_array "$manifest_pub" "$key_pub"
add_padding "$key_pub" "$max_pub" "00"

empty_file "$key_out"
output_binary_word "$key_id" "$key_out"
cat $key_pub >> $key_out
if [ "$manifest_key_type" = "RSA" ]; then
	output_binary_word "$key_len" "$key_out"
	output_binary_word "$key_exp" "$key_out"
fi

if [ "$root_key_type" = "RSA" ]; then
	get_public_key "$root_key"
else
	check_ecc_curve "$root_key"
fi
if [ -z "$pubin" ]; then
	generate_signature "$key_out" "$key_sig" "$root_key" "$sig_hash_type"
	add_padding "$key_sig" "$max_sig" "00"
else
	empty_file "$key_sig"
fi

cat $key_out $key_sig > $key_img
echo $key_img
