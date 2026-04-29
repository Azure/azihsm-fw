#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

script_dir=`dirname "$(realpath "$BASH_SOURCE")"`
if ! source "$script_dir/../../img_functions.sh"; then
	exit 1
fi

iopt_shift() {
	if ((OPTIND <= $*)); then
		((--OPTIND))
	fi

	echo $OPTIND
}

#ARGS: <bDebug> ...<args>
echo_dbg() {
	local -i bDebug=$1; shift

	if ((bDebug)); then
		echo "$@" 1>&2
	fi
}

#ARGS: <bDebug> ...<args>
printf_dbg() {
	local -i bDebug=$1; shift

	if ((bDebug)); then
		printf "$@" 1>&2
	fi
}

#ARGS: <hex value>
parse_hex_value() {
	local a=$*

	if [ "${a:0:2}" = 0x ]; then
		local -i a
	else
		local -i a=0x$a
	fi

	echo $a
}

#ARGS: <svn>
test_svn() {
	local -i svn=$*

	if (((svn & (svn + 1)) != 0)); then
		echo "SVN $1 not a valid value.  Must have only contiguous 0's and 1's."
		exit 1
	fi
}


#ARGS: <postfix value>
get_filename_prefix() {
	if (($#)); then
		echo "$*_"
	fi
}

#ARGS: <postfix value>
get_filename_postfix() {
	if (($#)); then
		echo "_$*"
	fi
}

get_output_dir() {
	if [ -z "$OUTPUT_DIR" ]; then
		echo build
	else
		echo "$OUTPUT_DIR"
	fi
}

#EXPORTS: out_dir, postfix
init_filenames() {
	out_dir=`get_output_dir`
	postfix=`get_filename_postfix $*`

	if [ ! -d $out_dir ]; then
		mkdir -p $out_dir
		if [ $? -ne 0 ]; then
			exit 1
		fi
	fi
}

#ARGS: <file>
get_filesize() {
	stat -c %s "$*"
}

#ARGS: <key file> <keys output file>
output_ecc_p384_public_key_to_new_file() {
	local fw_key=$1; shift
	local keys_out=$1; shift

	empty_file "$keys_out"
	output_ecc_public_key "$fw_key" "$keys_out" P-384
}

#ARGS: <magic> <key file> <header output file>
output_ecc_p384_public_key_unsigned_header() {
	local -i magic=$1; shift
	local fw_key=$1; shift
	local keys_out=$1; shift

	empty_file "$keys_out"
	output_binary_word "$magic" "$keys_out"
	output_ecc_public_key "$fw_key" "$keys_out" P-384
}

#ARGS: <magic> <key file> <header output file> <key2 file>
output_ecc_p384_public_key_unsigned_header_v2() {
	local -i magic=$1; shift
	local fw_key=$1; shift
	local keys_out=$1; shift
	local fw_2key=$1; shift

	empty_file "$keys_out"
	output_binary_word "$magic" "$keys_out"
	output_ecc_public_key "$fw_key" "$keys_out" P-384
	if [ -n "$fw_2key" ]; then
		output_ecc_public_key "$fw_2key" "$keys_out" P-384
	else
		add_padding "$keys_out" 196 "00"
	fi

}

#ARGS: <file>
generate_sha384_digest() {
	local digest=

	generate_digest "$*" sha384
	echo "$digest"
}

#ARGS: <file>
get_file_attrs() {
	get_filesize $*
	generate_sha384_digest $*
}

#ARGS: <header output file> <signing key file> <header signature file>
generate_standard_signature() {
	local hdr_out=$1; shift
	local fw_key=$1; shift
	local hdr_sig=$1; shift

	local sig_r=
	local sig_s=

	generate_signature "$hdr_out" "$hdr_sig" "$fw_key" sha384
	parse_ecc_signature "$hdr_sig" 48
	echo "$sig_r" "$sig_s"
}

#ARGS: <sig_r> <sig_s> <unsigned header>
output_standard_signature() {
	local sig_r=$1; shift
	local sig_s=$1; shift
	local unsigned_out=$1; shift

	output_binary_array "$sig_r" "$unsigned_out"
	output_binary_array "$sig_s" "$unsigned_out"
}

#ARGS: <signed header> <payload> <sig_r> <sig_s> <unsigned header> <output image file>
output_standard_image() {
	local hdr_sig=$1; shift
	local binary=$1; shift
	local sig_r=$1; shift
	local sig_s=$1; shift
	local unsigned_out=$1; shift
	local img=$1; shift

	output_standard_signature "$sig_r" "$sig_s" "$unsigned_out"

	cat "$unsigned_out" "$hdr_sig" "$binary" > "$img"
}

#ARGS: <signed header> <payload> <sig_r> <sig_s> <unsigned header> <output image file> <sig_2r> <sig_2s> <root_2key>
output_standard_image_v2() {
	local hdr_sig=$1; shift
	local binary=$1; shift
	local sig_r=$1; shift
	local sig_s=$1; shift
	local unsigned_out=$1; shift
	local img=$1; shift
	local sig_2r=$1; shift
	local sig_2s=$1; shift
	local root_2key=$1; shift

	output_standard_signature "$sig_r" "$sig_s" "$unsigned_out"
	if [ -n "$root_2key" ]; then
		output_standard_signature "$sig_2r" "$sig_2s" "$unsigned_out"
	else
		add_padding "$unsigned_out" 388 "00"
	fi

	cat "$unsigned_out" "$hdr_sig" "$binary" > "$img"
}

#ARGS: <expected size> <image file>
validate_image_size() {
	local -i expect=$1; shift
	local -i size=`get_filesize $*`

	if ((size != expect)); then
		echo "Unexpected image size.  Size is $size, but should be $expect."
		exit 1
	fi
}
