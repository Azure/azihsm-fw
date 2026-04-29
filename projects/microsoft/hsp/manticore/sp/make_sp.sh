#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

set -e
top=`dirname "$(realpath "$BASH_SOURCE")"`
build_base=$top/build


##
# Version of the current 1SP release that should be used for firmware builds.
##
sp1_version="3.5.0.1"


##
# Build variables
##

# Released 1SP image that should be used with firmware builds.  --new_1sp will override this setting
# and use the freshly built 1SP image instead.
sp1_img="$top/dc_scm_1sp/dc_scm_1sp_v$sp1_version"

# The type of released 1SP image that should be used with firmware builds.  --dev_1sp will override
# this setting to use a dev-signed 1SP.
sp1_signing="prod"

# SVN to apply to the 1SP key manifest and firmware image.  This only applies to newly built 1SP
# images.
sp1_svn=

# Build version number for the 1SP firmware image.  This only applies to newly built 1SP images.
sp1_version=

# Target hardware platform for the image.
hw_target="--prod"

# Transfer manifest type.  No argument uses a null manifest.
xfer_manifest=

# Signing keys for the image.
root_key=
sp1_key=
sp1_secondary_key=
transfer_key=
grant_token=

# Build option for the SP build script.
build_opt="--refresh"

# Test features to enable in the build.  By default, nothing is enabled.
test_features="-DMANTICORE_TEST_FEATURES="

# I2C slave mode
i2c_slv_option=

##
# Parse input arguments to adjust build properties.
##
ARGS=`getopt --unquoted -o "" -l "rebuild,owner,tenant,prod,fpga,haps,evb,A0,new_1sp,dev_1sp,fips,\
	test_features,i2c_slv,no_watchdog,no_crashdump,crash_reset,enable_acvp_testing,\
	acvp_ecdsa_rom,enable_cmvp_testing,memory_fencing_sprt_config_aebs,svn:,version:,key_root:,\
	key_1sp:,key_1sp_sec:,key_xfer:,grant:,enable_cp_cdma_access,no_graceful_shutdown," -- "$@"`
if [ $? -ne 0 ]; then
	exit 1
fi

set -- $ARGS
while [ $# -gt 0 ]; do
	case "$1" in
		--rebuild)
			# Completely wipe out all the build directories and start fresh.
			build_opt=$1
			shift
		;;

		--owner)
			# Create an ownership transfer manifest.
			xfer_manifest="-o"
			shift
		;;

		--tenant)
			# Create a tenant manifest.
			xfer_manifest="-t"
			shift
		;;

		--prod|--evb|--fpga|--haps)
			# Select the HW target.  Default is for production.
			hw_target=$1
			shift
		;;

		--A0)
			# Enable A0 support in SPRT.  1SP always supports A0 to ensure proper EMC handling.
			test_features="${test_features}MANTICORE_ENABLE_A0_SUPPORT;"
			shift
		;;

		--new_1sp)
			# Use the 1SP image that was just built in the final package instead of the released
			# version.  This takes precedence if both --new_1sp and --dev_1sp are specified.
			sp1_img=
			shift
		;;

		--dev_1sp)
			# Use the dev-signed version of the released 1SP image instead of the prod-signed
			# version.  This will have no effect if both --dev_1sp and --new_1sp are specified.
			sp1_signing="dev"
			shift
		;;

		--fips)
			# Use the dual-signed FIPS 1SP version instead of the single-signed version.  This will
			# have no effect if --new_1sp is specified.
			sp1_signing="fips"
			shift
		;;

		--test_features)
			# Enable test features (use format -DMANTICORE_TEST_FEATURES=FEATURE1;FEATURE2;...)
			test_features="${test_features}CMD_SUPPORT_PLATFORM_RESET;MANTICORE_SPRT_BOOT_TIME_REPORT;"
			shift
		;;

		--no_watchdog)
			# Disable the hardware watchdog during SPRT execution.
			test_features="${test_features}MANTICORE_DISABLE_WATCHDOG;"
			shift
		;;

		--no_crashdump)
			# Disable the reset and crash dump caused by SP watchdog monitoring CP, FP.
			test_features="${test_features}MANTICORE_DISABLE_CRASHDUMP;"
			shift
		;;

		--crash_reset)
			# Changes rot reset behavior from impactless reset to crash reset.
			test_features="${test_features}MANTICORE_ROT_RESET_CRASH;"
			shift
		;;

		--memory_fencing_sprt_config_aebs)
			# Disabling aeb bits to test memory feincing in EVB.
			test_features="${test_features}MANTICORE_MEMORY_FENCING_SPRT_CONFIG_AEBS;"
			shift
		;;

		--enable_acvp_testing)
			# Enables features for FIPS ACVP testing.
			test_features="${test_features}MANTICORE_ENABLE_FIPS_ACVP_TESTING;"
			shift
		;;

		--acvp_ecdsa_rom)
			# Enables ECDSA ROM for FIPS ACVP testing.  In doing so, testing of other ECDSA
			# implementations is disabled.
			test_features="${test_features}ECDSA_ROM_ENABLE_FIPS_ACVP_TESTING;"
			shift
		;;

		--enable_cmvp_testing)
			# Enable CMVP commands for FIPS testing.
			test_features="${test_features}MANTICORE_ENABLE_FIPS_CMVP_TESTING;"
			test_features="${test_features}ECDSA_ENABLE_FIPS_CMVP_TESTING;"
			test_features="${test_features}ECDH_ENABLE_FIPS_CMVP_TESTING;"
			test_features="${test_features}CCS_KSU_ENABLE_FIPS_CMVP_TESTING;"

			# Use the CMVP-enabled 1SP image.
			sp1_signing="test"
			shift
		;;

		--no_graceful_shutdown)
			# Add the support for disabling the graceful shutdown
			test_features="${test_features}MANTICORE_NO_GRACEFUL_SHUTDOWN;"
			shift
		;;
		
		--enable_cp_cdma_access)
			# Enable CDMA memory region access for cp cdma ecc error testing.
			test_features="${test_features}MANTICORE_ENABLE_CP_CDMA_ACCESS;"
			shift
		;;

		--i2c_slv)
			# Flag to enable I2C in slave mode instead of multi-master.
			i2c_slv_option="-DI2C_SLAVE_MODE=1"
			shift
		;;

		--svn)
			sp1_svn="$1 $2"
			shift 2
		;;

		--version)
			sp1_version="$1 $2"
			shift 2
		;;

		--key_root)
			root_key="$1 $2"
			shift 2
		;;

		--key_1sp)
			sp1_key="$1 $2"
			shift 2
		;;

		--key_1sp_sec)
			sp1_secondary_key="$1 $2"
			shift 2
		;;

		--key_xfer)
			transfer_key="$1 $2"
			shift 2
		;;

		--grant)
			grant_token="$1 $2"
			shift 2
		;;

		--)
			shift
		;;
	esac
done

# Set the build directory based on the configured HW target.
build=$build_base-$(echo -n $hw_target | tail -c +3)
cmake_out=$build/cmake


##
# Build the SP images and package 1SP.
##
./sp_build.sh $hw_target $build_opt $sp1_svn $sp1_version $xfer_manifest $root_key $sp1_key \
	$sp1_secondary_key $transfer_key $grant_token $test_features $i2c_slv_option
if [ $? -ne 0 ]; then
	exit 1
fi


##
# This script generates two files for consumption by the top-level image generator.
#	1. A file containing the bootable 1SP binary and SPRT reset vector.
#	2. A list of SPRT images that should be part of the Manticore firmware package.  Each image is
#		reported as a <bin,load addr> tuple, with each entry separated by whitespace.
##
sp1_list=$build/sp1.list
sprt_list=$build/sprt.list

##
# Determine the 1SP image to use and update the SP image listing.
##
if [ -z "$sp1_img" ]; then
	sp1_img=$cmake_out/dc_scm_1sp/dc_scm_1sp.img
else
	sp1_img=${sp1_img}_${sp1_signing}.img
fi

if [ -e "$sp1_img" ]; then
	echo "1SP: $sp1_img" > $sp1_list
else
	echo "1SP image not found: $sp1_img"
	exit 1
fi

##
# Parse the SPRT image and update the SP image listing.
##
sprt_bin=$cmake_out/dc_scm_sprt/dc_scm_sprt.bin
sprt_elf=$cmake_out/dc_scm_sprt/dc_scm_sprt.elf

sprt_load_addr=`readelf -e $sprt_elf | grep 'Entry point' | awk '{print $4}'`
sprt_reset_vertor=$sprt_load_addr

echo "$sprt_bin,$sprt_load_addr" > $sprt_list
echo "Reset: $sprt_reset_vertor" >> $sp1_list
