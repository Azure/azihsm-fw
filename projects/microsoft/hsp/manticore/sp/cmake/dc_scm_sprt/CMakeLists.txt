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
#	CMake script to build Manticore 2SP
#
# --

project(dc-scm-sprt LANGUAGES C ASM)

set(TARGET_IMG dc_scm_sprt)
set(TARGET_NAME ${TARGET_IMG}.elf)

include(Mbedtls)
include(FreeRTOS)

list(FIND MANTICORE_TEST_FEATURES "MANTICORE_ENABLE_FIPS_ACVP_TESTING" acvp)
if(${acvp} GREATER -1)
	include(Acvpparser)
endif()

set(SPRT_DIR ${CERBERUS_ROOT}/projects/microsoft/hsp/manticore/sp/src/dc_scm/sprt)

# Location to find config files such as FreeRTOSConfig.h
set(FREERTOS_CONFIG_DIR "${SPRT_DIR}")

# @@MUST@@ come after setting the FREERTOS_CONFIG_DIR to ensure we pick up app-specific
# config files first.
include(${MANTICORE_SP_DIR}/../cmake/freertos_port.cmake)

if(DEFINED I2C_SLAVE_MODE)
	set(I2C_SOURCES
		${HSP_HW_DIR}/drivers/i2c_dw_apb_slave.c
		${HSP_HW_DIR}/cmd_interface/cmd_channel_i2c_dw_apb_slave.c
		${DC_SCM_DIR}/init/init_i2c_slave.c
	)

	set(I2C_OPTIONS "-DI2C_SLAVE")
else()
	set(I2C_SOURCES
		${HSP_HW_DIR}/drivers/i2c_dw_apb_multimaster.c
		${HSP_HW_DIR}/cmd_interface/cmd_channel_i2c_dw_apb_multimaster.c
		${DC_SCM_DIR}/init/init_i2c_multimaster.c
	)
endif()

if(${acvp} GREATER -1)
	# Add all files needed for FIPS ACVP testing-related features
	set(FIPS_SOURCES
		${ACVPPARSER_SOURCES}
		${CORE_DIR}/acvp/acvp_proto.c
		${CORE_DIR}/acvp/acvp_proto_tester_adapter.c
		${CORE_DIR}/acvp/backend_aead.c
		${CORE_DIR}/acvp/backend_ecdh.c
		${CORE_DIR}/acvp/backend_ecdsa.c
		${CORE_DIR}/acvp/backend_hkdf.c
		${CORE_DIR}/acvp/backend_hmac.c
		${CORE_DIR}/acvp/backend_rsa.c
		${CORE_DIR}/acvp/backend_sha.c
		${CORE_DIR}/acvp/backend_sym.c
		${MICROSOFT_DIR}/fips/cmd_interface_msft_fips_acvp.c
		${HSP_DIR}/acvp/backend_ecdsa_hsp_fw.c
		${DC_SCM_DIR}/init/init_acvp.c
	)

	set(FIPS_INCLUDES ${ACVPPARSER_INCLUDES})

	set(FIPS_OPTIONS
		-DMANTICORE_ENABLE_ACVP

		# MbedTLS configs needed for RSA ACVP testing
		-DMBEDTLS_CTR_DRBG_C
		-DMBEDTLS_ENTROPY_C
		-DMBEDTLS_ENTROPY_HARDWARE_ALT
		-DMBEDTLS_ENTROPY_MIN_HARDWARE=4
	)

	list(FIND MANTICORE_TEST_FEATURES "ACVP_ECDSA_ROM" acvp_ecdsa_rom)
	if(${acvp_ecdsa_rom} GREATER -1)
		list(APPEND FIPS_OPTIONS -DECDSA_ROM_ENABLE_FIPS_ACVP_TESTING)
	endif()
else()
	# Enable full host and manifest initialization flows as long as ACVP testing is disabled.
	set(INIT_SOURCES
		${DC_SCM_DIR}/init/init_host.c
		${DC_SCM_DIR}/init/init_manifest.c
	)
endif()

list(FIND MANTICORE_TEST_FEATURES "MANTICORE_ENABLE_FIPS_CMVP_TESTING" cmvp)
if(${cmvp} GREATER -1)
	set(FIPS_SOURCES
		${FIPS_SOURCES}
		${HSP_DIR}/drivers/ccs_ksu_interface.c
		${MICROSOFT_DIR}/fips/cmd_interface_msft_fips_cmvp.c
		${MANTICORE_DIR}/fips/cmvp_test_manticore.c
		${MANTICORE_SP_DIR}/crashdump/soc_crashdump_ras_fault_injection.c
		${DC_SCM_DIR}/fips/self_test_cmvp_fault_injection.c
	)
endif()

add_executable(
	${TARGET_NAME}
	${MBEDTLS_SOURCES}
	${FREERTOS_SOURCES}
	${FREERTOS_PORTABLE_ROOT}/MemMang/heap_4.c
	${FIPS_SOURCES}
	${I2C_SOURCES}
	${INIT_SOURCES}
	${CORE_DIR}/asn1/asn1_util.c
	${CORE_DIR}/asn1/base64_mbedtls.c
	${CORE_DIR}/asn1/base64_thread_safe.c
	${CORE_DIR}/asn1/ecc_der_util.c
	${CORE_DIR}/asn1/x509_mbedtls.c
	${CORE_DIR}/asn1/x509_thread_safe.c
	${CORE_DIR}/attestation/attestation_responder.c
	${CORE_DIR}/attestation/aux_attestation.c
	${CORE_DIR}/attestation/pcr.c
	${CORE_DIR}/attestation/pcr_store.c
	${CORE_DIR}/cmd_interface/authorized_execution_config_reset.c
	${CORE_DIR}/cmd_interface/cerberus_protocol_debug_commands.c
	${CORE_DIR}/cmd_interface/cerberus_protocol_diagnostic_commands.c
	${CORE_DIR}/cmd_interface/cerberus_protocol_master_commands.c
	${CORE_DIR}/cmd_interface/cerberus_protocol_optional_commands.c
	${CORE_DIR}/cmd_interface/cerberus_protocol_required_commands.c
	${CORE_DIR}/cmd_interface/cmd_authorization.c
	${CORE_DIR}/cmd_interface/cmd_background_handler.c
	${CORE_DIR}/cmd_interface/cmd_channel.c
	${CORE_DIR}/cmd_interface/cmd_channel_handler.c
	${CORE_DIR}/cmd_interface/cmd_interface.c
	${CORE_DIR}/cmd_interface/cmd_interface_multi_handler.c
	${CORE_DIR}/cmd_interface/cmd_interface_system.c
	${CORE_DIR}/cmd_interface/config_reset.c
	${CORE_DIR}/cmd_interface/counter_manager_registers.c
	${CORE_DIR}/cmd_interface/device_manager.c
	${CORE_DIR}/cmd_interface/msg_transport.c
	${CORE_DIR}/cmd_interface/msg_transport_intermediate.c
	${CORE_DIR}/common/auth_token.c
	${CORE_DIR}/common/authorization_allowed.c
	${CORE_DIR}/common/authorization_challenge.c
	${CORE_DIR}/common/authorization_disallowed.c
	${CORE_DIR}/common/authorization_global.c
	${CORE_DIR}/common/authorized_data_token_only.c
	${CORE_DIR}/common/authorized_data_with_aad.c
	${CORE_DIR}/common/authorized_execution.c
	${CORE_DIR}/common/buffer_util.c
	${CORE_DIR}/common/common_math.c
	${CORE_DIR}/common/image_header.c
	${CORE_DIR}/common/observable.c
	${CORE_DIR}/crypto/aes_gcm_mbedtls.c
	${CORE_DIR}/crypto/aes_key_wrap.c
	${CORE_DIR}/crypto/aes_key_wrap_with_padding.c
	${CORE_DIR}/crypto/checksum.c
	${CORE_DIR}/crypto/ecc_ecc_hw.c
	${CORE_DIR}/crypto/ecc_thread_safe.c
	${CORE_DIR}/crypto/ecdh.c
	${CORE_DIR}/crypto/ecdsa.c
	${CORE_DIR}/crypto/ephemeral_key_generation_rsa.c
	${CORE_DIR}/crypto/hash.c
	${CORE_DIR}/crypto/hash_thread_safe.c
	${CORE_DIR}/crypto/hkdf.c
	${CORE_DIR}/crypto/kdf.c
	${CORE_DIR}/crypto/rng_mbedtls.c
	${CORE_DIR}/crypto/rng_thread_safe.c
	${CORE_DIR}/crypto/rsa.c
	${CORE_DIR}/crypto/rsa_mbedtls.c
	${CORE_DIR}/crypto/rsa_thread_safe.c
	${CORE_DIR}/crypto/signature_verification.c
	${CORE_DIR}/crypto/signature_verification_ecc.c
	${CORE_DIR}/crypto/kat/aes_kat.c
	${CORE_DIR}/crypto/kat/aes_kat_vectors.c
	${CORE_DIR}/crypto/kat/aes_key_wrap_kat.c
	${CORE_DIR}/crypto/kat/aes_key_wrap_kat_vectors.c
	${CORE_DIR}/crypto/kat/ecc_kat_vectors.c
	${CORE_DIR}/crypto/kat/ecdh_kat.c
	${CORE_DIR}/crypto/kat/ecdsa_kat.c
	${CORE_DIR}/crypto/kat/hash_kat.c
	${CORE_DIR}/crypto/kat/hash_kat_vectors.c
	${CORE_DIR}/crypto/kat/hmac_kat_vectors.c
	${CORE_DIR}/crypto/kat/kdf_kat.c
	${CORE_DIR}/crypto/kat/kdf_kat_vectors.c
	${CORE_DIR}/crypto/kat/signature_verification_kat_vectors.c
	${CORE_DIR}/fips/cmd_channel_error_state.c
	${CORE_DIR}/fips/periodic_self_test_handler.c
	${CORE_DIR}/firmware/app_context_null.c
	${CORE_DIR}/firmware/authorized_execution_allow_impactful.c
	${CORE_DIR}/firmware/authorized_execution_prepare_firmware_update.c
	${CORE_DIR}/firmware/firmware_component.c
	${CORE_DIR}/firmware/firmware_header.c
	${CORE_DIR}/firmware/firmware_update.c
	${CORE_DIR}/firmware/firmware_update_handler.c
	${CORE_DIR}/firmware/firmware_update_handler_revoke_after_reset.c
	${CORE_DIR}/firmware/firmware_update_observer_impactful.c
	${CORE_DIR}/firmware/impactful_update.c
	${CORE_DIR}/firmware/impactful_update_handler.c
	${CORE_DIR}/flash/flash_common.c
	${CORE_DIR}/flash/flash_store_contiguous_blocks.c
	${CORE_DIR}/flash/flash_store_contiguous_blocks_key_wrap.c
	${CORE_DIR}/flash/flash_updater.c
	${CORE_DIR}/flash/flash_util.c
	${CORE_DIR}/flash/spi_flash.c
	${CORE_DIR}/flash/spi_flash_sfdp.c
	${CORE_DIR}/host_fw/bmc_recovery.c
	${CORE_DIR}/host_fw/host_cmd_handler.c
	${CORE_DIR}/host_fw/host_flash_initialization.c
	${CORE_DIR}/host_fw/host_flash_manager.c
	${CORE_DIR}/host_fw/host_flash_manager_dual.c
	${CORE_DIR}/host_fw/host_flash_manager_single.c
	${CORE_DIR}/host_fw/host_fw_util.c
	${CORE_DIR}/host_fw/host_irq_handler.c
	${CORE_DIR}/host_fw/host_irq_handler_auth_check.c
	${CORE_DIR}/host_fw/host_irq_handler_mask_irqs.c
	${CORE_DIR}/host_fw/host_processor.c
	${CORE_DIR}/host_fw/host_processor_dual.c
	${CORE_DIR}/host_fw/host_processor_dual_full_bypass.c
	${CORE_DIR}/host_fw/host_processor_filtered.c
	${CORE_DIR}/host_fw/host_processor_observer_pcr.c
	${CORE_DIR}/host_fw/host_processor_single.c
	${CORE_DIR}/host_fw/host_processor_single_full_bypass.c
	${CORE_DIR}/host_fw/host_state_manager.c
	${CORE_DIR}/host_fw/host_state_observer_dirty_reset.c
	${CORE_DIR}/intrusion/authorized_execution_reset_intrusion.c
	${CORE_DIR}/intrusion/intrusion_manager.c
	${CORE_DIR}/keystore/ephemeral_key_manager.c
	${CORE_DIR}/keystore/key_cache_flash.c
	${CORE_DIR}/keystore/keystore_flash.c
	${CORE_DIR}/logging/crash_dump_logging.c
	${CORE_DIR}/logging/debug_log.c
	${CORE_DIR}/logging/log_flush_handler.c
	${CORE_DIR}/logging/logging_flash.c
	${CORE_DIR}/manifest/manifest_cmd_handler.c
	${CORE_DIR}/manifest/manifest_flash.c
	${CORE_DIR}/manifest/manifest_manager.c
	${CORE_DIR}/manifest/manifest_manager_flash.c
	${CORE_DIR}/manifest/manifest_pcr.c
	${CORE_DIR}/manifest/manifest_verification.c
	${CORE_DIR}/manifest/cfm/cfm_flash.c
	${CORE_DIR}/manifest/cfm/cfm_manager.c
	${CORE_DIR}/manifest/cfm/cfm_manager_flash.c
	${CORE_DIR}/manifest/cfm/cfm_observer_pcr.c
	${CORE_DIR}/manifest/cfm/manifest_cmd_handler_cfm.c
	${CORE_DIR}/manifest/pcd/manifest_cmd_handler_pcd.c
	${CORE_DIR}/manifest/pcd/pcd_flash.c
	${CORE_DIR}/manifest/pcd/pcd_manager.c
	${CORE_DIR}/manifest/pcd/pcd_manager_flash.c
	${CORE_DIR}/manifest/pcd/pcd_observer_pcr.c
	${CORE_DIR}/manifest/pfm/manifest_cmd_handler_pfm.c
	${CORE_DIR}/manifest/pfm/pfm_flash.c
	${CORE_DIR}/manifest/pfm/pfm_manager.c
	${CORE_DIR}/manifest/pfm/pfm_manager_flash.c
	${CORE_DIR}/manifest/pfm/pfm_observer_pcr.c
	${CORE_DIR}/manifest/pfm/pfm_observer_pending_reset.c
	${CORE_DIR}/mctp/cmd_interface_mctp_control.c
	${CORE_DIR}/mctp/cmd_interface_protocol_mctp_msft_vdm.c
	${CORE_DIR}/mctp/cmd_interface_protocol_mctp.c
	${CORE_DIR}/mctp/mctp_base_protocol.c
	${CORE_DIR}/mctp/mctp_control_protocol_commands.c
	${CORE_DIR}/mctp/mctp_interface.c
	${CORE_DIR}/mctp/msg_transport_mctp_message.c
	${CORE_DIR}/mmio/mmio_util.c
	${CORE_DIR}/mmio/mmio_register_block.c
	${CORE_DIR}/pcisig/doe/doe_interface.c
	${CORE_DIR}/pcisig/doe/doe_cmd_channel.c
	${CORE_DIR}/pcisig/ide/cmd_interface_ide_responder.c
	${CORE_DIR}/pcisig/ide/ide_commands.c
	${CORE_DIR}/pcisig/tdisp/cmd_interface_tdisp_responder.c
	${CORE_DIR}/pcisig/tdisp/firmware_update_observer_tdisp.c
	${CORE_DIR}/pcisig/tdisp/tdisp_commands.c
	${CORE_DIR}/pcisig/tdisp/tdisp_tdi_context_manager.c
	${CORE_DIR}/recovery/recovery_image.c
	${CORE_DIR}/recovery/recovery_image_cmd_handler.c
	${CORE_DIR}/recovery/recovery_image_header.c
	${CORE_DIR}/recovery/recovery_image_manager.c
	${CORE_DIR}/recovery/recovery_image_observer_pcr.c
	${CORE_DIR}/recovery/recovery_image_section_header.c
	${CORE_DIR}/rma/authorized_execution_rma.c
	${CORE_DIR}/riot/riot_key_manager.c
	${CORE_DIR}/rma/rma_unlock_token.c
	${CORE_DIR}/spdm/cmd_interface_protocol_spdm_pcisig.c
	${CORE_DIR}/spdm/cmd_interface_protocol_spdm_vdm.c
	${CORE_DIR}/spdm/cmd_interface_spdm.c
	${CORE_DIR}/spdm/cmd_interface_spdm_responder.c
	${CORE_DIR}/spdm/impactful_check_spdm.c
	${CORE_DIR}/spdm/spdm_certificate_chain_dice.c
	${CORE_DIR}/spdm/spdm_commands.c
	${CORE_DIR}/spdm/spdm_measurements.c
	${CORE_DIR}/spdm/spdm_measurements_discovery.c
	${CORE_DIR}/spdm/spdm_persistent_context_no_secure_session.c
	${CORE_DIR}/spdm/spdm_secure_session_manager.c
	${CORE_DIR}/spdm/spdm_state.c
	${CORE_DIR}/spdm/spdm_transcript_manager.c
	${CORE_DIR}/spi_filter/spi_filter_interface.c
	${CORE_DIR}/spi_filter/spi_filter_irq_handler.c
	${CORE_DIR}/spi_filter/spi_filter_irq_handler_dirty.c
	${CORE_DIR}/state_manager/state_manager.c
	${CORE_DIR}/state_manager/state_persistence_handler.c
	${CORE_DIR}/system/device_unlock_token.c
	${CORE_DIR}/system/event_task.c
	${CORE_DIR}/system/fatal_error.c
	${CORE_DIR}/system/periodic_task.c
	${CORE_DIR}/system/secure_device_unlock_policy.c
	${CORE_DIR}/system/security_manager.c
	${CORE_DIR}/system/security_policy.c
	${CORE_DIR}/system/system.c
	${CORE_DIR}/system/system_state_manager.c
	${MICROSOFT_DIR}/intrusion/intrusion_manager_msft.c
	${MICROSOFT_DIR}/fips/cmd_interface_msft_fips_on_demand_self_test.c
	${MICROSOFT_DIR}/fips/fips_commands.c
	${MICROSOFT_DIR}/msft_protocol/cmd_interface_msft.c
	${MICROSOFT_DIR}/msft_protocol/cmd_interface_msft_base.c
	${MICROSOFT_DIR}/msft_protocol/cmd_interface_msft_rot.c
	${MICROSOFT_DIR}/msft_protocol/cmd_interface_protocol_msft.c
	${MICROSOFT_DIR}/msft_protocol/cmd_interface_protocol_msft_cmd_set.c
	${MICROSOFT_DIR}/msft_protocol/mctp_notifier_msft.c
	${MICROSOFT_DIR}/msft_protocol/mctp_notifier_msft_const_list.c
	${MICROSOFT_DIR}/msft_protocol/msft_base_commands.c
	${MICROSOFT_DIR}/msft_protocol/msft_mctp_protocol.c
	${MICROSOFT_DIR}/msft_protocol/rot_commands.c
	${MICROSOFT_DIR}/msft_protocol/temperature_sensor_cluster.c
	${MICROSOFT_DIR}/riot/dice_oid.c
	${ARM_DIR}/crash_dump_arm.c
	${RISCV_DIR}/crash_dump_riscv.c
	${GCC_DIR}/platform_compiler.c
	${CERBERUS_FREERTOS_DIR}/cmd_channel_freertos.c
	${CERBERUS_FREERTOS_DIR}/event_task_freertos.c
	${CERBERUS_FREERTOS_DIR}/periodic_task_freertos.c
	${CERBERUS_FREERTOS_DIR}/platform.c
	${CERBERUS_FREERTOS_DIR}/static_os_tasks.c
	${CERBERUS_FREERTOS_DIR}/system_observer_stack_usage.c
	${HSP_DIR}/common/sram_util.c
	${HSP_DIR}/crashdump/hsp_crashdump_handler.c
	${HSP_DIR}/crashdump/hsp_crashdump_logging.c
	${HSP_DIR}/crypto/aes_ecb_hsp.c
	${HSP_DIR}/crypto/aes_hsp_hw_intf.c
	${HSP_DIR}/crashdump/hsp_crashdump_hw_err_handler.c
	${HSP_DIR}/crashdump/hsp_crashdump_hw_err_handler_access_err.c
	${HSP_DIR}/crashdump/hsp_crashdump_hw_err_handler_bus_err.c
	${HSP_DIR}/crashdump/hsp_crashdump_hw_err_handler_chk_err.c
	${HSP_DIR}/crashdump/hsp_crashdump_hw_err_handler_dmb_err.c
	${HSP_DIR}/crashdump/hsp_crashdump_hw_err_handler_mem_err.c
	${HSP_DIR}/crashdump/hsp_crashdump_hw_err_handler_mpu_err.c
	${HSP_DIR}/crashdump/hsp_crashdump_hw_err_handler_rng_err.c
	${HSP_DIR}/crashdump/hsp_crashdump_hw_err_handler_watchdog_timeout.c
	${HSP_DIR}/crashdump/hsp_crashdump_hw_err_handler_wdt_err.c
	${HSP_DIR}/drivers/ccs_ksu_interface.c
	${HSP_DIR}/drivers/kat/ccs_ksu_kat.c
	${HSP_DIR}/drivers/kat/ccs_ksu_kat_vectors.c
	${HSP_DIR}/firmware/hsp_fw_1sp.c
	${HSP_DIR}/firmware/hsp_fw_util.c
	${HSP_DIR}/firmware/key_manifest_hsp_firmware.c
	${HSP_DIR}/firmware/key_manifest_hsp_rom.c
	${HSP_DIR}/intrusion/hsp_intrusion_irq_handler.c
	${HSP_DIR}/intrusion/intrusion_state_hsp.c
	${HSP_DIR}/mmio/mmio_register_block_hsp.c
	${HSP_DIR}/mmio/mmio_register_block_soc.c
	${HSP_DIR}/mpu/hsp_mpu.c
	${HSP_DIR}/mpu/memory_protection_mpu_only.c
	${HSP_DIR}/riot/authorized_execution_identity_renewal.c
	${HSP_DIR}/rma/device_rma_transition_hsp_retest.c
	${HSP_DIR}/spi_filter/flash_mfg_filter_handler_hsp.c
	${HSP_DIR}/spi_filter/macronix_opcodes.c
	${HSP_DIR}/spi_filter/micron_opcodes.c
	${HSP_DIR}/spi_filter/spi_filter_hsp_irq_handler.c
	${HSP_DIR}/spi_filter/winbond_opcodes.c
	${HSP_DIR}/system/fatal_error_handler_hsp.c
	${HSP_DIR}/system/security_manager_hsp.c
	${HSP_DIR}/trap/hsp_interrupt_group.c
	${HSP_DIR}/trap/hsp_interrupt_vector.c
	${HSP_HW_DIR}/cmd_interface/cmd_channel_i2c_dw_apb.c
	${HSP_HW_DIR}/cmd_interface/cmd_device_hsp.c
	${HSP_HW_DIR}/cmd_interface/cmd_device_hsp_freertos.c
	${HSP_HW_DIR}/crypto/ecc_hw_pka.c
	${HSP_HW_DIR}/crypto/hash_hs_sha.c
	${HSP_HW_DIR}/crypto/hash_hs_sha_soc.c
	${HSP_HW_DIR}/crypto/hs_sha_multi_update.c
	${HSP_HW_DIR}/crypto/rng_hsp.c
	${HSP_HW_DIR}/drivers/ccs_ksu.c
	${HSP_HW_DIR}/drivers/crypto_hw.c
	${HSP_HW_DIR}/drivers/fuse_controller.c
	${HSP_HW_DIR}/drivers/hs_sha.c
	${HSP_HW_DIR}/drivers/hsp_aeb.c
	${HSP_HW_DIR}/drivers/hsp_aes.c
	${HSP_HW_DIR}/drivers/hsp_dmb.c
	${HSP_HW_DIR}/drivers/hsp_gpio.c
	${HSP_HW_DIR}/drivers/hsp_rng_hw.c
	${HSP_HW_DIR}/drivers/i2c_dw_apb.c
	${HSP_HW_DIR}/drivers/spi_dwc_ssi.c
	${HSP_HW_DIR}/drivers/sram.c
	${HSP_HW_DIR}/drivers/kat/hsp_rng_hw_kat.c
	${HSP_HW_DIR}/drivers/kat/hsp_rng_hw_kat_vectors.c
	${HSP_HW_DIR}/fips/self_test_hsp_rng_hw.c
	${HSP_HW_DIR}/firmware/hw_rot_hsp_rom.c
	${HSP_HW_DIR}/firmware/hw_rot_hsp_rom_no_fuses.c
	${HSP_HW_DIR}/firmware/identity_renewal.c
	${HSP_HW_DIR}/flash/flash_master_dwc_ssi.c
	${HSP_HW_DIR}/freertos/freertos_utils.c
	${HSP_HW_DIR}/host_fw/host_control_hsp_gpio.c
	${HSP_HW_DIR}/host_fw/host_gpio_irq_event_manager.c
	${HSP_HW_DIR}/host_fw/host_gpio_irq_handler.c
	${HSP_HW_DIR}/host_fw/host_irq_control_hsp_gpio.c
	${HSP_HW_DIR}/mbedtls/mbedtls_port.c
	${HSP_HW_DIR}/mbedtls/mbedtls_ecc_pka.c
	${HSP_HW_DIR}/mbedtls/mbedtls_ecdsa_pka.c
	${HSP_HW_DIR}/mbedtls/mbedtls_hs_sha.c
	${HSP_HW_DIR}/spi_filter/spi_filter_hsp.c
	${HSP_HW_DIR}/system/hsp_watchdog.c
	${HSP_HW_DIR}/system/hsp_watchdog_handler.c
	${HSP_HW_DIR}/system/real_time_clock_hsp.c
	${HSP_HW_DIR}/trap/hsp_interrupt.c
	${HSP_HW_DIR}/trap/hsp_trap.c
	${HSP_HW_DIR}/trap/hsp_trap_asm.S
	${HSP_API_DIR}/platform_clock_hsp.c
	${HSP_SDK_SOURCES}
	${MANTICORE_DIR}/crashdump/soc_crashdump_handler.c
	${MANTICORE_DIR}/crashdump/soc_crashdump_interface.c
	${MANTICORE_DIR}/crashdump/soc_crashdump_mbx_err_handler.c
	${MANTICORE_DIR}/drivers/soc_reset_control.c
	${MANTICORE_DIR}/fips/fips_self_test_manticore.c
	${MANTICORE_DIR}/fips/manticore_error_state.c
	${MANTICORE_DIR}/fips/self_test_manticore.c
	${MANTICORE_DIR}/firmware/firmware_image_manticore.c
	${MANTICORE_DIR}/firmware/graceful_shutdown.c
	${MANTICORE_DIR}/firmware/manticore_device_keys.c
	${MANTICORE_DIR}/firmware/manticore_firmware_descriptor.c
	${MANTICORE_DIR}/firmware/manticore_firmware_package.c
	${MANTICORE_DIR}/ipc/cmd_interface_ipc_admin.c
	${MANTICORE_DIR}/ipc/cmd_interface_ipc_hsm.c
	${MANTICORE_DIR}/ipc/ipc_channel.c
	${MANTICORE_DIR}/ipc/ipc_message.c
	${MANTICORE_DIR}/ipc/ipc_message_handler.c
	${MANTICORE_DIR}/logging/logging_collector.c
	${MANTICORE_DIR}/mpu/fence_manticore.c
	${MANTICORE_DIR}/mpu/memory_protection_manticore_sprt.c
	${MANTICORE_DIR}/msft_protocol/cmd_interface_msft_manticore.c
	${MANTICORE_DIR}/msft_protocol/manticore_commands.c
	${MANTICORE_DIR}/pcie/cmd_interface_tdisp_event_policy.c
	${MANTICORE_DIR}/pcie/ide_driver_manticore.c
	${MANTICORE_DIR}/pcie/ide_driver_manticore.c
	${MANTICORE_DIR}/pcie/tdisp_pcie_utils.c
	${MANTICORE_DIR}/pcie/tdisp_driver_manticore.c
	${MANTICORE_DIR}/pcie/tdisp_tdi_context_manager_manticore.c
	${MANTICORE_DIR}/spdm/spdm_persistent_context_manticore_gsram.c
	${MANTICORE_DIR}/system/fatal_error_handler_hsp_manticore.c
	${MANTICORE_DIR}/system/security_manager_hsp_manticore.c
	${MANTICORE_DIR}/system/security_policy_hsp_manticore.c
	${MANTICORE_DIR}/telemetry/telemetry_temperature_handler.c
	${MANTICORE_DIR}/telemetry/telemetry_pcie_handler.c
	${MANTICORE_PLATFORM_DIR}/system/temperature_sensor_tsen.c
	${MANTICORE_SP_DIR}/drivers/fuse_controller_manticore_fips.c
	${MANTICORE_SP_DIR}/firmware/hw_rot_firmware_key_manifest.c
	${DC_SCM_DIR}/build_version.c
	${DC_SCM_DIR}/reset_counter_init.c
	${DC_SCM_DIR}/sp_boot.c
	${DC_SCM_DIR}/firmware/firmware_update_observer_sprt.c
	${DC_SCM_DIR}/init/init_attestation.c
	${DC_SCM_DIR}/init/init_cmd.c
	${DC_SCM_DIR}/init/init_crashdump.c
	${DC_SCM_DIR}/init/init_crypto.c
	${DC_SCM_DIR}/init/init_ephemeral_key.c
	${DC_SCM_DIR}/init/init_error.c
	${DC_SCM_DIR}/init/init_firmware.c
	${DC_SCM_DIR}/init/init_flash.c
	${DC_SCM_DIR}/init/init_i2c.c
	${DC_SCM_DIR}/init/init_intrusion.c
	${DC_SCM_DIR}/init/init_ipc.c
	${DC_SCM_DIR}/init/init_log.c
	${DC_SCM_DIR}/init/init_log_flush_handlers.c
	${DC_SCM_DIR}/init/init_tdisp.c
	${DC_SCM_DIR}/init/init_spdm.c
	${DC_SCM_DIR}/init/init_system.c
	${SPRT_DIR}/sprt.c
	${SPRT_DIR}/traps_sprt.c
	${SPRT_DIR}/system/heartbeat_led_handler.c
	)

target_include_directories(
	${TARGET_NAME}
	PUBLIC
		${MBEDTLS_INCLUDES}
		${FREERTOS_INCLUDES}
		${FIPS_INCLUDES}
		${ARM_DIR}
		${RISCV_DIR}
		${CORE_DIR}
		${MICROSOFT_DIR}
		${GCC_DIR}
		${CERBERUS_FREERTOS_DIR}
		${HSP_DIR}
		${HSP_HW_DIR}
		${HSP_HW_DIR}/mbedtls
		${HSP_API_DIR}
		${HSP_SDK_INCLUDES}
		${MANTICORE_DIR}
		${MANTICORE_PLATFORM_DIR}
		${MANTICORE_SP_DIR}
		${ROM_DIR}
		${DC_SCM_DIR}
		${SPRT_DIR}
	)

target_compile_definitions(
	${TARGET_NAME}
	PRIVATE
		CMD_ENABLE_DEBUG_LOG
		CMD_ENABLE_HEAP_STATS
		CMD_ENABLE_INTRUSION
		CMD_ENABLE_ISSUE_REQUEST
		CMD_ENABLE_RESET_CONFIG
		ECC_ENABLE_ECDH
		ECC_ENABLE_GENERATE_KEY_PAIR
		FATAL_ERROR_CONST_INSTANCE
		HASH_ENABLE_SHA384
		HASH_ENABLE_SHA512
		LOGGING_SUPPORT_DEBUG_LOG
		RSA_DISABLE_DECRYPT
		RSA_ENABLE_PRIVATE_KEY
		X509_ENABLE_AUTHENTICATION
		MBEDTLS_CONFIG_FILE="${SPRT_DIR}/mbedTLS_config.h"
		PLATFORM_CLOCK_API="platform_clock_hsp.h"
		${FREERTOS_COMPILE_DEFS}
		${HW_BUILD_TARGET}
		DEBUG_STACK_USAGE
		${MANTICORE_TEST_FEATURES}
		${I2C_OPTIONS}
		${FIPS_OPTIONS}
	)

target_compile_options(
	${TARGET_NAME}
	PRIVATE
		-Os
		-fstack-protector-strong
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
