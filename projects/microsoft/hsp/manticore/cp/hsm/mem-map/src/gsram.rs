// Copyright (c) Microsoft Corporation. All rights reserved.

use core::mem::MaybeUninit;

use mcr_gdma_controller::GdmaRxQueueDesc;
use mcr_gdma_controller::GdmaTxQueueDesc;
use mcr_io_controller::IoRxEntry;
use mcr_io_controller::IoRxFreeListDesc;
use mcr_io_controller::IoRxQueueDesc;
use mcr_io_controller::IoTxEntry;
use mcr_io_controller::IoTxFreeListDesc;
use mcr_io_controller::IoTxQueueDesc;
use mcr_ipc_controller::IpcMessage;
use mcr_mem_map_derive::mem_map;
use mcr_self_test::PreopsNegativeTest;
use mcr_types::PorMeasurementData;
use mcr_types::*;

/// GS-RAM Memory Map
///
/// # Notes
///
/// UCD and GDMA hardware expect the queue base address to have 32-byte (0x20) alignment.
///
/// TODO: Optimize 32-byte aligned allocations to minimize GSRAM fragmentation.
#[mem_map(address = 0x6100_0000, length = 0x200000)]
pub struct GsRamMemMap {
    // ######## Below this line: preserved by SP on all non-POR resets ########
    /// Power On Reset Measurement
    por_measurements: PorMeasurementData,

    // ######## Below this line: wiped by SP on all resets ########
    /// Firmware package version
    #[field(cardinality = 32)]
    fw_package_version: u8,

    /// SoC ID
    #[field(cardinality = 32)]
    soc_id: u8,

    /// Length of the CP alias key.  This will be -1 if the key was too long
    #[field(volatile = true)]
    alias_key_length: u32,

    /// Storage for the CP alias private key
    #[field(cardinality = 64)]
    alias_key: u8,

    /// Length of the alias certificate.  This will be -1 if the certificate was too long
    #[field(volatile = true)]
    alias_cert_length: u32,

    /// Storage for the CP alias certificate signed by the device ID
    #[field(cardinality = 1160)]
    alias_cert: u8,

    // ######## 8kB SP Reserved Block ########
    /// Logger synchronization primitive.
    #[field(alignment = 0x1000, mutable = true, volatile = true)]
    logger_lock: u32,

    // ######## Below this line: preserved by SP on all non-POR resets ########
    /// Admin PCIe Function Resource Table
    #[field(cardinality = 65, mutable = true)]
    admin_pcie_resource_table: Resource,

    /// CRC for Admin PCIe Function Resource Table
    #[field(mutable = true, volatile = true)]
    admin_pcie_resource_table_crc: u32,

    /// Perform pre operational negative self test
    #[field(mutable = true)]
    negative_self_test: PreopsNegativeTest,

    /// BKS table
    /// Each table entry is 41 bytes wide
    /// there are 12 BKS entries including the 1 latest BKS1, last ten SVN's BKS1 and one BKS2.
    #[field(cardinality = 12, mutable = false)]
    bks_table: BksTableEntry,

    /// Reserved for SP shared data
    #[field(cardinality = 3316, mutable = false)]
    reserved_sp_shared: u8,

    /// Start CP1 Cerberus debug_log_buffer C struct fields and buffer array.
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    cp0_debug_log_ring_buffer_size: u32,

    /// Corresponds to consumer index (ci) in SimplexPipe
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    cp0_debug_log_ring_buffer_head: u32,

    /// Corresponds to producer index (pi) in SimplexPipe
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    cp0_debug_log_ring_buffer_tail: u32,

    #[field(alignment = 0x4, mutable = true, volatile = true)]
    cp0_debug_log_buffer_sender_overflows: u32,

    /// The queue size is 0x800 / sizeof(DebugLogEntryParameters)
    #[field(alignment = 0x4, cardinality = 0xAA, mutable = true)]
    cp0_debug_log_buffer_queue: mcr_types::DebugLogEntryParameters,

    /// Start CP1 Cerberus debug_log_buffer C struct fields and buffer array.
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    cp1_debug_log_ring_buffer_size: u32,

    /// Corresponds to consumer index (ci) in SimplexPipe
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    cp1_debug_log_ring_buffer_head: u32,

    /// Corresponds to producer index (pi) in SimplexPipe
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    cp1_debug_log_ring_buffer_tail: u32,

    #[field(alignment = 0x4, mutable = true, volatile = true)]
    cp1_debug_log_buffer_sender_overflows: u32,

    /// The queue size is 0x800 / sizeof(DebugLogEntryParameters)
    #[field(alignment = 0x4, cardinality = 0xAA, mutable = true)]
    cp1_debug_log_buffer_queue: mcr_types::DebugLogEntryParameters,

    // ########################### 4KB Region End ###########################
    // ######## Below this line: wiped by SP only on non-graceful and POR resets ########
    /// IDE-KM context (PCIe IDE key management context)
    #[field(cardinality = 2048)]
    ide_km_context: u32,

    // ## 4KB Region with access permission for FP cores, Admin, HSM, CDMA ##
    /// Source buffer for AES bulk self test vectors
    #[field(cardinality = 512, mutable = true)]
    aes_bulk_self_test_src_buf: u8,

    /// Destination buffer for AES bulk self test vectors
    #[field(cardinality = 512, mutable = true)]
    aes_bulk_self_test_dest_buf: u8,

    /// Reserved for FP restricted region
    #[field(cardinality = 3072)]
    rsvd_fp_restricted: u8,

    /// DOE message buffer
    #[field(cardinality = 1024, mutable = true)]
    doe_buffer: u32,

    /// Hsm core boot status
    #[field(alignment = 0x1000, volatile = true)]
    boot_status: u32,

    /// Admin(Requestor) to Hsm(Responder) ipc message tx queue producer index
    #[field(mutable = true, volatile = true)]
    admin_to_hsm_ipc_tx_queue_pi: u32,

    /// Admin(Requestor) to Hsm(Responder) ipc message tx queue consumer index
    #[field(mutable = true, volatile = true)]
    admin_to_hsm_ipc_tx_queue_ci: u32,

    /// Admin(Requestor) to Hsm(Responder) ipc message rx queue producer index
    #[field(mutable = true, volatile = true)]
    admin_to_hsm_ipc_rx_queue_pi: u32,

    /// Admin(Requestor) to Hsm(Responder) ipc message rx queue consumer index
    #[field(mutable = true, volatile = true)]
    admin_to_hsm_ipc_rx_queue_ci: u32,

    /// Admin(Requestor) to Hsm(Responder) ipc message tx queue
    #[field(cardinality = 2, mutable = true)]
    admin_to_hsm_ipc_tx_queue: IpcMessage,

    /// Admin(Requestor) to Hsm(Responder) ipc message rx queue
    #[field(cardinality = 2, mutable = true)]
    admin_to_hsm_ipc_rx_queue: IpcMessage,

    /// Admin(Requestor) to Hsp(Responder) ipc message tx queue consumer index
    #[field(mutable = true, volatile = true)]
    admin_to_hsp_ipc_tx_queue_ci: u32,

    /// Admin(Requestor) to Hsp(Responder) ipc message tx queue producer index
    #[field(mutable = true, volatile = true)]
    admin_to_hsp_ipc_tx_queue_pi: u32,

    /// Admin(Requestor) to Hsp(Responder) ipc message tx queue
    #[field(cardinality = 2, mutable = true)]
    admin_to_hsp_ipc_tx_queue: IpcMessage,

    /// Admin(Requestor) to Hsp(Responder) ipc message rx queue consumer index
    #[field(mutable = true, volatile = true)]
    admin_to_hsp_ipc_rx_queue_ci: u32,

    /// Admin(Requestor) to Hsp(Responder) ipc message rx queue producer index
    #[field(mutable = true, volatile = true)]
    admin_to_hsp_ipc_rx_queue_pi: u32,

    /// Admin(Requestor) to Hsp(Responder) ipc message rx queue
    #[field(cardinality = 2, mutable = true)]
    admin_to_hsp_ipc_rx_queue: IpcMessage,

    /// Hsm(Requestor) to Hsp(Responder) ipc message tx queue consumer index
    #[field(mutable = true, volatile = true)]
    hsm_to_hsp_ipc_tx_queue_ci: u32,

    /// Hsm(Requestor) to Hsp(Responder) ipc message tx queue producer index
    #[field(mutable = true, volatile = true)]
    hsm_to_hsp_ipc_tx_queue_pi: u32,

    /// Hsm(Requestor) to Hsp(Responder) ipc message tx queue to be used by Admin core
    #[field(cardinality = 2, mutable = true)]
    hsm_to_hsp_ipc_tx_queue: IpcMessage,

    /// Hsm(Requestor) to Hsp(Responder) ipc message rx queue consumer index
    #[field(mutable = true, volatile = true)]
    hsm_to_hsp_ipc_rx_queue_ci: u32,

    /// Hsm(Requestor) to Hsp(Responder) ipc message rx queue producer index
    #[field(mutable = true, volatile = true)]
    hsm_to_hsp_ipc_rx_queue_pi: u32,

    /// Hsm(Requestor) to Hsp(Responder) ipc message rx queue
    #[field(cardinality = 2, mutable = true)]
    hsm_to_hsp_ipc_rx_queue: IpcMessage,

    /// Hsp(Requestor) to Admin(Responder) ipc message tx queue consumer index
    #[field(mutable = true, volatile = true)]
    hsp_to_admin_ipc_tx_queue_ci: u32,

    /// Hsp(Requestor) to Admin(Responder) ipc message tx queue producer index
    #[field(mutable = true, volatile = true)]
    hsp_to_admin_ipc_tx_queue_pi: u32,

    /// Hsp(Requestor) to Admin(Responder) ipc message tx queue
    #[field(cardinality = 2, mutable = true)]
    hsp_to_admin_ipc_tx_queue: IpcMessage,

    /// Hsp(Requestor) to Admin(Responder) ipc message rx queue consumer index
    #[field(mutable = true, volatile = true)]
    hsp_to_admin_ipc_rx_queue_ci: u32,

    /// Hsp(Requestor) to Admin(Responder) ipc message rx queue producer index
    #[field(mutable = true, volatile = true)]
    hsp_to_admin_ipc_rx_queue_pi: u32,

    /// Hsp(Requestor) to Admin(Responder) ipc message rx queue
    #[field(cardinality = 2, mutable = true)]
    hsp_to_admin_ipc_rx_queue: IpcMessage,

    /// Deferred queue delete response from HSM to Admin
    #[field(cardinality = 66, mutable = true)]
    queue_delete_req_queue: QueueDeleteResponse,

    /// Deferred queue delete response from HSM to Admin consumer index
    #[field(mutable = true, volatile = true)]
    queue_delete_req_ci: u32,

    /// Deferred queue delete response from HSM to Admin producer index
    #[field(mutable = true, volatile = true)]
    queue_delete_req_pi: u32,

    /// SoftAes request queue from HSM to Admin
    #[field(cardinality = 66, mutable = true)]
    soft_aes_req_queue: SoftAesOffloadReq,

    /// SoftAes request queue from HSM to Admin consumer index
    #[field(mutable = true, volatile = true)]
    soft_aes_req_ci: u32,

    /// SoftAes request queue from HSM to Admin producer index
    #[field(mutable = true, volatile = true)]
    soft_aes_req_pi: u32,

    /// SoftAes response queue from Admin to HSM
    #[field(cardinality = 66, mutable = true)]
    soft_aes_resp_queue: SoftAesOffloadResp,

    /// Hsm(Requestor) to Admin(Responder) ipc message tx queue consumer index
    #[field(mutable = true, volatile = true)]
    hsm_to_admin_ipc_tx_queue_ci: u32,

    /// Hsm(Requestor) to Admin(Responder) ipc message tx queue producer index
    #[field(mutable = true, volatile = true)]
    hsm_to_admin_ipc_tx_queue_pi: u32,

    /// Hsm(Requestor) to Admin(Responder) ipc message tx queue
    #[field(cardinality = 2, mutable = true)]
    hsm_to_admin_ipc_tx_queue: IpcMessage,

    /// Hsm(Requestor) to Admin(Responder) ipc message rx queue consumer index
    #[field(mutable = true, volatile = true)]
    hsm_to_admin_ipc_rx_queue_ci: u32,

    /// Hsm(Requestor) to Admin(Responder) ipc message rx queue producer index
    #[field(mutable = true, volatile = true)]
    hsm_to_admin_ipc_rx_queue_pi: u32,

    /// Hsm(Requestor) to Admin(Responder) ipc message rx queue
    #[field(cardinality = 2, mutable = true)]
    hsm_to_admin_ipc_rx_queue: IpcMessage,

    /// Hsp(Requestor) to Admin(Responder) stop interface ipc message tx queue consumer index
    #[field(mutable = true, volatile = true)]
    hsp_to_admin_stop_interface_ipc_tx_queue_ci: u32,

    /// Hsp(Requestor) to Admin(Responder) stop interface ipc message tx queue producer index
    #[field(mutable = true, volatile = true)]
    hsp_to_admin_stop_interface_ipc_tx_queue_pi: u32,

    /// Hsp(Requestor) to Admin(Responder) stop interface ipc message tx queue
    #[field(cardinality = 2, mutable = true)]
    hsp_to_admin_stop_interface_ipc_tx_queue: IpcMessage,

    /// Hsp(Requestor) to Admin(Responder) stop interface ipc message rx queue consumer index
    #[field(mutable = true, volatile = true)]
    hsp_to_admin_stop_interface_ipc_rx_queue_ci: u32,

    /// Hsp(Requestor) to Admin(Responder) stop interface ipc message rx queue producer index
    #[field(mutable = true, volatile = true)]
    hsp_to_admin_stop_interface_ipc_rx_queue_pi: u32,

    /// Hsp(Requestor) to Admin(Responder) stop interface ipc message rx queue
    #[field(cardinality = 2, mutable = true)]
    hsp_to_admin_stop_interface_ipc_rx_queue: IpcMessage,

    #[field(cardinality = 3900, mutable = true)]
    hsp_admin_hsm_shared_ipc_memory_free_space: u8,

    /// Admin Receive Queue Shadow Producer Index
    #[field(alignment = 0x1000, volatile = true)]
    admin_rx_queue_shadow_pi: u32,

    /// Admin Transmit Queue Shadow Producer Index
    #[field(alignment = 0x20, volatile = true)]
    admin_tx_queue_shadow_pi: u32,

    /// Admin Receive Queue
    #[field(alignment = 0x20, cardinality = 128)]
    admin_rx_queue: IoRxQueueDesc,

    /// Admin Receive Free List
    #[field(alignment = 0x20, cardinality = 128, mutable = true)]
    admin_rx_free_list: IoRxFreeListDesc,

    /// Admin Submission Queue Entry Pool
    #[field(cardinality = 128)]
    admin_sqe_pool: IoRxEntry,

    /// Admin Transmit Queue
    #[field(alignment = 0x20, cardinality = 128)]
    admin_tx_queue: IoTxQueueDesc,

    /// Admin Transmit Free List
    #[field(alignment = 0x20, cardinality = 128, mutable = true)]
    admin_tx_free_list: IoTxFreeListDesc,

    /// Admin Completion Queue Entry Pool
    #[field(cardinality = 128, mutable = true)]
    admin_cqe_pool: IoTxEntry,

    /// Hsm Receive Queue Shadow Producer Index
    #[field(alignment = 0x20, volatile = true)]
    hsm_rx_queue_shadow_pi: u32,

    /// Hsm Transmit Queue Shadow Producer Index
    #[field(alignment = 0x20, volatile = true)]
    hsm_tx_queue_shadow_pi: u32,

    /// Hsm Receive Queue
    #[field(alignment = 0x20, cardinality = 32)]
    hsm_rx_queue: IoRxQueueDesc,

    /// Hsm Receive Free List
    #[field(alignment = 0x20, cardinality = 32, mutable = true)]
    hsm_rx_free_list: IoRxFreeListDesc,

    /// Hsm Submission Queue Entry Pool
    #[field(cardinality = 32)]
    hsm_sqe_pool: IoRxEntry,

    /// Hsm Transmit Queue
    #[field(alignment = 0x20, cardinality = 32)]
    hsm_tx_queue: IoTxQueueDesc,

    /// Hsm Transmit Free List
    #[field(alignment = 0x20, cardinality = 32, mutable = true)]
    hsm_tx_free_list: IoTxFreeListDesc,

    /// Hsm Completion Queue Entry Pool
    #[field(cardinality = 32, mutable = true)]
    hsm_cqe_pool: IoTxEntry,

    /// Free space in Admin + Hsm + UCD shared memory region
    #[field(cardinality = 1920)]
    admin_hsm_ucd_free_space: u8,

    /// Admin GDMA Receive Queue Shadow Producer Index
    #[field(alignment = 0x1000, volatile = true)]
    admin_gdma_rx_queue_shadow_pi: u32,

    /// Admin GDMA Transmit Queue Shadow Producer Index
    #[field(alignment = 0x20, volatile = true)]
    admin_gdma_tx_queue_shadow_pi: u32,

    /// Admin GDMA Receive Queue
    #[field(cardinality = 128)]
    admin_gdma_rx_queue: GdmaRxQueueDesc,

    /// Admin GDMA Transmit Queue
    #[field(cardinality = 128, mutable = true)]
    admin_gdma_tx_queue: GdmaTxQueueDesc,

    /// Hsm GDMA Receive Queue Shadow Producer Index
    #[field(alignment = 0x20, volatile = true)]
    hsm_gdma_rx_queue_shadow_pi: u32,

    /// Hsm GDMA Transmit Queue Shadow Producer Index
    #[field(alignment = 0x20, volatile = true)]
    hsm_gdma_tx_queue_shadow_pi: u32,

    /// Hsm GDMA Receive Queue
    #[field(cardinality = 128)]
    hsm_gdma_rx_queue: GdmaRxQueueDesc,

    /// Hsm GDMA Transmit Queue
    #[field(cardinality = 128, mutable = true)]
    hsm_gdma_tx_queue: GdmaTxQueueDesc,

    /// HSM AES command descriptor
    #[field(mutable = true)]
    aes_cmd_buffer: AesCommandDesc,

    /// HSM PKA command descriptor
    #[field(cardinality = 16, mutable = true)]
    pka_cmd_buffer: PkaCommand,

    /// Maximum size of the PKA output buffer
    #[field(cardinality = 16, mutable = true)]
    pka_output_buffer: [u8; PKA_RESULT_MAX_SIZE_BYTES],

    /// Maximum size of the PKA input buffer
    #[field(cardinality = 16, mutable = true)]
    pka_input_buffer: [u8; PKA_INPUT_MAX_SIZE_BYTES],

    /// Maximum size of the PKA ECC constants buffer.
    ///
    ///  Index 0                                 PKA_CONST_MAX_SIZE_BYTES - 1
    ///  --------------------------------------------------------------------
    /// | BASE_256 | PRIME_256 | BASE_384 | PRIME_384 | BASE_521 | PRIME_521 |
    /// |  (64)    |   (32)    |   (96)   |   (48)    |   (136)  |  (68)     |
    ///  --------------------------------------------------------------------
    ///
    #[field(cardinality = 1, mutable = true)]
    pka_ecc_const_buffer: [u8; PKA_CONST_MAX_SIZE_BYTES],

    /// ECC self test buffers to be used for PKA engines
    #[field(cardinality = 1, mutable = true)]
    pka_self_test_buffer: [u8; PKA_SELF_TEST_MAX_SIZE_BYTES],

    /// HS-SHA command descriptor
    #[field(cardinality = 1, mutable = true)]
    sha_cmd_buffer: ShaCommandDesc,

    /// HS-SHA initial digest buffer
    #[field(cardinality = 1, mutable = true)]
    sha_init_digest_buffer: [u8; SHA_DIGEST_MAX_SIZE_BYTES],

    /// HS-SHA output buffer
    #[field(cardinality = 1, mutable = true)]
    sha_out_buffer: [u8; SHA_DIGEST_MAX_SIZE_BYTES],

    /// HS-SHA self test buffer
    #[field(cardinality = 1, mutable = true)]
    sha_self_test_buffer: [u8; SHA_SELF_TEST_BUF_MAX_SIZE_BYTES],

    // ###### Add new CP entries above this line and adjust admin_hsm_dma_free_space ######
    /// Admin core Heap Memory
    #[field(cardinality = 20480, mutable = true)]
    admin_heap: MaybeUninit<u8>,

    /// Hsm core Heap Memory
    #[field(cardinality = 622592, mutable = true)]
    hsm_heap: MaybeUninit<u8>,

    /// Free space in the Admin and HSM DMA-able regions reserved
    #[field(cardinality = 936)]
    admin_hsm_dma_free_space: u8,

    // ########################### 4KB Region End ###########################
    /// Fast Path CDMA AES key vault meta data
    #[field(cardinality = 65, mutable = true, offset = 0xB8000)]
    cdma_key_vault_meta_data: u8,

    /// HSM Partition table
    #[field(cardinality = 1, mutable = true)]
    hsm_partition_table: HsmPartDataStore,

    /// HSM keyvault common meta data
    #[field(cardinality = 6772)]
    key_vault_cmn_meta_data: u8,

    /// Persistent store for HSM partition store
    #[field(cardinality = 65)]
    hsm_part_persistent_store: HsmPartPersistentStore,

    /// HSM key vault: 17KB * 65 entries
    /// TODO: Start the keyvault at 4K boundary after memory adjustments to program fences.
    #[field(cardinality = 1131520, mutable = true)]
    key_vault: u8,
}

/// Compile-time assertions for GsRamMemMap
///
/// These assertions are only valid for 32-bit targets and are moved from the unit tests
/// to ensure they are evaluated during firmware build.
#[cfg(target_pointer_width = "32")]
const _: () = {
    const ALIGNMENT: usize = 0x1000;

    static_assertions::const_assert_eq!(size_of::<usize>(), 4);

    static_assertions::const_assert_eq!(GsRamMemMap::ADMIN_RX_QUEUE_SHADOW_PI_OFFSET, 0x61009000);

    static_assertions::const_assert_eq!(
        GsRamMemMap::ADMIN_RX_QUEUE_SHADOW_PI_OFFSET,
        (GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_OFFSET
            + GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_SIZE
            - 1
            + ALIGNMENT
            + ALIGNMENT)
            & !(ALIGNMENT - 1),
    );

    static_assertions::const_assert!(
        GsRamMemMap::ADMIN_HSM_DMA_FREE_SPACE_OFFSET + GsRamMemMap::ADMIN_HSM_DMA_FREE_SPACE_SIZE
            <= GsRamMemMap::CDMA_KEY_VAULT_META_DATA_OFFSET
    );

    static_assertions::const_assert_eq!(GsRamMemMap::SOFT_AES_REQ_QUEUE_SIZE, 0x528);

    static_assertions::const_assert_eq!(GsRamMemMap::SOFT_AES_RESP_QUEUE_SIZE, 0x420);
};

#[cfg(test)]
mod tests {
    use static_assertions as sa;

    use super::*;

    #[test]
    fn test_base_address() {
        assert_eq!(GsRamMemMap::BASE_ADDRESS, 0x6100_0000);
    }

    #[test]
    fn test_por_measurements() {
        assert_eq!(GsRamMemMap::POR_MEASUREMENTS_SIZE, 0xAF0);
        assert_eq!(
            GsRamMemMap::POR_MEASUREMENTS_OFFSET,
            GsRamMemMap::BASE_ADDRESS,
        );
        assert_eq!(GsRamMemMap::POR_MEASUREMENTS_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::POR_MEASUREMENTS_OFFSET + GsRamMemMap::POR_MEASUREMENTS_SIZE
                <= GsRamMemMap::LOGGER_LOCK_OFFSET
        );
    }

    #[test]
    fn test_fw_package_version() {
        assert_eq!(GsRamMemMap::FW_PACKAGE_VERSION_SIZE, 0x20);
        assert_eq!(
            GsRamMemMap::FW_PACKAGE_VERSION_OFFSET,
            GsRamMemMap::POR_MEASUREMENTS_OFFSET + GsRamMemMap::POR_MEASUREMENTS_SIZE,
        );
        assert_eq!(GsRamMemMap::FW_PACKAGE_VERSION_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::FW_PACKAGE_VERSION_OFFSET + GsRamMemMap::FW_PACKAGE_VERSION_SIZE
                <= GsRamMemMap::LOGGER_LOCK_OFFSET
        );
    }

    #[test]
    fn test_soc_id() {
        assert_eq!(GsRamMemMap::SOC_ID_SIZE, 0x20);
        assert_eq!(
            GsRamMemMap::SOC_ID_OFFSET,
            GsRamMemMap::FW_PACKAGE_VERSION_OFFSET + GsRamMemMap::FW_PACKAGE_VERSION_SIZE,
        );
        assert_eq!(GsRamMemMap::SOC_ID_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::SOC_ID_OFFSET + GsRamMemMap::SOC_ID_SIZE
                <= GsRamMemMap::LOGGER_LOCK_OFFSET
        );
    }

    #[test]
    fn test_alias_key_length() {
        assert_eq!(GsRamMemMap::ALIAS_KEY_LENGTH_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ALIAS_KEY_LENGTH_OFFSET,
            GsRamMemMap::SOC_ID_OFFSET + GsRamMemMap::SOC_ID_SIZE,
        );
        assert_eq!(GsRamMemMap::ALIAS_KEY_LENGTH_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ALIAS_KEY_LENGTH_OFFSET + GsRamMemMap::ALIAS_KEY_LENGTH_SIZE
                <= GsRamMemMap::LOGGER_LOCK_OFFSET
        );
    }

    #[test]
    fn test_alias_key() {
        assert_eq!(GsRamMemMap::ALIAS_KEY_SIZE, 0x40);
        assert_eq!(
            GsRamMemMap::ALIAS_KEY_OFFSET,
            GsRamMemMap::ALIAS_KEY_LENGTH_OFFSET + GsRamMemMap::ALIAS_KEY_LENGTH_SIZE,
        );
        assert_eq!(GsRamMemMap::ALIAS_KEY_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ALIAS_KEY_OFFSET + GsRamMemMap::ALIAS_KEY_SIZE
                <= GsRamMemMap::LOGGER_LOCK_OFFSET
        );
    }

    #[test]
    fn test_alias_cert_length() {
        assert_eq!(GsRamMemMap::ALIAS_CERT_LENGTH_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ALIAS_CERT_LENGTH_OFFSET,
            GsRamMemMap::ALIAS_KEY_OFFSET + GsRamMemMap::ALIAS_KEY_SIZE,
        );
        assert_eq!(GsRamMemMap::ALIAS_CERT_LENGTH_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ALIAS_CERT_LENGTH_OFFSET + GsRamMemMap::ALIAS_CERT_LENGTH_SIZE
                <= GsRamMemMap::LOGGER_LOCK_OFFSET
        );
    }

    #[test]
    fn test_alias_cert() {
        assert_eq!(GsRamMemMap::ALIAS_CERT_SIZE, 0x488);
        assert_eq!(
            GsRamMemMap::ALIAS_CERT_OFFSET,
            GsRamMemMap::ALIAS_CERT_LENGTH_OFFSET + GsRamMemMap::ALIAS_CERT_LENGTH_SIZE,
        );
        assert_eq!(GsRamMemMap::ALIAS_CERT_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ALIAS_CERT_OFFSET + GsRamMemMap::ALIAS_CERT_SIZE
                <= GsRamMemMap::LOGGER_LOCK_OFFSET
        );
    }

    #[test]
    fn test_logger_lock() {
        let alignment = 0x1000;
        assert_eq!(GsRamMemMap::LOGGER_LOCK_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::LOGGER_LOCK_OFFSET,
            GsRamMemMap::ALIAS_CERT_OFFSET + GsRamMemMap::ALIAS_CERT_SIZE,
        );

        assert_eq!(GsRamMemMap::LOGGER_LOCK_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::LOGGER_LOCK_OFFSET + GsRamMemMap::LOGGER_LOCK_SIZE
                <= GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_OFFSET
        );
    }

    #[test]
    fn test_admin_pcie_resource_table() {
        assert_eq!(GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_SIZE, 0x104);
        assert_eq!(
            GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_OFFSET,
            GsRamMemMap::LOGGER_LOCK_OFFSET + GsRamMemMap::LOGGER_LOCK_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_OFFSET
                + GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_SIZE
                <= GsRamMemMap::IDE_KM_CONTEXT_OFFSET
        );
    }

    #[test]
    fn test_admin_pcie_resource_table_crc() {
        assert_eq!(GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_CRC_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_CRC_OFFSET,
            GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_OFFSET
                + GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_SIZE
        );
        assert_eq!(GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_CRC_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_CRC_OFFSET
                + GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_CRC_SIZE
                <= GsRamMemMap::IDE_KM_CONTEXT_OFFSET
        );
    }

    #[test]
    fn test_negative_self_test() {
        assert_eq!(GsRamMemMap::NEGATIVE_SELF_TEST_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::NEGATIVE_SELF_TEST_OFFSET,
            GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_CRC_OFFSET
                + GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_CRC_SIZE,
        );
        assert_eq!(GsRamMemMap::NEGATIVE_SELF_TEST_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::NEGATIVE_SELF_TEST_OFFSET + GsRamMemMap::NEGATIVE_SELF_TEST_SIZE
                <= GsRamMemMap::IDE_KM_CONTEXT_OFFSET
        );
    }

    #[test]
    fn test_bks_table() {
        assert_eq!(GsRamMemMap::BKS_TABLE_SIZE, 0x1EC);
        assert_eq!(
            GsRamMemMap::BKS_TABLE_OFFSET,
            GsRamMemMap::NEGATIVE_SELF_TEST_OFFSET + GsRamMemMap::NEGATIVE_SELF_TEST_SIZE
        );
        assert_eq!(GsRamMemMap::BKS_TABLE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::BKS_TABLE_OFFSET + GsRamMemMap::BKS_TABLE_SIZE
                <= GsRamMemMap::RESERVED_SP_SHARED_OFFSET
        );
    }

    #[test]
    fn test_reserved_sp_shared() {
        assert_eq!(
            GsRamMemMap::RESERVED_SP_SHARED_SIZE,
            0x2000
                - GsRamMemMap::LOGGER_LOCK_SIZE
                - GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_SIZE
                - GsRamMemMap::ADMIN_PCIE_RESOURCE_TABLE_CRC_SIZE
                - GsRamMemMap::NEGATIVE_SELF_TEST_SIZE
                - GsRamMemMap::BKS_TABLE_SIZE
                - GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_SIZE
                - GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_HEAD_SIZE
                - GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_TAIL_SIZE
                - GsRamMemMap::CP0_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_SIZE
                - GsRamMemMap::CP0_DEBUG_LOG_BUFFER_QUEUE_SIZE
                - GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_SIZE_SIZE
                - GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_HEAD_SIZE
                - GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_TAIL_SIZE
                - GsRamMemMap::CP1_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_SIZE
                - GsRamMemMap::CP1_DEBUG_LOG_BUFFER_QUEUE_SIZE
        );
        assert_eq!(
            GsRamMemMap::RESERVED_SP_SHARED_OFFSET,
            GsRamMemMap::BKS_TABLE_OFFSET + GsRamMemMap::BKS_TABLE_SIZE
        );
        assert_eq!(GsRamMemMap::RESERVED_SP_SHARED_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::RESERVED_SP_SHARED_OFFSET + GsRamMemMap::RESERVED_SP_SHARED_SIZE
                <= GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
        );
    }

    #[test]
    fn test_cp0_debug_log_descriptor_region() {
        assert_eq!(GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_SIZE, 0x04);
        assert_eq!(GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_HEAD_SIZE, 0x04);
        assert_eq!(GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_TAIL_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::CP0_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_SIZE,
            0x04
        );

        assert_eq!(
            GsRamMemMap::RESERVED_SP_SHARED_OFFSET + GsRamMemMap::RESERVED_SP_SHARED_SIZE,
            GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
        );

        assert_eq!(
            GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
                - GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            0
        );
        assert_eq!(
            GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_HEAD_OFFSET
                - GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            4
        );
        assert_eq!(
            GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_TAIL_OFFSET
                - GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            8
        );
        assert_eq!(
            GsRamMemMap::CP0_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_OFFSET
                - GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            12
        );

        assert_eq!(GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET % 4, 0);
        assert_eq!(GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_HEAD_OFFSET % 4, 0);
        assert_eq!(GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_TAIL_OFFSET % 4, 0);
        assert_eq!(
            GsRamMemMap::CP0_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_OFFSET % 4,
            0
        );

        sa::const_assert!(
            GsRamMemMap::CP0_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + GsRamMemMap::CP0_DEBUG_LOG_BUFFER_QUEUE_SIZE
                <= GsRamMemMap::IDE_KM_CONTEXT_OFFSET
        );
    }

    #[test]
    fn test_cp0_debug_log_buffer_queue() {
        sa::const_assert!(GsRamMemMap::CP0_DEBUG_LOG_BUFFER_QUEUE_SIZE < 0x800);
        assert_eq!(
            GsRamMemMap::CP0_DEBUG_LOG_BUFFER_QUEUE_OFFSET,
            GsRamMemMap::CP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET + 0x10,
        );
        assert_eq!(GsRamMemMap::CP0_DEBUG_LOG_BUFFER_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::CP0_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + GsRamMemMap::CP0_DEBUG_LOG_BUFFER_QUEUE_SIZE
                <= GsRamMemMap::IDE_KM_CONTEXT_OFFSET
        );
    }

    #[test]
    fn test_cp1_debug_log_descriptor_region() {
        assert_eq!(GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_SIZE_SIZE, 0x04);
        assert_eq!(GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_HEAD_SIZE, 0x04);
        assert_eq!(GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_TAIL_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::CP1_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_SIZE,
            0x04
        );

        assert_eq!(
            GsRamMemMap::CP0_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + GsRamMemMap::CP0_DEBUG_LOG_BUFFER_QUEUE_SIZE,
            GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
        );

        assert_eq!(
            GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
                - GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            0
        );
        assert_eq!(
            GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_HEAD_OFFSET
                - GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            4
        );
        assert_eq!(
            GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_TAIL_OFFSET
                - GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            8
        );
        assert_eq!(
            GsRamMemMap::CP1_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_OFFSET
                - GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            12
        );

        assert_eq!(GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET % 4, 0);
        assert_eq!(GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_HEAD_OFFSET % 4, 0);
        assert_eq!(GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_TAIL_OFFSET % 4, 0);
        assert_eq!(
            GsRamMemMap::CP1_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_OFFSET % 4,
            0
        );

        sa::const_assert!(
            GsRamMemMap::CP1_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + GsRamMemMap::CP1_DEBUG_LOG_BUFFER_QUEUE_SIZE
                <= GsRamMemMap::IDE_KM_CONTEXT_OFFSET
        );
    }

    #[test]
    fn test_cp1_debug_log_buffer_queue() {
        sa::const_assert!(GsRamMemMap::CP1_DEBUG_LOG_BUFFER_QUEUE_SIZE < 0x800);
        assert_eq!(
            GsRamMemMap::CP1_DEBUG_LOG_BUFFER_QUEUE_OFFSET,
            GsRamMemMap::CP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET + 0x10,
        );
        assert_eq!(GsRamMemMap::CP1_DEBUG_LOG_BUFFER_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::CP1_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + GsRamMemMap::CP1_DEBUG_LOG_BUFFER_QUEUE_SIZE
                <= GsRamMemMap::IDE_KM_CONTEXT_OFFSET
        );
    }

    #[test]
    fn test_ide_km_context() {
        assert_eq!(GsRamMemMap::IDE_KM_CONTEXT_SIZE, 0x2000);
        assert_eq!(
            GsRamMemMap::IDE_KM_CONTEXT_OFFSET,
            GsRamMemMap::CP1_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + GsRamMemMap::CP1_DEBUG_LOG_BUFFER_QUEUE_SIZE,
        );

        assert_eq!(GsRamMemMap::IDE_KM_CONTEXT_OFFSET % 0x1000, 0);
        sa::const_assert!(
            GsRamMemMap::IDE_KM_CONTEXT_OFFSET + GsRamMemMap::IDE_KM_CONTEXT_SIZE
                <= GsRamMemMap::AES_BULK_SELF_TEST_SRC_BUF_OFFSET
        );
    }

    #[test]
    fn test_aes_bulk_self_test_src_buf() {
        assert_eq!(GsRamMemMap::AES_BULK_SELF_TEST_SRC_BUF_SIZE, 0x200);
        assert_eq!(
            GsRamMemMap::AES_BULK_SELF_TEST_SRC_BUF_OFFSET,
            GsRamMemMap::IDE_KM_CONTEXT_OFFSET + GsRamMemMap::IDE_KM_CONTEXT_SIZE
        );

        assert_eq!(GsRamMemMap::AES_BULK_SELF_TEST_SRC_BUF_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::AES_BULK_SELF_TEST_SRC_BUF_OFFSET
                + GsRamMemMap::AES_BULK_SELF_TEST_SRC_BUF_SIZE
                <= GsRamMemMap::AES_BULK_SELF_TEST_DEST_BUF_OFFSET
        );
    }

    #[test]
    fn test_aes_bulk_self_test_dest_buf() {
        assert_eq!(GsRamMemMap::AES_BULK_SELF_TEST_DEST_BUF_SIZE, 0x200);
        assert_eq!(
            GsRamMemMap::AES_BULK_SELF_TEST_DEST_BUF_OFFSET,
            GsRamMemMap::AES_BULK_SELF_TEST_SRC_BUF_OFFSET
                + GsRamMemMap::AES_BULK_SELF_TEST_SRC_BUF_SIZE,
        );

        assert_eq!(GsRamMemMap::AES_BULK_SELF_TEST_DEST_BUF_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::AES_BULK_SELF_TEST_DEST_BUF_OFFSET
                + GsRamMemMap::AES_BULK_SELF_TEST_DEST_BUF_SIZE
                <= GsRamMemMap::RSVD_FP_RESTRICTED_OFFSET
        );
    }

    #[test]
    fn test_rsvd_fp_restricted() {
        assert_eq!(
            GsRamMemMap::RSVD_FP_RESTRICTED_SIZE,
            0x1000
                - GsRamMemMap::AES_BULK_SELF_TEST_DEST_BUF_SIZE
                - GsRamMemMap::AES_BULK_SELF_TEST_SRC_BUF_SIZE
        );
        assert_eq!(
            GsRamMemMap::RSVD_FP_RESTRICTED_OFFSET,
            GsRamMemMap::AES_BULK_SELF_TEST_DEST_BUF_OFFSET
                + GsRamMemMap::AES_BULK_SELF_TEST_DEST_BUF_SIZE,
        );

        assert_eq!(GsRamMemMap::RSVD_FP_RESTRICTED_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::RSVD_FP_RESTRICTED_OFFSET + GsRamMemMap::RSVD_FP_RESTRICTED_SIZE
                <= GsRamMemMap::DOE_BUFFER_OFFSET
        );
    }

    #[test]
    fn test_doe_buffer() {
        assert_eq!(GsRamMemMap::DOE_BUFFER_SIZE, 0x1000);
        assert_eq!(
            GsRamMemMap::DOE_BUFFER_OFFSET,
            GsRamMemMap::RSVD_FP_RESTRICTED_OFFSET + GsRamMemMap::RSVD_FP_RESTRICTED_SIZE
        );
        assert_eq!(GsRamMemMap::DOE_BUFFER_OFFSET % 0x1000, 0);
        sa::const_assert!(
            GsRamMemMap::DOE_BUFFER_OFFSET + GsRamMemMap::DOE_BUFFER_SIZE
                <= GsRamMemMap::BOOT_STATUS_OFFSET
        );
    }

    #[test]
    fn test_boot_status() {
        assert_eq!(GsRamMemMap::BOOT_STATUS_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::BOOT_STATUS_OFFSET,
            GsRamMemMap::DOE_BUFFER_OFFSET + GsRamMemMap::DOE_BUFFER_SIZE,
        );

        assert_eq!(GsRamMemMap::BOOT_STATUS_OFFSET % 0x1000, 0);
        sa::const_assert!(
            GsRamMemMap::BOOT_STATUS_OFFSET + GsRamMemMap::BOOT_STATUS_SIZE
                <= GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_admin_to_hsm_ipc_tx_queue_pi() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_PI_OFFSET,
            GsRamMemMap::BOOT_STATUS_OFFSET + GsRamMemMap::BOOT_STATUS_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_PI_SIZE
                <= GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_admin_to_hsm_ipc_tx_queue_ci() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_CI_OFFSET,
            GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_PI_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_CI_SIZE
                <= GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn admin_to_hsm_ipc_rx_queue_pi() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_PI_OFFSET,
            GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_CI_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_PI_SIZE
                <= GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn admin_to_hsm_ipc_rx_queue_ci() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_CI_OFFSET,
            GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_PI_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_CI_SIZE
                <= GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_admin_to_hsm_ipc_tx_queue() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_SIZE, 0x80);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_OFFSET,
            GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_CI_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_SIZE
                <= GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_admin_to_hsm_ipc_rx_queue() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_SIZE, 0x80);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_OFFSET,
            GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_TX_QUEUE_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_SIZE
                <= GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_admin_to_hsp_ipc_tx_queue_ci() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_CI_OFFSET,
            GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_OFFSET
                + GsRamMemMap::ADMIN_TO_HSM_IPC_RX_QUEUE_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_CI_SIZE
                <= GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_admin_to_hsp_ipc_tx_queue_pi() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_PI_OFFSET,
            GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_CI_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_PI_SIZE
                <= GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_admin_to_hsp_ipc_tx_queue() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_SIZE, 0x80);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_OFFSET,
            GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_PI_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_SIZE
                <= GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_admin_to_hsp_ipc_rx_queue_ci() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_CI_OFFSET,
            GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_TX_QUEUE_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_CI_SIZE
                <= GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_admin_to_hsp_ipc_rx_queue_pi() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_PI_OFFSET,
            GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_CI_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_PI_SIZE
                <= GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_admin_to_hsp_ipc_rx_queue() {
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_SIZE, 0x80);
        assert_eq!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_OFFSET,
            GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_PI_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_SIZE
                <= GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_hsp_ipc_tx_queue_ci() {
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_CI_OFFSET,
            GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_OFFSET
                + GsRamMemMap::ADMIN_TO_HSP_IPC_RX_QUEUE_SIZE,
        );
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_CI_SIZE
                <= GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_hsp_ipc_tx_queue_pi() {
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_PI_OFFSET,
            GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_CI_SIZE,
        );
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_PI_SIZE
                <= GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_hsp_ipc_tx_queue() {
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_SIZE, 0x80);
        assert_eq!(
            GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_OFFSET,
            GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_PI_SIZE,
        );
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_OFFSET + GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_SIZE
                <= GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_hsp_ipc_rx_queue_ci() {
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_CI_OFFSET,
            GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_OFFSET + GsRamMemMap::HSM_TO_HSP_IPC_TX_QUEUE_SIZE,
        );
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_CI_SIZE
                <= GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_hsp_ipc_rx_queue_pi() {
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_PI_OFFSET,
            GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_CI_SIZE,
        );
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_PI_SIZE
                <= GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_hsp_ipc_rx_queue() {
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_SIZE, 0x80);
        assert_eq!(
            GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_OFFSET,
            GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_PI_SIZE,
        );
        assert_eq!(GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_OFFSET + GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_ipc_tx_queue_ci() {
        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET,
            GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_OFFSET + GsRamMemMap::HSM_TO_HSP_IPC_RX_QUEUE_SIZE,
        );

        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_CI_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_ipc_tx_queue_pi() {
        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_CI_SIZE,
        );

        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_PI_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_ipc_tx_queue() {
        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_SIZE, 0x80);
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_PI_SIZE,
        );

        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_ipc_rx_queue_ci() {
        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_TX_QUEUE_SIZE,
        );

        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_CI_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_ipc_rx_queue_pi() {
        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_CI_SIZE,
        );

        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_PI_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_ipc_rx_queue() {
        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_SIZE, 0x80);
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_PI_SIZE,
        );

        assert_eq!(GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_SIZE
                <= GsRamMemMap::QUEUE_DELETE_REQ_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_queue_delete_req_queue() {
        assert_eq!(GsRamMemMap::QUEUE_DELETE_REQ_QUEUE_SIZE, 0x108);
        assert_eq!(
            GsRamMemMap::QUEUE_DELETE_REQ_QUEUE_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_IPC_RX_QUEUE_SIZE
        );

        assert_eq!(GsRamMemMap::QUEUE_DELETE_REQ_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::QUEUE_DELETE_REQ_QUEUE_OFFSET + GsRamMemMap::QUEUE_DELETE_REQ_QUEUE_SIZE
                <= GsRamMemMap::QUEUE_DELETE_REQ_CI_OFFSET
        );
    }

    #[test]
    fn test_queue_delete_req_ci() {
        assert_eq!(GsRamMemMap::QUEUE_DELETE_REQ_CI_SIZE, 0x4);
        assert_eq!(
            GsRamMemMap::QUEUE_DELETE_REQ_CI_OFFSET,
            GsRamMemMap::QUEUE_DELETE_REQ_QUEUE_OFFSET + GsRamMemMap::QUEUE_DELETE_REQ_QUEUE_SIZE
        );

        assert_eq!(GsRamMemMap::QUEUE_DELETE_REQ_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::QUEUE_DELETE_REQ_CI_OFFSET + GsRamMemMap::QUEUE_DELETE_REQ_CI_SIZE
                <= GsRamMemMap::QUEUE_DELETE_REQ_PI_OFFSET
        );
    }

    #[test]
    fn test_queue_delete_req_pi() {
        assert_eq!(GsRamMemMap::QUEUE_DELETE_REQ_PI_SIZE, 0x4);
        assert_eq!(
            GsRamMemMap::QUEUE_DELETE_REQ_PI_OFFSET,
            GsRamMemMap::QUEUE_DELETE_REQ_CI_OFFSET + GsRamMemMap::QUEUE_DELETE_REQ_CI_SIZE
        );

        assert_eq!(GsRamMemMap::QUEUE_DELETE_REQ_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::QUEUE_DELETE_REQ_PI_OFFSET + GsRamMemMap::QUEUE_DELETE_REQ_PI_SIZE
                <= GsRamMemMap::SOFT_AES_REQ_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_soft_aes_req_queue() {
        assert_eq!(
            GsRamMemMap::SOFT_AES_REQ_QUEUE_OFFSET,
            GsRamMemMap::QUEUE_DELETE_REQ_PI_OFFSET + GsRamMemMap::QUEUE_DELETE_REQ_PI_SIZE
        );

        assert_eq!(GsRamMemMap::SOFT_AES_REQ_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::SOFT_AES_REQ_QUEUE_OFFSET + GsRamMemMap::SOFT_AES_REQ_QUEUE_SIZE
                <= GsRamMemMap::SOFT_AES_REQ_CI_OFFSET
        );
    }

    #[test]
    fn test_soft_aes_req_ci() {
        assert_eq!(GsRamMemMap::SOFT_AES_REQ_CI_SIZE, 0x4);
        assert_eq!(
            GsRamMemMap::SOFT_AES_REQ_CI_OFFSET,
            GsRamMemMap::SOFT_AES_REQ_QUEUE_OFFSET + GsRamMemMap::SOFT_AES_REQ_QUEUE_SIZE
        );

        assert_eq!(GsRamMemMap::SOFT_AES_REQ_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::SOFT_AES_REQ_CI_OFFSET + GsRamMemMap::SOFT_AES_REQ_CI_SIZE
                <= GsRamMemMap::SOFT_AES_REQ_PI_OFFSET
        );
    }

    #[test]
    fn test_soft_aes_req_pi() {
        assert_eq!(GsRamMemMap::SOFT_AES_REQ_PI_SIZE, 0x4);
        assert_eq!(
            GsRamMemMap::SOFT_AES_REQ_PI_OFFSET,
            GsRamMemMap::SOFT_AES_REQ_CI_OFFSET + GsRamMemMap::SOFT_AES_REQ_CI_SIZE
        );

        assert_eq!(GsRamMemMap::SOFT_AES_REQ_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::SOFT_AES_REQ_PI_OFFSET + GsRamMemMap::SOFT_AES_REQ_PI_SIZE
                <= GsRamMemMap::SOFT_AES_RESP_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_soft_aes_resp_queue() {
        assert_eq!(
            GsRamMemMap::SOFT_AES_RESP_QUEUE_OFFSET,
            GsRamMemMap::SOFT_AES_REQ_PI_OFFSET + GsRamMemMap::SOFT_AES_REQ_PI_SIZE
        );

        assert_eq!(GsRamMemMap::SOFT_AES_RESP_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::SOFT_AES_RESP_QUEUE_OFFSET + GsRamMemMap::SOFT_AES_RESP_QUEUE_SIZE
                <= GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_admin_ipc_tx_queue_ci() {
        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_CI_SIZE, 0x4);
        assert_eq!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET,
            GsRamMemMap::SOFT_AES_RESP_QUEUE_OFFSET + GsRamMemMap::SOFT_AES_RESP_QUEUE_SIZE,
        );

        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_CI_SIZE
                <= GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_admin_ipc_tx_queue_pi() {
        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_PI_SIZE, 0x4);
        assert_eq!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET,
            GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_CI_SIZE,
        );

        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_PI_SIZE
                <= GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_admin_ipc_tx_queue() {
        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_SIZE, 0x80);
        assert_eq!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_OFFSET,
            GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_PI_SIZE,
        );

        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_SIZE
                <= GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_admin_ipc_rx_queue_ci() {
        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_CI_SIZE, 0x4);
        assert_eq!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET,
            GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_TX_QUEUE_SIZE,
        );

        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_CI_SIZE
                <= GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_admin_ipc_rx_queue_pi() {
        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_PI_SIZE, 0x4);
        assert_eq!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET,
            GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_CI_SIZE,
        );

        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_PI_SIZE
                <= GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsm_to_admin_ipc_rx_queue() {
        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_SIZE, 0x80);
        assert_eq!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_OFFSET,
            GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_PI_SIZE,
        );

        assert_eq!(GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_stop_interface_ipc_tx_queue_ci() {
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI_SIZE,
            0x04
        );
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI_OFFSET,
            GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_OFFSET
                + GsRamMemMap::HSM_TO_ADMIN_IPC_RX_QUEUE_SIZE,
        );

        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI_OFFSET % 4,
            0
        );
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_stop_interface_ipc_tx_queue_pi() {
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_PI_SIZE,
            0x04
        );
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_PI_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI_SIZE,
        );

        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_PI_OFFSET % 4,
            0
        );
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_PI_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_stop_interface_ipc_tx_queue() {
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_SIZE,
            0x80
        );
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_PI_SIZE,
        );

        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_OFFSET % 4,
            0
        );
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_CI_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_stop_interface_ipc_rx_queue_ci() {
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_CI_SIZE,
            0x04
        );
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_CI_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_SIZE,
        );

        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_CI_OFFSET % 4,
            0
        );
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_TX_QUEUE_CI_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_PI_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_stop_interface_ipc_rx_queue_pi() {
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_PI_SIZE,
            0x04
        );
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_PI_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_CI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_CI_SIZE,
        );

        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_PI_OFFSET % 4,
            0
        );
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_PI_SIZE
                <= GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsp_to_admin_stop_interface_ipc_rx_queue() {
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_SIZE,
            0x80
        );
        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_PI_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_PI_SIZE,
        );

        assert_eq!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_OFFSET % 4,
            0
        );
        sa::const_assert!(
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_SIZE
                <= GsRamMemMap::HSP_ADMIN_HSM_SHARED_IPC_MEMORY_FREE_SPACE_OFFSET
        );
    }

    #[test]
    fn test_hsp_admin_hsm_shared_ipc_memory_free_space() {
        assert_eq!(
            GsRamMemMap::HSP_ADMIN_HSM_SHARED_IPC_MEMORY_FREE_SPACE_SIZE,
            0xF3C
        );
        assert_eq!(
            GsRamMemMap::HSP_ADMIN_HSM_SHARED_IPC_MEMORY_FREE_SPACE_OFFSET,
            GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_OFFSET
                + GsRamMemMap::HSP_TO_ADMIN_STOP_INTERFACE_IPC_RX_QUEUE_SIZE,
        );

        assert_eq!(
            GsRamMemMap::HSP_ADMIN_HSM_SHARED_IPC_MEMORY_FREE_SPACE_OFFSET % 4,
            0
        );
        sa::const_assert!(
            GsRamMemMap::HSP_ADMIN_HSM_SHARED_IPC_MEMORY_FREE_SPACE_OFFSET
                + GsRamMemMap::HSP_ADMIN_HSM_SHARED_IPC_MEMORY_FREE_SPACE_SIZE
                <= GsRamMemMap::ADMIN_RX_QUEUE_SHADOW_PI_OFFSET
        );
    }

    #[test]
    fn test_admin_rx_queue_shadow_pi() {
        let alignment = 0x1000;
        assert_eq!(GsRamMemMap::ADMIN_RX_QUEUE_SHADOW_PI_SIZE, 0x04);
        assert_eq!(GsRamMemMap::ADMIN_RX_QUEUE_SHADOW_PI_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_RX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::ADMIN_RX_QUEUE_SHADOW_PI_SIZE
                <= GsRamMemMap::ADMIN_TX_QUEUE_SHADOW_PI_OFFSET
        );
    }

    #[test]
    fn test_admin_tx_queue_shadow_pi() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::ADMIN_TX_QUEUE_SHADOW_PI_SIZE, 0x04);
        assert_eq!(
            GsRamMemMap::ADMIN_TX_QUEUE_SHADOW_PI_OFFSET,
            (GsRamMemMap::ADMIN_RX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::ADMIN_RX_QUEUE_SHADOW_PI_SIZE
                - 1
                + alignment)
                & !(alignment - 1)
        );
        assert_eq!(GsRamMemMap::ADMIN_TX_QUEUE_SHADOW_PI_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::ADMIN_TX_QUEUE_SHADOW_PI_SIZE
                <= GsRamMemMap::ADMIN_RX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_admin_rx_queue() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::ADMIN_RX_QUEUE_SIZE, 0x800);
        assert_eq!(
            GsRamMemMap::ADMIN_RX_QUEUE_OFFSET,
            (GsRamMemMap::ADMIN_TX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::ADMIN_TX_QUEUE_SHADOW_PI_SIZE
                - 1
                + alignment)
                & !(alignment - 1)
        );
        assert_eq!(GsRamMemMap::ADMIN_RX_QUEUE_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_RX_QUEUE_OFFSET + GsRamMemMap::ADMIN_RX_QUEUE_SIZE
                <= GsRamMemMap::ADMIN_RX_FREE_LIST_OFFSET
        );
    }

    #[test]
    fn test_admin_rx_free_list() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::ADMIN_RX_FREE_LIST_SIZE, 0x400);
        assert_eq!(
            GsRamMemMap::ADMIN_RX_FREE_LIST_OFFSET,
            (GsRamMemMap::ADMIN_RX_QUEUE_OFFSET + GsRamMemMap::ADMIN_RX_QUEUE_SIZE - 1 + alignment)
                & !(alignment - 1),
        );
        assert_eq!(GsRamMemMap::ADMIN_RX_FREE_LIST_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_RX_FREE_LIST_OFFSET + GsRamMemMap::ADMIN_RX_FREE_LIST_SIZE
                <= GsRamMemMap::ADMIN_SQE_POOL_OFFSET
        );
    }

    #[test]
    fn test_admin_sqe_pool() {
        assert_eq!(GsRamMemMap::ADMIN_SQE_POOL_SIZE, 0x2000);
        assert_eq!(
            GsRamMemMap::ADMIN_SQE_POOL_OFFSET,
            GsRamMemMap::ADMIN_RX_FREE_LIST_OFFSET + GsRamMemMap::ADMIN_RX_FREE_LIST_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_SQE_POOL_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_SQE_POOL_OFFSET + GsRamMemMap::ADMIN_SQE_POOL_SIZE
                <= GsRamMemMap::ADMIN_TX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_admin_tx_queue() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::ADMIN_TX_QUEUE_SIZE, 0x800);
        assert_eq!(
            GsRamMemMap::ADMIN_TX_QUEUE_OFFSET,
            (GsRamMemMap::ADMIN_SQE_POOL_OFFSET + GsRamMemMap::ADMIN_SQE_POOL_SIZE - 1 + alignment)
                & !(alignment - 1),
        );
        assert_eq!(GsRamMemMap::ADMIN_TX_QUEUE_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TX_QUEUE_OFFSET + GsRamMemMap::ADMIN_TX_QUEUE_SIZE
                <= GsRamMemMap::ADMIN_TX_FREE_LIST_OFFSET
        );
    }

    #[test]
    fn test_admin_tx_free_list() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::ADMIN_TX_FREE_LIST_SIZE, 0x800);
        assert_eq!(
            GsRamMemMap::ADMIN_TX_FREE_LIST_OFFSET,
            (GsRamMemMap::ADMIN_TX_QUEUE_OFFSET + GsRamMemMap::ADMIN_TX_QUEUE_SIZE - 1 + alignment)
                & !(alignment - 1),
        );
        assert_eq!(GsRamMemMap::ADMIN_TX_FREE_LIST_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_TX_FREE_LIST_OFFSET + GsRamMemMap::ADMIN_TX_FREE_LIST_SIZE
                <= GsRamMemMap::ADMIN_CQE_POOL_OFFSET
        );
    }

    #[test]
    fn test_admin_cqe_pool() {
        assert_eq!(GsRamMemMap::ADMIN_CQE_POOL_SIZE, 0x800);
        assert_eq!(
            GsRamMemMap::ADMIN_CQE_POOL_OFFSET,
            GsRamMemMap::ADMIN_TX_FREE_LIST_OFFSET + GsRamMemMap::ADMIN_TX_FREE_LIST_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_CQE_POOL_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_CQE_POOL_OFFSET + GsRamMemMap::ADMIN_CQE_POOL_SIZE
                <= GsRamMemMap::HSM_RX_QUEUE_SHADOW_PI_OFFSET
        );
    }

    #[test]
    fn test_hsm_rx_queue_shadow_pi() {
        assert_eq!(GsRamMemMap::HSM_RX_QUEUE_SHADOW_PI_SIZE, 0x4);
        assert_eq!(
            GsRamMemMap::HSM_RX_QUEUE_SHADOW_PI_OFFSET,
            GsRamMemMap::ADMIN_CQE_POOL_OFFSET + GsRamMemMap::ADMIN_CQE_POOL_SIZE,
        );
        assert_eq!(GsRamMemMap::HSM_RX_QUEUE_SHADOW_PI_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_RX_QUEUE_SHADOW_PI_OFFSET + GsRamMemMap::HSM_RX_QUEUE_SHADOW_PI_SIZE
                <= GsRamMemMap::HSM_TX_QUEUE_SHADOW_PI_OFFSET
        );
    }

    #[test]
    fn test_hsm_tx_queue_shadow_pi() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::HSM_TX_QUEUE_SHADOW_PI_SIZE, 0x4);
        assert_eq!(
            GsRamMemMap::HSM_TX_QUEUE_SHADOW_PI_OFFSET,
            (GsRamMemMap::HSM_RX_QUEUE_SHADOW_PI_OFFSET + GsRamMemMap::HSM_RX_QUEUE_SHADOW_PI_SIZE
                - 1
                + alignment)
                & !(alignment - 1)
        );
        assert_eq!(GsRamMemMap::HSM_TX_QUEUE_SHADOW_PI_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TX_QUEUE_SHADOW_PI_OFFSET + GsRamMemMap::HSM_TX_QUEUE_SHADOW_PI_SIZE
                <= GsRamMemMap::HSM_RX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsm_rx_queue() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::HSM_RX_QUEUE_SIZE, 0x200);
        assert_eq!(
            GsRamMemMap::HSM_RX_QUEUE_OFFSET,
            (GsRamMemMap::HSM_TX_QUEUE_SHADOW_PI_OFFSET + GsRamMemMap::HSM_TX_QUEUE_SHADOW_PI_SIZE
                - 1
                + alignment)
                & !(alignment - 1)
        );
        assert_eq!(GsRamMemMap::HSM_RX_QUEUE_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_RX_QUEUE_OFFSET + GsRamMemMap::HSM_RX_QUEUE_SIZE
                <= GsRamMemMap::HSM_RX_FREE_LIST_OFFSET
        );
    }

    #[test]
    fn test_hsm_rx_free_list() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::HSM_RX_FREE_LIST_SIZE, 0x100);
        assert_eq!(
            GsRamMemMap::HSM_RX_FREE_LIST_OFFSET,
            (GsRamMemMap::HSM_RX_QUEUE_OFFSET + GsRamMemMap::HSM_RX_QUEUE_SIZE - 1 + alignment)
                & !(alignment - 1),
        );
        assert_eq!(GsRamMemMap::HSM_RX_FREE_LIST_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_RX_FREE_LIST_OFFSET + GsRamMemMap::HSM_RX_FREE_LIST_SIZE
                <= GsRamMemMap::HSM_SQE_POOL_OFFSET
        );
    }

    #[test]
    fn test_hsm_sqe_pool() {
        assert_eq!(GsRamMemMap::HSM_SQE_POOL_SIZE, 0x800);
        assert_eq!(
            GsRamMemMap::HSM_SQE_POOL_OFFSET,
            GsRamMemMap::HSM_RX_FREE_LIST_OFFSET + GsRamMemMap::HSM_RX_FREE_LIST_SIZE,
        );
        assert_eq!(GsRamMemMap::HSM_SQE_POOL_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_SQE_POOL_OFFSET + GsRamMemMap::HSM_SQE_POOL_SIZE
                <= GsRamMemMap::HSM_TX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsm_tx_queue() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::HSM_TX_QUEUE_SIZE, 0x200);
        assert_eq!(
            GsRamMemMap::HSM_TX_QUEUE_OFFSET,
            (GsRamMemMap::HSM_SQE_POOL_OFFSET + GsRamMemMap::HSM_SQE_POOL_SIZE - 1 + alignment)
                & !(alignment - 1),
        );
        assert_eq!(GsRamMemMap::HSM_TX_QUEUE_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TX_QUEUE_OFFSET + GsRamMemMap::HSM_TX_QUEUE_SIZE
                <= GsRamMemMap::HSM_TX_FREE_LIST_OFFSET
        );
    }

    #[test]
    fn test_hsm_tx_free_list() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::HSM_TX_FREE_LIST_SIZE, 0x200);
        assert_eq!(
            GsRamMemMap::HSM_TX_FREE_LIST_OFFSET,
            (GsRamMemMap::HSM_TX_QUEUE_OFFSET + GsRamMemMap::HSM_TX_QUEUE_SIZE - 1 + alignment)
                & !(alignment - 1),
        );
        assert_eq!(GsRamMemMap::HSM_TX_FREE_LIST_OFFSET % alignment, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_TX_FREE_LIST_OFFSET + GsRamMemMap::HSM_TX_FREE_LIST_SIZE
                <= GsRamMemMap::HSM_CQE_POOL_OFFSET
        );
    }

    #[test]
    fn test_hsm_cqe_pool() {
        assert_eq!(GsRamMemMap::HSM_CQE_POOL_SIZE, 0x200);
        assert_eq!(
            GsRamMemMap::HSM_CQE_POOL_OFFSET,
            GsRamMemMap::HSM_TX_FREE_LIST_OFFSET + GsRamMemMap::HSM_TX_FREE_LIST_SIZE,
        );
        assert_eq!(GsRamMemMap::HSM_CQE_POOL_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_CQE_POOL_OFFSET + GsRamMemMap::HSM_CQE_POOL_SIZE
                <= GsRamMemMap::ADMIN_HSM_UCD_FREE_SPACE_OFFSET
        );
    }

    #[test]
    fn test_admin_hsm_ucd_free_space() {
        assert_eq!(GsRamMemMap::ADMIN_HSM_UCD_FREE_SPACE_SIZE, 0x780);
        assert_eq!(
            GsRamMemMap::ADMIN_HSM_UCD_FREE_SPACE_OFFSET,
            GsRamMemMap::HSM_CQE_POOL_OFFSET + GsRamMemMap::HSM_CQE_POOL_SIZE,
        );
        assert_eq!(GsRamMemMap::ADMIN_HSM_UCD_FREE_SPACE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_HSM_UCD_FREE_SPACE_OFFSET
                + GsRamMemMap::ADMIN_HSM_UCD_FREE_SPACE_SIZE
                <= GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SHADOW_PI_OFFSET
        );
    }

    #[test]
    fn test_admin_gdma_rx_queue_shadow_pi() {
        let alignment = 0x1000;
        assert_eq!(GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SHADOW_PI_SIZE, 4);

        assert_eq!(
            GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SHADOW_PI_OFFSET,
            (GsRamMemMap::ADMIN_HSM_UCD_FREE_SPACE_OFFSET
                + GsRamMemMap::ADMIN_HSM_UCD_FREE_SPACE_SIZE
                - 1
                + alignment)
                & !(alignment - 1)
        );
        assert_eq!(
            GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SHADOW_PI_OFFSET % alignment,
            0
        );
        sa::const_assert!(
            GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SHADOW_PI_SIZE
                <= GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SHADOW_PI_OFFSET
        );
    }

    #[test]
    fn test_admin_gdma_tx_queue_shadow_pi() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SHADOW_PI_SIZE, 4);
        assert_eq!(
            GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SHADOW_PI_OFFSET,
            (GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SHADOW_PI_SIZE
                - 1
                + alignment)
                & !(alignment - 1)
        );
        assert_eq!(
            GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SHADOW_PI_OFFSET % alignment,
            0
        );
        sa::const_assert!(
            GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SHADOW_PI_SIZE
                <= GsRamMemMap::ADMIN_GDMA_RX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_admin_gdma_rx_queue() {
        assert_eq!(GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SIZE, 0x800);
        assert_eq!(
            GsRamMemMap::ADMIN_GDMA_RX_QUEUE_OFFSET,
            (GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SHADOW_PI_SIZE)
        );
        assert_eq!(GsRamMemMap::ADMIN_GDMA_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_GDMA_RX_QUEUE_OFFSET + GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SIZE
                <= GsRamMemMap::ADMIN_GDMA_TX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_admin_gdma_tx_queue() {
        assert_eq!(GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SIZE, 0x2000);
        assert_eq!(
            GsRamMemMap::ADMIN_GDMA_TX_QUEUE_OFFSET,
            (GsRamMemMap::ADMIN_GDMA_RX_QUEUE_OFFSET + GsRamMemMap::ADMIN_GDMA_RX_QUEUE_SIZE)
        );
        assert_eq!(GsRamMemMap::ADMIN_GDMA_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_GDMA_TX_QUEUE_OFFSET + GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SIZE
                <= GsRamMemMap::HSM_GDMA_RX_QUEUE_SHADOW_PI_OFFSET
        );
    }

    #[test]
    fn test_hsm_gdma_rx_queue_shadow_pi() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::HSM_GDMA_RX_QUEUE_SHADOW_PI_SIZE, 4);
        assert_eq!(
            GsRamMemMap::HSM_GDMA_RX_QUEUE_SHADOW_PI_OFFSET,
            (GsRamMemMap::ADMIN_GDMA_TX_QUEUE_OFFSET + GsRamMemMap::ADMIN_GDMA_TX_QUEUE_SIZE - 1
                + alignment)
                & !(alignment - 1)
        );
        assert_eq!(
            GsRamMemMap::HSM_GDMA_RX_QUEUE_SHADOW_PI_OFFSET % alignment,
            0
        );
        sa::const_assert!(
            GsRamMemMap::HSM_GDMA_RX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::HSM_GDMA_RX_QUEUE_SHADOW_PI_SIZE
                <= GsRamMemMap::HSM_GDMA_TX_QUEUE_SHADOW_PI_OFFSET
        );
    }

    #[test]
    fn test_hsm_gdma_tx_queue_shadow_pi() {
        let alignment = 0x20;
        assert_eq!(GsRamMemMap::HSM_GDMA_TX_QUEUE_SHADOW_PI_SIZE, 4);
        assert_eq!(
            GsRamMemMap::HSM_GDMA_TX_QUEUE_SHADOW_PI_OFFSET,
            (GsRamMemMap::HSM_GDMA_RX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::HSM_GDMA_RX_QUEUE_SHADOW_PI_SIZE
                - 1
                + alignment)
                & !(alignment - 1)
        );
        assert_eq!(
            GsRamMemMap::HSM_GDMA_TX_QUEUE_SHADOW_PI_OFFSET % alignment,
            0
        );
        sa::const_assert!(
            GsRamMemMap::HSM_GDMA_TX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::HSM_GDMA_TX_QUEUE_SHADOW_PI_SIZE
                <= GsRamMemMap::HSM_GDMA_RX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsm_gdma_rx_queue() {
        assert_eq!(GsRamMemMap::HSM_GDMA_RX_QUEUE_SIZE, 0x800);
        assert_eq!(
            GsRamMemMap::HSM_GDMA_RX_QUEUE_OFFSET,
            (GsRamMemMap::HSM_GDMA_TX_QUEUE_SHADOW_PI_OFFSET
                + GsRamMemMap::HSM_GDMA_TX_QUEUE_SHADOW_PI_SIZE)
        );
        assert_eq!(GsRamMemMap::HSM_GDMA_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_GDMA_RX_QUEUE_OFFSET + GsRamMemMap::HSM_GDMA_RX_QUEUE_SIZE
                <= GsRamMemMap::HSM_GDMA_TX_QUEUE_OFFSET
        );
    }

    #[test]
    fn test_hsm_gdma_tx_queue() {
        assert_eq!(GsRamMemMap::HSM_GDMA_TX_QUEUE_SIZE, 0x2000);
        assert_eq!(
            GsRamMemMap::HSM_GDMA_TX_QUEUE_OFFSET,
            (GsRamMemMap::HSM_GDMA_RX_QUEUE_OFFSET + GsRamMemMap::HSM_GDMA_RX_QUEUE_SIZE)
        );
        assert_eq!(GsRamMemMap::HSM_GDMA_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_GDMA_TX_QUEUE_OFFSET + GsRamMemMap::HSM_GDMA_TX_QUEUE_SIZE
                <= GsRamMemMap::ADMIN_HEAP_OFFSET
        );
    }

    #[test]
    fn test_aes_cmd_buffer() {
        assert_eq!(GsRamMemMap::AES_CMD_BUFFER_SIZE, 0x18);
        assert_eq!(
            GsRamMemMap::AES_CMD_BUFFER_OFFSET,
            GsRamMemMap::HSM_GDMA_TX_QUEUE_OFFSET + GsRamMemMap::HSM_GDMA_TX_QUEUE_SIZE
        );
        assert_eq!(GsRamMemMap::AES_CMD_BUFFER_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::AES_CMD_BUFFER_OFFSET + GsRamMemMap::AES_CMD_BUFFER_SIZE
                <= GsRamMemMap::PKA_CMD_BUFFER_OFFSET
        );
    }

    #[test]
    fn test_pka_cmd_buffer() {
        assert_eq!(GsRamMemMap::PKA_CMD_BUFFER_SIZE, 0x140);
        assert_eq!(
            GsRamMemMap::PKA_CMD_BUFFER_OFFSET,
            GsRamMemMap::AES_CMD_BUFFER_OFFSET + GsRamMemMap::AES_CMD_BUFFER_SIZE
        );
        assert_eq!(GsRamMemMap::PKA_CMD_BUFFER_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::PKA_CMD_BUFFER_OFFSET + GsRamMemMap::PKA_CMD_BUFFER_SIZE
                <= GsRamMemMap::PKA_OUTPUT_BUFFER_OFFSET
        );
    }

    #[test]
    fn test_pka_output_buffer() {
        assert_eq!(GsRamMemMap::PKA_OUTPUT_BUFFER_SIZE, 0x2040);
        assert_eq!(
            GsRamMemMap::PKA_OUTPUT_BUFFER_OFFSET,
            GsRamMemMap::PKA_CMD_BUFFER_OFFSET + GsRamMemMap::PKA_CMD_BUFFER_SIZE
        );
        assert_eq!(GsRamMemMap::PKA_OUTPUT_BUFFER_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::PKA_OUTPUT_BUFFER_OFFSET + GsRamMemMap::PKA_OUTPUT_BUFFER_SIZE
                <= GsRamMemMap::PKA_INPUT_BUFFER_OFFSET
        );
    }

    #[test]
    fn test_pka_input_buffer() {
        assert_eq!(GsRamMemMap::PKA_INPUT_BUFFER_SIZE, 0x4080);
        assert_eq!(
            GsRamMemMap::PKA_INPUT_BUFFER_OFFSET,
            GsRamMemMap::PKA_OUTPUT_BUFFER_OFFSET + GsRamMemMap::PKA_OUTPUT_BUFFER_SIZE
        );
        assert_eq!(GsRamMemMap::PKA_INPUT_BUFFER_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::PKA_INPUT_BUFFER_OFFSET + GsRamMemMap::PKA_INPUT_BUFFER_SIZE
                <= GsRamMemMap::PKA_ECC_CONST_BUFFER_OFFSET
        );
    }

    #[test]
    fn test_pka_buffer_sizes() {
        assert_eq!(GsRamMemMap::pka_cmd_buffer().len(), 16);
        assert_eq!(GsRamMemMap::pka_output_buffer().len(), 16);
        assert_eq!(GsRamMemMap::pka_input_buffer().len(), 16);
    }

    #[test]
    fn test_pka_ecc_const_buffer() {
        assert_eq!(GsRamMemMap::PKA_ECC_CONST_BUFFER_SIZE, 0x1BC);
        assert_eq!(
            GsRamMemMap::PKA_ECC_CONST_BUFFER_OFFSET,
            GsRamMemMap::PKA_INPUT_BUFFER_OFFSET + GsRamMemMap::PKA_INPUT_BUFFER_SIZE
        );
        assert_eq!(GsRamMemMap::PKA_ECC_CONST_BUFFER_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::PKA_ECC_CONST_BUFFER_OFFSET + GsRamMemMap::PKA_ECC_CONST_BUFFER_SIZE
                <= GsRamMemMap::PKA_SELF_TEST_BUFFER_OFFSET
        );
    }

    #[test]
    fn test_pka_self_test_buffer() {
        assert_eq!(GsRamMemMap::PKA_SELF_TEST_BUFFER_SIZE, 0x600);
        assert_eq!(
            GsRamMemMap::PKA_SELF_TEST_BUFFER_OFFSET,
            GsRamMemMap::PKA_ECC_CONST_BUFFER_OFFSET + GsRamMemMap::PKA_ECC_CONST_BUFFER_SIZE
        );
        assert_eq!(GsRamMemMap::PKA_SELF_TEST_BUFFER_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::PKA_SELF_TEST_BUFFER_OFFSET + GsRamMemMap::PKA_SELF_TEST_BUFFER_SIZE
                <= GsRamMemMap::SHA_CMD_BUFFER_OFFSET
        );
    }

    #[test]
    fn test_sha_cmd_buffer() {
        assert_eq!(GsRamMemMap::SHA_CMD_BUFFER_SIZE, 0x20);
        assert_eq!(
            GsRamMemMap::SHA_CMD_BUFFER_OFFSET,
            GsRamMemMap::PKA_SELF_TEST_BUFFER_OFFSET + GsRamMemMap::PKA_SELF_TEST_BUFFER_SIZE
        );
        assert_eq!(GsRamMemMap::SHA_CMD_BUFFER_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::SHA_CMD_BUFFER_OFFSET + GsRamMemMap::SHA_CMD_BUFFER_SIZE
                <= GsRamMemMap::SHA_INIT_DIGEST_BUFFER_OFFSET
        );
    }

    #[test]
    fn test_sha_init_digest_buffer() {
        assert_eq!(GsRamMemMap::SHA_INIT_DIGEST_BUFFER_SIZE, 0x40);
        assert_eq!(
            GsRamMemMap::SHA_INIT_DIGEST_BUFFER_OFFSET,
            GsRamMemMap::SHA_CMD_BUFFER_OFFSET + GsRamMemMap::SHA_CMD_BUFFER_SIZE
        );
        assert_eq!(GsRamMemMap::SHA_INIT_DIGEST_BUFFER_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::SHA_INIT_DIGEST_BUFFER_OFFSET + GsRamMemMap::SHA_INIT_DIGEST_BUFFER_SIZE
                <= GsRamMemMap::SHA_OUT_BUFFER_OFFSET
        );
    }

    #[test]
    fn test_sha_out_buffer() {
        assert_eq!(GsRamMemMap::SHA_OUT_BUFFER_SIZE, 0x40);
        assert_eq!(
            GsRamMemMap::SHA_OUT_BUFFER_OFFSET,
            GsRamMemMap::SHA_INIT_DIGEST_BUFFER_OFFSET + GsRamMemMap::SHA_INIT_DIGEST_BUFFER_SIZE
        );
        assert_eq!(GsRamMemMap::SHA_OUT_BUFFER_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::SHA_OUT_BUFFER_OFFSET + GsRamMemMap::SHA_OUT_BUFFER_SIZE
                <= GsRamMemMap::SHA_SELF_TEST_BUFFER_OFFSET
        );
    }

    #[test]
    fn test_sha_self_test_buffer() {
        assert_eq!(GsRamMemMap::SHA_SELF_TEST_BUFFER_SIZE, 0x180);
        assert_eq!(
            GsRamMemMap::SHA_SELF_TEST_BUFFER_OFFSET,
            GsRamMemMap::SHA_OUT_BUFFER_OFFSET + GsRamMemMap::SHA_OUT_BUFFER_SIZE
        );
        assert_eq!(GsRamMemMap::SHA_SELF_TEST_BUFFER_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::SHA_SELF_TEST_BUFFER_OFFSET + GsRamMemMap::SHA_SELF_TEST_BUFFER_SIZE
                <= GsRamMemMap::ADMIN_HEAP_OFFSET
        );
    }

    #[test]
    fn test_admin_heap() {
        assert_eq!(GsRamMemMap::ADMIN_HEAP_SIZE, 0x5000);
        assert_eq!(
            GsRamMemMap::ADMIN_HEAP_OFFSET,
            GsRamMemMap::SHA_SELF_TEST_BUFFER_OFFSET + GsRamMemMap::SHA_SELF_TEST_BUFFER_SIZE
        );
        assert_eq!(GsRamMemMap::ADMIN_HEAP_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::ADMIN_HEAP_OFFSET + GsRamMemMap::ADMIN_HEAP_SIZE
                <= GsRamMemMap::HSM_HEAP_OFFSET
        );
    }

    #[test]
    fn test_hsm_heap() {
        assert_eq!(GsRamMemMap::HSM_HEAP_SIZE, 0x98000);
        assert_eq!(
            GsRamMemMap::HSM_HEAP_OFFSET,
            (GsRamMemMap::ADMIN_HEAP_OFFSET + GsRamMemMap::ADMIN_HEAP_SIZE)
        );

        assert_eq!(GsRamMemMap::HSM_HEAP_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_HEAP_OFFSET + GsRamMemMap::HSM_HEAP_SIZE
                <= GsRamMemMap::ADMIN_HSM_DMA_FREE_SPACE_OFFSET
        );
    }

    #[test]
    fn test_admin_hsm_dma_free_space() {
        assert_eq!(GsRamMemMap::ADMIN_HSM_DMA_FREE_SPACE_SIZE, 0x3A8);
        assert_eq!(
            GsRamMemMap::ADMIN_HSM_DMA_FREE_SPACE_OFFSET,
            GsRamMemMap::HSM_HEAP_OFFSET + GsRamMemMap::HSM_HEAP_SIZE
        );

        assert_eq!(GsRamMemMap::ADMIN_HSM_DMA_FREE_SPACE_OFFSET % 4, 0);
    }

    #[test]
    fn test_cdma_key_vault_meta_data() {
        assert_eq!(GsRamMemMap::CDMA_KEY_VAULT_META_DATA_SIZE, 0x41);
        assert_eq!(
            GsRamMemMap::CDMA_KEY_VAULT_META_DATA_OFFSET,
            GsRamMemMap::BASE_ADDRESS + 0xB8000,
        );

        sa::const_assert!(
            GsRamMemMap::CDMA_KEY_VAULT_META_DATA_OFFSET
                + GsRamMemMap::CDMA_KEY_VAULT_META_DATA_SIZE
                <= GsRamMemMap::HSM_PARTITION_TABLE_OFFSET
        );
    }

    #[test]
    fn test_hsm_partition_table() {
        assert_eq!(GsRamMemMap::HSM_PARTITION_TABLE_SIZE, 0x1548);
        assert_eq!(
            GsRamMemMap::HSM_PARTITION_TABLE_OFFSET,
            GsRamMemMap::CDMA_KEY_VAULT_META_DATA_OFFSET
                + GsRamMemMap::CDMA_KEY_VAULT_META_DATA_SIZE
                + 3,
        );

        assert_eq!(GsRamMemMap::HSM_PARTITION_TABLE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_PARTITION_TABLE_OFFSET + GsRamMemMap::HSM_PARTITION_TABLE_SIZE
                <= GsRamMemMap::KEY_VAULT_CMN_META_DATA_OFFSET
        );
    }

    #[test]
    fn test_cmn_key_vault_cmn_meta_data() {
        assert_eq!(GsRamMemMap::KEY_VAULT_CMN_META_DATA_SIZE, 6772);
        assert_eq!(
            GsRamMemMap::KEY_VAULT_CMN_META_DATA_OFFSET,
            GsRamMemMap::HSM_PARTITION_TABLE_OFFSET + GsRamMemMap::HSM_PARTITION_TABLE_SIZE,
        );
        assert_eq!(GsRamMemMap::KEY_VAULT_CMN_META_DATA_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::KEY_VAULT_CMN_META_DATA_OFFSET + GsRamMemMap::KEY_VAULT_CMN_META_DATA_SIZE
                <= GsRamMemMap::HSM_PART_PERSISTENT_STORE_OFFSET
        );
    }

    #[test]
    fn test_hsm_part_persistent_store() {
        assert_eq!(GsRamMemMap::HSM_PART_PERSISTENT_STORE_SIZE, 65 * 3072);
        assert_eq!(
            GsRamMemMap::HSM_PART_PERSISTENT_STORE_OFFSET,
            GsRamMemMap::KEY_VAULT_CMN_META_DATA_OFFSET + GsRamMemMap::KEY_VAULT_CMN_META_DATA_SIZE
        );

        assert_eq!(GsRamMemMap::HSM_PART_PERSISTENT_STORE_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::HSM_PART_PERSISTENT_STORE_OFFSET
                + GsRamMemMap::HSM_PART_PERSISTENT_STORE_SIZE
                <= GsRamMemMap::KEY_VAULT_OFFSET
        );
    }

    #[test]
    fn test_key_vault() {
        assert_eq!(GsRamMemMap::KEY_VAULT_SIZE, 65 * 17 * 1024);
        assert_eq!(
            GsRamMemMap::KEY_VAULT_OFFSET,
            GsRamMemMap::HSM_PART_PERSISTENT_STORE_OFFSET
                + GsRamMemMap::HSM_PART_PERSISTENT_STORE_SIZE,
        );
        assert_eq!(
            GsRamMemMap::KEY_VAULT_OFFSET + GsRamMemMap::KEY_VAULT_SIZE,
            GsRamMemMap::BASE_ADDRESS + GsRamMemMap::LENGTH,
        );
        assert_eq!(GsRamMemMap::KEY_VAULT_OFFSET % 4, 0);
        sa::const_assert!(
            GsRamMemMap::KEY_VAULT_OFFSET + GsRamMemMap::KEY_VAULT_SIZE
                <= GsRamMemMap::BASE_ADDRESS + GsRamMemMap::LENGTH
        );
    }
}
