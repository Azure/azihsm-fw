# ++
#
# Copyright (c) Microsoft Corporation. All rights reserved.
#
# Module Name:
#
#	sp.cmake
#
# Abstract:
#
#	CMake script to build Manticore SP firmware images.
#
# --

# Do not build the FPGA ROM since that needs to built with clang.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dc_scm_1sp)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dc_scm_sprt)
