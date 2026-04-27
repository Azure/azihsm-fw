#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/1sp_functions.sh"; then
	exit 1
fi

declare -i OPTIND=
OPTNAM=
OPTARG=
aes_key=
fw_key2=

while getopts :e:k: OPTNAM; do
	case "$OPTNAM" in
		e)
			aes_key=$OPTARG
		;;

		k)
			fw_key2=$OPTARG
		;;

		*)
			break
		;;
	esac
done
shift `iopt_shift $#`

if (($# != 5)); then
	echo "Usage: $0 [-e aes key] [-k fw key2] <hex svn> <hex version> <load addr> <fw img> <fw key>"
	exit 1
fi

declare -i \
	svn=`parse_hex_value $1` \
	version=`parse_hex_value $2` \
	addr=`parse_hex_value $3`

fw_binary=$4
fw_key=$5

test_svn $svn

validate_1sp_args "$fw_binary" "$fw_key" "$aes_key" "$fw_key2"

generate_1sp_filenames

fw_hash=`align_1sp_fw_and_get_digest "$fw_binary"`
declare -i fw_size=`stat -c %s "$fw_binary"`
iv=

if try_1sp_encrypt_img "$fw_binary" "$aes_key" "$fw_enc"; then
	fw_binary="$fw_enc"
fi

create_1sp_signed_header $svn $version $addr $fw_size $fw_hash "$hdr_out"

create_1sp_unsigned_header $SP1_MAGIC "$aes_key" "$iv" "$unsigned_out"

grep -q "PUBLIC" "$fw_key"
if (($? != 0)); then
	retvals=( `generate_standard_signature "$hdr_out" "$fw_key" "$hdr_sig"` )
	sig_r=${retvals[0]}
	sig_s=${retvals[1]}

	output_standard_signature "$sig_r" "$sig_s" "$unsigned_out"

	if [ -n "$fw_key2" ]; then
		retvals=( `generate_standard_signature "$hdr_out" "$fw_key2" "$hdr_sig2"` )
		sig_r=${retvals[0]}
		sig_s=${retvals[1]}

		output_standard_signature "$sig_r" "$sig_s" "$unsigned_out"
	else
		pad_1sp_for_nul_key2 "$unsigned_out"
	fi

	cat "$unsigned_out" "$hdr_out" "$fw_binary" > "$fw_img"

	echo "$fw_img"
else
	echo "$unsigned_out"
	echo "$hdr_out"
	echo "$fw_binary"
fi
