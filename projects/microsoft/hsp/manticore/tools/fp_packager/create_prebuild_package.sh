#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

set -e

top=`dirname "$(realpath "$BASH_SOURCE")"`
today=$(date +%y_%m_%d)
PREBUILD_PKG_NAME="manticore_build_$today"
MANTICORE_DIR=$(realpath "$top/../../")
ROOT_DIR="$top/../../../../../../"
PREBUILD_PKG_DIR=$(realpath "$top/../../$PREBUILD_PKG_NAME")/
FP_OUT_ZIP="fp_out.zip"

SP1_VERSION=$(grep -m1 -oP 'sp1_version="\K[^"]+' $MANTICORE_DIR/sp/make_sp.sh)

# Number of template files in the list below.
TOTAL_TEMPLATES=4

# List of files
files=(
    # List templates
    "projects/microsoft/hsp/manticore/cp/hsm/target/thumbv7em-none-eabihf/firmware/cp.list.template"
    "projects/microsoft/hsp/manticore/sp/build-evb/sp1.list.template"
    "projects/microsoft/hsp/manticore/sp/build-evb/sprt.list.template"
    "projects/microsoft/hsp/manticore/pcie/build/pcie.list.template"

    # SP build
    "projects/microsoft/hsp/manticore/sp/dc_scm_1sp/dc_scm_1sp_v${SP1_VERSION}_prod.img"

    # Binaries, certificates and scripts
    "core/testing/keys/ecc384priv2.pem"
    "core/testing/keys/ecc384priv3.pem"
    "core/testing/keys/ecc384priv.pem"
    "core/testing/keys/ecc384priv4.pem"
    "core/testing/keys/ecc384priv5.pem"
    "projects/microsoft/firmware_component.sh"
    "projects/microsoft/generate_checksum.sh"
    "projects/microsoft/generate_authorized_fw_token.sh"
    "projects/microsoft/img_functions.sh"
    "projects/microsoft/hsp/manticore/cp/hsm/target/thumbv7em-none-eabihf/firmware/mcr-admin.data.bin"
    "projects/microsoft/hsp/manticore/cp/hsm/target/thumbv7em-none-eabihf/firmware/mcr-admin.text.bin"
    "projects/microsoft/hsp/manticore/cp/hsm/target/thumbv7em-none-eabihf/firmware/mcr-hsm.data.bin"
    "projects/microsoft/hsp/manticore/cp/hsm/target/thumbv7em-none-eabihf/firmware/mcr-hsm.text.bin"
    "projects/microsoft/hsp/manticore/fp/make_fp.sh"
    "projects/microsoft/hsp/manticore/make_manticore.sh"
    "projects/microsoft/hsp/manticore/sp/build-evb/cmake/dc_scm_sprt/dc_scm_sprt.bin"
    "projects/microsoft/hsp/manticore/sp/keys/dc_scm/HKMS_ClearManifest_469408_469420_AOC_20220215.pem"
    "projects/microsoft/hsp/manticore/sp/keys/dc_scm/HKMS_DebugUnlock_469407_469419_AOC_20220215.pem"
    "projects/microsoft/hsp/manticore/sp/keys/dc_scm/HKMS_IdentityRenewal_500080_500850_AOC_20230404.pem"
    "projects/microsoft/hsp/manticore/sp/keys/dc_scm/HKMS_IntrusionReset_469409_469421_AOC_20220215.pem"
    "projects/microsoft/hsp/manticore/sp/keys/dc_scm/HKMS_RMA_500081_500851_AOC_20230404.pem"
    "projects/microsoft/hsp/manticore/sp/keys/dc_scm/Manticore_ManifestRoot_P384.pem"
    "projects/microsoft/hsp/manticore/pcie/build/PCIE_REF100MHz_CMN_0_1_0_7_mant.bin.app"
    "projects/microsoft/hsp/manticore/pcie/build/PCIE_LANE_0_1_0_7_mant.bin.sig"
    "projects/microsoft/hsp/manticore/pcie/build/PCIE_REF100MHz_CMN_0_1_0_7_mant.bin.sig"
    "projects/microsoft/hsp/manticore/pcie/build/PCIE_LANE_0_1_0_7_mant.bin"
    "projects/microsoft/hsp/manticore/pcie/build/main_0_1_0_7_mant.bin.app"
    "projects/microsoft/hsp/manticore/pcie/build/main_0_1_0_7_mant.bin.sig"
    "projects/microsoft/hsp/manticore/pcie/build/PCIE_REF100MHz_CMN_0_1_0_7_mant.bin"
    "projects/microsoft/hsp/manticore/pcie/build/main_0_1_0_7_mant.bin"
    "projects/microsoft/hsp/manticore/pcie/build/PCIE_LANE_0_1_0_7_mant.bin.img"
    "projects/microsoft/hsp/manticore/pcie/build/PCIE_REF100MHz_CMN_0_1_0_7_mant.bin.img"
    "projects/microsoft/hsp/manticore/pcie/build/PCIE_LANE_0_1_0_7_mant.bin.app"
    "projects/microsoft/hsp/manticore/pcie/build/main_0_1_0_7_mant.bin.img"
    "projects/microsoft/hsp/manticore/tools/create_firmware_package.sh"
    "projects/microsoft/hsp/manticore/tools/generate_firmware_descriptor.sh"
    "projects/microsoft/hsp/manticore/tools/prepare_fp_tokenizer_source.sh"
    "projects/microsoft/hsp/manticore/tools/fp_packager/prebuild_patch_list.sh"
    "projects/microsoft/hsp/manticore/tools/telemetry_tokenizer/log_parser.py"
    "projects/microsoft/hsp/manticore/tools/telemetry_tokenizer/log_tokenizer.py"
    "projects/microsoft/hsp/manticore/version.h"
    "projects/microsoft/hsp/tools/boot_functions.sh"
    "projects/microsoft/hsp/tools/generate_firmware_key_manifest.sh"
    "projects/microsoft/img_functions.sh"

    # Utility FP log tokens source files
    "projects/microsoft/hsp/manticore/src/logging/manticore_fp_log_tokens.c"
    "projects/microsoft/hsp/manticore/src/logging/manticore_fp_log_tokens.h"
)

function create_list_template() {
	local template=$1
	local list_file=$(echo $template | sed "s|.template||g")
    list_file=$(realpath $list_file)

    if [ -f $list_file ]; then
		local list_file_data=`cat $list_file`
		list_file_data=$(echo "$list_file_data" | sed "s|$MANTICORE_DIR|"TEMPLATE_PATH"|g")
		echo "$list_file_data" > $template
	else
		echo "$list_file does not exist."
		exit 0
	fi
}

echo -e "\e[1;36mCreating the $PREBUILD_PKG_DIR..\e[0m"
echo "Using SP1_VERSION: $SP1_VERSION"

# create the $PREBUILD_PKG_DIR if not present
if [ -d $PREBUILD_PKG_NAME ]; then
	rm -rf $PREBUILD_PKG_NAME
fi

# create the archive $PREBUILD_PKG_DIR.zip if not present
if [ -f $PREBUILD_PKG_NAME.zip ]; then
	rm -rf $PREBUILD_PKG_NAME.zip
fi

mkdir -p $PREBUILD_PKG_DIR

# Copy manticore fW bin and FP _out directory zip file
mkdir -p $PREBUILD_PKG_DIR/binary/
cp $MANTICORE_DIR/build-evb/manticore.bin $PREBUILD_PKG_DIR/binary/.
cp $MANTICORE_DIR/build-evb/manticore_update_auth.bin $PREBUILD_PKG_DIR/binary/.
cd $MANTICORE_DIR/fp/qmgr/
zip -r $FP_OUT_ZIP _out/
mv $FP_OUT_ZIP $PREBUILD_PKG_DIR/binary/.

# go to the project root directory
cd $ROOT_DIR

# Create the template list files for SP, CP & SPRT
for list_file in "${files[@]:0:$TOTAL_TEMPLATES}"; do
	echo "Creating: $list_file"
	create_list_template $list_file
done

echo -e "\e[1;36mCoping the prebuild images & scripts into $PREBUILD_PKG_DIR..\e[0m"

# Iterate over each element of files
for item in "${files[@]}"; do
    # echo "Files: $item"
    cp --parents $item $PREBUILD_PKG_DIR
done

echo "Adding readme.md"
cp "projects/microsoft/hsp/manticore/tools/fp_packager/README.md" $PREBUILD_PKG_DIR

cd $MANTICORE_DIR
zip -r $PREBUILD_PKG_NAME.zip $PREBUILD_PKG_NAME

echo -e "\e[1;36mCreated pre-build images archive $PREBUILD_PKG_NAME.zip..\e[0m"
echo "$MANTICORE_DIR/$PREBUILD_PKG_NAME.zip"
