#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/tgm_functions.sh"; then
	exit 1
fi

declare -i OPTIND=
OPTNAM=
OPTARG=
grant_key=

while getopts :k: OPTNAM; do
	case "$OPTNAM" in
		k)
			grant_key=$OPTARG
		;;
		
		*)
			break
		;;
	esac
done
shift `iopt_shift $#`

if (($# != 3)); then
	echo "Usage: $0 [-k <grant key>] <FW key> <token type> <token>"
	exit 1
fi

fw_key=$1
type=$2
grant_token=$3

validate_tgm_args "$fw_key" "$type" "$grant_token" "$grant_key"

generate_tgm_filenames

output_tgm_keys "$type" "$grant_token" "$fw_key" "$keys_out"

retvals=( `get_file_attrs $keys_out` )
declare -i keys_size=${retvals[0]}
keys_hash=${retvals[1]}

output_tgm_signed_header $keys_size $keys_hash "$hdr_out"

output_tgm_unsigned_header $TGM_MAGIC "$unsigned_out"

if [ -n "$grant_key" ]; then
	retvals=( `generate_standard_signature "$hdr_out" "$grant_key" "$hdr_sig"` )
	sig_r=${retvals[0]}
	sig_s=${retvals[1]}
	output_standard_image "$hdr_out" "$keys_out" "$sig_r" "$sig_s" "$unsigned_out" "$manifest_img"

	validate_tgm_image_size "$manifest_img"

	echo $manifest_img
else
	echo $unsigned_out
	echo $hdr_out
	echo $keys_out
fi
