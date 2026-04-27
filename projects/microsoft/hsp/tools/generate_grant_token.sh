#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/boot_functions.sh"; then
	exit 1
fi

declare -i ctr_size=0

declare -i OPTIND=
declare OPTNAM=
declare OPTARG=
while getopts :c: OPTNAM; do
	case "$OPTNAM" in
		c)
			ctr_size=$OPTARG
		;;

		*)
			break
		;;
	esac
done
shift `iopt_shift $#`

if (($# != 4)); then
	echo "Usage: $0 [-c <ctr size>] <owner key> <tenant key> <hmac key> <counter byte>"
	exit 1
fi

if ((ctr_size <= 0)); then
	ctr_size=256
fi

root_key=$1
tenant_key=$2
hmac_key=$3

declare -i ctr=$4

if [ ! -e "$root_key" ]; then
	echo "Unknown owner key: $root_key"
	exit 1
fi

if [ ! -e "$tenant_key" ]; then
	echo "Unknown tenant key: $tenant_key"
	exit 1
fi

if [ ! -e "$hmac_key" ]; then
	echo "Unknown HMAC key: $hmac_key"
	exit 1
fi

# NOTE: There are many more valid tenancy counter values,
# but these are the only ones currently supported in the script.
case $ctr in
	$((-1)) | $((0x01)) | $((0x07)) | $((0x1f)) | $((0x7f)) | $((0x1FFFFFFFF)))
	;;

	*)
		echo "Invalid tenancy counter value for an active tenancy transfer: $ctr"
		exit 1
	;;
esac

out_dir=`get_output_dir`
temp=$out_dir/grant_token.tmp
token=$out_dir/grant_token.bin

keys_out=$out_dir/owner_trans_manifest_keys.out
hdr_out=$out_dir/owner_trans_manifest_header.out
hdr_sig=$out_dir/owner_trans_manifest_header.sig
unsigned_out=$out_dir/owner_trans_manifest_unsigned.out
manifest_img=$out_dir/owner_trans_manifest.img

output_ecc_p384_public_key_to_new_file "$root_key" "$temp"

digest=`generate_sha384_digest $temp`

empty_file "$temp"
output_binary_array "$digest" "$temp"

if ((ctr >= 0)); then
	while ((ctr > 0)); do
		output_binary_byte $((ctr & 0xFF)) "$temp"
		((ctr >>= 8))
		((--ctr_size))
	done
	if ((ctr_size)); then
		head -c $ctr_size /dev/zero >> "$temp"
	fi
else
	for ((i=1; i < ctr_size; ++i)); do
		output_binary_byte $((0xFF)) "$temp"
	done
	output_binary_byte $((0x7F)) "$temp"
fi
output_ecc_public_key "$tenant_key" "$temp" P-384

openssl dgst -sha384 -mac hmac -macopt hexkey:$(cat "$hmac_key" | xxd -p | tr -d '\n') -binary "$temp" > "$token"
if (($? != 0)); then
	exit 1
fi

echo $token