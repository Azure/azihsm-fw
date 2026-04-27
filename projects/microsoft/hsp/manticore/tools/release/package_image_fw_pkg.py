import os
import sys
import argparse
from prod_image_manticore import *


def bundle_firmware_package (unsigned_pkg_dir, unsigned_comp_dir, signed_comp_dir):
    """
    Create the complete firmware package from the signed components, excluding the full package
    signature an the IV table.

    :param unsigned_pkg_dir: The directory to store the unsigned firmware package.
    :param unsigned_comp_dir: The directory containing the component data.
    :param signed_comp_dir: The directory containing the component signatures.
    """

    component = {}
    fw_pkg = bytearray ([])

    for name in os.listdir (unsigned_comp_dir):
        component[name] = {}
        component[name]["data"] = read_file_data (os.path.join (unsigned_comp_dir, name))
        component[name]["sig"] = build_ecdsa_p384_der_signature (
            os.path.join (signed_comp_dir, name), SIG_LENGTH)

    _, offset, _ = parse_firmware_descriptor (component["desc"]["data"], 0)
    version, _, build_type = parse_build_version_number (component["desc"]["data"], offset + 23)
    print ("Build FW Pkg v{0}{1}".format (version, build_type))

    print ("Add: desc")
    fw_pkg.extend (component["desc"]["data"])
    fw_pkg.extend (component["desc"]["sig"])
    del component["desc"]

    for key, comp in sorted (component.items ()):
        print ("Add: {0}".format (key))
        fw_pkg.extend (comp["data"])
        fw_pkg.extend (comp["sig"])

    write_file_data (os.path.join (unsigned_pkg_dir, "v{0}{1}".format (version, build_type)),
        fw_pkg)


########################
# Script start
########################

if __name__ == '__main__':
    """
    This script will rebuild a Manticore firmware package from a combination of unsigned components
    and raw signatures.  The result will be a complete, unsigned firmware package.  It will also not
    include the IV table, since that is appended after the signature.

    Args: [img path]

    The image path must be the root directory that contains both the 'unsigned' and 'signed'
    directories.  If this path is not specified, the script directory will be used.
    """
    parser = argparse.ArgumentParser ()
    parser.add_argument("img_dir", nargs="?", default=".", help="Image directory (defaults to current directory if not specified).")

    args, _ = parser.parse_known_args(sys.argv[1:])
    img_dir = args.img_dir
    
    unsigned_pkg_dir = os.path.join (img_dir, UNSIGNED_DIR, SIGNING_DIR_FW_PACKAGE)
    prepare_signing_directory (unsigned_pkg_dir)

    unsigned_comp_dir = os.path.join (img_dir, UNSIGNED_DIR, SIGNING_DIR_FW_COMPONENTS)
    signed_comp_dir = os.path.join (img_dir, SIGNED_DIR, SIGNING_DIR_FW_COMPONENTS)

    bundle_firmware_package (unsigned_pkg_dir, unsigned_comp_dir, signed_comp_dir)
