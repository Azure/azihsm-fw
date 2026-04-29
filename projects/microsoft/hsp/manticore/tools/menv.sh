#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if [ -z "$BUILD_TOOLS" ]; then
	BUILD_TOOLS=`realpath ~/build_tools`
	echo "\$BUILD_TOOLS not set. Using $BUILD_TOOLS"
	mkdir -p $BUILD_TOOLS
else
	echo "Using existing \$BUILD_TOOLS=$BUILD_TOOLS"
fi

if command -v riscv64-unknown-elf-gcc-10.2.0 &> /dev/null
then
	echo "Riscv64 compiler v10 already in PATH"
	riscv64-unknown-elf-gcc-10.2.0 --version
else
	export PATH=$PATH:`realpath $BUILD_TOOLS`/riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-ubuntu14/bin/
	riscv64-unknown-elf-gcc-10.2.0 --version
fi

if command -v arm-none-eabi-gcc-9.2.1 &> /dev/null
then
	echo "Arm compiler v9 already in PATH"
	arm-none-eabi-gcc-9.2.1 --version
else
	export PATH=$PATH:`realpath $BUILD_TOOLS`/gcc-arm-none-eabi-9-2019-q4-major/bin
	arm-none-eabi-gcc-9.2.1 --version
fi

if command -v arm-none-eabi-gcc-7.2.1 &> /dev/null
then
	echo "Arm compiler v7 already in PATH"
	arm-none-eabi-gcc-7.2.1 --version
else
	export PATH=$PATH:`realpath $BUILD_TOOLS`/gcc-arm-none-eabi-7-2017-q4-major/bin
	arm-none-eabi-gcc-7.2.1 --version
fi

if command -v uncrustify &> /dev/null
then
	echo "Uncrustify already in PATH"
	uncrustify --version
else
	export PATH=$PATH:`realpath $BUILD_TOOLS`/uncrustify/build
	uncrustify --version
fi
