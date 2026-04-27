#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

set -e

if [ $# -lt 1 ]; then
	echo "$0 <path>"
	exit 1
fi

script_dir=`dirname "$(realpath "$BASH_SOURCE")"`
if ! source "$script_dir/../../../img_functions.sh"; then
	exit 1
fi

path=$1
list=`find $path -maxdepth 1 -iname '*.h'`

for header in $list; do
	start=`grep -n "unsigned" $header | cut -d: -f 1`
	let 'start = start + 1'

	bin=$path/$(basename -s .h $header).bin

	dos2unix $header
	arr=`tail -n +$start $header | head -n -1`
	echo $arr > $bin.tmp

	empty_file $bin
	for line in $arr; do
		echo $line | tr -d ',\n' | tail -c 8 | sed -E 's/(..)(..)(..)(..)/\4\3\2\1/' | xxd -r -p >> $bin
	done
done
