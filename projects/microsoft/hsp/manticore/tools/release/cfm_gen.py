"""
Copyright (c) Microsoft Corporation. All rights reserved.
"""
import hashlib
import binascii
import yaml
import argparse
from prod_image_manticore import *
from parse_image_manticore import *
import xml.etree.ElementTree as ET
from xml.dom import minidom


def get_subimage_comp_count (img, region) :
	"""
	Get the Sub-image component count according to fw descriptor

	:param img: A binary array containing the firmware image
	:param region: Dictionary of the tuples of the component regions of the manticore image

	:return Dictionary of images component count
	"""
	comp_count = {}
	_, header_len, _ = parse_firmware_descriptor (img, region['desc'][0])
	desc_start = region['desc'][0] + header_len

	comp_count['SPRTImage'] = img[desc_start + 43]
	comp_count['CPImage'] = img[desc_start + 44]
	comp_count['FP0Image'] = img[desc_start + 45]
	comp_count['FP1Image'] = img[desc_start + 46]
	comp_count['FP2Image'] = img[desc_start + 47]
	comp_count['PHYImage'] = img[desc_start + 48]

	return comp_count

def get_subimage_hash (name, event_id, version, img, region) :
	"""
	Get the hash(s) of sub-image(s) of the binary file

	:param name: Name of the subimage to be parsed
	:param event_id: event id in the event header of the measurement
	:param version: version in the event header of the measurement
	:param img: A binary array containing the firmware image
	:param region: Dictionary of the tuples of the component regions of the manticore image

	:return Concat of the hash (sha384) of the subimages
	"""
	# Get the count of image components from fw header
	subimage_comp_count = get_subimage_comp_count (img, region)

	list_keys = list (subimage_comp_count.keys ())
	list_values = list (subimage_comp_count.values ())
	# Start and End index of the ['comp'] list from the parsed manticore fw image
	image_index_start = sum (list_values[:list_keys.index (name)])
	image_index_end = sum (list_values[:list_keys.index (name) + 1])



	# Changing the endianess of the event id and adding version to the event header
	event_header = bytearray.fromhex (event_id)[::-1].hex () + version
	# Updating the header from  string to hex and then converting to the bytes
	event_header = binascii.unhexlify(event_header.encode ('utf-8'))

	concat_hash = b''

	for i in range(image_index_start , image_index_end):
		start = region['comp'][i][0]
		end = region['comp'][i][1]
		concat_hash += hashlib.sha384 (bytes (img[start : end])).digest ()

	# Hashing the concat of image components
	content_hash = hashlib.sha384 (concat_hash).digest ()
	# Adding header which is (Event id + Version)
	content_hash_with_header = event_header + content_hash
	final_hash = hashlib.sha384 (content_hash_with_header).hexdigest ()

	return final_hash

def get_offset (region,img,entry):
	"""
	Get the start and end address for entries in the yaml

	:param region: Dictionary of the tuples of the component regions of the manticore image
	:param img: A binary array containing the firmware image
	:param entry: Dictionary entry from the yaml 'Measurement[]' which is list of dictionary

	:return start and end address
	"""

	comp = entry['img_comp']
	offset = entry['offset']
	length = entry['length']

	mv = memoryview (img)

	if (comp == 'keys' or comp == 'root' or comp == 'boot') :
		start = region[comp][0] + offset
		end = start + length
	elif (comp == 'desc') :
		_, header_len, _ = parse_firmware_descriptor (mv, region['desc'][0])
		start = region[comp][0] + header_len + offset
		end = start + length
	return start, end


def get_dataSHA (region,img,entry):
	"""
	Get the SHA384 for entries in the yaml

	:param region: Dictionary of the tuples of the component regions of the manticore image
	:param img: A binary array containing the firmware image
	:param entry: Dictionary entry from the yaml 'Measurement[]' which is list of dictionary

	:return SHA384 digest of the entry element
	"""

	comp = entry['img_comp']
	event_id = entry['event_id']
	version =  entry['version']
	digest = ""
	mv = memoryview (img)

	if (comp == 'keys' or comp == 'root' or comp == 'boot' or comp == 'desc') :
		start, end = get_offset (region, img, entry)
		event_header = bytearray.fromhex (event_id)[::-1].hex () + version
		# Updating the header from  string to hex
		event_header = event_header.encode ('utf-8')
		# Adding header to the parsed data
		data_with_header = binascii.unhexlify (event_header) + bytes (mv[start:end])
		digest = hashlib.sha384 (data_with_header).hexdigest ()
	elif (comp == 'image') :
		digest= get_subimage_hash (entry['name'],event_id,version,mv,region)

	return digest

def dict_to_xml (element, data, exclude_keys=None) :
	"""
	Converts Dictionary to xml tree

	:param element: Root element of the xml tree
	:param data: Dictionary to be converted into xml
	:param exclude_keys: List of the Dictionary keys to exclude in the xml
	"""

	exclude_keys = exclude_keys or []
	for key, value in data.items () :
		if (key in exclude_keys) :
			continue
		if (isinstance (value, dict)) :
			sub_element = ET.SubElement (element, key)
			dict_to_xml (sub_element, value, exclude_keys)
		elif (key.startswith ('@')) :
			element.set (key[1:], str (value))
		elif (isinstance (value, list)) :
			for item in value:
				if ('AllowableData' in item) :
					sub_element = ET.SubElement (element, 'MeasurementData')
				else:
					sub_element = ET.SubElement (element, key)

				dict_to_xml (sub_element, item, exclude_keys)
		else :
			sub_element = ET.SubElement (element, key)
			sub_element.text = str (value)

########################
# Script start
########################

if __name__ == '__main__' :
	"""
	This script generates the cfm.xml from the input yaml file

	Args: -bf <Manticore image path> -yf <YAML file path> -o <output path>

	If output path is not specified script will consider current directory.

	"""

	parser = argparse.ArgumentParser ()
	parser.add_argument ('-bf', '--bin_file', required=True, help='Manticore firmware binary file')
	parser.add_argument ('-yf', '--yaml_file', required=True, help='CFM config YAML file')
	parser.add_argument ('-o', '--output_directory', required=False, help='Output directory')
	args = parser.parse_args ()

	binary_file_path = args.bin_file
	yaml_file_path = args.yaml_file
	out_dir = args.output_directory or '.'


	# Load manticore image
	with open (binary_file_path, 'rb') as binary_file:
		img_data = memoryview (binary_file.read ())

	# Parsing the manticore image
	fw = {}
	fw = parse_image (img_data)

	# Parse and get firmware build version
	_, desc_hdr_len, _ = parse_firmware_descriptor (img_data, fw['desc'][0])
	fw_version_start = fw['desc'][0] + desc_hdr_len + 23
	manticore_version, build_ext, _ = parse_build_version_number (img_data, fw_version_start)

	# Load yaml
	with open (yaml_file_path, 'r') as file :
		xml_data = yaml.safe_load (file)

	# Populate sha384
	for entry in xml_data['CFMComponent']['Measurement'] :
		if 'AllowableData' in entry:
			if entry['AllowableData']['parse'] == True :
				start,end = get_offset(fw, img_data, entry)
				raw_data = img_data[start:end]
				# Adding event id after changing the endianess
				data = bytearray.fromhex(entry['event_id'])[::-1].hex()
				# Adding version
				data += (entry['version'])
				# Adding the raw data of the measurement
				data += binascii.hexlify(bytes(raw_data)).decode('utf-8')
				entry['AllowableData']['Data'] = data
		elif 'Digest' not in entry:
			entry['Digest'] = get_dataSHA (fw, img_data, entry)

	# Create the root element
	root_element_name = list (xml_data.keys ())[0]
	root = ET.Element (root_element_name)

	# Keys/attribute to exclude in the xml
	exclude_keys = ['img_comp','offset','length','event_id','version','name','path', 'parse']

	# Convert dictionary to XML
	xml_data = xml_data.pop (root_element_name, None)
	dict_to_xml (root, xml_data,exclude_keys)
	tree = ET.ElementTree (root)

	# Save the XML file
	with open (f'{out_dir}/CFM.xml', "w") as xml_file :
		pretty_xml_string = minidom.parseString (ET.tostring (root)).toprettyxml ()
		# Removing the xml declaration and extra white spaces
		xml_file.write (pretty_xml_string.replace ('<?xml version="1.0" ?>', '').strip ())
