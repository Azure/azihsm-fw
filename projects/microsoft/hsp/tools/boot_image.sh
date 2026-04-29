#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.

if ! source "`dirname "$(realpath "$BASH_SOURCE")"`/boot_functions.sh"; then
	exit 1
fi

usage() {
	echo "Usage: $0 [-e <aes key>] [-k <fw key2>] [-v <img_ver>] [-r <root_2key>] [-t <transfer key 2>] <hex svn> <img type> <hex version> <load addr> <fw img> <root key> <fw key> [transfer key] [grant token]"
}

declare -i OPTIND=
OPTNAM=
OPTARG=
aes_key=
fw_key2=
root_2key=
tx_key2=
img_ver=1

while getopts :e:k:v:r:t: OPTNAM; do
	case "$OPTNAM" in
		e)
			aes_key=$OPTARG
		;;

		k)
			fw_key2=$OPTARG
		;;

		v)
			img_ver=$OPTARG
		;;

		r)
			root_2key=$OPTARG
		;;

		t)
			tx_key2=$OPTARG
		;;

		*)
			break
		;;
	esac
done
shift `iopt_shift $#`

declare -i \
	type=$2 \
	xfer=-1

svn=$1
version=$3
addr=$4
fw_img=$5
root_key=$6
fw_key=$7
tx_key=$8
token=$9

case $img_ver in
    1|2)
        :
    ;;

    *)
        echo "Unknown version number: $img_ver"
        exit 1
    ;;
esac

case $type in
	0)
		# Null manifest
		if (($# < 7)); then
			usage
			exit 1
		fi

		xfer=0
		type=0
	;;

	1)
		# Ownership transfer
		if (($# < 8)); then
			usage
			exit 1
		fi

		xfer=1
		type=0
	;;

	2)
		# Tenancy grant
		if (($# < 9)); then
			usage
			exit 1
		fi

		xfer=2
		type=1
	;;

	*)
		echo "Invalid image type: $type"
		exit 1
	;;
esac

out_dir=`get_output_dir`
boot_img=$out_dir/boot.img

case $xfer in
	0)
		xfer_manifest=`$script_dir/generate_null_manifest.sh`
	;;

	1)
		# Note that the TX keys are the current owner key and the input root keys are the new owner keys.
		if [ "$img_ver" = "1" ]; then
			xfer_manifest=`$script_dir/generate_ownership_transfer_manifest.sh "$tx_key" "$root_key"`
		else
			xfer_manifest=`$script_dir/generate_ownership_transfer_manifest_dual_root.sh -k "$root_2key" -r "$tx_key2" "$tx_key" "$root_key"`
		fi
	;;

	2)
		xfer_manifest=`$script_dir/generate_tenancy_grant_manifest.sh -k "$tx_key" "$fw_key" file "$token"`
	;;
esac

if (($? != 0)); then
	echo "Failed to create Transfer Manifest"
	echo "$xfer_manifest"
	exit 1
fi

case $type in
	0)
		if [ "$img_ver" = "1" ]; then
			key_manifest=`$script_dir/generate_rom_key_manifest.sh -k "$fw_key2" $svn $type "$root_key" "$fw_key"`
		else
			key_manifest=`$script_dir/generate_rom_key_manifest_dual_root.sh -k "$fw_key2" -r "$root_2key" $svn $type "$root_key" "$fw_key"`
		fi
	;;

	1)
		if [ "$img_ver" = "1" ]; then
			key_manifest=`$script_dir/generate_rom_key_manifest.sh $svn $type "$root_key" "$tx_key"`
		else
			key_manifest=`$script_dir/generate_rom_key_manifest_dual_root.sh -k "$fw_key2" -r "$root_2key" $svn $type "$root_key" "$tx_key"`
		fi
	;;
esac

if (($? != 0)); then
	echo "Failed to create Key Manifest"
	echo "$key_manifest"
	exit 1
fi

sp1_img=`$script_dir/create_1sp_image.sh -e "$aes_key" -k "$fw_key2" $svn $version $addr "$fw_img" "$fw_key"`

if (($? != 0)); then
	echo "Failed to create 1SP Image"
	echo "$sp1_img"
	exit 1
fi

cat "$xfer_manifest" "$key_manifest" "$sp1_img" > "$boot_img"
echo $boot_img
