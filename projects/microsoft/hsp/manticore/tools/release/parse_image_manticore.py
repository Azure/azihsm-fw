import os
import re
import sys
import argparse
from prod_image_manticore import *


def parse_image (img):
    """
    Parse the image to identify each component within the image.

    :param img: A binary array containing the firmware image.

    :return A dictionary of tuples for each image component.  Each tuple will be offsets in the
    array for the start and end of data, not including signatures.  The following components will be
    reported in the dictionary:
        - xfer: The transfer manifest on the boot image.
        - root: The root key manifest on the boot image.
        - boot: The 1SP boot image.
        - keys: The firmware key manifest.
        - pkg: The entire firmware package.
        - desc: The firmware descriptor.
        - comp: An array of tuples for each firmware component.
        - iv: The encryption IV table
    """

    fw = {}

    xfer_length, root_length, boot_length = parse_boot_image (img)

    # Firmware key manifest is constant length
    fw_manifest_start = xfer_length + root_length + boot_length
    fw_manifest_length = 1472

    desc_start = fw_manifest_start + fw_manifest_length
    desc_length, _, desc_sig = parse_firmware_descriptor (img, desc_start)

    extra_images = img[desc_start + 10]
    comp_img = []

    comp_start = desc_start + desc_length + desc_sig
    for i in range (extra_images):
        comp_length, comp_sig = parse_firmware_component (img, comp_start)
        comp_img.append ((comp_start, comp_start + comp_length))

        comp_start += comp_length + comp_sig

    pkg_length = read_4byte_word (img, desc_start + 13)
    pkg_sig = read_2byte_word (img, desc_start + 17)

    iv_start = desc_start + pkg_length + pkg_sig
    iv_length = 4 + (16 * extra_images) + 48

    fw["xfer"] = (0, xfer_length)
    fw["root"] = (xfer_length, xfer_length + root_length)
    fw["boot"] = (xfer_length + root_length, fw_manifest_start)
    fw["keys"] = (fw_manifest_start, desc_start)
    fw["pkg"] = (desc_start, desc_start + pkg_length)
    fw["desc"] = (desc_start, desc_start + desc_length)
    fw["comp"] = comp_img
    fw["iv"] = (iv_start, iv_start + iv_length)

    return fw

def get_1sp_build_version_number (img, components):
    """
    Get the build version number for the 1SP image.

    :param img: A binary array containing the firmware image.
    :param components: The parsed information about the firmware components.

    :return A tuple with the base firmware version and the version extension.
    """

    version, version_ext, _ = parse_build_version_number (img, components["boot"][0] + 220)
    return version, version_ext

def update_1sp_build_version (img, components):
    """
    Update the build version number in the 1SP image to indicate a production release.

    :param img: A binary array containing the firmware image.
    :param components The parsed information about the firmware components.

    :return A tuple wit the base firmware version and the version extension.
    """

    return update_build_version (img, components["boot"][0] + 220)

def update_package_build_version (img, components):
    """
    Update the build version number in the Firmware Package to indicate a production release.

    :param img: A binary array containing the firmware image.
    :param components: The parsed information about the firmware components.

    :return A tuple wit the base firmware version and the version extension.
    """

    _, desc_offset, _ = parse_firmware_descriptor (img, components["desc"][0])
    version, version_ext = update_build_version (img, components["desc"][0] + desc_offset + 23)

    for i, comp in enumerate (components["comp"]):
        comp_version, comp_version_ext = update_build_version (img, comp[0] + 23)

        if ((version != comp_version) or (version_ext != comp_version_ext)):
            print ("Firmware component {0} does not match the package build number".format (i))
            sys.exit (1)

    return version, version_ext

def run_parse_image (img_data, version, unsigned_dir, unsigned_comp_dir, unsigned_1sp_dir, fips):
    """
    Parse the image and prepare the components for signing.

    :param img_data: Raw data for the firmware image to sign.
    :param version: The version number of the firmware image.
    :param unsigned_dir: The root directory for all unsigned image components.
    :param unsigned_comp_dir: The directory for storing the individual components to sign.
    :param unsigned_1sp_dir: The directory for storing the 1SP image to sign.
    :param fips: The flag indicating whether FIPS signing should be applied, when "true" FIPS_OWNER_KEY_MANIFEST is used.
    """

    fw_imgs = parse_image (img_data)

    img_version, _ = update_package_build_version (img_data, fw_imgs)
    if (version != img_version):
        print ("Unexpected build version in the image: {0}".format (img_version))
        sys.exit (1)

    print ("Production signing version {0}".format (version))
    print ("FW: {0}".format (fw_imgs))

    # Store the unmodified 1SP image
    version_1sp, _ = get_1sp_build_version_number (img_data, fw_imgs)

    write_file_data (
        os.path.join (unsigned_1sp_dir, "dc_scm_1sp_v{0}_dev.img".format (version_1sp)),
        img_data[fw_imgs["xfer"][0]:fw_imgs["boot"][1]])
    
    # Handle 1SP components
    version_1sp, _ = update_1sp_build_version (img_data, fw_imgs)

    # Dev 1SP with prod owner transfer and key manifests.
    if fips:
        write_file_data (os.path.join (unsigned_1sp_dir, "boot.img"), OWNER_XFER_MANIFEST,
            FIPS_OWNER_KEY_MANIFEST, img_data[fw_imgs["boot"][0]:fw_imgs["boot"][1]])
    else:
        write_file_data (os.path.join (unsigned_1sp_dir, "boot.img"), OWNER_XFER_MANIFEST,
            OWNER_KEY_MANIFEST, img_data[fw_imgs["boot"][0]:fw_imgs["boot"][1]])

    # Dev 1SP with prod owner transfer manifest and dev key manifest.
    write_file_data (os.path.join (unsigned_1sp_dir, "boot-dev.img"), OWNER_XFER_MANIFEST,
        img_data[fw_imgs["root"][0]:fw_imgs["boot"][1]])

    # Dev 1SP with no transfer manifest and dev key manifest.
    write_file_data (os.path.join (unsigned_1sp_dir, "boot"),
        img_data[fw_imgs["root"][0]:fw_imgs["boot"][1]])

    # Handle Firmware Package components
    write_file_data (os.path.join (unsigned_comp_dir, "desc"),
        img_data[fw_imgs["desc"][0]:fw_imgs["desc"][1]])

    for i, comp in enumerate (fw_imgs["comp"]):
        write_file_data(os.path.join (unsigned_comp_dir, "{0}{1:02d}".format ("comp", i)),
            img_data[comp[0]:comp[1]])

    # Store the IV table
    write_file_data (os.path.join (unsigned_dir, "iv"), img_data[fw_imgs["iv"][0]:fw_imgs["iv"][1]])


########################
# Script start
########################

if __name__ == '__main__':
    """
    This script will take a complete Manticore firmware image and parse out the individual
    components for signing.  The signature for firmware components will be removed and the data
    requiring signing will be stored in an 'unsigned' directory.

    Args: <fw image path> [output path] [--fips]

    If an output path is not specified, the 'unsigned' directory will be created in the script
    directory.

    If --fips is specified then FIPS signing part is enabled
    """

    parser = argparse.ArgumentParser ()
    parser.add_argument("img_path",help="Path to the Firmware image")
    parser.add_argument("out_dir",nargs="?", default=".", help="Output directory (defaults to current directory).")
    parser.add_argument ('-f', '--fips', action='store_true', help='FIPS flag')
    
    args, _ = parser.parse_known_args(sys.argv[1:])
    img_path = args.img_path
    out_dir = args.out_dir
    img_name = os.path.basename (img_path)
        
    version = re.match ("manticore_v(((\\d{1,3})\\.(\\d{1,3})\\.\\d{1,3})\\.\\d{1,3})", img_name)
    if (not version):
        print ("Invalid filename: {0}.  Expected:  manticore_vxxx.xxx.xxx.xxx".format (img_name))
        sys.exit (1)

    major = int (version.group (3))
    if (major != 3):
        print ("Incorrect major FW image version: {0}".format (major))
        sys.exit (1)

    # Load input image to sign
    img_data = bytearray (read_file_data (img_path))

    unsigned_dir = os.path.join (out_dir, UNSIGNED_DIR)

    unsigned_comp_dir = os.path.join (out_dir, UNSIGNED_DIR, SIGNING_DIR_FW_COMPONENTS)
    prepare_signing_directory (unsigned_comp_dir)

    unsigned_1sp_dir = os.path.join (out_dir, UNSIGNED_DIR, SIGNING_DIR_1SP)
    prepare_signing_directory (unsigned_1sp_dir)

    print ("FW length: {0}".format (len (img_data)))
    run_parse_image (img_data, version.group (1), unsigned_dir, unsigned_comp_dir, unsigned_1sp_dir, args.fips)
