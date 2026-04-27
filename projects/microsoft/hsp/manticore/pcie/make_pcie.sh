#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

set -e
top=`dirname "$(realpath "$BASH_SOURCE")"`
build=$top/build

if ! source "$top/../../../img_functions.sh"; then
	exit 1
fi


##
# Version of the current PCIe PHY release that should be used for firmware builds.
##
pcie_version="0_1_0_7"


##
# Build variables
##

# Released Main PCIe PHY image that should be used with firmware builds.
main_img_bin="$top/phy/v$pcie_version/main_$pcie_version.bin"
main_img_address=0x61003000
main_img_marker=0x6d61696e
main_img_length=`stat -c %s $main_img_bin`
let 'main_img_length = main_img_length / 4'

# Released Common PCIe PHY image that should be used with firmware builds.
cmn_img_bin="$top/phy/v$pcie_version/PCIE_REF100MHz_CMN_$pcie_version.bin"
cmn_img_address=0x61013010
cmn_img_marker=0x636f6d6e
cmn_img_length=`stat -c %s $cmn_img_bin`
let 'cmn_img_length = cmn_img_length / 4'

# Released Lane PCIe PHY image that should be used with firmware builds.
lane_img_bin="$top/phy/v$pcie_version/PCIE_LANE_$pcie_version.bin"
lane_img_address=0x61013820
lane_img_marker=0x6c616e65
lane_img_length=`stat -c %s $lane_img_bin`
let 'lane_img_length = lane_img_length / 4'


##
# Verify the binary measurement against the CFM.
##
fw_digest=`cat $main_img_bin $cmn_img_bin $lane_img_bin | openssl dgst -sha384 | awk '{print $2}'`
fw_measurement=`echo -n 090200e100${fw_digest} | xxd -r -p | openssl dgst -sha384 | awk '{print $2'}`

set +e

cfm_config=$top/../tools/release/cfm_config.yaml
grep -q "Digest: '${fw_measurement}'" $cfm_config
if [ $? -ne 0 ]; then
	echo "Measurement of PHY FW in the CFM does not match the source files for v$pcie_version"
	echo "If the firmware has changed, the CFM digest and IDFU version need to be updated."
	echo ""
	echo "Calculated Measurement: $fw_measurement"
	exit 1
fi

set -e

##
# Wrap the binary files for consumption by 1SP.
##
mkdir -p $build

main_img=$build/main_${pcie_version}_mant.bin
empty_file $main_img
output_binary_word $main_img_marker $main_img
output_binary_word $main_img_length $main_img
output_binary_qword 0 $main_img
cat $main_img_bin >> $main_img

cmn_img=$build/PCIE_REF100MHz_CMN_${pcie_version}_mant.bin
empty_file $cmn_img
output_binary_word $cmn_img_marker $cmn_img
output_binary_word $cmn_img_length $cmn_img
output_binary_qword 0 $cmn_img
cat $cmn_img_bin >> $cmn_img

lane_img=$build/PCIE_LANE_${pcie_version}_mant.bin
empty_file $lane_img
output_binary_word $lane_img_marker $lane_img
output_binary_word $lane_img_length $lane_img
output_binary_qword 0 $lane_img
cat $lane_img_bin >> $lane_img

##
# Generate the list of images for inclusion in the firmware package.
##
img_list="$main_img,$main_img_address $cmn_img,$cmn_img_address $lane_img,$lane_img_address"

echo $img_list > $build/pcie.list
