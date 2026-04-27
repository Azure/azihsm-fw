import os
import sys
import argparse
from prod_image_manticore import *


def parse_signed_image (img_data, signed_1sp_dir):
    """
    Parse the image extract the signed components.

    :param img_data: Raw data for the signed 1SP image.
    :param signed_1sp_dir: The directory for storing the signed 1SP components.
    """

    xfer_length, keys_length, fw_length = parse_boot_image (img_data)

    xfer_data = img_data[0:xfer_length]

    keys_end = xfer_length + keys_length
    keys_data = img_data[xfer_length:keys_end]

    fw_end = keys_end + fw_length
    fw_data = img_data[keys_end:fw_end]

    version, _, _ = parse_build_version_number (img_data, keys_end + 220)

    # Write out individual pieces
    write_file_data (os.path.join (signed_1sp_dir, "xfer"), xfer_data)
    write_file_data (os.path.join (signed_1sp_dir, "keys"), keys_data)
    write_file_data (os.path.join (signed_1sp_dir, "fw"), fw_data)

    # Construct the signed 1SP image to use during firmware image builds
    write_file_data (os.path.join (signed_1sp_dir, "dc_scm_1sp_v{0}_prod.img".format (version)),
        NULL_XFER_MANIFEST, keys_data, fw_data)


########################
# Script start
########################

if __name__ == '__main__':
    """
    The script is used to parse out components of a 1SP image.  This would typically be used to
    update pre-built and pre-signed 1SP components with new signed data.  The 1SP components will be
    stored in a 'signed' directory.

    Args: <1SP image path> [output path]

    If no output path is specified, the 'signed' directory will be created in the script directory.
    """
    parser = argparse.ArgumentParser ()
    parser.add_argument("img_path",help="Path to the 1SP image")
    parser.add_argument("out_dir", nargs="?", default=".", help="Output directory (defaults to current directory if not specified).")

    args, _ = parser.parse_known_args(sys.argv[1:])
    img_path = args.img_path
    out_dir = args.out_dir
    img_name = os.path.basename (img_path)

    # Load input signed 1SP image.
    img_data = bytearray (read_file_data (img_path))

    signed_1sp_dir = os.path.join (out_dir, SIGNED_DIR, SIGNING_DIR_1SP)
    prepare_signing_directory (signed_1sp_dir)

    print ("FW length: {0}".format (len (img_data)))
    parse_signed_image (img_data, signed_1sp_dir)
