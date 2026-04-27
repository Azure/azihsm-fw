#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if [ $# -lt 3 ]; then
	echo "Usage: $0 <file> <signature> <key>"
	exit 1
fi

if [ ! -e "$1" ]; then
	echo "Can't find signed file: $1"
	exit 1
fi

if [ ! -e "$2" ]; then
	echo "No signature file: $2"
	exit 1
fi

if [ ! -e "$3" ]; then
	echo "Unknown signing key: $3"
	exit 1
fi

if [ -n "$BUILD_SOURCESDIRECTORY" ]; then
	DIR=$BUILD_SOURCESDIRECTORY
else
	DIR=`dirname ${0}`
fi
source $DIR/img_functions.sh
if [ $? -ne 0 ]; then
	exit 1
fi

get_public_key $3
sig_size=`stat -c %s $2`
if [ $? -ne 0 ] || [ $key_len -gt $sig_size ]; then
	echo "Invalid signature file."
	exit 1
fi

tail -c $key_len $2 >> $1
