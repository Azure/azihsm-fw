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
#	CMake script to build Manticore SP images
#
# --

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(TOOLCHAIN_VERSION "-12")

include(${CMAKE_CURRENT_LIST_DIR}/../../../../../Cerberus.cmake)
include(${CERBERUS_ROOT}/projects/microsoft/hsp/OneFleetHsp.cmake)
include(${CERBERUS_ROOT}/cmake/toolchain/GccRiscV32.cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pedantic")
add_compile_definitions(UNALIGNED_16BIT_MEMORY_ACCESS_NOT_SUPPORTED)
add_compile_definitions(UNALIGNED_32BIT_MEMORY_ACCESS_NOT_SUPPORTED)
add_compile_definitions(UNALIGNED_64BIT_MEMORY_ACCESS_NOT_SUPPORTED)
add_compile_definitions(PLATFORM_RISCV)
add_compile_definitions(LITTLEENDIAN_CPU)
add_compile_definitions(CRASH_DUMP_ARM_HAS_AFSR)

set(CLANG_FORMAT clang-format${TOOLCHAIN_VERSION})

project(manticore-sp LANGUAGES C ASM)

set(CORE_DIR ${CERBERUS_ROOT}/core)
set(ARM_DIR ${CERBERUS_ROOT}/projects/arm)
set(BARE_METAL_BASE_DIR ${CERBERUS_ROOT}/projects/bare_metal)
set(BARE_METAL_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/platform/bare_metal)
set(CERBERUS_FREERTOS_DIR ${CERBERUS_ROOT}/projects/freertos)
set(GCC_DIR ${CERBERUS_ROOT}/projects/gcc)
set(RISCV_DIR ${CERBERUS_ROOT}/projects/riscv)
set(MICROSOFT_DIR ${CERBERUS_ROOT}/projects/microsoft/src)
set(HSP_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/src)
set(HSP_HW_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/platform)
set(HSP_API_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/platform/api)
set(MANTICORE_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/manticore/src)
set(MANTICORE_PLATFORM_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/manticore/platform)
set(MANTICORE_SP_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/manticore/sp/src)
set(ROM_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/manticore/rom/src)
set(DC_SCM_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/manticore/sp/src/dc_scm)

# Setup HSP SDK dependencies
set(HSP_SDK_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/platform/openhsp)
set(HSP_SDK_HSP_DIR ${HSP_SDK_DIR}/sources/hsp)
set(HSP_SDK_LINK_DIR ${HSP_SDK_HSP_DIR}/lds)
set(HSP_SDK_LIBS_DIR ${HSP_SDK_HSP_DIR}/splibs)
set(HSP_SDK_PORT_DIR ${HSP_SDK_DIR}/sources)
set(HSP_SDK_PORT_INC ${HSP_SDK_PORT_DIR}/inc/port)

set(HSP_SDK_INCLUDES ${HSP_SDK_HSP_DIR} ${HSP_SDK_PORT_DIR})

# TODO: Consider creating OneFleet versions of these files to eliminate all dependencies on Pluton
# code.
set(HSP_SDK_SOURCES
	${HSP_SDK_LIBS_DIR}/hspcore/uart.c
	${HSP_SDK_LIBS_DIR}/hsprt/hsprtmain.c
	${HSP_SDK_LIBS_DIR}/hsprt/printf.c
	${HSP_SDK_LIBS_DIR}/hsprt/stackguard.c
	${HSP_SDK_LIBS_DIR}/hsprt/startup.S
	)

include(cmake/sp.cmake)
