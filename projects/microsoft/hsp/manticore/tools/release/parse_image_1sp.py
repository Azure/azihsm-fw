import os
import re
import sys
import argparse
from prod_image_manticore import *

def run_parse_image (img_data, unsigned_dir, unsigned_1sp_dir, fips):
    """
    Parse the image and prepare it for signing.

    :param img_data: Raw data for the 1SP image to sign.
    :param unsigned_dir: The root directory for all unsigned image components.
    :param unsigned_1sp_dir: The directory for storing the 1SP image to sign.
    :param fips: The flag indicating whether FIPS signing should be applied, when "true" FIPS_OWNER_KEY_MANIFEST is used.

    """

    xfer_length, root_length, boot_length = parse_boot_image (img_data)

    version_offset = xfer_length + root_length + 220
    version, _ = update_build_version (img_data, version_offset)

    print ("1SP version {0}".format (version))

    root_start = xfer_length
    boot_start = xfer_length + root_length
    boot_end = xfer_length + root_length + boot_length

    # Dev 1SP with prod owner transfer and key manifests.
    if fips:
        write_file_data (os.path.join (unsigned_1sp_dir, "boot.img"), OWNER_XFER_MANIFEST,
            FIPS_OWNER_KEY_MANIFEST, img_data[boot_start:boot_end])
    else:
        write_file_data (os.path.join (unsigned_1sp_dir, "boot.img"), OWNER_XFER_MANIFEST,
            OWNER_KEY_MANIFEST, img_data[boot_start:boot_end])

    # Dev 1SP with prod owner transfer manifest and dev key manifest.
    write_file_data (os.path.join (unsigned_1sp_dir, "boot-dev.img"), OWNER_XFER_MANIFEST,
        img_data[root_start:boot_end])

    # Dev 1SP with no transfer manifest and dev key manifest.
    write_file_data (os.path.join (unsigned_1sp_dir, "boot"), img_data[root_start:boot_end])


########################
# Script start
########################

if __name__ == '__main__':
    """
    This script will take a dev-signed 1SP firmware image and construct images structures suitable
    for production signing.  The images that can be submitted for signing will be stored in an
    'unsigned' directory.

    Args: <1SP image path> [output path] [--fips]

    If an output path is not specified, the 'unsigned' directory will be created in the script
    directory.

    If --fips is specified then FIPS signing part is enabled
    """
    parser = argparse.ArgumentParser ()
    parser.add_argument("img_path",help="Path to the 1SP image")
    parser.add_argument("out_dir", nargs="?", default=".", help="Output directory (defaults to current directory if not specified).")
    parser.add_argument ('-f', '--fips', action='store_true', help='FIPS flag')
    
    args, _ = parser.parse_known_args(sys.argv[1:])
    img_path = args.img_path
    out_dir = args.out_dir
    img_name = os.path.basename (img_path)

    # Load input 1SP image to sign
    img_data = bytearray (read_file_data (img_path))

    unsigned_dir = os.path.join (out_dir, UNSIGNED_DIR)

    unsigned_1sp_dir = os.path.join (out_dir, UNSIGNED_DIR, SIGNING_DIR_1SP)
    prepare_signing_directory (unsigned_1sp_dir)

    print ("FW length: {0}".format (len (img_data)))
    run_parse_image (img_data, unsigned_dir, unsigned_1sp_dir, args.fips)
