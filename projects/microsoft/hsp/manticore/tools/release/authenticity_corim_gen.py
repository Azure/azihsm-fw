import argparse
import json
import os
import sys
from pathlib import Path
from pyautomation.scripts.build_utils import BuildUtils

from pathlib import Path

def fetch_file(folder_path, pattern):
    """Fetch files that are matching the pattern directly in the folder. If none are found, check subdirectories.

    :param folder_path: The directory path as a string
    :param pattern: Pattern of the file to fetch (e.g., '*.json')
    :return: List of file paths matching the pattern
    """
    corim_files = []
    f_path = Path(folder_path)

    if f_path.is_dir():
        direct_files = list(f_path.glob(pattern))
        if direct_files:
            corim_files.extend(direct_files)
        else:
            sub_files = list(f_path.rglob(pattern))
            if sub_files:
                corim_files.extend(sub_files)
            else:
                print(f"Corim file is not found in '{folder_path}'")
                sys.exit(1)
    else:
        print(f"Error: {folder_path} is not a valid directory")

    return [str(file) for file in corim_files]


########################
# Script start
########################

if __name__ == '__main__' :
    """
    This script generates the aggregated corim file from the input of corim_manifest.xml

    Args: -f <corim_manifest.xml> -t<target directory for placing the downloaded artifacts> -o <output path>

    If output path is not specified script will consider current directory.

    """

    parser = argparse.ArgumentParser ()
    parser.add_argument('-f','--corim_manifest',required=True,help='Corim manifest xml')
    parser.add_argument('-t','--target_directory',required=True,help='Downloaded artifacts target dir')
    parser.add_argument ('-o', '--output_directory', required=False, help='Output directory')

    
    args = parser.parse_args ()
    corim_xml = args.corim_manifest 
    corim_json_dir= args.target_directory
    out_dir = args.output_directory or '.'

    BuildUtils.install_az_cli()
    BuildUtils.get_az_packages(corim_xml,corim_json_dir)

    corim_folder_path = fetch_file(corim_json_dir,'*.json')
    layer_map = {}

    corim_id = None
    tag_identity = None

    for idx, corim_file in enumerate(corim_folder_path):
        with open(corim_file, 'r') as f:
            data = json.load(f)

            if idx == 0:
                corim_id = data.get('corim-id')
                tag_identity = data.get('tag-identity')

            ref_values = data.get('triples', {}).get('reference-values', [])
            for entry in ref_values:
                env = entry.get("environment", {})
                layer = env.get("class", {}).get("layer")
                measurements = entry.get("measurements", [])
                if layer is not None:
                    if layer not in layer_map:
                        layer_map[layer] = {
                            "environment": env,
                            "measurements": []
                        }
                    layer_map[layer]["measurements"].extend(measurements)

    merged_data = {
        "corim-id": corim_id,
        "tag-identity": tag_identity,
        "triples": {
            "reference-values": list(layer_map.values())
        }
    }

    with open(f'{out_dir}/aggregated_corim_config.json', "w") as f:
        json.dump(merged_data, f, indent=4)




