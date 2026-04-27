"""
Copyright (c) Microsoft Corporation. All rights reserved.
"""
import re
import os
import argparse
import logging

# Regex pattern constants for C
C_MACRO_REGEX = r'#define\s+(\w+)\s*\('
C_ENUM_ELEMENT_REGEX = r'\s*([^=]+)\s*=\s*([^,]+)(?:,\s*)?\/\*\*\<(.*?)\*\/'

# Regex pattern constants for Rust
RUST_MCR_ENUM_REGEX = r'\senum\sMcrComponent\s\{([^}]*)\}'
RUST_ENUM_ELEMENT_REGEX = r'/{2,3}(.*?)\n\s*(\w+)\s*=\s*(0x[0-9a-fA-F]+|\d+),?'

# Generic Regex pattern constant
HEX_STR_REGEX = r'0x[0-9a-fA-F]+'

def c_get_rot_error_macro_regex (module_name):
    """
    Returns the rot error macro regex with the updated module name

    :param module_name (str): Module name

    :return (str): Regex string with the updated module name
    """
    return r'ROT_ERROR \(' + module_name + r', code\)'

def rust_get_mcr_err_macro_regex (module_name):
    """
    Returns the mcr error macro regex with the updated module name

    :param module_name (str): Module name

    :return (str): Regex string with the updated module name
    """
    return r'mcr_err_decl!\s*{\s*' + module_name + r'\s*,\s*' \
              + module_name + r'Err' + r'\s*{\s*([^}]*)}'

def c_get_macro_name(file_path, line_number):
    """
    Extracts macro name from a given line in a file.

    :param file_path (str): Path to the file.
    :param line_number (int): Line number in the file.

    :return (str or None): Macro name if found, else None.
    """

    with open(file_path, 'r') as file:
        lines = file.readlines()
        match = re.match(C_MACRO_REGEX, lines[line_number - 1])
        return match.group(1) if match else None

def c_calculate_error_code(module_id, code=None):
    """
    Calculates the error code for C files.

    :param module_id (str): Module ID.
    :param code (str): Error code (optional).

    :return (str) : Calculated error code.
    """

    module_id = int(module_id, 16 if module_id.startswith("0x") else 10)
    if code is not None:
        code = int(code, 16 if code.startswith("0x") else 10)
        return hex((0x7f000000) | module_id << 8 | code)
    else:
        masked_value = hex((0x7f000000) | (module_id << 8))
        return masked_value[:-2] + 'XX'

def rust_calculate_error_code(module_id, code=None):
    """
    Calculates the error code for Rust files.

    :param module_id (str): Module ID.
    :param code (str): Error code (optional).

    :return (str): Calculated error code.
    """

    module_id = int(module_id, 16 if module_id.startswith("0x") else 10)
    if code is None:
        masked_value = hex(module_id << 16)[:-4] + 'XXXX'
        return '0x' + masked_value[2:].zfill(8)
    else:
        code = int(code, 16 if code.startswith("0x") else 10)
        return '0x' + hex((module_id << 16) | code)[2:].zfill(8)

def c_parse_enum(file_path, start=1):
    """
    Parses enum from C files.

    :param file_path (str): Path to the file.
    :param start (int): Starting line number.

    :return (list): List of tuples containing enum information.
    """

    enum_info = []
    enum_started = False
    line_index=0

    with open(file_path, 'r') as file:
        lines = file.readlines()[start - 1:]

    while line_index < len(lines):
        line = lines[line_index]
        if not enum_started:
            if line.strip() == "enum {":
                enum_started = True
        else:
            if line.strip() == "};":
                break
            if not (line.strip().startswith(("//", "/*", "*")) or line.strip() == ""):
                try:
                    match = re.match(C_ENUM_ELEMENT_REGEX, line)
                    if match is None :
                        line+= lines[line_index + 1]
                        match = re.match(C_ENUM_ELEMENT_REGEX, line)
                        line_index+=1
                    module_name = match.group(1).strip()
                    value = re.findall(HEX_STR_REGEX, match.group(2).strip())[0]
                    comment = match.group(3).strip()
                    enum_info.append((module_name, value, comment))
                except:
                    logging.warning(f'file {file_path} at line number : '
                                    f'{start + line_index} is not properly formatted')
        line_index+=1

    return enum_info

def rust_parse_enum(file_path, enum_pattern):
    """
    Parses enum from Rust files.

    :params file_path (str): Path to the file.
    :params enum_pattern (str): Pattern to match enum.

    :return (list): List of tuples containing enum information.
    """

    enum_info = []
    with open(file_path, 'r') as file:
        content = file.read()
    enum_match = re.search(enum_pattern, content, re.DOTALL)
    if enum_match:
        enum_content = enum_match.group(1)
        enum_info = re.findall(RUST_ENUM_ELEMENT_REGEX, enum_content)
        enum_info = [(name, value, comment) for comment, name, value in enum_info]

        return enum_info
    else:
        logging.warning(f'Code is not properly formatted in {file_path}')


def search_text_in_files(directory, text, file_ext):
    """
    Searches for a specific text in files with a given file extension.

    directory (str): Directory to search in.
    text (str): Text to search for.
    file_ext (str): File extension to filter files.

    :return (tuple or None): Tuple of (file_path, line_number), or None.
    """

    for root, _, files in os.walk(directory):
        for file_name in files:
            if file_name.endswith(file_ext):
                file_path = os.path.join(root, file_name)
                try:
                    with open(file_path, 'r', encoding='ISO-8859-1') as file:
                        content = file.read()
                        match = re.search(text, content, flags=re.MULTILINE | re.DOTALL)
                        if match:
                            line_number = content.count('\n', 0, match.start()) + 1
                            return file_path, line_number
                except OSError:
                    return None

def fetch_error_codes(root_dir, module_enum, file_extension=".h"):
    """
    Fetches error codes from C or Rust files.

    :param root_dir (str): Root directory to search for files.
    :param module_enum (list): List of tuples containing module information.
    :param file_extension (str): File extension to search for (default is ".h").

    :return (dict) : Dictionary containing error codes.
    """

    error_dict = {}
    for module_name, module_id, module_comment in module_enum:
        # Search for error codes in the specified file extension
        if file_extension == ".h":
            pattern = c_get_rot_error_macro_regex(module_name)
        elif file_extension == ".rs":
            pattern = rust_get_mcr_err_macro_regex(module_name)
        ret = search_text_in_files(root_dir, pattern, file_extension)

        # Initialize error_dict entry for the module
        error_dict.setdefault(module_name, {
            'description': module_comment,
            'module_id': module_id,
            'data': []
        })

        # Process the retrieved error codes
        if ret is not None:
            print(f'{module_name} in {ret[0]}' + ' '*100 + '\r', end='')
            file_path, line = ret
            if file_path.endswith('.h'):
                error_codes_enum = c_parse_enum(file_path, line)
            else:
                error_codes_enum = rust_parse_enum(file_path, pattern)
            if error_codes_enum:
                for error_name, error_code, comment in error_codes_enum:
                    if file_extension == ".h":
                        calculated_error_code = c_calculate_error_code(module_id, error_code)
                    else:
                        calculated_error_code = rust_calculate_error_code(module_id, error_code)
                    error_dict[module_name]["data"].append({
                        'error_code': calculated_error_code,
                        'error_name': error_name,
                        'description': comment
                    })
            else:
                # If no error codes found, add a placeholder entry
                error_dict[module_name]['data'].append({
                    'error_code': c_calculate_error_code(module_id) if file_extension == ".h" \
                                else rust_calculate_error_code(module_id),
                    'error_name': c_get_macro_name(file_path, line) if file_extension == ".h" \
                                else module_name + 'Err',
                    'description': 'The actual error codes will vary based on the platform, '
                                   'but the error will be reported with the same module ID.'
                })
        else:
            # If no file found, add a placeholder entry
            error_dict[module_name]['data'].append({
                'error_code': c_calculate_error_code(module_id) if file_extension == ".h" \
                    else rust_calculate_error_code(module_id),
                'error_name': "" if file_extension == ".h" else module_name + 'Err',
                'description': 'The actual error codes will vary based on the platform, '
                               'but the error will be reported with the same module ID.'
            })
    return error_dict

def create_md(error_dict, output_dir):
    """
    Creates a markdown file containing error code information.

    :param error_dict (dict): Dictionary containing error code information.
    :param output_dir (str): Output directory.
    """

    with open(f'{output_dir}/error_codes.md', 'w') as f:
        f.write(f'\n## Modules that can generate error\n')
        f.write("| Module Name | Module ID |    Description   |\n")
        f.write("|-------------|-------------|------------|\n")
        for key in error_dict:
            f.write(f'| {key.upper()}<a id="{key}"></a> | [{error_dict[key]["module_id"]}]'
                    f'(#{key}-table) |  {error_dict[key]["description"]}  |\n')

        for key in error_dict:
            f.write(f'\n<a id="{key}-table"></a>')
            f.write(f'\n## Error codes for module [{key}](#{key})\n')
            f.write("| Error Code | Error Name | Description |\n")
            f.write("|------------|------------|-------------|\n")
            for error in error_dict[key]["data"]:
                f.write(f"| {error['error_code']} | {error['error_name']} "
                        f"| {error['description']} |\n")

    with open(f'{output_dir}/conf.debuglogread', 'w') as f:
        header = """
## Debuglogread colorization and error code replacement configuration file

## To use:
# 1.  install `grc` then add the following to /etc/grc.conf:
# debuglogread
# \\b\w+\\b.*debuglogread\\b
# conf.debuglogread
#
# 2.  copy this file to /etc/grc.conf.d/ or ~/.grc/
#
# 3.  run `grc cerberus_utility debuglogread` to colorize the log file
##

## GRC configuration for debuglogread
regexp=^[0-9 ]+\|[0-9:\. ]*\|
colours=bright_black
=======
regexp=.*reset:(.*)
colours=underline
=======
regexp=Info
colours=green
=======
regexp=Warning
colours=yellow
=======
regexp=Error
colours=red
=======
regexp=0x7f[0-9a-fA-F]+
colours=bold red
=======
regexp=(Firmware version:) (.*)
colours=blue,white
=======
regexp=Mcr_CP[01]
colours=blue
=======
regexp=Mcr_FP[012]
colours=green
=======
regexp=Mcr_SP
colours=magenta
=======
regexp=Cerberus command completed successfully.
colours=green
=======
regexp=Cerberus command failed.
colours=red
=======
regexp=(Version:) (.*)
colours=blue,yellow
=======
regexp=Failed
colours=red
=======
regexp=Failure
colours=red
=======
## Error code replacement
        """
        f.write(header)
        for key in error_dict:
            f.write(f'\n## Error codes for module [{key}](#{key})\n\n')
            for error in error_dict[key]["data"]:
                if "XX" not in error['error_name']:
                    f.write(f"regexp={error['error_code']}\n"
                            f"replace={error['error_code']} "
                            f"({error['error_name']}) {error['description']}\n"
                            f"=======\n")

if __name__ == '__main__':
    """"
    This script generates the markdown file from given input files (.rs and .h).

    Args: -f <path(s) of the file(s) containing module ids for errors> -t <target directory to hunt error codes in > -o <output path>

    If output path is not specified script will consider current directory.

    """


    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--files', nargs='+', required=True, help='Input files')
    parser.add_argument('-t', '--target', required=True, help='Target directory to '
                                                              'fetch error codes from')
    parser.add_argument('-o', '--output', default='.', help='Output directory')
    args = parser.parse_args()

    input_files = args.files
    out_dir = args.output
    search_dir = args.target

    error_data = {}
    for file in input_files:
        _, extension = os.path.splitext(file)
        if extension == '.h':
            enum_data = c_parse_enum(file)
            temp_error_data = fetch_error_codes(search_dir, enum_data)
            error_data.update(temp_error_data)
        elif extension == '.rs':
            parsed_module = rust_parse_enum(file, RUST_MCR_ENUM_REGEX)
            temp_error_data = fetch_error_codes(search_dir, parsed_module, '.rs')
            error_data.update(temp_error_data)
        print(f'{file} DONE' + ' '*100)

    create_md(error_data, out_dir)
