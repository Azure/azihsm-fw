#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/../../tools/boot_functions.sh"; then
	exit 1
fi

fw_component="`dirname "$(realpath "$BASH_SOURCE")"`/../../../firmware_component.sh"
if [ ! -e "$fw_component" ]; then
	echo "Can't find firmware component script: $fw_component"
	exit 1
fi

usage() {
	echo "Usage: `basename $0` [-s <SPRT count>] [-c <CP count>] [-f <FP0 count>] [-F <FP1 count>] [-p <FP2 count>] [-P <PHY count>] [-l <total imgs length>] [-L <pkg sig length>] [-r <recovery rev>] [-R <min rollback] [-CB] <SPRT vector> <hex svn> <hex version> <compat rev> <key>"
	exit 1
}

declare -i OPTIND=
OPTNAM=
OPTARG=
sprt_count="0"
cp_count="0"
fp0_count="0"
fp1_count="0"
fp2_count="0"
pcie_count="0"
imgs_length="0"
sig_length="0"
recovery_rev="0"
min_rollback="0"
fips_certified="0"
bks_isolation="0"

while getopts :Bc:Cf:F:l:L:p:P:r:R:s: OPTNAM; do
	case "$OPTNAM" in
		B)
			bks_isolation="1"
		;;

		c)
			cp_count=$OPTARG
		;;

		C)
			fips_certified="1"
		;;

		f)
			fp0_count=$OPTARG
		;;

		F)
			fp1_count=$OPTARG
		;;

		l)
			imgs_length=$OPTARG
		;;

		L)
			sig_length=$OPTARG
		;;

		p)
			fp2_count=$OPTARG
		;;

		P)
			pcie_count=$OPTARG
		;;

		r)
			recovery_rev=$OPTARG
		;;

		R)
			min_rollback=$OPTARG
		;;

		s)
			sprt_count=$OPTARG
		;;

		*)
			usage
		;;
	esac
done
shift `iopt_shift $#`

if (($# < 5)); then
	usage
fi

declare -i reset_vector=`parse_hex_value $1`
declare -i svn=`parse_hex_value $2`
version=$3
declare -i compat_version=$4
img_key=$5

if [ ! -e "$img_key" ]; then
	echo "Unknown signing key: $img_key"
	exit 1
fi

test_svn $svn

init_filenames
desc_bin="firmware_descriptor"
desc_out=$out_dir/$desc_bin

let 'total_imgs = sprt_count + cp_count + fp0_count + fp1_count + fp2_count + pcie_count'

empty_file "$desc_out"
output_binary_qword $svn "$desc_out"
output_binary_word $reset_vector "$desc_out"
output_binary_byte "$sprt_count" "$desc_out"
output_binary_byte "$cp_count" "$desc_out"
output_binary_byte "$fp0_count" "$desc_out"
output_binary_byte "$fp1_count" "$desc_out"
output_binary_byte "$fp2_count" "$desc_out"
output_binary_byte "$pcie_count" "$desc_out"
output_binary_short $compat_version "$desc_out"
output_binary_byte "$fips_certified" "$desc_out"
output_binary_byte "$bks_isolation" "$desc_out"

# Wrap the descriptor as a firmware component with a firmware header.
HASH_TYPE="sha384" OUTPUT_DIR="$out_dir" COMPONENT_MARKER="0x4d465744" \
	BUILD_VERSION_NUMBER="$version" RECOVERY_REVISION="$recovery_rev" MIN_ROLLBACK="$min_rollback" \
	EXTRA_IMAGES="$total_imgs" EXTRA_LENGTH="$imgs_length" EXTRA_SIG_LENGTH="$sig_length" \
	$fw_component "$desc_bin" "$img_key" "$out_dir"
