#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if (($# != 2)); then
	echo "Usage: $0 <owner key> <new owner key>"
	exit 1
fi

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/otm_functions.sh"; then
	exit 1
fi

root_key=$1
new_key=$2

validate_otm_args "$root_key" "$new_key"

generate_otm_filenames

output_ecc_p384_public_key_to_new_file "$new_key" "$keys_out"

retvals=( `get_file_attrs $keys_out` )
declare -i keys_size=${retvals[0]}
keys_hash=${retvals[1]}

output_otm_signed_header $keys_size $keys_hash "$hdr_out"

output_ecc_p384_public_key_unsigned_header $OTM_MAGIC "$root_key" "$unsigned_out"

if [ -z "$pubin" ]; then
	retvals=( `generate_standard_signature "$hdr_out" "$root_key" "$hdr_sig"` )
	sig_r=${retvals[0]}
	sig_s=${retvals[1]}
	
	output_standard_image "$hdr_out" "$keys_out" "$sig_r" "$sig_s" "$unsigned_out" "$manifest_img"
	
	validate_otm_image_size "$manifest_img"

	echo $manifest_img
else
	echo $unsigned_out
	echo $hdr_out
	echo $keys_out
fi
