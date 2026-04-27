# ++
#
# Copyright (c) Microsoft Corporation. All rights reserved.
#
# Module Name:
#
#	freertos_port.cmake
#
# Abstract:
#
#	This file tailors the build for our specific application/implementation of FreeRTOS.
#
# --

set(FREERTOS_PORT_ROOT ${FREERTOS_PORTABLE_ROOT}/GCC/RISC-V)
set(FREERTOS_COMPILE_DEFS __FREERTOS__)

# Note: be sure to EXCLUDE any files in the PORT_ROOT directory that we are copying
# modifying for the port. Use our tailored versions instead.
list(APPEND FREERTOS_SOURCES "${CERBERUS_ROOT}/projects/microsoft/hsp/platform/freertos/hsp_freertos.c")
list(APPEND FREERTOS_SOURCES "${CERBERUS_ROOT}/projects/microsoft/hsp/platform/freertos/hsp_freertos.S")
list(APPEND FREERTOS_SOURCES "${FREERTOS_PORT_ROOT}/port.c")

###############
#### WARNING - preserve order, keeping APPLICATION_ROOT first
###############
# FreeRTOSConfig.h lives in both the external freertos dir as well as
# a modified copy in the application directory. We need to add both paths
# so must ensure we pick up our modified version.
list(APPEND FREERTOS_INCLUDES ${FREERTOS_CONFIG_DIR})
# Ensure the HSP portmacro.h is referenced by FreeRTOS but it includes the OSS portmacro.h as a
# base from the portable GCC directory.
list(APPEND FREERTOS_INCLUDES "${CERBERUS_ROOT}/projects/microsoft/hsp/platform/freertos")
list(APPEND FREERTOS_INCLUDES "${FREERTOS_PORTABLE_ROOT}/GCC")

# Hopefully ensure _zicsr_zifencei_zba_zbb added to -march since those don't seem to be currently supported directly
set(FREERTOS_MCU_FLAGS "-mcpu=sifive-e20")
