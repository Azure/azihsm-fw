#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

set -e
pwd=`pwd`
top=`dirname "$(realpath "$BASH_SOURCE")"`
build_base=$top/build


##
# Firmware image properties.
##
MANTICORE_1SP_SVN=1
MANTICORE_FW_SVN=1
MANTICORE_RECOVERY_VERSION=0
MANTICORE_MIN_RECOVERY=0

prod_keys=$top/sp/keys/dc_scm
dev_keys=$top/../../../../core/testing/keys
root_key=$dev_keys/ecc384priv.pem
sp1_key=$dev_keys/ecc384priv2.pem
fw_key=$dev_keys/ecc384priv3.pem
fips_key=$dev_keys/ecc384priv5.pem
manifest_root_key=$prod_keys/Manticore_ManifestRoot_P384.pem
unlock_key=$prod_keys/HKMS_DebugUnlock_469407_469419_AOC_20220215.pem
manifest_clear_key=$prod_keys/HKMS_ClearManifest_469408_469420_AOC_20220215.pem
intrusion_reset_key=$prod_keys/HKMS_IntrusionReset_469409_469421_AOC_20220215.pem
identity_renew_key=$prod_keys/HKMS_IdentityRenewal_500080_500850_AOC_20230404.pem
rma_key=$prod_keys/HKMS_RMA_500081_500851_AOC_20230404.pem
fw_update_key=$dev_keys/ecc384priv4.pem
# [OSS] CFM removed

##
# Build options.
##
# Target hardware platform for the image.
hw_target="--prod"

# Flag to clean everything and rebuild the image
rebuild=

# Flag to include a fresh 1SP build as part of the image.
new_1sp=

# Flag to include the dev-signed version of the released 1SP image.
dev_1sp=

# Flag to include the FIPS-signed version of the released 1SP image.
fips=

# Flag to only include SP firmware images.
sp_only=0

# Flag to create package using pre-build images.
prebuild_only=0

# Flag to enable test features.
test_features=

# Flag to enable I2C in slave mode instead of multi-master.
i2c_slv=

# Additional features to provide for CP build.  See cp/hsm/app/Cargo.toml for available features.
cp_features=""

# Additional features to provide for FP build.  See fp/qmgr/Makefile for available features.
fp_features=""

# Build the image as if it was FIPS certified.  This is just for test purposes.
secondary_key=""
fips_certified=""


##
# Parse input arguments to adjust build properties.
##
ARGS=`getopt --unquoted -o "" -l "rebuild,prod,evb,A0,new_1sp,dev_1sp,fips,sp_only, \
	prebuild_only,test_features,no_watchdog,no_crashdump,crash_reset,i2c_slv,fp_features:, \
	cp_features:,enable_acvp_testing,acvp_ecdsa_rom,enable_cmvp_testing, \
	memory_fencing_sprt_config_aebs,enable_cp_cdma_access,no_graceful_shutdown," -- "$@"`
if [ $? -ne 0 ]; then
	exit 1
fi

set -- $ARGS
while [ $# -gt 0 ]; do
	case "$1" in
		--rebuild)
			# Completely wipe out all the build directories and start fresh.
			rebuild=$1
			shift
		;;

		--prod|--evb)
			# Select the HW target.  Default is for production.
			hw_target=$1
			shift
		;;

		--A0)
			# Add support for A0 chips to SPRT firmware update flows.
			test_features="$test_features $1"
			shift
		;;

		--new_1sp)
			# Use the 1SP image that was just built in the final package instead of a pre-built
			# version.
			new_1sp=$1
			shift
		;;

		--dev_1sp)
			# Use the dev-signed version of the pre-built 1SP image.
			dev_1sp=$1
			shift
		;;

		--fips)
			# Mark the build as FIPS certified.  This will be used for images that are being
			# submitted for certification.
			secondary_key="--key_1sp_sec $fips_key"
			fips_certified="-C"
			fips=$1
			shift
		;;

		--sp_only)
			# Build a Manticore image that only includes the SP firmware images.  CP and FP images
			# won't be built nor will they be included in the package.
			sp_only=1
			shift
		;;

		--cp_features)
			# Build the CP image with additional features.
			if [[ "$2" =~ ^-- ]] || [ -z "$2" ]; then
				echo "Error: --cp_features requires a comma-delimited list of features"
				exit 1
			fi

			if [[ -z "$cp_features" ]]; then
				cp_features="$1=$2"
			else
				cp_features="$cp_features,$2"
			fi

			shift 2
		;;

		--fp_features)
			# Build the FP image with additional features.
			if [[ "$2" =~ ^-- ]] || [ -z "$2" ]; then
				echo "Error: --fp_features requires a separated list of features"
				exit 1
			fi
			fp_features="$1=$2"
			shift 2
		;;

		--test_features)
			# Add test features:
			#	- Support for `cerberus_utility msft rotreset`
			test_features="$test_features $1"
			shift
		;;

		--no_watchdog)
			# Disable SPRT hardware watchdog
			test_features="$test_features $1"
			shift
		;;

		--no_crashdump)
			# Disable the reset and crash dump caused by SP watchdog monitoring CP, FP.
			test_features="$test_features $1"
			shift
		;;

		--crash_reset)
			# Changes rot reset behavior from impactless reset to crash reset.
			test_features="$test_features $1"
			shift
		;;

		--memory_fencing_sprt_config_aebs)
			# Disabling aeb bits to test memory feincing in EVB.
			test_features="$test_features $1"
			shift
		;;

		--prebuild_only)
			# Flag to create package using pre-build images.
			prebuild_only=1
			shift
		;;

		--i2c_slv)
			# Flag to create
			i2c_slv=$1
			shift
		;;

		--enable_acvp_testing)
			# Add support for FIPS ACVP testing features in SP & CP.
			test_features="$test_features $1"

			if [[ -z "$cp_features" ]]; then
				cp_features="--cp_features=fips_validation_hooks"
			else
				cp_features="$cp_features,fips_validation_hooks"
			fi

			shift
		;;

		--acvp_ecdsa_rom)
			# Replace support for core ECDSA ACVP testing with support for ROM ECDSA ACVP testing.
			test_features="$test_features $1"
			shift
		;;

		--enable_cmvp_testing)
			# Add support for CMVP verification testing features.
			test_features="$test_features $1"
			shift
		;;

		--no_graceful_shutdown)
			# Add the support for disabling the graceful shutdown
			test_features="$test_features $1"
			shift
		;;

		--enable_cp_cdma_access)
			# Add support for cp cdma ecc error testing.
			test_features="$test_features $1"

			if [[ -z "$cp_features" ]]; then
				cp_features="--cp_features=mcr_test_hooks,mcr_test_hooks_cdma_ecc_err"
			else
				cp_features="$cp_features,mcr_test_hooks_cdma_ecc_err"
			fi

			shift
		;;

		--)
			shift
		;;
	esac
done

##
# Create the top-level build directory.
##
build_type=`echo -n $hw_target | tail -c +3`
build=$build_base-$build_type

if [ -n "$rebuild" ]; then
	rm -rf $build
fi

if [ ! -e "$build" ]; then
	mkdir -p $build
	if [ $? -ne 0 ]; then
		exit 1;
	fi
fi

##
# Determine the build version number.
##
version_file=$top/version.h

if [ -n "$test_features" ] || [ -n "$i2c_slv" ] || [ -n "$cp_features" ] || \
[ -n "$fp_features" ]; then
	test_build=1
else
	test_build=0
fi

major=`grep -m 1 FW_VERSION_MAJOR $version_file | awk '{print $3}' | tr -d '\r\n'`
minor=`grep -m 1 FW_VERSION_MINOR $version_file | awk '{print $3}' | tr -d '\r\n'`
release=`grep -m 1 FW_VERSION_RELEASE $version_file | awk '{print $3}' | tr -d '\r\n'`
build_num=`grep -m 1 FW_VERSION_BUILD $version_file | awk '{print $3}' | tr -d '\r\n'`
compat_version=`grep -m 1 FW_VERSION_IDFU $version_file | awk '{print $3}' | tr -d '\r\n'`

declare -i build_time
declare -i ext_base
declare -i ext_num
build_time=`date -u +%y%m%d%H%M | tr -d '\r\n' | tail -c +2 | head -c -1`
let 'ext_base = build_time * 32'

if [ "$build_type" = "evb" ]; then
	let 'ext_num = ext_base + 4'		# EVB build
	ext_str="evb"
elif [ $test_build = 1 ]; then
	let 'ext_num = ext_base + 12'		# Test build
	ext_str="test"
else
	is_release=`grep -m 1 FW_VERSION_IS_RELEASE $version_file | awk '{print $3}' | tr -d '\r\n'`
	if [ "$is_release" = "1" ]; then
		let 'ext_num = ext_base + 1'	# RC build
		ext_str="rel"
	else
		let 'ext_num = ext_base + 3'	# beta build
		ext_str="beta"
	fi
fi

build_ver=`printf "%02x%02x%02x%02x" $build_num $release $minor $major`
build_ext=`printf "%08x" $ext_num | tail -c 8 | sed -E 's/(..)(..)(..)(..)/\4\3\2\1/'`

MANTICORE_1SP_BUILD_VERSION=$build_ver$build_ext
MANTICORE_FW_BUILD_VERSION=$build_ver$build_ext

##
# Build is Impactless or Impactful.
##
if [ $prebuild_only -eq 0 ]; then
	# ./idfu_compatible_build.sh  # Skipped for OSS
	if [ $? -ne 0 ]; then
		exit 1
	fi
fi

##
# Build the SP firmware.
##
echo -e "\e[1;36mBuilding SP firmware...\e[0m"
sp_dir=$top/sp
sp_build=$sp_dir/build-$build_type
sp1_list=$sp_build/sp1.list
sprt_list=$sp_build/sprt.list


if [ $prebuild_only -eq 0 ]; then
	pushd . >/dev/null; cd $sp_dir
	./make_sp.sh $hw_target $rebuild $new_1sp $dev_1sp $fips $test_features $i2c_slv \
		--svn $MANTICORE_1SP_SVN --version $MANTICORE_1SP_BUILD_VERSION \
		--key_root $root_key --key_1sp $sp1_key $secondary_key
	if [ $? -ne 0 ]; then
		popd >/dev/null
		exit 1
	fi
	popd >/dev/null
fi

if [ ! -e "$sp1_list" ]; then
	echo "No 1SP image list file found: $sp1_list"
	exit 1
fi
if [ ! -e "$sprt_list" ]; then
	echo "No SPRT image list file found: $sprt_list"
	exit 1
fi

sp1_img=`grep 1SP $sp1_list | awk '{print $2}'`
sprt_reset=`grep Reset $sp1_list | awk '{print $2}'`
sprt_img=`cat $sprt_list`

##
# Build the CP firmware.
##
if [ $sp_only -eq 0 ]; then
	echo -e "\e[1;36mBuilding CP firmware...\e[0m"

	cp_dir=$top/cp

	cp_build=$cp_dir/hsm/target/thumbv7em-none-eabihf/firmware
	cp_list=$cp_build/cp.list

	if [ $prebuild_only -eq 0 ]; then
		pushd . >/dev/null; cd $cp_dir
		echo "./make_cp.sh $rebuild $cp_features"
		./make_cp.sh $rebuild $cp_features
		if [ $? -ne 0 ]; then
			popd >/dev/null
			exit 1
		fi
		popd >/dev/null
	fi

	if [ -e "$cp_list" ]; then
		cp_img=`cat $cp_list`
	else
		echo "ERROR: No CP image list found at $cp_list"
		exit 1
	fi
else
	echo "Skipping CP build."
	cp_img=
fi

##
# Build the FP firmware.
##
if [ $sp_only -eq 0 ]; then
	echo -e "\e[1;36mBuilding FP firmware...\e[0m"
	fp_dir=$top/fp
	fp_build=$fp_dir/qmgr/_out
	fp0_list=$fp_build/fp0.list
	fp1_list=$fp_build/fp1.list
	fp2_list=$fp_build/fp2.list

	if [ $prebuild_only -eq 0 ]; then
		pushd . >/dev/null; cd $fp_dir
		echo "./make_fp.sh $rebuild $fp_features"
		./make_fp.sh $rebuild $fp_features
		if [ $? -ne 0 ]; then
			popd >/dev/null
			exit 1
		fi
		popd >/dev/null
	fi

	if [ -e "$fp0_list" ]; then
		fp0_img=`cat $fp0_list`
	else
		echo "ERROR: No FP0 image list found at $fp0_list."
		exit 1
	fi

	if [ -e "$fp1_list" ]; then
		fp1_img=`cat $fp1_list`
	else
		echo "ERROR: No FP1 image list found at $fp1_list."
		exit 1
	fi

	if [ -e "$fp2_list" ]; then
		fp2_img=`cat $fp2_list`
	else
		echo "ERROR: No FP2 image list found at $fp2_list."
		exit 1
	fi
else
	echo "Skipping FP build."
	fp0_img=
	fp1_img=
	fp2_img=
fi

##
# PCIe PHY firmware.
##
if [ $sp_only -eq 0 ]; then
	echo -e "\e[1;36mInclude PCIe firmware...\e[0m"

	pcie_dir=$top/pcie
	pcie_build=$pcie_dir/build
	pcie_list=$pcie_build/pcie.list

	if [ $prebuild_only -eq 0 ]; then
		pushd . >/dev/null; cd $pcie_dir
		./make_pcie.sh
		if [ $? -ne 0 ]; then
			popd >/dev/null
			exit 1
		fi
		popd >/dev/null
	fi

	if [ -e "$pcie_list" ]; then
		pcie_img=`cat $pcie_list`
	else
		echo "ERROR: No PCIe image list found at $pcie_list"
		exit 1
	fi
else
	echo "Skipping PCIe firmware."
	pcie_img=
fi


##
# Build the firmware package.
##
echo -e "\e[1;36mBuilding firmware package...\e[0m"

sprt_args=
for sp in $sprt_img; do
	sprt_args="$sprt_args -s $sp"
done

cp_args=
for cp in $cp_img; do
	cp_args="$cp_args -c $cp"
done

fp0_args=
for fp in $fp0_img; do
	fp0_args="$fp0_args -f $fp"
done

fp1_args=
for fp in $fp1_img; do
	fp1_args="$fp1_args -F $fp"
done

fp2_args=
for fp in $fp2_img; do
	fp2_args="$fp2_args -p $fp"
done

pcie_args=
for pcie in $pcie_img; do
	pcie_args="$pcie_args -P $pcie"
done

fw_package=$(OUTPUT_DIR="$build" tools/create_firmware_package.sh $sprt_args $cp_args $fp0_args \
	$fp1_args $fp2_args $pcie_args -r $MANTICORE_RECOVERY_VERSION -R $MANTICORE_MIN_RECOVERY \
	$fips_certified $sprt_reset $MANTICORE_FW_SVN $MANTICORE_FW_BUILD_VERSION \
	$compat_version $fw_key)
if [ $? -ne 0 ]; then
	echo "$fw_package"
	exit 1
fi


##
# Build the firmware key manifest.
##
fw_key_manifest=$(OUTPUT_DIR="$build" ../tools/generate_firmware_key_manifest.sh -0 $fw_key \
	-1 $manifest_root_key -2 $unlock_key -3 $manifest_clear_key -4 $intrusion_reset_key \
	-5 $identity_renew_key -6 $fw_update_key -7 $rma_key $MANTICORE_FW_SVN $sp1_key)
if [ $? -ne 0 ]; then
	echo "$fw_key_manifest"
	exit 1
fi

# TODO: Bug 2767702: [Build Failure] build is failed due to PR merge in FP
# ##
# # Prepare FP tokenizer.
# ##
# if [ $prebuild_only -eq 0 ]; then
# 	./tools/prepare_fp_tokenizer_source.sh
# 	if [ $? -ne 0 ]; then
# 		exit 1
# 	fi
# fi

##
# Build the final Manticore firmware image.
##
if [ -z "$OUTPUT_DIR" ]; then
	out_dir=$build
else
	out_dir="$OUTPUT_DIR"
fi

manticore_out=$out_dir/manticore.bin
manticore_update_token=$out_dir/manticore_update_auth.bin

cat $sp1_img $fw_key_manifest $fw_package > $manticore_out
if [ $? -ne 0 ]; then
	exit 1
fi

$top/../../generate_checksum.sh $manticore_out \
	"Manticore FW v$major.$minor.$release.$build_num-${build_time}${ext_str}" > \
	$out_dir/checksum.txt

fw_update_auth=$(OUTPUT_DIR="$build" $top/../../generate_authorized_fw_token.sh $manticore_out \
	sha384 $fw_update_key sha384)
if [ $? -ne 0 ]; then
	echo "$fw_update_auth"
	exit 1
fi

cp $fw_update_auth $manticore_update_token

##
##

echo
echo "Manticore FW v$major.$minor.$release.$build_num-${build_time}${ext_str}:"
echo $manticore_out
