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

declare -i OPTIND=
OPTNAM=
OPTARG=
sp_images=""
cp_images=""
fp0_images=""
fp1_images=""
fp2_images=""
pcie_images=""
recovery_rev="0"
min_rollback="0"
fips_certified=""

while getopts :c:Cf:F:p:P:r:R:s: OPTNAM; do
	case "$OPTNAM" in
		c)
			cp_images="$cp_images $OPTARG"
		;;

		C)
			fips_certified="-C"
		;;

		f)
			fp0_images="$fp0_images $OPTARG"
		;;

		F)
			fp1_images="$fp1_images $OPTARG"
		;;

		p)
			fp2_images="$fp2_images $OPTARG"
		;;

		P)
			pcie_images="$pcie_images $OPTARG"
		;;

		r)
			recovery_rev=$OPTARG
		;;

		R)
			min_rollback=$OPTARG
		;;

		s)
			sp_images="$sp_images $OPTARG"
		;;

		*)
			break
		;;
	esac
done
shift `iopt_shift $#`

if (($# < 5)); then
	echo "Usage: `basename $0` [-s <SP image,addr>] [-c <CP image,addr>] [-f <FP0 image,addr>] [-F <FP1 image,addr>] [-p <FP2 image,addr>] [-P <PCIe image,addr>] [-r <recovery rev>] [-R <min rollback] [-C] <SPRT vector> <hex svn> <hex version> <compat rev> <key>"
	exit 1
fi

reset_vector=$1
svn=$2
version=$3
compat_version=$4
img_key=$5

if [ ! -e "$img_key" ]; then
	echo "Unknown signing key: $img_key"
	exit 1
fi

svn_tmp=`parse_hex_value $svn`
test_svn $svn_tmp

init_filenames
pkg_iv=$out_dir/firmware_package.iv
pkg_bin=$out_dir/firmware_package.bin
pkg_sig=$out_dir/firmware_package.sig
pkg_img=$out_dir/firmware_package.img

total_img_length=0	# The total length of all wrapped firmware components.
total_iv=0			# Keep track of the number of encryption IVs needed for the package.


# Generate firmware components for each image in a provided list.  All images will have the same
# build version and be signed with same key.
#
# Args:
#	- The marker to use in the component header.
#	- A list of <img,addr> tuples that need to be wrapped.
#
# Outputs the list of firmware components in the img_components variable.
# The total_img_length variable will get updated for the created components.
create_image_components() {
	local marker=$1
	local images=$2
	img_components=""

	for fw_img in $images; do
		local img=(`echo $fw_img | awk -F , '{print $1} {print $2}'`)
		local comp_out=`dirname ${img[0]}`
		local comp_bin=`basename ${img[0]}`
		local comp_load=${img[1]}

		if [ ! -e "${img[0]}" ]; then
			echo "Unknown firmware image: ${img[0]}"
			exit 1
		fi

		# All components must be 16-byte aligned.
		align_file "${img[0]}" 16 "FF"

		comp_img=$(HASH_TYPE="sha384" OUTPUT_DIR="$comp_out" COMPONENT_MARKER="$marker" \
			LOAD_ADDRESS="$comp_load" BUILD_VERSION_NUMBER="$version" $fw_component \
			"$comp_bin" "$img_key" "$comp_out")
		if [ $? -ne 0 ]; then
			echo "$comp_img"
			exit 1
		fi

		img_components="$img_components $comp_img"

		local comp_len=`stat -c %s $comp_img`
		let 'total_img_length = total_img_length + comp_len'
	done
}


# Generate the firmware components for each image being included in the package.
echo "SP: $sp_images, $total_img_length" >&2
create_image_components "0x53505254" "$sp_images"
sp_components=($img_components)
total_iv=$((total_iv + ((${#sp_components[@]}))))

echo "CP: $cp_images, $total_img_length" >&2
create_image_components "0x43504657" "$cp_images"
cp_components=($img_components)
total_iv=$((total_iv + ((${#cp_components[@]}))))

echo "FP0: $fp0_images, $total_img_length" >&2
create_image_components "0x46504657" "$fp0_images"
fp0_components=($img_components)
total_iv=$((total_iv + ((${#fp0_components[@]}))))

echo "FP1: $fp1_images, $total_img_length" >&2
create_image_components "0x46504657" "$fp1_images"
fp1_components=($img_components)
total_iv=$((total_iv + ((${#fp1_components[@]}))))

echo "FP2: $fp2_images, $total_img_length" >&2
create_image_components "0x46504657" "$fp2_images"
fp2_components=($img_components)
total_iv=$((total_iv + ((${#fp2_components[@]}))))

echo "PCIe: $pcie_images, $total_img_length" >&2
create_image_components "0x50434965" "$pcie_images"
pcie_components=($img_components)
total_iv=$((total_iv + ((${#pcie_components[@]}))))


# Generate the IV block.
let 'total_iv = (total_iv * 16) + 48 + 4'

empty_file "$pkg_iv"
output_binary_word "0x656e6976" "$pkg_iv"
add_padding "$pkg_iv" $total_iv "FF"


# Generate the Firmware Descriptor
get_max_signature_length "$img_key"

fw_descriptor=$(OUTPUT_DIR="$out_dir" \
	`dirname "$(realpath "$BASH_SOURCE")"`/generate_firmware_descriptor.sh \
	-s ${#sp_components[@]} -c ${#cp_components[@]} -f ${#fp0_components[@]} \
	-F ${#fp1_components[@]} -p ${#fp2_components[@]} -P ${#pcie_components[@]} \
	-l $total_img_length -L $sig_len -r "$recovery_rev" -R "$min_rollback" $fips_certified \
	"$reset_vector" "$svn" "$version" "$compat_version" "$img_key")
if [ $? -ne 0 ]; then
	echo "$fw_descriptor"
	exit 1
fi


# Build the complete Firmware Package
cat $fw_descriptor > $pkg_bin
for comp in ${sp_components[@]} ${cp_components[@]} ${fp0_components[@]} ${fp1_components[@]} ${fp2_components[@]} ${pcie_components[@]}; do
	cat $comp >> $pkg_bin
done

generate_signature "$pkg_bin" "$pkg_sig" "$img_key" "sha384"
add_padding "$pkg_sig" "$sig_len" "00"

cat $pkg_bin $pkg_sig $pkg_iv > $pkg_img
echo $pkg_img
