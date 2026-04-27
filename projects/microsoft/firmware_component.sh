#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if [ $# -lt 2 ]; then
	echo "Usage: $0 <image> <key> [path]"
	exit 1
fi

if [ ! -e "$2" ]; then
	echo "Unknown key for component signing: $2"
	exit 1
fi

if [ -z "$COMPONENT_MARKER" ]; then
	echo "No marker defined for the component."
	exit 1
fi

if [ -z "$HASH_TYPE" ]; then
	hdr_format="0"		# If using the default hash type, only format 0 is needed.
	hdr_length="14"
	HASH_TYPE="sha256"
elif [ "$HASH_TYPE" = "sha256" ] && [ -z "$LOAD_ADDRESS" ]; then
	hdr_format="0"		# Use format 0 since the default hash is used without a load address.
	hdr_length="14"
else
	hdr_format="1"		# Use format 1 to support load addresses and different hash algorithms.
	hdr_length="31"

	if [ -z "$LOAD_ADDRESS" ]; then
		LOAD_ADDRESS="0"
	fi

	if [ -z "$BUILD_VERSION_NUMBER" ]; then
		BUILD_VERSION_NUMBER="0x0000000000000000"
	fi

	case "$HASH_TYPE" in
		sha256)
			hash_id="1"
		;;

		sha384)
			hash_id="2"
		;;

		sha512)
			hash_id="3"
		;;

		*)
			echo "Unknown hash algorithm: $HASH_TYPE"
			exit 1
		;;
	esac
fi

if [ -z "$EXTRA_LENGTH" ]; then
	EXTRA_LENGTH=0
fi

if [ -z "$OUTPUT_DIR" ]; then
	OUTPUT_DIR=Keil/Objects
fi

if [ -z "$3" ]; then
	output=$OUTPUT_DIR/$1
	bin=$output/$1.bin
else
	output=$OUTPUT_DIR
	bin=$3/$1
fi

if [ ! -e "$bin" ]; then
	echo "Unknown binary file: $bin"
	exit 1
fi

if [ -n "$BUILD_SOURCESDIRECTORY" ]; then
	DIR=$BUILD_SOURCESDIRECTORY/build/cerberus/projects/microsoft
else
	DIR=`dirname ${0}`
fi
source $DIR/img_functions.sh
if [ $? -ne 0 ]; then
	exit 1
fi

app_no_hdr=$output/$1.tmp
hdr=$output/$1.hdr
app_out=$output/$1.app
signature=$output/$1.sig
image=$output/$1.img

if [ -n "$RECOVERY_REVISION" ]; then
	app=$app_no_hdr
else
	app=$app_out
fi

length=`stat -c %s $bin`
get_max_signature_length "$2"

# Construct the component first so the length can be known for the optional firmware header.
empty_file "$app"
output_binary_short "$hdr_length" "$app"
output_binary_short "$hdr_format" "$app"
output_binary_word "$COMPONENT_MARKER" "$app"
output_binary_word "$length" "$app"
output_binary_short "$sig_len" "$app"
if [ "$hdr_format" != "0" ]; then
	output_binary_byte "$hash_id" "$app"
	output_binary_qword "$LOAD_ADDRESS" "$app"
	output_binary_array "$BUILD_VERSION_NUMBER" "$app"
	add_padding "$app" "$hdr_length" "00"
fi
cat $bin >> $app

if [ -n "$RECOVERY_REVISION" ]; then
	length=`stat -c %s $app_no_hdr`

	# Add a firmware header to the image.  Determine what format to add.
	if [ $EXTRA_LENGTH -ne 0 ]; then
		fw_hdr_length=19
		fw_hdr_format=3

		# Set total length of the package that will be signed.  This is the total of firmware
		# header, length of this component, and length of any other components.  If there aren't any
		# other components, there is no need for an additional signature.
		let 'signed_length = fw_hdr_length + length + sig_len + EXTRA_LENGTH'
	else
		fw_hdr_length=13
		fw_hdr_format=2
		signed_length=0
	fi

	empty_file "$hdr"
	output_binary_short "$fw_hdr_length" "$hdr"
	output_binary_short "$fw_hdr_format" "$hdr"
	output_binary_word "0x43494d47" "$hdr"
	output_binary_short "$RECOVERY_REVISION" "$hdr"
	output_binary_byte "$EXTRA_IMAGES" "$hdr"
	output_binary_short "$MIN_ROLLBACK" "$hdr"
	if [ $fw_hdr_format -ge 3 ]; then
		output_binary_word "$signed_length" "$hdr"
		output_binary_short "$EXTRA_SIG_LENGTH" "$hdr"
	fi

	cat $hdr $app_no_hdr > $app_out
	app=$app_out
fi

generate_signature $app $signature $2 $HASH_TYPE
add_padding "$signature" "$sig_len" "00"

cat $app $signature > $image
echo $image
