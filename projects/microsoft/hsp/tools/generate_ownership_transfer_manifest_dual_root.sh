#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/otm_functions.sh"; then
	exit 1
fi

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/rkm_functions.sh"; then
	exit 1
fi

declare -i OPTIND=
OPTNAM=
OPTARG=
new_key2=
root_key2=
test_file_generate=

while getopts :k:r:t: OPTNAM; do
	case "$OPTNAM" in
		k)
			new_key2=$OPTARG
		;;

		r)
			root_key2=$OPTARG
		;;

		t)
			test_file_generate=$OPTARG
		;;

		*)
			break
		;;
	esac
done
shift `iopt_shift $#`

if (($# != 2)); then
	echo "Usage: $0 [-k new owner key 2 ] [-r owner key 2] [-t test_file_generate] <owner key> <new owner key>"
	exit 1
fi

root_key=$1
new_key=$2
root_key2=`get_secondary_key "0" "$root_key2"`
new_key2=`get_secondary_key "0" "$new_key2"`

validate_otm_args_dual_root "$root_key" "$root_key2" "$new_key" "$new_key2"

generate_otm_filenames

output_otm_keys_file "$new_key" "$new_key2" "$keys_out"

retvals=( `get_file_attrs $keys_out` )
declare -i keys_size=${retvals[0]}
keys_hash=${retvals[1]}

output_otm_signed_header $keys_size $keys_hash "$hdr_out"

output_ecc_p384_public_key_unsigned_header_v2 $OTM_MAGIC_DUAL_ROOT "$root_key" "$unsigned_out" "$root_key2"

if [ -z "$pubin" ]; then
	retvals=( `generate_standard_signature "$hdr_out" "$root_key" "$hdr_sig"` )
	sig_r=${retvals[0]}
	sig_s=${retvals[1]}

	if [ -n "$root_key2" ]; then
		retvals=( `generate_standard_signature "$hdr_out" "$root_key2" "$hdr_sig"` )
		sig_2r=${retvals[0]}
		sig_2s=${retvals[1]}
	fi
	
	output_standard_image_v2 "$hdr_out" "$keys_out" "$sig_r" "$sig_s" "$unsigned_out" "$manifest_img" "$sig_2r" "$sig_2s" "$root_key2"
	
	validate_otm_image_size_dual_root "$manifest_img"

	if [ "$test_file_generate" == "true" ]; then
		# Run some additional processing on our generated files to create testing data.
		echo "Building test collateral"

		echo "Creating $manifest_img array at $manifest_img.c"
		`../../../../tools/testing/to_array.sh $manifest_img $manifest_img`

		echo "Creating $hdr_out signed hash at app_data.c"
		`openssl dgst -sha384 -binary $hdr_out | ../../../../tools/testing/to_array.sh -`

		echo "Creating $unsigned_out array at $unsigned_out.c"
		`../../../../tools/testing/to_array.sh $unsigned_out $unsigned_out`

		# Get the signatures from the $unsigned_out.c header file
		header="$(cat $unsigned_out.c)"
		data=`echo $header | cut -d "{" -f2 | cut -d "}" -f1`
		data=`echo $data | tr -d ' '`
		IFS=',' read -ra data_split <<< "$data"

		# data_array is an array of all the raw data in the unsigned header. Get signatures.
		authenticity_sig_raw=`echo "${data_split[@]: 196:96}"`
		authority_sig_raw=`echo "${data_split[@]: 292:96}"`

		# Now re-add the commas and write to file.
		authenticity_sig_raw=`echo $authenticity_sig_raw | tr ' ' ','`
		authority_sig_raw=`echo $authority_sig_raw | tr ' ' ','`

		echo "Creating signatures file in ./signatures.c"
		printf "%s" "$authenticity_sig_raw" $'\n' "$authority_sig_raw" > "./signatures.c"
	fi

	echo $manifest_img
else
	echo $unsigned_out
	echo $hdr_out
	echo $keys_out
fi
