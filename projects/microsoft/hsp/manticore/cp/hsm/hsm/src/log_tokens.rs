// Copyright (c) Microsoft Corporation. All rights reserved.
// This is an auto-generated file. Please do not modify manually.
// To regenerate use command: `cargo xtask telemetry-tokenize`

use hashbrown::HashMap;

lazy_static::lazy_static! {
    pub static ref MANTICORE_HSM_LOG_TOKENS_MAP: HashMap<&'static str, u8> = {
        let mut m = HashMap::new();
        m.insert("Starting HSM event loop...", 0);
        m.insert("Failed to send IPC message to FP: {:?}", 1);
        m.insert("Failed to decode IPC message header: {:?}", 2);
        m.insert("Invalid IPC response with status {}", 3);
        m.insert("Spurious Message", 4);
        m.insert("Failed to send IPC message to Admin: {:?}", 5);
        m.insert("Invalid core id", 6);
        m.insert("Negative self test failed with status: {}", 7);
        m.insert("failed to delete key cdma_key_id={:?} key_id={:?} {:?}", 8);
        m.insert("Establish cred key deletion failed {:?}. Ignoring error.", 9);
        m.insert("DebugLogSender::new", 10);
        m.insert("Debug log sender initialized in HSM", 11);
        m.insert("Interrupt::tcon_wakeup1_irq", 12);
        m.insert("IpcController::new", 13);
        m.insert("IpcChannelId::AdminToHsmIoCore", 14);
        m.insert("admin_ipc_channel", 15);
        m.insert("IpcChannelId::HsmIoCoreToFpIoCore", 16);
        m.insert("IpcChannelId::HsmIoCoreToHsp", 17);
        m.insert("IpcChannelId::HsmIoCoreToAdmin", 18);
        m.insert("GdmaChannelId::Channel1", 19);
        m.insert("IoControllerId::Core0", 20);
        m.insert("GsRamMemMap::hsm_heap", 21);
        m.insert("InterruptController", 22);
        m.insert("rng_calibration_config", 23);
        m.insert("create_aes", 24);
        m.insert("Sha", 25);
        m.insert("Pka", 26);
        m.insert("on_request_ready: spurious event", 27);
        m.insert("on_response_complete: spurious event", 28);
        m.insert("on_dma_complete: spurious event", 29);
        m.insert("Failed to send FLR complete event", 30);
        m.insert("on_flr: spurious event", 31);
        m.insert("Invalid Header found in IPC message", 32);
        m.insert("Invalid PcieFunction enable/disable message", 33);
        m.insert("Invalid Set Resource message {:?}", 34);
        m.insert("Invalid IPC Create Delete Submission Queue Message {:?}", 35);
        m.insert("{:?} not Enabled", 36);
        m.insert("Invalid Shutdown for reset request message", 37);
        m.insert("iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err", 38);
        m.insert("Failed to send response to Admin core {:?}", 39);
        m.insert("Schedule is busy in draining IOs", 40);
        m.insert("iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err", 41);
        m.insert("iDFU Fault Injected: idfu_fault_pre_reset_hsm_drain_timeout", 42);
        m.insert("Invalid IPC message header {:?}", 43);
        m.insert("Unhandled message with opcode {:#X}", 44);
        m.insert("on_pka: spurious event. Index: {}", 45);
        m.insert("on_fp_ipc_response: spurious event", 46);
        m.insert("on_hsp_ipc_response: spurious event", 47);
        m.insert("on_admin_ipc_response: spurious event", 48);
        m.insert("on_soft_aes_resp: spurious event", 49);
        m.insert("Failed to receive self test request", 50);
        m.insert("[tag: {}] begin_establish_credential err: {:?}", 51);
        m.insert("[tag: {}] continue_establish_cred err: {:?}", 52);
        m.insert("[tag: {}] end_establish_credential  err: {:?}", 53);
        m.insert("[tag: {}], state:{:?}, event: {:?}", 54);
        m.insert("For tag: {}, begin_import_der_crt_key returned err: {:?}", 55);
        m.insert("For tag: {}, begin_import_der_aesbulk256_key returned err: {:?}", 56);
        m.insert("For tag: {}, begin_delete_aesbulk256_key returned err: {:?}", 57);
        m.insert("For tag: {}, begin_ecc_gen_key returned err: {:?}", 58);
        m.insert("[tag: {}] begin_open_session err: {:?}", 59);
        m.insert("[tag: {}] continue_open_session err: {:?}", 60);
        m.insert("[tag: {}] begin_close_user_session err: {:?}", 61);
        m.insert("Tag {}, begin_delete_aesbulk256_user_keys {:?}", 62);
        m.insert("[tag: {}] state: {:?} On event {:?}", 63);
        m.insert("Timeout!! State: {:?} Res Op State: {:?}", 64);
        m.insert("For tag: {}, begin_close_user_session returned err: {:?}", 65);
        m.insert("For tag: {}, begin_aesbulk256_gen_key returned err: {:?}", 66);
        m.insert("[tag: {}] IO Timed out while in {:?}", 67);
        m.insert("[tag: {}] DMA begin_txn error. (0x{:08x})", 68);
        m.insert("[tag:{}] IO channel send error. (0x{:08x})", 69);
        m.insert("send_cqe: [tag:{}] DMA buffer not found", 70);
        m.insert("[tag: {}] begin_change_pin err: {:?}", 71);
        m.insert("[tag: {}] continue_change_pin err: {:?}", 72);
        m.insert("[tag: {}] ECC PCT Sign/Verify failed. Reporting failure to FSM.", 73);
        m.insert("[tag: {}] ECC Verify operation failed: {:?}. Stopping validation.", 74);
        m.insert("[tag: {}] Missing first shared secret for ECDH verification!", 75);
        m.insert("[tag: {}] ECC PCT Key Agreement failed. Reporting failure to FSM.", 76);
        m.insert("[tag: {}] ECC structural validation Public key length mismatch", 77);
        m.insert("[tag: {}] ECC structural validation Public key mismatch", 78);
        m.insert("[tag: {}] ECC structural validation Private key length mismatch", 79);
        m.insert("[tag: {}] ECC structural validation Scalar d is not in the range 0 < d < n", 80);
        m.insert("[tag: {}] RSA PCT Unwrap Verification failed! Forcing crash for recovery.", 81);
        m.insert("[tag: {}] failed due to {:?}.", 82);
        m.insert("[tag: {}] ECC PCT {:?} Verification failed! Forcing crash for recovery.", 83);
        m.insert("[tag: {}] {} failed due to {:?}.", 84);
        m.insert("Soft AES Invalid Response", 85);
        m.insert("Unexpected: Resource cleanup for an already cleaned up resource {}.", 86);
        m.insert("Unexpected: Resource cleanup completed for resource {}.", 87);
        m.insert("Failed the self test {:?}", 88);
        m.insert("#### Send notification to other cores for self test error", 89);
        m.insert("#### HSM HAL Init ####", 90);
        m.insert("Preop-Self test failed for {:?} with error {}", 91);
        m
    };
}

lazy_static::lazy_static! {
    pub static ref MANTICORE_HSM_LOG_TOKENS_INDEX_TO_MESSAGE_MAP: HashMap<u8, &'static str> = {
        let mut m = HashMap::new();
        m.insert(0, "Starting HSM event loop...");
        m.insert(1, "Failed to send IPC message to FP: {:?}");
        m.insert(2, "Failed to decode IPC message header: {:?}");
        m.insert(3, "Invalid IPC response with status {}");
        m.insert(4, "Spurious Message");
        m.insert(5, "Failed to send IPC message to Admin: {:?}");
        m.insert(6, "Invalid core id");
        m.insert(7, "Negative self test failed with status: {}");
        m.insert(8, "failed to delete key cdma_key_id={:?} key_id={:?} {:?}");
        m.insert(9, "Establish cred key deletion failed {:?}. Ignoring error.");
        m.insert(10, "DebugLogSender::new");
        m.insert(11, "Debug log sender initialized in HSM");
        m.insert(12, "Interrupt::tcon_wakeup1_irq");
        m.insert(13, "IpcController::new");
        m.insert(14, "IpcChannelId::AdminToHsmIoCore");
        m.insert(15, "admin_ipc_channel");
        m.insert(16, "IpcChannelId::HsmIoCoreToFpIoCore");
        m.insert(17, "IpcChannelId::HsmIoCoreToHsp");
        m.insert(18, "IpcChannelId::HsmIoCoreToAdmin");
        m.insert(19, "GdmaChannelId::Channel1");
        m.insert(20, "IoControllerId::Core0");
        m.insert(21, "GsRamMemMap::hsm_heap");
        m.insert(22, "InterruptController");
        m.insert(23, "rng_calibration_config");
        m.insert(24, "create_aes");
        m.insert(25, "Sha");
        m.insert(26, "Pka");
        m.insert(27, "on_request_ready: spurious event");
        m.insert(28, "on_response_complete: spurious event");
        m.insert(29, "on_dma_complete: spurious event");
        m.insert(30, "Failed to send FLR complete event");
        m.insert(31, "on_flr: spurious event");
        m.insert(32, "Invalid Header found in IPC message");
        m.insert(33, "Invalid PcieFunction enable/disable message");
        m.insert(34, "Invalid Set Resource message {:?}");
        m.insert(35, "Invalid IPC Create Delete Submission Queue Message {:?}");
        m.insert(36, "{:?} not Enabled");
        m.insert(37, "Invalid Shutdown for reset request message");
        m.insert(38, "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err");
        m.insert(39, "Failed to send response to Admin core {:?}");
        m.insert(40, "Schedule is busy in draining IOs");
        m.insert(41, "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err");
        m.insert(42, "iDFU Fault Injected: idfu_fault_pre_reset_hsm_drain_timeout");
        m.insert(43, "Invalid IPC message header {:?}");
        m.insert(44, "Unhandled message with opcode {:#X}");
        m.insert(45, "on_pka: spurious event. Index: {}");
        m.insert(46, "on_fp_ipc_response: spurious event");
        m.insert(47, "on_hsp_ipc_response: spurious event");
        m.insert(48, "on_admin_ipc_response: spurious event");
        m.insert(49, "on_soft_aes_resp: spurious event");
        m.insert(50, "Failed to receive self test request");
        m.insert(51, "[tag: {}] begin_establish_credential err: {:?}");
        m.insert(52, "[tag: {}] continue_establish_cred err: {:?}");
        m.insert(53, "[tag: {}] end_establish_credential  err: {:?}");
        m.insert(54, "[tag: {}], state:{:?}, event: {:?}");
        m.insert(55, "For tag: {}, begin_import_der_crt_key returned err: {:?}");
        m.insert(56, "For tag: {}, begin_import_der_aesbulk256_key returned err: {:?}");
        m.insert(57, "For tag: {}, begin_delete_aesbulk256_key returned err: {:?}");
        m.insert(58, "For tag: {}, begin_ecc_gen_key returned err: {:?}");
        m.insert(59, "[tag: {}] begin_open_session err: {:?}");
        m.insert(60, "[tag: {}] continue_open_session err: {:?}");
        m.insert(61, "[tag: {}] begin_close_user_session err: {:?}");
        m.insert(62, "Tag {}, begin_delete_aesbulk256_user_keys {:?}");
        m.insert(63, "[tag: {}] state: {:?} On event {:?}");
        m.insert(64, "Timeout!! State: {:?} Res Op State: {:?}");
        m.insert(65, "For tag: {}, begin_close_user_session returned err: {:?}");
        m.insert(66, "For tag: {}, begin_aesbulk256_gen_key returned err: {:?}");
        m.insert(67, "[tag: {}] IO Timed out while in {:?}");
        m.insert(68, "[tag: {}] DMA begin_txn error. (0x{:08x})");
        m.insert(69, "[tag:{}] IO channel send error. (0x{:08x})");
        m.insert(70, "send_cqe: [tag:{}] DMA buffer not found");
        m.insert(71, "[tag: {}] begin_change_pin err: {:?}");
        m.insert(72, "[tag: {}] continue_change_pin err: {:?}");
        m.insert(73, "[tag: {}] ECC PCT Sign/Verify failed. Reporting failure to FSM.");
        m.insert(74, "[tag: {}] ECC Verify operation failed: {:?}. Stopping validation.");
        m.insert(75, "[tag: {}] Missing first shared secret for ECDH verification!");
        m.insert(76, "[tag: {}] ECC PCT Key Agreement failed. Reporting failure to FSM.");
        m.insert(77, "[tag: {}] ECC structural validation Public key length mismatch");
        m.insert(78, "[tag: {}] ECC structural validation Public key mismatch");
        m.insert(79, "[tag: {}] ECC structural validation Private key length mismatch");
        m.insert(80, "[tag: {}] ECC structural validation Scalar d is not in the range 0 < d < n");
        m.insert(81, "[tag: {}] RSA PCT Unwrap Verification failed! Forcing crash for recovery.");
        m.insert(82, "[tag: {}] failed due to {:?}.");
        m.insert(83, "[tag: {}] ECC PCT {:?} Verification failed! Forcing crash for recovery.");
        m.insert(84, "[tag: {}] {} failed due to {:?}.");
        m.insert(85, "Soft AES Invalid Response");
        m.insert(86, "Unexpected: Resource cleanup for an already cleaned up resource {}.");
        m.insert(87, "Unexpected: Resource cleanup completed for resource {}.");
        m.insert(88, "Failed the self test {:?}");
        m.insert(89, "#### Send notification to other cores for self test error");
        m.insert(90, "#### HSM HAL Init ####");
        m.insert(91, "Preop-Self test failed for {:?} with error {}");
        m
    };
}

#[allow(dead_code)]
pub const MANTICORE_HSM_LOG_TOKENS: [&str; 92] = [
    "Starting HSM event loop...",
    "Failed to send IPC message to FP: {:?}",
    "Failed to decode IPC message header: {:?}",
    "Invalid IPC response with status {}",
    "Spurious Message",
    "Failed to send IPC message to Admin: {:?}",
    "Invalid core id",
    "Negative self test failed with status: {}",
    "failed to delete key cdma_key_id={:?} key_id={:?} {:?}",
    "Establish cred key deletion failed {:?}. Ignoring error.",
    "DebugLogSender::new",
    "Debug log sender initialized in HSM",
    "Interrupt::tcon_wakeup1_irq",
    "IpcController::new",
    "IpcChannelId::AdminToHsmIoCore",
    "admin_ipc_channel",
    "IpcChannelId::HsmIoCoreToFpIoCore",
    "IpcChannelId::HsmIoCoreToHsp",
    "IpcChannelId::HsmIoCoreToAdmin",
    "GdmaChannelId::Channel1",
    "IoControllerId::Core0",
    "GsRamMemMap::hsm_heap",
    "InterruptController",
    "rng_calibration_config",
    "create_aes",
    "Sha",
    "Pka",
    "on_request_ready: spurious event",
    "on_response_complete: spurious event",
    "on_dma_complete: spurious event",
    "Failed to send FLR complete event",
    "on_flr: spurious event",
    "Invalid Header found in IPC message",
    "Invalid PcieFunction enable/disable message",
    "Invalid Set Resource message {:?}",
    "Invalid IPC Create Delete Submission Queue Message {:?}",
    "{:?} not Enabled",
    "Invalid Shutdown for reset request message",
    "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err",
    "Failed to send response to Admin core {:?}",
    "Schedule is busy in draining IOs",
    "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err",
    "iDFU Fault Injected: idfu_fault_pre_reset_hsm_drain_timeout",
    "Invalid IPC message header {:?}",
    "Unhandled message with opcode {:#X}",
    "on_pka: spurious event. Index: {}",
    "on_fp_ipc_response: spurious event",
    "on_hsp_ipc_response: spurious event",
    "on_admin_ipc_response: spurious event",
    "on_soft_aes_resp: spurious event",
    "Failed to receive self test request",
    "[tag: {}] begin_establish_credential err: {:?}",
    "[tag: {}] continue_establish_cred err: {:?}",
    "[tag: {}] end_establish_credential  err: {:?}",
    "[tag: {}], state:{:?}, event: {:?}",
    "For tag: {}, begin_import_der_crt_key returned err: {:?}",
    "For tag: {}, begin_import_der_aesbulk256_key returned err: {:?}",
    "For tag: {}, begin_delete_aesbulk256_key returned err: {:?}",
    "For tag: {}, begin_ecc_gen_key returned err: {:?}",
    "[tag: {}] begin_open_session err: {:?}",
    "[tag: {}] continue_open_session err: {:?}",
    "[tag: {}] begin_close_user_session err: {:?}",
    "Tag {}, begin_delete_aesbulk256_user_keys {:?}",
    "[tag: {}] state: {:?} On event {:?}",
    "Timeout!! State: {:?} Res Op State: {:?}",
    "For tag: {}, begin_close_user_session returned err: {:?}",
    "For tag: {}, begin_aesbulk256_gen_key returned err: {:?}",
    "[tag: {}] IO Timed out while in {:?}",
    "[tag: {}] DMA begin_txn error. (0x{:08x})",
    "[tag:{}] IO channel send error. (0x{:08x})",
    "send_cqe: [tag:{}] DMA buffer not found",
    "[tag: {}] begin_change_pin err: {:?}",
    "[tag: {}] continue_change_pin err: {:?}",
    "[tag: {}] ECC PCT Sign/Verify failed. Reporting failure to FSM.",
    "[tag: {}] ECC Verify operation failed: {:?}. Stopping validation.",
    "[tag: {}] Missing first shared secret for ECDH verification!",
    "[tag: {}] ECC PCT Key Agreement failed. Reporting failure to FSM.",
    "[tag: {}] ECC structural validation Public key length mismatch",
    "[tag: {}] ECC structural validation Public key mismatch",
    "[tag: {}] ECC structural validation Private key length mismatch",
    "[tag: {}] ECC structural validation Scalar d is not in the range 0 < d < n",
    "[tag: {}] RSA PCT Unwrap Verification failed! Forcing crash for recovery.",
    "[tag: {}] failed due to {:?}.",
    "[tag: {}] ECC PCT {:?} Verification failed! Forcing crash for recovery.",
    "[tag: {}] {} failed due to {:?}.",
    "Soft AES Invalid Response",
    "Unexpected: Resource cleanup for an already cleaned up resource {}.",
    "Unexpected: Resource cleanup completed for resource {}.",
    "Failed the self test {:?}",
    "#### Send notification to other cores for self test error",
    "#### HSM HAL Init ####",
    "Preop-Self test failed for {:?} with error {}",
];
