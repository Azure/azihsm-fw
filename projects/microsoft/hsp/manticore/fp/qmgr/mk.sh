#!/bin/bash

# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

# Redirect output of make_manticore.sh to a log file
build_log="build_output.txt"

DIR="manticore_build/"
arg1="$1"
upper_arg1=$(echo "$arg1" | tr '[:lower:]' '[:upper:]')

if [ "$upper_arg1" == "MANTICORE" ]; then
    #Delete old _out directory
    sudo rm -rf _out
    if [ -d "$DIR" ]; then
    arg2="$2"
    lower_arg2=$(echo "$arg2" | tr '[:upper:]' '[:lower:]')
    ./build_manticore.sh $lower_arg2 2>&1 | tee $build_log
    else
     echo "Directory $DIR does not exist."
    fi
    #Copy build log to out directory
    cp -r build_output.txt _out/

    # Extract firmware version from build_output.txt and save to a text file
    touch _out/fw_version.txt
    grep -oP 'Manticore FW v\d+\.\d+\.\d+\.\d+-\d+' $build_log | sudo tee _out/fw_version.txt
    #Remove core folder for optimization
    rm -rf _out/core/
else

if [ "$upper_arg1" == "KWINJECT" ]; then
    sudo rm -fr _out
else
    #Delete old _out directory
    rm -rf _out
fi

#Create a new _out directory
mkdir -p _out

#Copy all the required files to _out directory for compilation
cp -r CM7 hal tool Makefile makefile.conf make.sh ldscripts _out
if [ -f tool/Tokens.dat ]; then
    cp tool/Tokens.dat _out/
fi
touch _out/tool/ParsedTokens.dat

#Enter _out directory
cd _out

#Give necessary permissions for execution
chmod +x make.sh

#Compile with input options [e.g. NORMAL etc]
./make.sh $upper_arg1

#Remove unwanted files and directories from _out directory
rm -rf Makefile makefile.conf make.sh tool ldscripts Tokens.dat
cp tokenize/Tokens.dat .

#Go back to parent directory
cd ..

fi
