#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if [ $# -lt 2 ]; then
	echo "$0 <remove|add|check> <file>"
	exit 1
fi

action=$1
input=$2

# Remove the MIT license statement from the specified file, if it exists.
remove_license_header() {
	grep -q "Licensed under the MIT license." $input
	if [ $? -eq 0 ]; then
		line=`grep -n "Licensed under the MIT license." $input | cut -d: -f 1`
		if [ $? -ne 0 ]; then
			exit 1
		fi

		out=`mktemp`

		let 'top = line - 1'
		let 'bottom = line + 1'

		head -n $top $input > $out
		tail -n +$bottom $input >> $out

		chmod $mode $out
		mv $out $input
	fi
}

# Add the MIT license statement in the specified file, if it doesn't exist.
add_license_header() {
	grep -q "Copyright (c) Microsoft Corporation. All rights reserved."
	if [ $? -eq 0 ]; then
		msft=`grep -n "Copyright (c) Microsoft Corporation. All rights reserved." $input | \
			cut -d: -f 1`
		if [ $? -ne 0 ]; then
			exit 1
		fi

		out=`mktemp`

		# Detemine the type of prefix to add to the license header.
		line=`head -n $msft $input | tail -n 1`
		offset=`echo -n $line | \
			grep -o -b "Copyright (c) Microsoft Corporation. All rights reserved." | cut -d: -f 1`
		if [ $? -ne 0 ]; then
			exit 1
		fi

		let 'bottom = msft + 1'

		head -n $msft $input > $out
		echo -n "$line" | head -c $offset >> $out
		echo "Licensed under the MIT license." >> $out
		tail -n +$bottom $input >> $out

		chmod $mode $out
		mv $out $input
	fi
}


if [ -f "$input" ]; then
	mode=`stat -c %a $input`

	case $action in
		remove)
			remove_license_header
		;;

		add)
			add_license_header
		;;

		check)
			# Check if the file contains an MIT license header.
			grep -q "Licensed under the MIT license." $input
			if [ $? -ne 0 ]; then
				exit 1
			fi
		;;

		*)
			echo "Unknown option: $action"
			exit 1
		;;
	esac
else
	echo "Unknown file: $input"
	exit 1
fi
