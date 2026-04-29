#!/bin/bash

if [ $# -lt 1 ]; then
	echo "Usage $(basename $0) <file> [header]"
	exit 1
fi

if [ ! -e $1 ]; then
	echo "Unknown file: $1"
	exit 1
fi

if [ -n "$2" ]; then
	echo -e "$2"
fi

echo "SHA256: $(sha256sum $1 | awk '{print $1}')"
echo "SHA384: $(sha384sum $1 | awk '{print $1}')"
echo "SHA512: $(sha512sum $1 | awk '{print $1}')"
