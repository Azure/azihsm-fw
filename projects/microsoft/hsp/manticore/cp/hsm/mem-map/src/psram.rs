// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ipc_controller::IpcMessage;
use mcr_mem_map_derive::mem_map;
use mcr_types::CdmaIoCqe;
use mcr_types::CdmaIoSqe;

/// PS-RAM Memory Map
#[mem_map(address = 0xA3E0_0000, length = 0x8000)]
pub struct PsRamMemMap {
    /// Admin(Requestor) to FP(Responder) IPC transmit queue
    #[field(cardinality = 16, mutable = true)]
    admin_to_fp_ipc_tx_queue: IpcMessage,

    /// Admin(Requestor) to FP(Responder) IPC receive queue
    #[field(cardinality = 16, mutable = true)]
    admin_to_fp_ipc_rx_queue: IpcMessage,

    /// Reserved, see M7MemMap.h
    #[field(cardinality = 384)]
    psram_rsvd1: u8,

    /// Hsm(Requestor) to FP(Responder) transmit queue
    #[field(cardinality = 16, mutable = true, offset = 0x0000_0980)]
    hsm_to_fp_ipc_tx_queue: IpcMessage,

    /// Hsm(Requestor) to FP(Responder) receive queue
    #[field(cardinality = 16, mutable = true, offset = 0x0000_0D80)]
    hsm_to_fp_ipc_rx_queue: IpcMessage,

    /// Reset reason type
    #[field(volatile = true, offset = 0x0000_3AFC)]
    reset_type: u32,

    /// SQE entry for CDMA io request
    #[field(mutable = true, offset = 0x0000_3B00)]
    cdma_io_rx_entry: CdmaIoSqe,

    /// CQE entry for CDMA io completion
    #[field(offset = 0x0000_3B00)]
    cdma_io_tx_entry: CdmaIoCqe,

    /// FP boot status to be consumed by Admin core
    #[field(offset = 0x0000_3B88, volatile = true)]
    fp_boot_status: u32,

    /// Admin(Requestor) to FP(Responder) message tx producer index
    #[field(volatile = true)]
    admin_to_fp_ipc_tx_queue_pi: u32,

    /// Admin(Requestor) to FP(Responder) message tx consumer index
    #[field(volatile = true)]
    admin_to_fp_ipc_tx_queue_ci: u32,

    /// Admin(Requestor) to FP(Responder) message rx producer index
    #[field(volatile = true)]
    admin_to_fp_ipc_rx_queue_pi: u32,

    /// Admin(Requestor) to FP(Responder) message rx consumer index
    #[field(volatile = true)]
    admin_to_fp_ipc_rx_queue_ci: u32,

    /// Hsm(Requestor) to FP(Responder) message tx producer index
    #[field(offset = 0x0000_3BD4, volatile = true)]
    hsm_to_fp_ipc_tx_queue_pi: u32,

    /// Hsm(Requestor) to FP(Responder) tx consumer index
    #[field(volatile = true)]
    hsm_to_fp_ipc_tx_queue_ci: u32,

    /// Hsm(Requestor) to FP(Responder) rx producer index
    #[field(volatile = true)]
    hsm_to_fp_ipc_rx_queue_pi: u32,

    /// Hsm(Requestor) to FP(Responder) rx consumer index
    #[field(volatile = true)]
    hsm_to_fp_ipc_rx_queue_ci: u32,

    /// FP(Requestor) to Admin(Responder) IPC tx queue
    #[field(cardinality = 8, mutable = true, offset = 0x0000_3BF4)]
    fp_to_admin_ipc_tx_queue: IpcMessage,

    /// FP(Requestor) to Admin(Responder) IPC tx queue producer index
    #[field(volatile = true)]
    fp_to_admin_ipc_tx_queue_pi: u32,

    /// FP(Requestor) to Admin(Responder) IPC tx queue consumer index
    #[field(volatile = true)]
    fp_to_admin_ipc_tx_queue_ci: u32,

    /// FP(Requestor) to Admin(Responder) IPC rx queue
    #[field(cardinality = 8, mutable = true, offset = 0x0000_3DFC)]
    fp_to_admin_ipc_rx_queue: IpcMessage,

    /// FP(Requestor) to Admin(Responder) IPC rx queue producer index
    #[field(volatile = true)]
    fp_to_admin_ipc_rx_queue_pi: u32,

    /// FP(Requestor) to Admin(Responder) IPC rx queue consumer index
    #[field(volatile = true)]
    fp_to_admin_ipc_rx_queue_ci: u32,

    /// FP(Requestor) to HSM(Responder) IPC tx queue
    #[field(cardinality = 8, mutable = true, offset = 0x0000_4004)]
    fp_to_hsm_ipc_tx_queue: IpcMessage,

    /// FP(Requestor) to HSM(Responder) IPC tx queue producer index
    #[field(volatile = true)]
    fp_to_hsm_ipc_tx_queue_pi: u32,

    /// FP(Requestor) to HSM(Responder) IPC tx queue consumer index
    #[field(volatile = true)]
    fp_to_hsm_ipc_tx_queue_ci: u32,

    /// FP(Requestor) to HSM(Responder) IPC rx queue
    #[field(cardinality = 8, mutable = true, offset = 0x0000_420C)]
    fp_to_hsm_ipc_rx_queue: IpcMessage,

    /// FP(Requestor) to HSM(Responder) IPC rx queue producer index
    #[field(volatile = true)]
    fp_to_hsm_ipc_rx_queue_pi: u32,

    /// FP(Requestor) to HSM(Responder) IPC rx queue consumer index
    #[field(volatile = true)]
    fp_to_hsm_ipc_rx_queue_ci: u32,

    /// Start fp0 Cerberus debug_log_buffer C struct fields and buffer array.
    #[field(alignment = 0x4, mutable = true, volatile = true, offset = 0x0000_4b80)]
    fp0_debug_log_ring_buffer_size: u32,

    /// Corresponds to consumer index (ci) in SimplexPipe
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    fp0_debug_log_ring_buffer_head: u32,

    /// Corresponds to producer index (pi) in SimplexPipe
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    fp0_debug_log_ring_buffer_tail: u32,

    #[field(alignment = 0x4, mutable = true, volatile = true)]
    fp0_debug_log_buffer_sender_overflows: u32,

    /// The queue size is 0x1000 / sizeof(DebugLogEntryParameters)
    #[field(cardinality = 0x150, alignment = 0x4, mutable = true)]
    fp0_debug_log_buffer_queue: mcr_types::DebugLogEntryParameters,

    /// Start fp1 Cerberus debug_log_buffer C struct fields and buffer array.
    #[field(alignment = 0x4, mutable = true, volatile = true, offset = 0x0000_5b80)]
    fp1_debug_log_ring_buffer_size: u32,

    /// Corresponds to consumer index (ci) in SimplexPipe
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    fp1_debug_log_ring_buffer_head: u32,

    /// Corresponds to producer index (pi) in SimplexPipe
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    fp1_debug_log_ring_buffer_tail: u32,

    #[field(alignment = 0x4, mutable = true, volatile = true)]
    fp1_debug_log_buffer_sender_overflows: u32,

    /// The queue size is 0x1000 / sizeof(DebugLogEntryParameters)
    #[field(cardinality = 0x150, alignment = 0x4, mutable = true)]
    fp1_debug_log_buffer_queue: mcr_types::DebugLogEntryParameters,

    /// Start fp2 Cerberus debug_log_buffer C struct fields and buffer array.
    #[field(alignment = 0x4, mutable = true, volatile = true, offset = 0x0000_6b80)]
    fp2_debug_log_ring_buffer_size: u32,

    /// Corresponds to consumer index (ci) in SimplexPipe
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    fp2_debug_log_ring_buffer_head: u32,

    /// Corresponds to producer index (pi) in SimplexPipe
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    fp2_debug_log_ring_buffer_tail: u32,

    #[field(alignment = 0x4, mutable = true, volatile = true)]
    fp2_debug_log_buffer_sender_overflows: u32,

    /// The queue size is 0x1000 / sizeof(DebugLogEntryParameters)
    #[field(cardinality = 0x150, alignment = 0x4, mutable = true)]
    fp2_debug_log_buffer_queue: mcr_types::DebugLogEntryParameters,
}

#[cfg(test)]
mod tests {
    use static_assertions as sa;

    use super::*;

    #[test]
    fn test_base_address() {
        assert_eq!(PsRamMemMap::BASE_ADDRESS, 0xA3E0_0000);
        assert_eq!(PsRamMemMap::LENGTH, 0x0000_8000);
    }

    #[test]
    fn test_admin_to_fp_tx_queue() {
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_SIZE, 0x0000_0400);
        assert_eq!(
            PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_OFFSET,
            PsRamMemMap::BASE_ADDRESS
        );
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_OFFSET
                + PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_admin_to_fp_rx_queue() {
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_SIZE, 0x0000_0400);
        assert_eq!(
            PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_0400
        );
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_OFFSET
                + PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_hsm_to_fp_tx_queue() {
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_SIZE, 0x0000_0400);
        assert_eq!(
            PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_0980
        );
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_OFFSET + PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_hsm_to_fp_rx_queue() {
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_SIZE, 0x0000_0400);
        assert_eq!(
            PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_0D80
        );
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_OFFSET + PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_reset_type() {
        assert_eq!(PsRamMemMap::RESET_TYPE_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::RESET_TYPE_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x3AFC,
        );
        assert_eq!(PsRamMemMap::RESET_TYPE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::RESET_TYPE_OFFSET + PsRamMemMap::RESET_TYPE_SIZE
                <= PsRamMemMap::CDMA_IO_RX_ENTRY_OFFSET
        );
    }

    #[test]
    fn test_hsm_cdma_io_sqe_entry() {
        assert_eq!(PsRamMemMap::CDMA_IO_RX_ENTRY_SIZE, 0x0000_0080);
        assert_eq!(
            PsRamMemMap::CDMA_IO_RX_ENTRY_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3B00
        );
        assert_eq!(PsRamMemMap::CDMA_IO_RX_ENTRY_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::CDMA_IO_RX_ENTRY_OFFSET + PsRamMemMap::CDMA_IO_RX_ENTRY_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_hsm_cdma_io_cqe_entry() {
        assert_eq!(PsRamMemMap::CDMA_IO_TX_ENTRY_SIZE, 0x0000_0040);
        assert_eq!(
            PsRamMemMap::CDMA_IO_TX_ENTRY_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3B00
        );
        assert_eq!(PsRamMemMap::CDMA_IO_TX_ENTRY_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::CDMA_IO_TX_ENTRY_OFFSET + PsRamMemMap::CDMA_IO_TX_ENTRY_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_boot_status() {
        assert_eq!(PsRamMemMap::FP_BOOT_STATUS_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP_BOOT_STATUS_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3B88
        );
        assert_eq!(PsRamMemMap::FP_BOOT_STATUS_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_BOOT_STATUS_OFFSET + PsRamMemMap::FP_BOOT_STATUS_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_admin_to_fp_ipc_tx_queue_pi() {
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_PI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3B8C
        );
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_PI_OFFSET
                + PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_PI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_admin_to_fp_ipc_tx_queue_ci() {
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_CI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3B90
        );
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_CI_OFFSET
                + PsRamMemMap::ADMIN_TO_FP_IPC_TX_QUEUE_CI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_admin_to_fp_ipc_rx_queue_pi() {
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_PI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3B94
        );
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_PI_OFFSET
                + PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_PI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }
    #[test]
    fn test_admin_to_fp_ipc_rx_queue_ci() {
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_CI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3B98
        );
        assert_eq!(PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_CI_OFFSET
                + PsRamMemMap::ADMIN_TO_FP_IPC_RX_QUEUE_CI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_hsm_to_fp_ipc_tx_queue_pi() {
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_PI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3BD4
        );
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_PI_OFFSET
                + PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_PI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_hsm_to_fp_ipc_tx_queue_ci() {
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_CI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3BD8
        );
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_CI_OFFSET
                + PsRamMemMap::HSM_TO_FP_IPC_TX_QUEUE_CI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_hsm_to_fp_ipc_rx_queue_pi() {
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_PI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3BDC
        );
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_PI_OFFSET
                + PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_PI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_hsm_to_fp_ipc_rx_queue_ci() {
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_CI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3BE0
        );
        assert_eq!(PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_CI_OFFSET
                + PsRamMemMap::HSM_TO_FP_IPC_RX_QUEUE_CI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_admin_ipc_tx_queue() {
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_SIZE, 0x0000_0200);
        assert_eq!(
            PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3BF4
        );
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_OFFSET
                + PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_admin_ipc_tx_queue_pi() {
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3DF4
        );
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_PI_OFFSET
                + PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_PI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_admin_ipc_tx_queue_ci() {
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3DF8
        );
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_CI_OFFSET
                + PsRamMemMap::FP_TO_ADMIN_IPC_TX_QUEUE_CI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_admin_ipc_rx_queue() {
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_SIZE, 0x0000_0200);
        assert_eq!(
            PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3DFC
        );
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_OFFSET
                + PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_admin_ipc_rx_queue_pi() {
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_3FFC
        );
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_PI_OFFSET
                + PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_PI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_admin_ipc_rx_queue_ci() {
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_4000
        );
        assert_eq!(PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_CI_OFFSET
                + PsRamMemMap::FP_TO_ADMIN_IPC_RX_QUEUE_CI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_hsm_ipc_tx_queue() {
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_SIZE, 0x0000_0200);
        assert_eq!(
            PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_4004
        );
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_OFFSET + PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_hsm_ipc_tx_queue_pi() {
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_PI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_4204
        );
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_PI_OFFSET
                + PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_PI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_hsm_ipc_tx_queue_ci() {
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_CI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_4208
        );
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_CI_OFFSET
                + PsRamMemMap::FP_TO_HSM_IPC_TX_QUEUE_CI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_hsm_ipc_rx_queue() {
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_SIZE, 0x0000_0200);
        assert_eq!(
            PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_420C
        );
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_OFFSET + PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_hsm_ipc_rx_queue_pi() {
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_PI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_PI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_440C
        );
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_PI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_PI_OFFSET
                + PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_PI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_to_hsm_ipc_rx_queue_ci() {
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_CI_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_CI_OFFSET,
            PsRamMemMap::BASE_ADDRESS + 0x0000_4410
        );
        assert_eq!(PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_CI_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_CI_OFFSET
                + PsRamMemMap::FP_TO_HSM_IPC_RX_QUEUE_CI_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp0_debug_log_descriptor_region() {
        assert_eq!(PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_SIZE_SIZE, 0x04);
        assert_eq!(PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_HEAD_SIZE, 0x04);
        assert_eq!(PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_TAIL_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP0_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_SIZE,
            0x04
        );

        assert_eq!(
            PsRamMemMap::BASE_ADDRESS + 0x0000_4b80,
            PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
        );

        assert_eq!(
            PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
                - PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            0
        );
        assert_eq!(
            PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_HEAD_OFFSET
                - PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            4
        );
        assert_eq!(
            PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_TAIL_OFFSET
                - PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            8
        );
        assert_eq!(
            PsRamMemMap::FP0_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_OFFSET
                - PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            12
        );

        assert_eq!(PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET % 4, 0);
        assert_eq!(PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_HEAD_OFFSET % 4, 0);
        assert_eq!(PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_TAIL_OFFSET % 4, 0);
        assert_eq!(
            PsRamMemMap::FP0_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_OFFSET % 4,
            0
        );

        sa::const_assert!(
            PsRamMemMap::FP0_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + PsRamMemMap::FP0_DEBUG_LOG_BUFFER_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp0_debug_log_buffer_queue() {
        sa::const_assert!(PsRamMemMap::FP0_DEBUG_LOG_BUFFER_QUEUE_SIZE < 0x1000);
        assert_eq!(
            PsRamMemMap::FP0_DEBUG_LOG_BUFFER_QUEUE_OFFSET,
            PsRamMemMap::FP0_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET + 0x10,
        );
        assert_eq!(PsRamMemMap::FP0_DEBUG_LOG_BUFFER_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP0_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + PsRamMemMap::FP0_DEBUG_LOG_BUFFER_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp1_debug_log_descriptor_region() {
        assert_eq!(PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_SIZE_SIZE, 0x04);
        assert_eq!(PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_HEAD_SIZE, 0x04);
        assert_eq!(PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_TAIL_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP1_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_SIZE,
            0x04
        );

        assert_eq!(
            PsRamMemMap::BASE_ADDRESS + 0x0000_5b80,
            PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
        );

        assert_eq!(
            PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
                - PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            0
        );
        assert_eq!(
            PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_HEAD_OFFSET
                - PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            4
        );
        assert_eq!(
            PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_TAIL_OFFSET
                - PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            8
        );
        assert_eq!(
            PsRamMemMap::FP1_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_OFFSET
                - PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            12
        );

        assert_eq!(PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET % 4, 0);
        assert_eq!(PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_HEAD_OFFSET % 4, 0);
        assert_eq!(PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_TAIL_OFFSET % 4, 0);
        assert_eq!(
            PsRamMemMap::FP1_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_OFFSET % 4,
            0
        );

        sa::const_assert!(
            PsRamMemMap::FP1_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + PsRamMemMap::FP1_DEBUG_LOG_BUFFER_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp1_debug_log_buffer_queue() {
        sa::const_assert!(PsRamMemMap::FP1_DEBUG_LOG_BUFFER_QUEUE_SIZE < 0x1000);
        assert_eq!(
            PsRamMemMap::FP1_DEBUG_LOG_BUFFER_QUEUE_OFFSET,
            PsRamMemMap::FP1_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET + 0x10,
        );
        assert_eq!(PsRamMemMap::FP1_DEBUG_LOG_BUFFER_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP1_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + PsRamMemMap::FP1_DEBUG_LOG_BUFFER_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp2_debug_log_descriptor_region() {
        assert_eq!(PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_SIZE_SIZE, 0x04);
        assert_eq!(PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_HEAD_SIZE, 0x04);
        assert_eq!(PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_TAIL_SIZE, 0x04);
        assert_eq!(
            PsRamMemMap::FP2_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_SIZE,
            0x04
        );

        assert_eq!(
            PsRamMemMap::BASE_ADDRESS + 0x0000_6b80,
            PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
        );

        assert_eq!(
            PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET
                - PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            0
        );
        assert_eq!(
            PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_HEAD_OFFSET
                - PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            4
        );
        assert_eq!(
            PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_TAIL_OFFSET
                - PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            8
        );
        assert_eq!(
            PsRamMemMap::FP2_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_OFFSET
                - PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET,
            12
        );

        assert_eq!(PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET % 4, 0);
        assert_eq!(PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_HEAD_OFFSET % 4, 0);
        assert_eq!(PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_TAIL_OFFSET % 4, 0);
        assert_eq!(
            PsRamMemMap::FP2_DEBUG_LOG_BUFFER_SENDER_OVERFLOWS_OFFSET % 4,
            0
        );

        sa::const_assert!(
            PsRamMemMap::FP2_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + PsRamMemMap::FP2_DEBUG_LOG_BUFFER_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp2_debug_log_buffer_queue() {
        sa::const_assert!(PsRamMemMap::FP2_DEBUG_LOG_BUFFER_QUEUE_SIZE < 0x1000);
        assert_eq!(
            PsRamMemMap::FP2_DEBUG_LOG_BUFFER_QUEUE_OFFSET,
            PsRamMemMap::FP2_DEBUG_LOG_RING_BUFFER_SIZE_OFFSET + 0x10,
        );
        assert_eq!(PsRamMemMap::FP2_DEBUG_LOG_BUFFER_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            PsRamMemMap::FP2_DEBUG_LOG_BUFFER_QUEUE_OFFSET
                + PsRamMemMap::FP2_DEBUG_LOG_BUFFER_QUEUE_SIZE
                <= PsRamMemMap::BASE_ADDRESS + PsRamMemMap::LENGTH
        );
    }
}
