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
#	CMake script to build a test ROM for running SP firmware on FPGA platforms.
#
#	TODO:  This should be built with clang, along with the HSP SDK, just like the Manticore ROM
#	build.  However, there is no need to make this change now since this build no longer serves any
#	purpose.
# --

project(sp-test-rom LANGUAGES C ASM)

set(TARGET_IMG fpga_rom)
set(TARGET_NAME ${TARGET_IMG}.elf)

set(PLATFORM_DIR
	${BARE_METAL_BASE_DIR}
	${BARE_METAL_DIR})

add_executable(
	${TARGET_NAME}
	${HSP_HW_DIR}/drivers/checkpoint.c
	${HSP_HW_DIR}/rom/rom_chkpt_unsecure.c
	${HSP_HW_DIR}/testing/psi_fpga/rom_jump_iram.c
	)

target_compile_options(
	${TARGET_NAME}
	PRIVATE
		-ffreestanding
)

target_include_directories(
	${TARGET_NAME}
	PRIVATE
		${CORE_DIR}
		${PLATFORM_DIR}
		${MICROSOFT_DIR}
		${HSP_DIR}
		${HSP_HW_DIR}
		${HSP_API_DIR}
		${HSP_SDK_INCLUDES}
		${MANTICORE_DIR}
		${MANTICORE_SP_DIR}
		${MANTICORE_SP_DIR}/testing
	)

target_compile_definitions(
	${TARGET_NAME}
	PRIVATE
		BUILD_FOR_FPGA
	)

target_link_libraries(
	${TARGET_NAME}
	PRIVATE
		hspcore
		hsprt
	)

target_link_options(
	${TARGET_NAME}
	PRIVATE
		-nostdlib
		-nostartfiles
		-L${HSP_SDK_LIB_BIN}/splibs/hspcore
		-L${HSP_SDK_LIB_BIN}/splibs/hsprt
		-Wl,--script=${CMAKE_CURRENT_LIST_DIR}/${TARGET_IMG}.ld
		-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${TARGET_IMG}.map
	)

add_custom_command(
	TARGET
		${TARGET_NAME}
	POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET_NAME} ${TARGET_IMG}.bin
	BYPRODUCTS
		${TARGET_IMG}.bin
	)

add_custom_command(
	TARGET
		${TARGET_NAME}
	POST_BUILD
		COMMAND ${CMAKE_OBJDUMP} -xdhlSC ${TARGET_NAME} -Mno-aliases > ${TARGET_IMG}.dis
	BYPRODUCTS
		${TARGET_IMG}.dis
	)

add_custom_command(
	TARGET
		${TARGET_NAME}
	POST_BUILD
		COMMAND /usr/bin/objcopy -O verilog --set-section-flags .bss=alloc,load,contents ${TARGET_NAME} ${TARGET_IMG}In.vhx
	BYPRODUCTS
		${TARGET_IMG}In.vhx
	)

add_custom_command(
	TARGET
		${TARGET_NAME}
	POST_BUILD
		COMMAND "${HSP_SDK_UTIL_BIN}/eccgen/eccgen" "-t" "20000000" "-m" "64" "-b" "32" "-a" "16"
		"-f" "1" "-s" "-p" "-g" "-o" "${TARGET_IMG}.vhx" "${TARGET_IMG}In.vhx"
	DEPENDS
		${TARGET_IMG}In.vhx
	BYPRODUCTS
		${TARGET_IMG}.vhx
	)
