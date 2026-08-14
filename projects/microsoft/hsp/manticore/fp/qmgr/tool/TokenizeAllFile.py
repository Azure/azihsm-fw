# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#!/usr/bin/env python
import os, fnmatch, sys, subprocess

"""Tool to tokenize all files"""
"""Scan all C/CPP files, and execute Tokenizer for each file"""
"""Generate new (tokenized) C/CPP files and Tokens.dat"""

#-----------------------------------------------------------------------------
#
#-----------------------------------------------------------------------------
def Usage():
    print ("Usage: TokenizeAllFile")
    return

#=============================================================================
#
#=============================================================================
def tokenizeSingleFile(fileFullPath, filename):
    print ("[token] tokenizing - " + fileFullPath)

    inputFile=fileFullPath
    outputFile=fileFullPath
    tokensFile="./tokenize/" +filename + ".tcpp"
    ret = subprocess.call(["mono", "./tool/Tokenizer.exe", "-mode=tokenize",
                           "-input=" + inputFile, "-output=" + outputFile,
                           "-tokens=" + tokensFile])
    #print ("ret: " + str(ret))
    if (ret):
        sys.exit(1)

def compare_and_update_files(file1_path, file2_path):
    with open(file1_path, 'r') as file1:
        file1_lines = file1.readlines()

    with open(file2_path, 'r') as file2:
        file2_lines = file2.readlines()

    missing_lines = [line for line in file1_lines if line not in file2_lines]

    if missing_lines:
        with open(file2_path, 'a') as file2:
            file2.writelines(missing_lines)

#=============================================================================
#
#=============================================================================
def py_main():
    print ("[token] tokenize all C/CPP files!")

    ret = subprocess.call(["mono", "./tool/Tokenizer.exe", "-mode=parseTokens"])
    dirList = ["CM7", "hal"]
    filelist = []
    for dirStr in dirList:
        #print ("dirStr: " + dirStr)
        for root, dirnames, filenames in os.walk("./" + dirStr):
            for fileStr in filenames:
                if fileStr.endswith(('.c', '.cpp')):
                    #print ("skip")
                    lst = [str(os.path.join(root, fileStr)), fileStr]
                    filelist.append(lst)
    filelist.sort(key=lambda x: x[0])
    for file in filelist:
        tokenizeSingleFile(file[0], file[1])

    print ("[token] All C/CPP files tokenized.")
    #generate Tokens.dat

    ret = subprocess.call(["mono", "./tool/Tokenizer.exe", "-mode=categoryJson",
                           "-tokens=./tokenize/Tokens.dat"])
    if (ret):
        sys.exit(1)

    with open('./tokenize/Tokens.dat', 'a+') as outfile:
        tokenizedfilelist = []
        for root, dirnames, filenames in os.walk("./tokenize/"):
            for fileStr in filenames:
                if fileStr.endswith(('.tcpp')):
                    tokenizedfilelist.append(os.path.join(root, fileStr))
        tokenizedfilelist.sort()
        for file in tokenizedfilelist:
            with open(file) as infile:
                outfile.write(infile.read())

    # Add all the previously deleted logs from old Tokens.dat to newly
    # generated Tokens.dat under tokenize directory
    if os.path.exists('./tool/Tokens.dat'):
        compare_and_update_files('./tool/Tokens.dat', './tokenize/Tokens.dat')

    print ("[token] ./tokenize/Tokens.dat generated.")

#=============================================================================
#
#=============================================================================
if __name__ == "__main__":
    py_main()
