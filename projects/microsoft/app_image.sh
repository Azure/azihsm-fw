#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if [ $# -lt 2 ]; then
	echo "Usage: $0 <image> <key> [path]"
	exit 1
fi

if [ ! -e "$2" ]; then
	echo "Unknown key for app image signing: $2"
	exit 1
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

if [ -n "$BUILD_SOURCESDIRECTORY" ]; then
	DIR=$BUILD_SOURCESDIRECTORY/build/cerberus/projects/microsoft
else
	DIR=`dirname ${0}`
fi
source $DIR/img_functions.sh
if [ $? -ne 0 ]; then
	exit 1
fi

app=$output/$1.app
signature=$output/$1.sig
image=$output/$1.img

rm -f $app

length=`stat -c %s $bin`

if [ -n "$RECOVERY_REVISION" ]; then
	header_length=19
	if [ $SIGNED_LENGTH -ne 0 ]; then
		let 'SIGNED_LENGTH = SIGNED_LENGTH + header_length + 4 + length + SIG_LENGTH'
	fi

	# Add a firmware header to the image.
	output_binary_short "$header_length" "$app"
	output_binary_short "3" "$app"		# Using format 3
	output_binary_word "0x43494d47" "$app"
	output_binary_short "$RECOVERY_REVISION" "$app"
	output_binary_byte "$EXTRA_IMAGES" "$app"
	output_binary_short "$MIN_ROLLBACK" "$app"
	output_binary_word "$SIGNED_LENGTH" "$app"
	output_binary_short "$SIG_LENGTH" "$app"
fi

output_binary_word "$length" "$app"
cat $bin >> $app
generate_signature $app $signature $2

cat $app $signature > $image
echo $image
