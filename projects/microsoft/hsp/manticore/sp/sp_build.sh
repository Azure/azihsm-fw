#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

##
# This script is intended for two purposes:
#	1. Build all the SP targets for Manticore
#	2. Package 1SP files as bootable images.  All 1SP images will have the same SVN and build
#		version applied and will be signed with the same keys.
##

build_base=`pwd`/build
build_prod=$build_base-prod
build_fpga=$build_base-fpga
build_haps=$build_base-haps
build_evb=$build_base-evb
build=$build_prod
hsp_dir=`pwd`/../../platform
util_dir=$hsp_dir/openhsp/sources/utils
keys_dir=`pwd`/../../../../../core/testing/keys

xfer_type=0
target="all"
root_key=$keys_dir/ecc384priv.pem
sp1_key=$keys_dir/ecc384priv2.pem
sp1_secondary_key=
transfer_key=
grant_token=
encryption_key=
SVN=0
VERSION=0000000000000000
REC_VERSION=$VERSION
extra_defs=
img_out=
hw_target=BUILD_FOR_PRODUCTION
make_img=1
do_cmake=1
rebuild=0
refresh=0


########################
# Parse input arguments
########################

ARGS=`getopt --unquoted -o "otD:" -l "rebuild,refresh,no_cmake,build_only,prod,fpga,haps,evb,\
	target:,svn:,version:,addr:,key_root:,key_1sp:,key_1sp_sec:,key_xfer:,grant:,encrypt:,output:" \
	-- "$@"`
if [ $? -ne 0 ]; then
	exit 1
fi

set -- $ARGS
while [ $# -gt 0 ]; do
	case "$1" in
		-o)
			# Create an ownership transfer manifest.
			xfer_type=1
			shift
		;;

		-t)
			# Create a tenant manifest.
			xfer_type=2
			shift
		;;

		-D)
			extra_defs="$extra_defs -D$2"
			do_cmake=1
			shift 2
		;;

		--rebuild)
			# Completely wipe out all the build directories and start fresh.
			rebuild=1
			do_cmake=1
			shift
		;;

		--refresh)
			# Rebuild the cmake cache for the SP code.  This may be necessary when adding or, more
			# likely, removing extra command line compile definitions.
			#
			# This does not affect the Pluton SDK or utility builds.
			refresh=1
			do_cmake=1
			shift
		;;

		--no_cmake)
			# Do not make a call to cmake
			do_cmake=0
			shift
		;;

		--build_only)
			# Only build the binary, do not make a boot image
			make_img=0
			shift
		;;

		--prod)
			# Target the build to run on the production device.  This is the default build option.
			hw_target=BUILD_FOR_PRODUCTION
			build=$build_prod
			shift
		;;

		--fpga)
			# Target the build to run on the MPS3 FPGA instead of the production device.
			hw_target=BUILD_FOR_FPGA
			build=$build_fpga
			shift
		;;

		--haps)
			# Target the build to run on the HAPS FPGA instead of the production device.
			hw_target=BUILD_FOR_HAPS
			build=$build_haps
			shift
		;;

		--evb)
			# Target the Marvell evaluation board instead of the production configuration.
			hw_target=BUILD_FOR_EVB
			build=$build_evb
			shift
		;;

		--target)
			if [ "$2" = "all" ]; then
				target="all"
			else
				target=$2.elf
			fi
			shift 2
		;;

		--svn)
			SVN=$2
			shift 2
		;;

		--version)
			VERSION=$2
			shift 2
		;;

		--addr)
			# Specify the desired load address for the test_1sp target.  Changing this parameter
			# will change the build/linker properties for this target.
			shift 2
		;;

		--key_root)
			if [ -e "$2" ]; then
				root_key=$2
			else
				root_key=$keys_dir/$2
			fi
			shift 2
		;;

		--key_1sp)
			if [ -e "$2" ]; then
				sp1_key=$2
			else
				sp1_key=$keys_dir/$2
			fi
			shift 2
		;;

		--key_1sp_sec)
			if [ -e "$2" ]; then
				sp1_secondary_key="-k $2"
			else
				sp1_secondary_key="-k $keys_dir/$2"
			fi
			shift 2
		;;

		--key_xfer)
			if [ -e "$2" ]; then
				transfer_key=$2
			else
				transfer_key=$keys_dir/$2
			fi
			shift 2
		;;

		--grant)
			grant_token=$2
			shift 2
		;;

		--encrypt)
			encryption_key="-e $2"
			shift 2
		;;

		--output)
			img_out=$2
			shift 2
		;;

		--)
			shift
		;;
	esac
done

# Get the version ext type from the version parsed
ext_type=`echo "$VERSION" | tr -d  '\r\n'| head -c -6 | tail -c +9`

# mask ext type bits from version, o/p value will be  from 0 to 4
ext_type=$((0x$ext_type&0xe0))

# update ext type value for recovery image build (BUILD_VERSION_EXTENSION_REC = 8)
ext_type=$(($ext_type+8))

# Convert value to hex
ext_type=$(printf '%02x' "$ext_type")

# create recover version
REC_VERSION=`echo "$VERSION" | head -c -9`$ext_type`echo "$VERSION" | tail -c +11`

if [ $xfer_type -ne 0 ] && [ -z "$transfer_key" ]; then
	echo "Ownership and Tenancy transfers need a transfer key (--key_xfer)"
	exit 1
fi

if [ $xfer_type -eq 2 ] && [ -z "$grant_token" ]; then
	echo "Tenancy transfer requires a grant token (--grant)"
	exit 1
fi

# Set the HSP SDK utility build path.
util_build=$build/build-util


#######################
# Build the SP targets
#######################

if [ $rebuild -ne 0 ]; then
	rm -rf $build
elif [ $refresh -ne 0 ]; then
	rm $build/CMakeCache.txt
fi

if [ $do_cmake -ne 0 ]; then
	if [ ! -e $util_build ]; then
		mkdir -p $util_build
	fi

	cmake -G Ninja -S $util_dir -B $util_build -DCMAKE_BUILD_TYPE=release
	if [ $? -ne 0 ]; then
		exit 1;
	fi

	cmake -G Ninja -S `pwd` -B $build -DHSP_SDK_UTIL_BIN=$util_build -DHW_BUILD_TARGET=$hw_target \
		$extra_defs
	if [ $? -ne 0 ]; then
		exit 1;
	fi
fi

ninja -C $util_build
if [ $? -ne 0 ]; then
	exit 1;
fi

ninja -C $build $target
if [ $? -ne 0 ]; then
	exit 1;
fi


#####################
# Package 1SP Images
#####################

##
# Read the ELF file to determine the load address of the target.
# 	- Takes a build target name as input.
#	- Populates $load_addr on output.
##
get_load_address() {
	img_dir=$build/cmake/$1
	img_elf=$img_dir/$1.elf

	load_addr=`readelf -e $img_elf | grep 'Entry point' | awk '{print $4}'`
}

##
# Create a wrapped 1SP image.
#	- Takes a build target name as input.
#	- Creates a file <target>.img in the same directory as <target>.elf.
##
create_bootable_1sp() {
	tools_dir=`pwd`/../../tools
	sp1=$1
	ver=$2
	sp1_dir=$build/cmake/$sp1
	sp1_bin=$sp1_dir/$sp1.bin
	sp1_img=$sp1_dir/$sp1.img

	get_load_address $sp1

	img=`OUTPUT_DIR=$sp1_dir $tools_dir/boot_image.sh $encryption_key $sp1_secondary_key $SVN \
		$xfer_type $ver $load_addr $sp1_bin $root_key $sp1_key $transfer_key $grant_token`
	if [ $? -ne 0 ]; then
		echo "$img"
		exit 1
	fi

	if [ -n "$img_out" ]; then
		img_dir=`dirname $img_out`
		if [ ! -e "$img_dir" ]; then
			mkdir -p $img_dir
		fi

		cp $img $img_out
		sp1_img=$img_out
	else
		mv $img $sp1_img
	fi

	echo $sp1_img
}

if [ $make_img -ne 0 ]; then
	case "$target" in
		all)
			create_bootable_1sp "dc_scm_1sp" $VERSION
			create_bootable_1sp "dc_scm_recovery" $REC_VERSION
														;;

		dc_scm_1sp.elf)
			create_bootable_1sp "dc_scm_1sp" $VERSION
		;;

		dc_scm_recovery.elf)
			create_bootable_1sp "dc_scm_recovery" $REC_VERSION
		;;

									esac
fi
