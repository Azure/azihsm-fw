#! /bin/env python3

# Copyright (c) Microsoft Corporation. All rights reserved.

import sys
import re
import argparse

parser = argparse.ArgumentParser (description='Script to generate hsp_top.h')
parser.add_argument ('in_hsp_top', type=str,
	help='the path to the hsp_top.h that will be converted')
parser.add_argument ('out_hsp_top', type=str,
	help='the path to the converted hsp_top.h')
parser.add_argument ("--pioneer", action='store_true',
	help="set if converting an hsp_top.h for Pioneer")
parser.add_argument ("--athena", action='store_true',
	help="set if converting an hsp_top.h for Athena")
parser.add_argument ( "--kingsgate", action="store_true",
	help="set if converting an hsp_top.h for Kingsgate")
parser.add_argument( "--braga", action="store_true", 
	help="set if converting an hsp_top.h for Braga")
args = parser.parse_args ()

out = []
type_pos = -1
hsp_addr_map = False
skip_lines = 0
reg_group = None
pioneer = args.pioneer
athena = args.athena
braga = args.braga
kingsgate = args.kingsgate

kingsgate_braga_replacement_table = {
	"IC_DATA_CMD_STOP": "IC_DATA_CMD_RSVD_STOP",
	"Creg_regs_DW_apb_i2c_APB_Slave i2c": "Creg_regs_DW_apb_i2c_APB_Slave i2c0",
	"HSP_ADDR_MAP_CREG_SPI_SPI": "HSP_ADDR_MAP_CREG_SPI_SPI0",
	"HSP_ADDR_MAP_CREG_I2C": "HSP_ADDR_MAP_CREG_I2C0",
	"DWC_ssi_AHB_Slave spi": "DWC_ssi_AHB_Slave spi0",
	"Ssi_regs creg_ssi_group": "Ssi_regs creg_ssi_group0",
	"HSP_ADDR_MAP_CREG_SPI_CREG_SSI_GROUP": "HSP_ADDR_MAP_CREG_SPI_CREG_SSI_GROUP0",
	"UPKA": "PKA",
	"Upka": "Pka",
	"upka": "pka",
}

kingsgate_braga_define_table = {
	"RNG_REGS_CTRL": "RNG_REGS_CTRL_CLK_DIV_RESET 0xC0u",
	"HSP_ADDR_MAP_PKA_ADDRESS" : "HSP_ADDR_MAP_UPKA_ADDRESS HSP_ADDR_MAP_PKA_ADDRESS",						
}

with open (args.in_hsp_top, "r") as input:
	out_temp = input.read ().splitlines ()

if athena or pioneer:
	Creg_regs_DW_apb_i2c_APB_Slave_DW_apb_i2c_addr_block1_exist = False
	Creg_regs_DW_apb_i2c_APB_Slave_exist = False

	# check to see if Creg_regs_DW_apb_i2c_APB_Slave_DW_apb_i2c_addr_block1
	# or Creg_regs_DW_apb_i2c_APB_Slave structure exist
	for line in out_temp:
		if line.__contains__ ("Creg_regs_DW_apb_i2c_APB_Slave_DW_apb_i2c_addr_block1"):
			Creg_regs_DW_apb_i2c_APB_Slave_DW_apb_i2c_addr_block1_exist = True
		elif line.__contains__ ("Creg_regs_DW_apb_i2c_APB_Slave"):
			Creg_regs_DW_apb_i2c_APB_Slave_exist = True

	# if Creg_regs_DW_apb_i2c_APB_Slave_DW_apb_i2c_addr_block1 doesn't exist
	# and Creg_regs_DW_apb_i2c_APB_Slave does, change Creg_regs_DW_apb_i2c_APB_Slave
	# to Creg_regs_DW_apb_i2c_APB_Slave_DW_apb_i2c_addr_block1 and create a new
	# Creg_regs_DW_apb_i2c_APB_Slave that has the correct hierarchy
	if ((Creg_regs_DW_apb_i2c_APB_Slave_DW_apb_i2c_addr_block1_exist == False) and 
		(Creg_regs_DW_apb_i2c_APB_Slave_exist == True)):
		count = 0
		end_if_index = 0
		for line in out_temp:
			if line.__contains__ ("Creg_regs_DW_apb_i2c_APB_Slave"):
				line = line.replace ("Creg_regs_DW_apb_i2c_APB_Slave",
					"Creg_regs_DW_apb_i2c_APB_Slave_DW_apb_i2c_addr_block1")
				out_temp[count] = line
			elif line.__contains__ ("#endif"):
				end_if_index = count
				break
			count = count + 1

		out_temp.insert (end_if_index,
			"} Creg_regs_DW_apb_i2c_APB_Slave, *PCreg_regs_DW_apb_i2c_APB_Slave;\n")
		out_temp.insert (end_if_index,
			"\tCreg_regs_DW_apb_i2c_APB_Slave_DW_apb_i2c_addr_block1 DW_apb_i2c_addr_block1;")
		out_temp.insert (end_if_index,
			"typedef struct {\n")

if braga or kingsgate:
	count = 0
	define_index = {}
	for line in out_temp:
		for key, value in kingsgate_braga_replacement_table.items ():
			if key in line:
				line = line.replace (key, value)
				out_temp[count] = line

		for key, value in kingsgate_braga_define_table.items ():
			if key in line and key not in define_index.keys ():
				define_index[key] = count

		count = count + 1

	count = 0
	for key, value in define_index.items ():
		# Assuming the index will shift by 1 every interations
		out_temp.insert (value + count, f"#define {kingsgate_braga_define_table[key]}")
		count += 1

for line in out_temp:
	if skip_lines > 0:
		# Skip unnecessary lines based on a previously parsed line.
		skip_lines = skip_lines - 1
		continue

	if reg_group:
		# Continue processing an array of registers from a previously parsed line.
		reg_array = re.match (
			"\s+volatile uint32_t {0}_{1};.*".format (reg_group["name"], reg_group["count"]), line)
		if reg_array:
			# This is the same group as before, increment the total count.
			reg_group["count"] = reg_group["count"] + 1
			continue
		else:
			# We've reached the end of the group, so output the final result.  Then continue to
			# process the current line.
			if reg_group["count"] > 1:
				out.append (reg_group["prefix"] + "[" + str (reg_group["count"]) + "]" +
					reg_group["postfix"] + "\n")
			else:
				# It's not really a group, since there is only one entry of this type
				out.append (reg_group["prefix"] + "_0" + reg_group["postfix"] + "\n")
			reg_group = None

	# Make sure there are no anonymous typedefs for structs.
	if line.startswith ("typedef struct"):
		type_pos = len (out)
		out.append (line)
	elif (type_pos > 0) and line.startswith ("}"):
		type_name = re.match ("} (\S+),", line)
		if not type_name:
			print ("Could not determine type name on line: {0}".format (line))
			sys.exit (1)

		out[type_pos] = "typedef struct __attribute__((aligned(4))) {0} {{\n".format (type_name.group (1))
		type_pos = -1

		out.append (line + "\n")
		if hsp_addr_map:
			out.append ("#endif\n")
			hsp_addr_map = False

	# Make sure all registers are defined as volatile.
	elif line.startswith ("   uint32_t"):
		out.append ("   volatile " + line[3:] + "\n")

	# Collapse multi-word registers into a single array.
	elif (line.find ("volatile uint32_t") != -1) and (line.find ("_0") != -1):
		reg_array = re.match ("(\s+volatile uint32_t \S+)_0_(\d+)(;.*)", line)
		if reg_array:
			out.append (reg_array.group (1) + "[" + reg_array.group (2) + "]" +
				reg_array.group (3) + "\n")
			skip_lines = int (reg_array.group (2)) - 1
		else:
			reg_array = re.match ("(\s+volatile uint32_t (\S+))_0(;.*)", line)
			if reg_array:
				reg_group = {
					"prefix" : reg_array.group (1),
					"postfix" : reg_array.group(3),
					"name" : reg_array.group(2),
					"count" : 1
				}
			else:
				# This is not an array that we need to process.
				out.append (line + "\n")

	# Adjust include guard naming
	elif line.startswith ("#ifndef _HSP_TOP_H_"):
		out.append ("#ifndef HSP_TOP_H_\n")

	# Make sure stdint.h is included.
	elif line.startswith ("#define _HSP_TOP_H_"):
		out.append ("#define HSP_TOP_H_\n")
		out.append ("\n")
		out.append ("#include <stdint.h>\n")

	# Remove the entire HSP memory map definition when using GCC.
	elif line.find ("Typedef for Addressmap: hsp_addr_map") != -1:
		out.append ("#ifdef __clang__\n")
		out.append (line + "\n")
		hsp_addr_map = True

	# All other lines that need no special processing.
	else:
		out.append (line + "\n")

out[-1] = "#endif /* HSP_TOP_H_ */\n"

with open (args.out_hsp_top, "w") as output:
	output.writelines (out)
