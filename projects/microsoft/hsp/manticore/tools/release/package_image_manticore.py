import os
import sys
import argparse
from prod_image_manticore import *


def bundle_signed_image (unsigned_dir, signed_dir, signed_img_dir, fips):
    """
    Create the complete firmware image from the signed components.

    :param unsigned_dir: The directory containing the unsigned image components.
    :param signed_dir: The directory containing the component signatures.
    :param signed_img_dir: The directory where the signed firmware image should be stored.
    :param fips: The flag controlling Manticore firmware naming; when "true", the signed binary includes a "-fips" suffix.

    :return The path to the signed firmware image.
    """

    unsigned_pkg_dir = os.path.join (unsigned_dir, SIGNING_DIR_FW_PACKAGE)
    signed_pkg_dir = os.path.join (signed_dir, SIGNING_DIR_FW_PACKAGE)

    component = {}
    for name in os.listdir (unsigned_pkg_dir):
        component[name] = {}
        component[name]["data"] = read_file_data (os.path.join (unsigned_pkg_dir, name))
        component[name]["sig"] = build_ecdsa_p384_der_signature (
            os.path.join (signed_pkg_dir, name), SIG_LENGTH)

    if (len (component.keys ()) > 1):
        print ("There should only be one firmware package: {0}".format (component.keys ()))
        sys.exit (1)

    pkg_version = list (component)[0]

    _, offset, _ = parse_firmware_descriptor (component[pkg_version]["data"], 0)
    img_version = "{0}-{1}".format (
        *parse_build_version_number (component[pkg_version]["data"], offset + 23))

    img_1sp = read_file_data (os.path.join (unsigned_dir, SIGNING_DIR_1SP, "boot.img"))
    img_iv = read_file_data (os.path.join (unsigned_dir, "iv"))

    if fips:
        manticore_img_name = f"manticore_{pkg_version}-fips.bin"
    else:
        manticore_img_name = f"manticore_{pkg_version}.bin"

    release_img = os.path.join (signed_img_dir, manticore_img_name)

    write_file_data (release_img, img_1sp, FIRMWARE_KEY_MANIFEST, component[pkg_version]["data"],
        component[pkg_version]["sig"], img_iv)

    generate_image_checksum (release_img, img_version,
        os.path.join (signed_img_dir, "checksum_{0}.txt".format (pkg_version)))

    unsigned_auth_dir = os.path.join (unsigned_dir, SIGNING_DIR_FW_AUTH)
    prepare_signing_directory (unsigned_auth_dir)

    generate_image_authorization (release_img,
        os.path.join (unsigned_auth_dir, "manticore_update_auth.bin".format (pkg_version)))

    return release_img


########################
# Script start
########################

if __name__ == '__main__':
    """
    This script will construct a complete Manticore release image from the available signed image
    components.

    Args: [img path] [--fips]

    The image path must be the root directory that contains both the 'unsigned' and 'signed'
    directories.  If this path is not specified, the script directory will be used.
    """
    parser = argparse.ArgumentParser ()
    parser.add_argument("img_dir", nargs="?", default=".", help="Image directory (defaults to current directory if not specified).")
    parser.add_argument ('-f', '--fips', action='store_true', help='FIPS flag')
    
    args, _ = parser.parse_known_args(sys.argv[1:])
    img_dir = args.img_dir

    unsigned_dir = os.path.join (img_dir, UNSIGNED_DIR)
    signed_dir = os.path.join (img_dir, SIGNED_DIR)

    signed_img_dir = os.path.join (signed_dir, "img")
    prepare_signing_directory (signed_img_dir)

    bundle_signed_image (unsigned_dir, signed_dir, signed_img_dir, args.fips)
