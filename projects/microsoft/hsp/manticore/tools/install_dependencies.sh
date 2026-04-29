#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

set -e
top=`dirname "$(realpath "$BASH_SOURCE")"`

download_file() {
	url=$1
	filename=$(basename $url)

	if [ -f $filename ]; then
		echo "File $filename already exists."
	else
		# Download the file using wget
		curl -o $filename $url
		echo "File $filename downloaded."
	fi
}

if [ -z "$BUILD_TOOLS" ]; then
	BUILD_TOOLS=`realpath ~/build_tools`
	echo "\$BUILD_TOOLS not set. Using $BUILD_TOOLS"
	mkdir -p $BUILD_TOOLS
else
	echo "Using existing \$BUILD_TOOLS=$BUILD_TOOLS"
fi

unpack_archive() {
	tar_cmd=$1
	file=$2
	expected_out_folder=$3

	if [ -d $BUILD_TOOLS/$expected_out_folder ]; then
		echo "Folder $BUILD_TOOLS/$expected_out_folder already exists."
	else
		echo "Folder $BUILD_TOOLS/$expected_out_folder does not exist."
		tar $tar_cmd $file
	fi
}

pushd $BUILD_TOOLS

# Required for SP
download_file https://static.dev.sifive.com/dev-tools/freedom-tools/v2020.12/riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-ubuntu14.tar.gz

# Required for CP / C
download_file https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2

# Required for FP
download_file https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/7-2017q4/gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2

unpack_archive xvjf gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 gcc-arm-none-eabi-9-2019-q4-major
unpack_archive xvzf riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-ubuntu14.tar.gz riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-ubuntu14
unpack_archive xvjf gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2 gcc-arm-none-eabi-7-2017-q4-major

popd

if command -v msrustup &> /dev/null
then
	echo "msrustup command exists."
else
	echo "ERROR: CP Build : msrustup must be installed. See https://eng.ms/docs/more/rust/services/msrustup"
	exit 2
fi

if command -v cargo &> /dev/null
then
	echo "cargo command exists."
else
	echo "ERROR: CP Build : Rust must be installed. See https://www.rust-lang.org/tools/install"
	exit 2
fi

if command -v mono &> /dev/null
then
	echo "mono command exists."
else
	echo "ERROR: FP Build : Mono must be installed. See https://www.mono-project.com/docs/getting-started/install/"
	exit 3
fi

pushd .
# Required for SP
UNC_DIR=$BUILD_TOOLS/uncrustify
UNC_VERSION=uncrustify-0.78.1
UNC_BUILD_DIR=$UNC_DIR/build
UNC_EXECUTABLE=$UNC_BUILD_DIR/uncrustify

# Check for uncrustify tool install
if ! $UNC_EXECUTABLE --version &> /dev/null; then
	echo "Cloning Uncrustify in $UNC_DIR"
	cd "$BUILD_TOOLS"
	git clone https://github.com/uncrustify/uncrustify.git &> /dev/null 2>&1
	cd "$UNC_DIR"
	git checkout "$UNC_VERSION" &> /dev/null 2>&1
	mkdir -p "$UNC_BUILD_DIR"
	cd "$UNC_BUILD_DIR"
	cmake -DCMAKE_BUILD_TYPE=Release ..
	cmake --build . --config Release &> /dev/null 2>&1
	make
else
	# get version if Uncrustify present
	echo `$UNC_EXECUTABLE --version`
fi

if command -v $UNC_EXECUTABLE &> /dev/null
then
	echo "Uncrustify exists"
else
	echo "ERROR: Manticore Build: uncrstify must be installed. See https://github.com/uncrustify/uncrustify.git"
	exit 1
fi

popd

# Require for SP
python -m pip install -r $top/requirements.txt

# TF_BUILD will be set to true in the env, if the script is being run by a build task (e.g. pipeline).
# This skips the msrustup commands (which require interactive input for auth) in that case, but
# They will have already been installed by the RustInstaller@1 task in the pipeline.
#
# see: https://learn.microsoft.com/en-us/azure/devops/pipelines/build/variables?view=azure-devops&tabs=yaml
if [ -z "$TF_BUILD" ] ; then
	pushd ./cp/hsm
	msrustup update
	msrustup toolchain install ms-prod-1.92
	msrustup target add thumbv7em-none-eabihf
	cargo xtask setup
	popd
fi

echo "All dependencies detected."
echo -e "Before building, run:\n"
echo -e "\t. ./tools/menv.sh"
