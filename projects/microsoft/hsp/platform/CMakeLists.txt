# ++
#
# Copyright (c) Microsoft Corporation. All rights reserved.
#
# Module Name:
#
#	CMakeLists.txt
#
# Abstract:
#
#	CMake script to build HSP SDK libraries.
#
# --

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

if(NOT TOOLCHAIN_VERSION)
	set(TOOLCHAIN_VERSION "-12")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../../../../Cerberus.cmake)
include(${CERBERUS_ROOT}/projects/microsoft/hsp/OneFleetHsp.cmake)
include(${CERBERUS_ROOT}/cmake/toolchain/ClangRiscV32.cmake)

project(hsp-sdk LANGUAGES C ASM)

set(TARGET_NAME hsp-sdk)

set(HSP_SDK_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/platform/openhsp)
set(HSP_SDK_HSP_DIR ${HSP_SDK_DIR}/sources/hsp)
set(HSP_SDK_LINK_DIR ${HSP_SDK_HSP_DIR}/lds)
set(HSP_SDK_LIBS_DIR ${HSP_SDK_HSP_DIR}/splibs)
set(HSP_SDK_PORT_DIR ${HSP_SDK_DIR}/sources)
set(HSP_SDK_PORT_INC ${HSP_SDK_PORT_DIR}/inc/port)
set(HSP_SDK_UTIL_BIN ${HSP_SDK_DIR}/build)

set(HSP_SDK_INCLUDES ${HSP_SDK_HSP_DIR} ${HSP_SDK_PORT_DIR})

include_directories(
	${HSP_SDK_INCLUDES}
	)

add_compile_options(
	-ffreestanding				# Asserts that stdlibs do not exists
	)

list(APPEND CMAKE_MODULE_PATH ${HSP_SDK_DIR}/scripts/cmake)
include(miscutils)
include(libstatic)

set(ENABLE_FEATURE_RSA_SIGNATURE 1)
set(ENABLE_FEATURE_SHACK1 1)
set(ENABLE_FEATURE_TIMER1 1)

add_subdirectory(${HSP_SDK_LIBS_DIR} splibs)
