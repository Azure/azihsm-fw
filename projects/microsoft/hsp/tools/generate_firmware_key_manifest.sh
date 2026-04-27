#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/boot_functions.sh"; then
	exit 1
fi

declare -i OPTIND=
OPTNAM=
OPTARG=
key0=
key1=
key2=
key3=
key4=
key5=
key6=
key7=
key8=
key9=
secondary_key=

while getopts :0:1:2:3:4:5:6:7:8:9:k: OPTNAM; do
	case "$OPTNAM" in
		0)
			key0=$OPTARG
		;;

		1)
			key1=$OPTARG
		;;

		2)
			key2=$OPTARG
		;;

		3)
			key3=$OPTARG
		;;

		4)
			key4=$OPTARG
		;;

		5)
			key5=$OPTARG
		;;

		6)
			key6=$OPTARG
		;;

		7)
			key7=$OPTARG
		;;

		8)
			key8=$OPTARG
		;;

		9)
			key9=$OPTARG
		;;

		k)
			secondary_key=$OPTARG
		;;

		*)
			break
		;;
	esac
done
shift `iopt_shift $#`

if (($# < 2)); then
	echo "Usage: $0 [-<#> key_slot# ] [-k <secondary key>] <hex svn> <root key>"
	exit 1
fi

declare -i svn=`parse_hex_value $1`
root_key=$2

if [ ! -e "$root_key" ]; then
	echo "Unknown root key: $root_key"
	exit 1
fi

if [ -n "$secondary_key" ] && [ ! -e "$secondary_key" ]; then
	echo "Unknown secondary key: $secondary_key"
	exit 1
fi

if [ -n "$key0" ] && [ ! -e "$key0" ]; then
	echo "Unknown slot 0 key: $key0"
	exit 1
fi

if [ -n "$key1" ] && [ ! -e "$key1" ]; then
	echo "Unknown slot 1 key: $key1"
	exit 1
fi

if [ -n "$key2" ] && [ ! -e "$key2" ]; then
	echo "Unknown slot 2 key: $key2"
	exit 1
fi

if [ -n "$key3" ] && [ ! -e "$key3" ]; then
	echo "Unknown slot 3 key: $key3"
	exit 1
fi

if [ -n "$key4" ] && [ ! -e "$key4" ]; then
	echo "Unknown slot 4 key: $key4"
	exit 1
fi

if [ -n "$key5" ] && [ ! -e "$key5" ]; then
	echo "Unknown slot 5 key: $key5"
	exit 1
fi

if [ -n "$key6" ] && [ ! -e "$key6" ]; then
	echo "Unknown slot 6 key: $key6"
	exit 1
fi

if [ -n "$key7" ] && [ ! -e "$key7" ]; then
	echo "Unknown slot 7 key: $key7"
	exit 1
fi

if [ -n "$key8" ] && [ ! -e "$key8" ]; then
	echo "Unknown slot 8 key: $key8"
	exit 1
fi

if [ -n "$key9" ] && [ ! -e "$key9" ]; then
	echo "Unknown slot 9 key: $key9"
	exit 1
fi

test_svn $svn

init_filenames
keys_out=$out_dir/firmware_key_manifest_keys.out
hdr_out=$out_dir/firmware_key_manifest_header.out
hdr_sig=$out_dir/firmware_key_manifest_header.sig
hdr_sig2=$out_dir/firmware_key_manifest_header.sig2
unsigned_out=$out_dir/firmware_key_manifest_unsigned.out
manifest_img=$out_dir/firmware_key_manifest.img

add_key() {
	local slot=$1
	local out=$2
	local key=$3
	local total_size=

	let 'total_size = (slot + 1) * 120'

	if [ -n "$key" ]; then
		get_ecc_der_public_key $key

		if [ $key_len -gt 48 ]; then
			echo "Key in slot $slot is too long ($key_length bytes)"
			exit 1
		fi

		output_binary_array "$key_pub_der" "$out"
	fi

	add_padding "$out" "$total_size" "00"
}

empty_file "$keys_out"
add_key 0 "$keys_out" "$key0"
add_key 1 "$keys_out" "$key1"
add_key 2 "$keys_out" "$key2"
add_key 3 "$keys_out" "$key3"
add_key 4 "$keys_out" "$key4"
add_key 5 "$keys_out" "$key5"
add_key 6 "$keys_out" "$key6"
add_key 7 "$keys_out" "$key7"
add_key 8 "$keys_out" "$key8"
add_key 9 "$keys_out" "$key9"

key_attr=(`get_file_attrs $keys_out`)

empty_file "$hdr_out"
output_binary_qword $svn "$hdr_out"
output_binary_word "${key_attr[0]}" "$hdr_out"
output_binary_array "${key_attr[1]}" "$hdr_out"

check_ecc_curve "$root_key" "P-384"

empty_file "$unsigned_out"
output_binary_word "0x46574b4d" "$unsigned_out"
if [ -z "$pubin" ]; then
	generate_signature "$hdr_out" "$hdr_sig" "$root_key" "sha384"

	cat $hdr_sig >> $unsigned_out
	add_padding "$unsigned_out" "108" "00"

	if [ -n "$secondary_key" ]; then
		check_ecc_curve "$secondary_key" "P-384"
		generate_signature "$hdr_out" "$hdr_sig2" "$secondary_key" "sha384"

		cat $hdr_sig2 >> $unsigned_out
	fi
	add_padding "$unsigned_out" "212" "00"

	cat $unsigned_out $hdr_out $keys_out > $manifest_img
	echo $manifest_img
else
	echo $unsigned_out
	echo $hdr_out
	echo $keys_out
fi
