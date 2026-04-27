# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#!/usr/bin/env python
import getopt, sys, os, fnmatch

"""Tool to create logIndex.dat"""
"""logIndex.dat is used for increasing Index for the same logLevel and LogCategoty during tokenization"""
"""This python will be executed once when tokenization starts, to initialize all Index to 0 (index starts from 1)"""
"""Then logIndex.dat will be modified by DebugLog.cs everytime when it meets a token function"""

LogLevel_t = ["cLogError", "cLogWarning", "cLogInfo"]
LogCategory_t = \
[\
    "cLogCPU0Common",\
    "cLogCPU1Common",\
    "cLogCPU2Common"\
]

#-----------------------------------------------------------------------------
#
#-----------------------------------------------------------------------------
def Usage():
    print ("Usage: CreateLogIndexFile")
    return

#=============================================================================
#
#=============================================================================
def py_main():
    print ("[token] create tool/logIndex.dat!")
    f= open("tool/logIndex.dat","w+")

    for category in LogCategory_t:
        f.write("logIndex[" + category + "]=0;\n")

    f.close()

#=============================================================================
#
#=============================================================================
if __name__ == "__main__":
    py_main()
