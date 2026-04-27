import hashlib
import xml.etree.ElementTree as ET
import argparse
import base64
import json
import os
import fnmatch
from prod_image_manticore import *
from parse_image_manticore import *

def extract_data(hex_string,start_byte:int,end_byte:id,extract_type):
    """Exract the data from the measurement raw data

     :param hex_string: The hex string from which the data needs to be extracted
     :param start_byte: Start byte of the extraction
     :param end_byte: End byte of the extraction
     :param extract_type: The extracted output type, int or str

    """
    byte_data = bytes.fromhex(hex_string)

    version_bytes = byte_data[start_byte:end_byte]
    
    if(extract_type == 'string'):
        version_parts = list(version_bytes)
        value = '.'.join(map(str, reversed(version_parts)))

    elif(extract_type == 'int'):
        byte_slice = byte_data[start_byte:end_byte]
        value = int.from_bytes(byte_slice, byteorder='little')

    return value

def extend_pcr_sha384(pcr_value: bytes, measurement: bytes) -> bytes:
    """Extend the PCR value using SHA-384.

      :param pcr_value: The pcr value 
      :param measurement: measurement data for extending

       :return: The hash out of extended measurement
    """
    return hashlib.sha384(pcr_value + measurement).digest()

def fetch_file(folder_path,pattern):
    """Fetch the file based on the pattern under the parsed directory.

      :param folder_path: The directory of the file
      :param pattern: Pattern of the file to fetch

       :return: The file path
    """
    for file_name in os.listdir(folder_path):
        if fnmatch.fnmatch(file_name,pattern):
            file_path = os.path.join(folder_path,file_name)
    
    return file_path

def extract_and_extend_measurements(cfm_file_path: str, start_id: int, end_id: int,init_pcr,extend:int,mfg_unlock=False) -> bytes:
    """Calculate the L0 TCB and L1 TCB measurements
        
        :param cfm_file_path: The Path for the CFM XML file for retrieveing the measurements
        :param start_id: The measurement start ID from the CFM.xml
        :param end_id: The measurement end ID from the CFM.xml
        :param extend: Extend flag , where this is parsed as true for extending the pcr measurements and false for using the hash update method.
    """
    tree = ET.parse(cfm_file_path)
    root = tree.getroot()
    measurement_digests=[]
    
    if extend:
        pcr_value = bytes([init_pcr, 0x10] + [0x00] * 46)

    for measurement_id in range(start_id, end_id + 1):
        measurement_bytes = None

        measurement = root.find(f".//Measurement[@measurement_id='{measurement_id}']")
        if measurement is not None:
            digest_elem = measurement.find('Digest')
            if digest_elem is not None and digest_elem.text:
                measurement_bytes = bytes.fromhex(digest_elem.text.strip())

        if measurement_bytes is None:
            measurement_data = root.find(f".//MeasurementData[@measurement_id='{measurement_id}']")
            if measurement_data is not None:
                data_elem = measurement_data.find('.//AllowableData/Data')
                if data_elem is not None and data_elem.text:
                    measurement = bytes.fromhex(data_elem.text.strip())
                    measurement_bytes = hashlib.sha384(measurement).digest()

        if measurement_id == 4:
            continue

        # Extracting the SVN Value of L0 and L1 measurement id 3 is for L0 , measurement id 20 is for L1
        if measurement_id == 3 or measurement_id == 20 or measurement_id == 14: 
            data_elem = measurement_data.find('.//AllowableData/Data')
            measurement = bytes.fromhex(data_elem.text.strip())
            extracted_svn= extract_data(measurement.hex(),5,9,'int')
        
        if measurement_id == 10 and mfg_unlock:
            data_elem = measurement_data.find('.//AllowableData/Data')
            measurement_elem = bytes.fromhex(data_elem.text.strip())
            flag_offset = 9
            unlock_flag = 0x07
            tmp = bytearray(measurement_elem)
            tmp[flag_offset] |= unlock_flag
            measurement_unlocked = bytes(tmp)
            measurement_bytes = hashlib.sha384(measurement_unlocked).digest()

        if measurement_bytes is not None and extend:
            pcr_value = extend_pcr_sha384(pcr_value , measurement_bytes)
        else:
            measurement_digests.append(measurement_bytes)

    if not extend:
        sha384 = hashlib.sha384()
        for digest in measurement_digests:
            sha384.update(digest)
        tcbmeasurement_digest = sha384.digest()
        return tcbmeasurement_digest,extracted_svn
    else:
        return pcr_value,extracted_svn

def get_L0_L1_version(cfm_file_path: str,measurement_id):
    """
    Retreive the L0 and L1 Version : 1 SP version, firmware build version
        :param cfm_file_path: Manticore CFM file
        :param measurement_id: Respective measurement id of 1SP version i.e.,8 and the SOCfirmwareversion i.e., 23

    """
    tree = ET.parse(cfm_file_path)
    root = tree.getroot()
    measurement_data = root.find(f".//MeasurementData[@measurement_id='{measurement_id}']")
    data_elem = measurement_data.find('.//AllowableData/Data')
    measurement = bytes.fromhex(data_elem.text.strip())
    relevant_bytes = measurement[5:9]
    version_parts = list(reversed(relevant_bytes))
    version_string = '.'.join(str(b) for b in version_parts)
    build_bytes = measurement[9:13]
    reverse_bytes = build_bytes[::-1]
    build_time = int.from_bytes(reverse_bytes, byteorder='big')
    build_time = build_time >> 5
    full_version = f"{version_string}-{build_time}"

    return full_version

########################
# Script start
########################

if __name__ == '__main__' :
    """
    This script generates the corim file from the input cfm xml file

    Args: -xf <Manticore CFM XML Path> -jf <CORIM JSON file path> -o <output path>

    If output path is not specified script will consider current directory.

    """

    parser = argparse.ArgumentParser ()
    parser.add_argument ('-f', '--cfm_folder', required=True, help='Folder that has the CFM file')
    parser.add_argument ('-jf', '--json_file',required=True, help='CORIM JSON file')
    parser.add_argument ('-o', '--output_directory', required=False, help='Output directory')
    

    args = parser.parse_args ()
    cfm_folder_path = args.cfm_folder  
    json_file_path = args.json_file
    out_dir = args.output_directory or '.'

    cfm_file_path = fetch_file(cfm_folder_path,'CFM*.xml')

    L0_TCB,L0_svn = extract_and_extend_measurements(cfm_file_path,1,9,None,extend=False)  # For L0 TCB measurement
    base64_L0tcbdigest = base64.b64encode(bytes.fromhex(L0_TCB.hex())).decode('utf-8')

    L1_SPRT_TCB,_ = extract_and_extend_measurements(cfm_file_path,10,16,None,extend=False) # For L1_SPRT TCB measurement
    base64_L1SPRTtcbdigest = base64.b64encode(bytes.fromhex(L1_SPRT_TCB.hex())).decode('utf-8')

    L1_SPRT_TCB_MFG_UNLOCK,_ = extract_and_extend_measurements(cfm_file_path,10,16,None,extend=False,mfg_unlock=True) # For L1_SPRT MFG UNLOCK TCB measurement
    base64_L1SPRTtcbdigest_MFG_UNLOCK = base64.b64encode(bytes.fromhex(L1_SPRT_TCB_MFG_UNLOCK.hex())).decode('utf-8')

    L1_HSM_TCB,L1_svn= extract_and_extend_measurements(cfm_file_path,19,28,0x03,extend=True)  # For L1_HSM TCB measurement
    base64_L1HSMtcbdigest = base64.b64encode(bytes.fromhex(L1_HSM_TCB.hex())).decode('utf-8')

    with open(json_file_path, "r") as f:
        corim_data = json.load(f)

    try:
        corim_data["triples"]["reference-values"][0]["measurements"][0]["value"]["svn"]["value"] = L0_svn
        corim_data["triples"]["reference-values"][1]["measurements"][0]["value"]["svn"]["value"] = L1_svn
        corim_data["triples"]["reference-values"][1]["measurements"][1]["value"]["svn"]["value"] = L1_svn
        corim_data["triples"]["reference-values"][0]["measurements"][0]["value"]["version"]["value"] = get_L0_L1_version(cfm_file_path,8)
        corim_data["triples"]["reference-values"][1]["measurements"][0]["value"]["version"]["value"] = get_L0_L1_version(cfm_file_path,23)
        corim_data["triples"]["reference-values"][1]["measurements"][1]["value"]["version"]["value"] = get_L0_L1_version(cfm_file_path,23)
        corim_data["triples"]["reference-values"][0]["measurements"][0]["value"]["integrity-registers"]["FW"]["value"][0] = f"sha-384;{base64_L0tcbdigest}"
        corim_data["triples"]["reference-values"][1]["measurements"][0]["value"]["digests"][0] = f"sha-384;{base64_L1HSMtcbdigest}"
        corim_data["triples"]["reference-values"][1]["measurements"][0]["value"]["integrity-registers"]["RoT"]["value"][0] = f"sha-384;{base64_L1SPRTtcbdigest}"
        corim_data["triples"]["reference-values"][1]["measurements"][1]["value"]["digests"][0] = f"sha-384;{base64_L1HSMtcbdigest}"
        corim_data["triples"]["reference-values"][1]["measurements"][1]["value"]["integrity-registers"]["RoT"]["value"][0] = f"sha-384;{base64_L1SPRTtcbdigest_MFG_UNLOCK}"



    except (KeyError, IndexError):
        sys.exit("The expected structure was not found in corim.json.")

    with open(f'{out_dir}/corim_config.json', "w") as f:
        json.dump(corim_data, f, indent=4)
        