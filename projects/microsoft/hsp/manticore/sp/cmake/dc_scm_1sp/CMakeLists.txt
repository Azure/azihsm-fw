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
#	CMake script to build Manticore 1SP
#
# --

project(dc-scm-1sp LANGUAGES C ASM)

set(TARGET_IMG dc_scm_1sp)
set(TARGET_NAME ${TARGET_IMG}.elf)

set(1SP_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/manticore/sp/src/dc_scm/1sp)

if(NOT HW_BUILD_TARGET)
	set(HW_BUILD_TARGET BUILD_FOR_PRODUCTION)
endif()

list(FIND MANTICORE_TEST_FEATURES "MANTICORE_ENABLE_FIPS_CMVP_TESTING" cmvp)
if(${cmvp} GREATER -1)
	set(FIPS_SOURCES
		${CORE_DIR}/crypto/ecdh.c
		${MANTICORE_SP_DIR}/drivers/fuse_controller_manticore_fips_cmvp.c
	)
endif()

add_executable(
	${TARGET_NAME}
	${FIPS_SOURCES}
	${CORE_DIR}/asn1/asn1_oid.c
	${CORE_DIR}/asn1/asn1_util.c
	${CORE_DIR}/asn1/base64_core.c
	${CORE_DIR}/asn1/ecc_der_util.c
	${CORE_DIR}/asn1/x509_cert_build.c
	${CORE_DIR}/asn1/x509_extension_builder.c
	${CORE_DIR}/asn1/x509_extension_builder_eku.c
	${CORE_DIR}/asn1/x509_oid.c
	${CORE_DIR}/asn1/dice/tcg_dice_oid.c
	${CORE_DIR}/asn1/dice/x509_extension_builder_dice_tcbinfo.c
	${CORE_DIR}/asn1/dice/x509_extension_builder_dice_ueid.c
	${CORE_DIR}/asn1/dme/dme_structure.c
	${CORE_DIR}/asn1/dme/dme_structure_raw_ecc.c
	${CORE_DIR}/asn1/dme/x509_extension_builder_dme.c
	${CORE_DIR}/asn1/spdm/spdm_oid.c
	${CORE_DIR}/cmd_interface/counter_manager_registers.c
	${CORE_DIR}/common/buffer_util.c
	${CORE_DIR}/common/common_math.c
	${CORE_DIR}/common/image_header.c
	${CORE_DIR}/mmio/mmio_register_block.c
	${CORE_DIR}/mmio/mmio_util.c
	${CORE_DIR}/crypto/ecc_ecc_hw.c
	${CORE_DIR}/crypto/ecdsa.c
	${CORE_DIR}/crypto/hash.c
	${CORE_DIR}/crypto/signature_verification.c
	${CORE_DIR}/crypto/signature_verification_ecc.c
	${CORE_DIR}/crypto/kat/ecc_kat_vectors.c
	${CORE_DIR}/crypto/kat/ecdsa_kat.c
	${CORE_DIR}/crypto/kat/hash_kat.c
	${CORE_DIR}/crypto/kat/hash_kat_vectors.c
	${CORE_DIR}/crypto/kat/hmac_kat_vectors.c
	${CORE_DIR}/crypto/kat/signature_verification_kat_vectors.c
	${CORE_DIR}/firmware/firmware_component.c
	${CORE_DIR}/firmware/firmware_header.c
	${CORE_DIR}/flash/flash_common.c
	${CORE_DIR}/flash/flash_store_contiguous_blocks.c
	${CORE_DIR}/flash/flash_util.c
	${CORE_DIR}/flash/spi_flash.c
	${CORE_DIR}/flash/spi_flash_sfdp.c
	${CORE_DIR}/logging/debug_log.c
	${CORE_DIR}/logging/logging_flash.c
	${CORE_DIR}/logging/logging_memory.c
	${CORE_DIR}/mmio/mmio_util.c
	${CORE_DIR}/riot/riot_core.c
	${CORE_DIR}/riot/reference/RiotBase64.c
	${CORE_DIR}/riot/reference/RiotDerDec.c
	${CORE_DIR}/riot/reference/RiotDerEnc.c
	${CORE_DIR}/riot/reference/RiotX509Bldr.c
	${CORE_DIR}/system/device_unlock_token.c
	${CORE_DIR}/system/fatal_error.c
	${CORE_DIR}/system/real_time_clock.c
	${CORE_DIR}/system/security_manager.c
	${CORE_DIR}/system/security_policy.c
	${BARE_METAL_BASE_DIR}/platform_base.c
	${BARE_METAL_BASE_DIR}/sbrk.c
	${BARE_METAL_DIR}/platform.c
	${GCC_DIR}/platform_compiler.c
	${MICROSOFT_DIR}/riot/dice_oid.c
	${HSP_DIR}/common/sram_util.c
	${HSP_DIR}/crypto/ecc_ccs.c
	${HSP_DIR}/drivers/ccs_ksu_fips.c
	${HSP_DIR}/drivers/ccs_ksu_interface.c
	${HSP_DIR}/drivers/kat/ccs_ksu_kat.c
	${HSP_DIR}/drivers/kat/ccs_ksu_kat_vectors.c
	${HSP_DIR}/firmware/firmware_loader_hsp_dmb.c
	${HSP_DIR}/firmware/firmware_loader_hsp_memory.c
	${HSP_DIR}/firmware/hsp_fw_1sp.c
	${HSP_DIR}/firmware/hsp_fw_util.c
	${HSP_DIR}/firmware/key_manifest_hsp_firmware.c
	${HSP_DIR}/firmware/key_manifest_hsp_rom.c
	${HSP_DIR}/logging/code_path_integrity.c
	${HSP_DIR}/mmio/mmio_register_block_hsp.c
	${HSP_DIR}/mmio/mmio_register_block_soc.c
	${HSP_DIR}/mpu/hsp_mpu.c
	${HSP_DIR}/riot/riot_core_hsp.c
	${HSP_DIR}/riot/riot_core_hsp_fips.c
	${HSP_DIR}/rom/boot_measurements_single_root.c
	${HSP_DIR}/system/security_manager_hsp.c
	${HSP_DIR}/trap/hsp_interrupt_vector.c
	${HSP_HW_DIR}/crypto/ecc_hw_pka.c
	${HSP_HW_DIR}/crypto/hash_hs_sha.c
	${HSP_HW_DIR}/crypto/hs_sha_multi_update.c
	${HSP_HW_DIR}/crypto/rng_hsp.c
	${HSP_HW_DIR}/drivers/ccs_ksu.c
	${HSP_HW_DIR}/drivers/checkpoint.c
	${HSP_HW_DIR}/drivers/crypto_hw.c
	${HSP_HW_DIR}/drivers/fuse_controller.c
	${HSP_HW_DIR}/drivers/hs_sha.c
	${HSP_HW_DIR}/drivers/hsp_aeb.c
	${HSP_HW_DIR}/drivers/hsp_aes.c
	${HSP_HW_DIR}/drivers/hsp_dmb.c
	${HSP_HW_DIR}/drivers/hsp_gpio.c
	${HSP_HW_DIR}/drivers/hsp_rng_hw.c
	${HSP_HW_DIR}/drivers/spi_dwc_ssi.c
	${HSP_HW_DIR}/drivers/sram.c
	${HSP_HW_DIR}/drivers/kat/hsp_rng_hw_kat.c
	${HSP_HW_DIR}/drivers/kat/hsp_rng_hw_kat_vectors.c
	${HSP_HW_DIR}/firmware/identity_renewal.c
	${HSP_HW_DIR}/flash/flash_master_dwc_ssi.c
	${HSP_HW_DIR}/riscv/riscvpmp.c
	${HSP_HW_DIR}/system/hsp_watchdog.c
	${HSP_HW_DIR}/system/real_time_clock_hsp.c
	${HSP_HW_DIR}/trap/hsp_interrupt.c
	${HSP_HW_DIR}/trap/hsp_trap.c
	${HSP_HW_DIR}/trap/hsp_trap_asm.S
	${HSP_API_DIR}/platform_clock_hsp.c
	${HSP_SDK_SOURCES}
	${MANTICORE_DIR}/firmware/manticore_bootloader.c
	${MANTICORE_DIR}/firmware/manticore_device_keys.c
	${MANTICORE_DIR}/firmware/manticore_firmware_descriptor.c
	${MANTICORE_DIR}/firmware/manticore_firmware_package.c
	${MANTICORE_DIR}/firmware/manticore_measurements.c
	${MANTICORE_DIR}/mpu/fence_manticore.c
	${MANTICORE_DIR}/mpu/memory_protection_manticore_1sp.c
	${MANTICORE_DIR}/system/security_manager_hsp_manticore.c
	${MANTICORE_DIR}/system/security_policy_hsp_manticore.c
	${MANTICORE_SP_DIR}/traps.c
	${MANTICORE_SP_DIR}/drivers/fuse_controller_manticore_fips.c
	${MANTICORE_SP_DIR}/firmware/hw_rot_firmware_key_manifest.c
	${DC_SCM_DIR}/build_version.c
	${DC_SCM_DIR}/reset_counter_init.c
	${DC_SCM_DIR}/sp_boot.c
	${DC_SCM_DIR}/init/pcie_phy.c
	${1SP_DIR}/1sp.c
	${1SP_DIR}/checkpoint/chkpt_1sp_init.c
	${1SP_DIR}/checkpoint/chkpt_1sp_load_external.c
	${1SP_DIR}/checkpoint/chkpt_1sp_load_internal.c
	${1SP_DIR}/checkpoint/chkpt_1sp_sprt_execute.c
	)

target_include_directories(
	${TARGET_NAME}
	PRIVATE
		${MBEDTLS_INCLUDES}
		${CORE_DIR}
		${BARE_METAL_BASE_DIR}
		${BARE_METAL_DIR}
		${GCC_DIR}
		${MICROSOFT_DIR}
		${HSP_DIR}
		${HSP_HW_DIR}
		${HSP_API_DIR}
		${HSP_SDK_INCLUDES}
		${MANTICORE_DIR}
		${MANTICORE_PLATFORM_DIR}
		${ROM_DIR}
		${MANTICORE_SP_DIR}
		${DC_SCM_DIR}
		${1SP_DIR}
	)

target_compile_definitions(
	${TARGET_NAME}
	PRIVATE
		CODE_PATH_INTEGRITY_ALWAYS_ENABLED
		ECC_ENABLE_GENERATE_KEY_PAIR
		HASH_ENABLE_SHA1
		HASH_ENABLE_SHA384
		LOGGING_SUPPORT_DEBUG_LOG
		LOGGING_DEBUG_TIMESTAMP_CONST_INSTANCE
		MANTICORE_ENABLE_A0_SUPPORT
		X509_ENABLE_CREATE_CERTIFICATES
		${HW_BUILD_TARGET}
		${MANTICORE_TEST_FEATURES}
	)

target_compile_options(
	${TARGET_NAME}
	PRIVATE
		-Os
		-fstack-protector-all
	)

target_link_options(
	${TARGET_NAME}
	PRIVATE
		-nostartfiles
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

# [OSS] .dis generation removed (debug artifact)

# [OSS] .vhx generation removed (FPGA artifact)

# [OSS] eccgen removed (FPGA ECC tool)
