#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.
set -e

if [ -z "$MARTICHORAS_ROOT" ]; then
	MARTICHORAS_ROOT=`realpath ~/Martichoras/`
	echo "\$MARTICHORAS_ROOT not set. Using $MARTICHORAS_ROOT"
else
	echo "Using existing \$MARTICHORAS_ROOT=$MARTICHORAS_ROOT"
fi

current_dir=$(pwd)

echo -e "\e[31mBUILD: FW release\e[0m (`pwd`)"
. ./tools/menv.sh
./make_manticore.sh --evb --i2c_slv
#./make_manticore.sh --evb --test_features --i2c_slv --no_crashdump --no_watchdog --cp_features=mcr_test_hooks,fips_validation_hooks
cd $current_dir

cd $MARTICHORAS_ROOT/drivers/linux/drvsrc
echo -e "\e[31mBUILD: Martichoras Driver\e[0m (`pwd`)"
make -C /lib/modules/6.5.0-41-generic/build M=$PWD clean
make -C /lib/modules/6.5.0-41-generic/build M=$PWD

cd $MARTICHORAS_ROOT/api
echo -e "\e[31mBUILD: Martichoras API and DDI(test_hooks) Tests\e[0m (`pwd`)"
cargo clean && cargo build --tests --release --package azihsm_ddi_test_hooks && cargo build --tests --release --package mcr_api
echo -e "\e[31mBUILD: Martichoras PERF Tests\e[0m (`pwd`)"
cargo build --all-targets --release

cd $MARTICHORAS_ROOT/sdk
echo -e "\e[31mBUILD: Martichoras DDI Tests\e[0m (`pwd`)"
cargo clean && cargo build --tests --release --package azihsm_ddi

cd $current_dir
echo -e "\e[31mPACK\e[0m (`pwd`)"
./tools/fp_packager/create_prebuild_package.sh

today=$(date +%y_%m_%d)
PREBUILD_PKG_NAME="manticore_build_$today"
mkdir -p $PREBUILD_PKG_NAME/driver
mkdir -p $PREBUILD_PKG_NAME/driver/tests

find $MARTICHORAS_ROOT/drivers/linux/drvsrc/ -name azihsm.ko -exec cp {} $PREBUILD_PKG_NAME/driver \;
find $MARTICHORAS_ROOT/api/target/release/ -name mcr_perf -exec cp {} $PREBUILD_PKG_NAME/driver \;
find $MARTICHORAS_ROOT/api/target/release/deps/ -executable -type f ! -name "*.so" -exec cp {} $PREBUILD_PKG_NAME/driver/tests \;
find $MARTICHORAS_ROOT/sdk/target/release/deps/ -executable -type f ! -name "*.so" -exec cp {} $PREBUILD_PKG_NAME/driver/tests \;

zip -r $PREBUILD_PKG_NAME.zip $PREBUILD_PKG_NAME
echo -e "\e[1;36mCreated pre-build images archive $PREBUILD_PKG_NAME.zip..\e[0m"
echo "$(realpath $(pwd))/$PREBUILD_PKG_NAME.zip"
