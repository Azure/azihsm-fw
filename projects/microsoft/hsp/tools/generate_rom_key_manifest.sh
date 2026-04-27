#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/rkm_functions.sh"; then
	exit 1
fi

declare -i OPTIND=
OPTNAM=
OPTARG=
fw_key2=

while getopts :k: OPTNAM; do
	case "$OPTNAM" in
		k)
			fw_key2=$OPTARG
		;;

		*)
			break
		;;
	esac
done
shift `iopt_shift $#`

if (($# != 4)); then
	echo "Usage: $0 [-k fw_key2 ] <hex svn> <type> <root key> <FW key>"
	exit 1
fi

declare -i \
	svn=`parse_hex_value $1` \
	type=$2

root_key=$3
fw_key=$4
fw_key2=`get_secondary_key $type "$fw_key2"`

test_svn $svn

validate_rkm_args "$root_key" "$fw_key" "$fw_key2"

generate_rkm_filenames

output_rkm_keys_file "$fw_key" "$fw_key2" "$keys_out"

retvals=( `get_file_attrs $keys_out` )
declare -i keys_size=${retvals[0]}
keys_hash=${retvals[1]}

output_rkm_signed_header $type $svn $keys_size $keys_hash "$fw_key2" "$hdr_out"

output_ecc_p384_public_key_unsigned_header $RKM_MAGIC "$root_key" "$unsigned_out"

if [ -z "$pubin" ]; then
	retvals=( `generate_standard_signature "$hdr_out" "$root_key" "$hdr_sig"` )
	sig_r=${retvals[0]}
	sig_s=${retvals[1]}

	output_standard_image "$hdr_out" "$keys_out" "$sig_r" "$sig_s" "$unsigned_out" "$manifest_img"

	validate_rkm_image_size "$manifest_img"

	echo $manifest_img
else
	echo $unsigned_out
	echo $hdr_out
	echo $keys_out
fi
