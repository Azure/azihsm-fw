// Copyright (c) Microsoft Corporation. All rights reserved.
// This is an auto-generated file. Please do not modify manually.
// To regenerate use command: `cargo xtask telemetry-tokenize`

use hashbrown::HashMap;

lazy_static::lazy_static! {
    pub static ref MANTICORE_ADMIN_LOG_TOKENS_MAP: HashMap<&'static str, u8> = {
        let mut m = HashMap::new();
        m.insert("IPC response status: {:x}", 0);
        m.insert("Starting Admin event loop...", 1);
        m.insert("Telemetry FSM received unexpected event. Event: {:?}", 2);
        m.insert("Error running telemetry PCIe monitor, code {:#x}", 3);
        m.insert("Telemetry PCIe monitor is running normally. Link Speed: {}, Link Width: x{}", 4);
        m.insert("Unexpected PCIe Link. Link Speed: {}, Link Width: {}", 5);
        m.insert("Telemetry PCIe monitor not able to start, code {:#x}", 6);
        m.insert("Received timer event while waiting for cast resource", 7);
        m.insert("Invalid event. state: {:?}", 8);
        m.insert("on_fp_to_admin_channel_ready: Invalid state {:?}", 9);
        m.insert("on_fp_to_admin_ipc_response: Invalid state {:?}", 10);
        m.insert("Invalid state transition. Current state = {}", 11);
        m.insert("VF Stop for PCIe function: {:?}", 12);
        m.insert("[Vf_Stop] Unsupported event, expected StartCmd", 13);
        m.insert("Late IPC_HSM_SHUTDOWN_RSP after {}us", 14);
        m.insert("Late IPC_FP_SHUTDOWN_RSP after {}us", 15);
        m.insert("Late resource {} acquire {}us", 16);
        m.insert("Starting", 17);
        m.insert("Stopped after {}us", 18);
        m.insert("Admin abort with status: {:?}. State: ({:?})", 19);
        m.insert("HSM abort with status: {:?}. State: ({:?})", 20);
        m.insert("FP abort with status: {:?}. State: ({:?})", 21);
        m.insert("error_dfu: IPC_SP_SHUTDOWN_RSP {:?}", 22);
        m.insert("Timeout in state {:?} resources: {:#x}", 23);
        m.insert("handle_dfu_started: IPC_SP_SHUTDOWN_RSP {:?}", 24);
        m.insert("Insufficient time to drain CP HSM/FP.", 25);
        m.insert("IPC_CP_HSM_SHUTDOWN_REQ {:#x}", 26);
        m.insert("IPC_FP_SHUTDOWN_REQ {:#x}", 27);
        m.insert("Ready in {} us", 28);
        m.insert("iDFU Abort with OperationTimeout: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}", 29);
        m.insert("iDFU Abort with OperationFailed: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}", 30);
        m.insert("IPC_SP_SHUTDOWN_RSP {:#x}", 31);
        m.insert("No reset detected.", 32);
        m.insert("Spurious IPC_CP_HSM_SHUTDOWN_RSP after {}us", 33);
        m.insert("Spurious IPC_FP_SHUTDOWN_RSP after {}us", 34);
        m.insert("Failed to receive IPC message from HSM", 35);
        m.insert("Failed to decode IPC message from HSM", 36);
        m.insert("Failed to receive IPC message from FP", 37);
        m.insert("Failed to decode IPC message from FP", 38);
        m.insert("Fail to receive queue_delete_notification", 39);
        m.insert("Mismatch tags: received ({:?}) vs expected ({:?})", 40);
        m.insert("Received deferred action for a different controller: {:?}", 41);
        m.insert("Error in enabling controller {:?}", 42);
        m.insert("[cntrl] Failed to send IPC message to FP: {:?}", 43);
        m.insert("[cntrl] Failed to send IPC message to HSM: {:?}", 44);
        m.insert("[delete_sq] Invalid SQE: {:?}", 45);
        m.insert("Get device Sq Failed {:?}", 46);
        m.insert("Process HSM IPC failed {:?}", 47);
        m.insert("[delete_sq] Delete device queue on_hsm_ipc_response: Delete submission queue failed {:?}", 48);
        m.insert("Process FP IPC failed {:?}", 49);
        m.insert("[delete_sq] Delete device queue on_fp_ipc_response: Delete submission queue failed {:?}", 50);
        m.insert("[delete_sq] on_deferred_queue_deletion: Delete submission queue failed {:?}", 51);
        m.insert("Failed to send FP IPC request {:?}", 52);
        m.insert("Failed to send HSM IPC request {:?}", 53);
        m.insert("Complete VFLR for {:?}", 54);
        m.insert("[vflr] Failed to send IPC message to FP: {:?}", 55);
        m.insert("[vflr] Failed to send IPC message to HSM: {:?}", 56);
        m.insert("VF Prepare for PCIe function: {:?}", 57);
        m.insert("[Vf Prepare] Unsupported event", 58);
        m.insert("[tdisp_int] TdispIntFsm timer elapsed, state: {}", 59);
        m.insert("[tdisp_int] Invalid interrupt source, source_mask: {}", 60);
        m.insert("[tdisp_int] Failed to send IPC message to HSP: {:?}", 61);
        m.insert("[tdisp_int] Failed to decode IPC message: {:?}", 62);
        m.insert("[tdisp_int] Invalid IPC response with status {}", 63);
        m.insert("[tdisp_int] Spurious Message", 64);
        m.insert("VF Restore for PCIe function: {:?}", 65);
        m.insert("[Vf Restore] Unsupported event", 66);
        m.insert("VF Save for PCIe function: {:?}", 67);
        m.insert("[Vf Save] Unsupported event", 68);
        m.insert("VF Start for PCIe function: {:?}", 69);
        m.insert("[Vf_Start] Unsupported event, expected StartCmd", 70);
        m.insert("[get_features] Invalid SQE: {:?}", 71);
        m.insert("[get_res] Invalid SQE: {:?}", 72);
        m.insert("[stop_interface] Timer elapsed, state: {:?}", 73);
        m.insert("[stop_interface] Failed to receive notification from queue", 74);
        m.insert("[stop_interface] Message tag {} is not expected {}", 75);
        m.insert("[stop_interface] Failed to send IPC message to FP: {:?}", 76);
        m.insert("[stop_interface] Failed to send IPC message to HSM: {:?}", 77);
        m.insert("[stop_interface] Failed to send response to HSP: {:?}", 78);
        m.insert("[doe] on_rx_ready Failed to recv DOE message: {:?}", 79);
        m.insert("[doe] on_doe_go: Failed to recv DOE message: {:?}", 80);
        m.insert("Failed to end recv DOE message: {:?}", 81);
        m.insert("Failed to decode IPC message: {:?}", 82);
        m.insert("[doe] Invalid IPC response with status {}", 83);
        m.insert("[doe] Spurious Message", 84);
        m.insert("[doe] on_ipc_response: Failed to send DOE message: {:?}", 85);
        m.insert("[doe] on_tx_ready: Failed to send DOE message: {:?}", 86);
        m.insert("Failed to send IPC message to HSP: {:?}", 87);
        m.insert("PCIE_PERST_UP Error: 0x{:08x}", 88);
        m.insert("PF FLR complete", 89);
        m.insert("Perst Down complete", 90);
        m.insert("[set_res] Invalid SQE: {:?}", 91);
        m.insert("Failed to set resource count: {:?}", 92);
        m.insert("[set_res] Invalid IPC response with status {}", 93);
        m.insert("Statemachine error {}", 94);
        m.insert("AES-GCM Decrypt Tag Mismatch", 95);
        m.insert("Failed to send AES GCM response entry to response queue", 96);
        m.insert("Invalid state transition in AesGcmExtFsm. Current state = {}", 97);
        m.insert("[delete_cq] Invalid SQE: {:?}", 98);
        m.insert("Delete Cq Failed {:?}", 99);
        m.insert("[set_features] Invalid SQE: {:?}", 100);
        m.insert("Failed the self test {:?}", 101);
        m.insert("Preop-Self test failed for {:?} with error {}", 102);
        m.insert("Invalid IPC message header", 103);
        m.insert("Cannot decode opcode. Invalid IPC message opcode", 104);
        m.insert("Opcode does not match. Invalid IPC message opcode", 105);
        m.insert("Invalid IPC message", 106);
        m.insert("Invalid IPC message for Negative self test.", 107);
        m.insert("Invalid test ID in negative self test IPC payload.", 108);
        m.insert("Invalid Header found in IPC message", 109);
        m.insert("Invalid Shutdown for reset request message", 110);
        m.insert("handle_invalid_message_opcode: Invalid IPC message header {:?}", 111);
        m.insert("Unhandled message with opcode {:#X}", 112);
        m.insert("Failed to send response to HSP core {:?}", 113);
        m.insert("send_hsm_ipc_response: Invalid IPC message header {:?}", 114);
        m.insert("Failed to send response to hsm core {:?}", 115);
        m.insert("DTCM error received. Fault code: {}", 116);
        m.insert("ITCM error received. Fault code: {}", 117);
        m.insert("GDMA Data Structure Error. Fault code: {}", 118);
        m.insert("GDMA Data Access Error. Fault code: {}", 119);
        m.insert("GDMA Delivery Queue Error. Fault code: {}", 120);
        m.insert("GDMA Completion Queue Error. Fault code: {}", 121);
        m
    };
}

lazy_static::lazy_static! {
    pub static ref MANTICORE_ADMIN_LOG_TOKENS_INDEX_TO_MESSAGE_MAP: HashMap<u8, &'static str> = {
        let mut m = HashMap::new();
        m.insert(0, "IPC response status: {:x}");
        m.insert(1, "Starting Admin event loop...");
        m.insert(2, "Telemetry FSM received unexpected event. Event: {:?}");
        m.insert(3, "Error running telemetry PCIe monitor, code {:#x}");
        m.insert(4, "Telemetry PCIe monitor is running normally. Link Speed: {}, Link Width: x{}");
        m.insert(5, "Unexpected PCIe Link. Link Speed: {}, Link Width: {}");
        m.insert(6, "Telemetry PCIe monitor not able to start, code {:#x}");
        m.insert(7, "Received timer event while waiting for cast resource");
        m.insert(8, "Invalid event. state: {:?}");
        m.insert(9, "on_fp_to_admin_channel_ready: Invalid state {:?}");
        m.insert(10, "on_fp_to_admin_ipc_response: Invalid state {:?}");
        m.insert(11, "Invalid state transition. Current state = {}");
        m.insert(12, "VF Stop for PCIe function: {:?}");
        m.insert(13, "[Vf_Stop] Unsupported event, expected StartCmd");
        m.insert(14, "Late IPC_HSM_SHUTDOWN_RSP after {}us");
        m.insert(15, "Late IPC_FP_SHUTDOWN_RSP after {}us");
        m.insert(16, "Late resource {} acquire {}us");
        m.insert(17, "Starting");
        m.insert(18, "Stopped after {}us");
        m.insert(19, "Admin abort with status: {:?}. State: ({:?})");
        m.insert(20, "HSM abort with status: {:?}. State: ({:?})");
        m.insert(21, "FP abort with status: {:?}. State: ({:?})");
        m.insert(22, "error_dfu: IPC_SP_SHUTDOWN_RSP {:?}");
        m.insert(23, "Timeout in state {:?} resources: {:#x}");
        m.insert(24, "handle_dfu_started: IPC_SP_SHUTDOWN_RSP {:?}");
        m.insert(25, "Insufficient time to drain CP HSM/FP.");
        m.insert(26, "IPC_CP_HSM_SHUTDOWN_REQ {:#x}");
        m.insert(27, "IPC_FP_SHUTDOWN_REQ {:#x}");
        m.insert(28, "Ready in {} us");
        m.insert(29, "iDFU Abort with OperationTimeout: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}");
        m.insert(30, "iDFU Abort with OperationFailed: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}");
        m.insert(31, "IPC_SP_SHUTDOWN_RSP {:#x}");
        m.insert(32, "No reset detected.");
        m.insert(33, "Spurious IPC_CP_HSM_SHUTDOWN_RSP after {}us");
        m.insert(34, "Spurious IPC_FP_SHUTDOWN_RSP after {}us");
        m.insert(35, "Failed to receive IPC message from HSM");
        m.insert(36, "Failed to decode IPC message from HSM");
        m.insert(37, "Failed to receive IPC message from FP");
        m.insert(38, "Failed to decode IPC message from FP");
        m.insert(39, "Fail to receive queue_delete_notification");
        m.insert(40, "Mismatch tags: received ({:?}) vs expected ({:?})");
        m.insert(41, "Received deferred action for a different controller: {:?}");
        m.insert(42, "Error in enabling controller {:?}");
        m.insert(43, "[cntrl] Failed to send IPC message to FP: {:?}");
        m.insert(44, "[cntrl] Failed to send IPC message to HSM: {:?}");
        m.insert(45, "[delete_sq] Invalid SQE: {:?}");
        m.insert(46, "Get device Sq Failed {:?}");
        m.insert(47, "Process HSM IPC failed {:?}");
        m.insert(48, "[delete_sq] Delete device queue on_hsm_ipc_response: Delete submission queue failed {:?}");
        m.insert(49, "Process FP IPC failed {:?}");
        m.insert(50, "[delete_sq] Delete device queue on_fp_ipc_response: Delete submission queue failed {:?}");
        m.insert(51, "[delete_sq] on_deferred_queue_deletion: Delete submission queue failed {:?}");
        m.insert(52, "Failed to send FP IPC request {:?}");
        m.insert(53, "Failed to send HSM IPC request {:?}");
        m.insert(54, "Complete VFLR for {:?}");
        m.insert(55, "[vflr] Failed to send IPC message to FP: {:?}");
        m.insert(56, "[vflr] Failed to send IPC message to HSM: {:?}");
        m.insert(57, "VF Prepare for PCIe function: {:?}");
        m.insert(58, "[Vf Prepare] Unsupported event");
        m.insert(59, "[tdisp_int] TdispIntFsm timer elapsed, state: {}");
        m.insert(60, "[tdisp_int] Invalid interrupt source, source_mask: {}");
        m.insert(61, "[tdisp_int] Failed to send IPC message to HSP: {:?}");
        m.insert(62, "[tdisp_int] Failed to decode IPC message: {:?}");
        m.insert(63, "[tdisp_int] Invalid IPC response with status {}");
        m.insert(64, "[tdisp_int] Spurious Message");
        m.insert(65, "VF Restore for PCIe function: {:?}");
        m.insert(66, "[Vf Restore] Unsupported event");
        m.insert(67, "VF Save for PCIe function: {:?}");
        m.insert(68, "[Vf Save] Unsupported event");
        m.insert(69, "VF Start for PCIe function: {:?}");
        m.insert(70, "[Vf_Start] Unsupported event, expected StartCmd");
        m.insert(71, "[get_features] Invalid SQE: {:?}");
        m.insert(72, "[get_res] Invalid SQE: {:?}");
        m.insert(73, "[stop_interface] Timer elapsed, state: {:?}");
        m.insert(74, "[stop_interface] Failed to receive notification from queue");
        m.insert(75, "[stop_interface] Message tag {} is not expected {}");
        m.insert(76, "[stop_interface] Failed to send IPC message to FP: {:?}");
        m.insert(77, "[stop_interface] Failed to send IPC message to HSM: {:?}");
        m.insert(78, "[stop_interface] Failed to send response to HSP: {:?}");
        m.insert(79, "[doe] on_rx_ready Failed to recv DOE message: {:?}");
        m.insert(80, "[doe] on_doe_go: Failed to recv DOE message: {:?}");
        m.insert(81, "Failed to end recv DOE message: {:?}");
        m.insert(82, "Failed to decode IPC message: {:?}");
        m.insert(83, "[doe] Invalid IPC response with status {}");
        m.insert(84, "[doe] Spurious Message");
        m.insert(85, "[doe] on_ipc_response: Failed to send DOE message: {:?}");
        m.insert(86, "[doe] on_tx_ready: Failed to send DOE message: {:?}");
        m.insert(87, "Failed to send IPC message to HSP: {:?}");
        m.insert(88, "PCIE_PERST_UP Error: 0x{:08x}");
        m.insert(89, "PF FLR complete");
        m.insert(90, "Perst Down complete");
        m.insert(91, "[set_res] Invalid SQE: {:?}");
        m.insert(92, "Failed to set resource count: {:?}");
        m.insert(93, "[set_res] Invalid IPC response with status {}");
        m.insert(94, "Statemachine error {}");
        m.insert(95, "AES-GCM Decrypt Tag Mismatch");
        m.insert(96, "Failed to send AES GCM response entry to response queue");
        m.insert(97, "Invalid state transition in AesGcmExtFsm. Current state = {}");
        m.insert(98, "[delete_cq] Invalid SQE: {:?}");
        m.insert(99, "Delete Cq Failed {:?}");
        m.insert(100, "[set_features] Invalid SQE: {:?}");
        m.insert(101, "Failed the self test {:?}");
        m.insert(102, "Preop-Self test failed for {:?} with error {}");
        m.insert(103, "Invalid IPC message header");
        m.insert(104, "Cannot decode opcode. Invalid IPC message opcode");
        m.insert(105, "Opcode does not match. Invalid IPC message opcode");
        m.insert(106, "Invalid IPC message");
        m.insert(107, "Invalid IPC message for Negative self test.");
        m.insert(108, "Invalid test ID in negative self test IPC payload.");
        m.insert(109, "Invalid Header found in IPC message");
        m.insert(110, "Invalid Shutdown for reset request message");
        m.insert(111, "handle_invalid_message_opcode: Invalid IPC message header {:?}");
        m.insert(112, "Unhandled message with opcode {:#X}");
        m.insert(113, "Failed to send response to HSP core {:?}");
        m.insert(114, "send_hsm_ipc_response: Invalid IPC message header {:?}");
        m.insert(115, "Failed to send response to hsm core {:?}");
        m.insert(116, "DTCM error received. Fault code: {}");
        m.insert(117, "ITCM error received. Fault code: {}");
        m.insert(118, "GDMA Data Structure Error. Fault code: {}");
        m.insert(119, "GDMA Data Access Error. Fault code: {}");
        m.insert(120, "GDMA Delivery Queue Error. Fault code: {}");
        m.insert(121, "GDMA Completion Queue Error. Fault code: {}");
        m
    };
}

lazy_static::lazy_static! {
    pub static ref MANTICORE_ADMIN_LOG_TOKENS_INDEX_TO_PREFIX_MAP: HashMap<u8, &'static str> = {
        let mut m = HashMap::new();
        m.insert(0, "[mcr_admin::preop_cdma_io]");
        m.insert(1, "[mcr_admin::main]");
        m.insert(2, "[mcr_admin::fsm::telemetry]");
        m.insert(3, "[mcr_admin::fsm::telemetry]");
        m.insert(4, "[mcr_admin::fsm::telemetry]");
        m.insert(5, "[mcr_admin::fsm::telemetry]");
        m.insert(6, "[mcr_admin::fsm::telemetry]");
        m.insert(7, "[mcr_admin::fsm::cast]");
        m.insert(8, "[mcr_admin::fsm::cast]");
        m.insert(9, "[mcr_admin::fsm::cast]");
        m.insert(10, "[mcr_admin::fsm::cast]");
        m.insert(11, "[mcr_admin::fsm::admin_fsm]");
        m.insert(12, "[mcr_admin::fsm::vf_stop]");
        m.insert(13, "[mcr_admin::fsm::vf_stop]");
        m.insert(14, "[mcr_admin::fsm::idfu]");
        m.insert(15, "[mcr_admin::fsm::idfu]");
        m.insert(16, "[mcr_admin::fsm::idfu]");
        m.insert(17, "[mcr_admin::fsm::idfu]");
        m.insert(18, "[mcr_admin::fsm::idfu]");
        m.insert(19, "[mcr_admin::fsm::idfu]");
        m.insert(20, "[mcr_admin::fsm::idfu]");
        m.insert(21, "[mcr_admin::fsm::idfu]");
        m.insert(22, "[mcr_admin::fsm::idfu]");
        m.insert(23, "[mcr_admin::fsm::idfu]");
        m.insert(24, "[mcr_admin::fsm::idfu]");
        m.insert(25, "[mcr_admin::fsm::idfu]");
        m.insert(26, "[mcr_admin::fsm::idfu]");
        m.insert(27, "[mcr_admin::fsm::idfu]");
        m.insert(28, "[mcr_admin::fsm::idfu]");
        m.insert(29, "[mcr_admin::fsm::idfu]");
        m.insert(30, "[mcr_admin::fsm::idfu]");
        m.insert(31, "[mcr_admin::fsm::idfu]");
        m.insert(32, "[mcr_admin::fsm::idfu]");
        m.insert(33, "[mcr_admin::fsm::idfu]");
        m.insert(34, "[mcr_admin::fsm::idfu]");
        m.insert(35, "[mcr_admin::fsm::cntrl]");
        m.insert(36, "[mcr_admin::fsm::cntrl]");
        m.insert(37, "[mcr_admin::fsm::cntrl]");
        m.insert(38, "[mcr_admin::fsm::cntrl]");
        m.insert(39, "[mcr_admin::fsm::cntrl]");
        m.insert(40, "[mcr_admin::fsm::cntrl]");
        m.insert(41, "[mcr_admin::fsm::cntrl]");
        m.insert(42, "[mcr_admin::fsm::cntrl]");
        m.insert(43, "[mcr_admin::fsm::cntrl]");
        m.insert(44, "[mcr_admin::fsm::cntrl]");
        m.insert(45, "[mcr_admin::fsm::delete_sq]");
        m.insert(46, "[mcr_admin::fsm::delete_sq]");
        m.insert(47, "[mcr_admin::fsm::delete_sq]");
        m.insert(48, "[mcr_admin::fsm::delete_sq]");
        m.insert(49, "[mcr_admin::fsm::delete_sq]");
        m.insert(50, "[mcr_admin::fsm::delete_sq]");
        m.insert(51, "[mcr_admin::fsm::delete_sq]");
        m.insert(52, "[mcr_admin::fsm::delete_sq]");
        m.insert(53, "[mcr_admin::fsm::delete_sq]");
        m.insert(54, "[mcr_admin::fsm::vflr]");
        m.insert(55, "[mcr_admin::fsm::vflr]");
        m.insert(56, "[mcr_admin::fsm::vflr]");
        m.insert(57, "[mcr_admin::fsm::vf_prep]");
        m.insert(58, "[mcr_admin::fsm::vf_prep]");
        m.insert(59, "[mcr_admin::fsm::tdisp_int]");
        m.insert(60, "[mcr_admin::fsm::tdisp_int]");
        m.insert(61, "[mcr_admin::fsm::tdisp_int]");
        m.insert(62, "[mcr_admin::fsm::tdisp_int]");
        m.insert(63, "[mcr_admin::fsm::tdisp_int]");
        m.insert(64, "[mcr_admin::fsm::tdisp_int]");
        m.insert(65, "[mcr_admin::fsm::vf_restore]");
        m.insert(66, "[mcr_admin::fsm::vf_restore]");
        m.insert(67, "[mcr_admin::fsm::vf_save]");
        m.insert(68, "[mcr_admin::fsm::vf_save]");
        m.insert(69, "[mcr_admin::fsm::vf_start]");
        m.insert(70, "[mcr_admin::fsm::vf_start]");
        m.insert(71, "[mcr_admin::fsm::get_features]");
        m.insert(72, "[mcr_admin::fsm::get_res]");
        m.insert(73, "[mcr_admin::fsm::stop_interface]");
        m.insert(74, "[mcr_admin::fsm::stop_interface]");
        m.insert(75, "[mcr_admin::fsm::stop_interface]");
        m.insert(76, "[mcr_admin::fsm::stop_interface]");
        m.insert(77, "[mcr_admin::fsm::stop_interface]");
        m.insert(78, "[mcr_admin::fsm::stop_interface]");
        m.insert(79, "[mcr_admin::fsm::doe]");
        m.insert(80, "[mcr_admin::fsm::doe]");
        m.insert(81, "[mcr_admin::fsm::doe]");
        m.insert(82, "[mcr_admin::fsm::doe]");
        m.insert(83, "[mcr_admin::fsm::doe]");
        m.insert(84, "[mcr_admin::fsm::doe]");
        m.insert(85, "[mcr_admin::fsm::doe]");
        m.insert(86, "[mcr_admin::fsm::doe]");
        m.insert(87, "[mcr_admin::fsm::doe]");
        m.insert(88, "[mcr_admin::fsm::pcie]");
        m.insert(89, "[mcr_admin::fsm::pcie]");
        m.insert(90, "[mcr_admin::fsm::pcie]");
        m.insert(91, "[mcr_admin::fsm::set_res]");
        m.insert(92, "[mcr_admin::fsm::set_res]");
        m.insert(93, "[mcr_admin::fsm::set_res]");
        m.insert(94, "[mcr_admin::fsm::aes_gcm_ext]");
        m.insert(95, "[mcr_admin::fsm::aes_gcm_ext]");
        m.insert(96, "[mcr_admin::fsm::aes_gcm_ext]");
        m.insert(97, "[mcr_admin::fsm::aes_gcm_ext]");
        m.insert(98, "[mcr_admin::fsm::delete_cq]");
        m.insert(99, "[mcr_admin::fsm::delete_cq]");
        m.insert(100, "[mcr_admin::fsm::set_features]");
        m.insert(101, "[mcr_admin::env]");
        m.insert(102, "[mcr_admin::env]");
        m.insert(103, "[mcr_admin::handler]");
        m.insert(104, "[mcr_admin::handler]");
        m.insert(105, "[mcr_admin::handler]");
        m.insert(106, "[mcr_admin::handler]");
        m.insert(107, "[mcr_admin::handler]");
        m.insert(108, "[mcr_admin::handler]");
        m.insert(109, "[mcr_admin::handler]");
        m.insert(110, "[mcr_admin::handler]");
        m.insert(111, "[mcr_admin::handler]");
        m.insert(112, "[mcr_admin::handler]");
        m.insert(113, "[mcr_admin::handler]");
        m.insert(114, "[mcr_admin::handler]");
        m.insert(115, "[mcr_admin::handler]");
        m.insert(116, "[app::exception_handlers]");
        m.insert(117, "[app::exception_handlers]");
        m.insert(118, "[app::exception_handlers]");
        m.insert(119, "[app::exception_handlers]");
        m.insert(120, "[app::exception_handlers]");
        m.insert(121, "[app::exception_handlers]");
        m
    };
}

#[allow(dead_code)]
pub const MANTICORE_ADMIN_LOG_TOKENS: [&str; 122] = [
    "IPC response status: {:x}",
    "Starting Admin event loop...",
    "Telemetry FSM received unexpected event. Event: {:?}",
    "Error running telemetry PCIe monitor, code {:#x}",
    "Telemetry PCIe monitor is running normally. Link Speed: {}, Link Width: x{}",
    "Unexpected PCIe Link. Link Speed: {}, Link Width: {}",
    "Telemetry PCIe monitor not able to start, code {:#x}",
    "Received timer event while waiting for cast resource",
    "Invalid event. state: {:?}",
    "on_fp_to_admin_channel_ready: Invalid state {:?}",
    "on_fp_to_admin_ipc_response: Invalid state {:?}",
    "Invalid state transition. Current state = {}",
    "VF Stop for PCIe function: {:?}",
    "[Vf_Stop] Unsupported event, expected StartCmd",
    "Late IPC_HSM_SHUTDOWN_RSP after {}us",
    "Late IPC_FP_SHUTDOWN_RSP after {}us",
    "Late resource {} acquire {}us",
    "Starting",
    "Stopped after {}us",
    "Admin abort with status: {:?}. State: ({:?})",
    "HSM abort with status: {:?}. State: ({:?})",
    "FP abort with status: {:?}. State: ({:?})",
    "error_dfu: IPC_SP_SHUTDOWN_RSP {:?}",
    "Timeout in state {:?} resources: {:#x}",
    "handle_dfu_started: IPC_SP_SHUTDOWN_RSP {:?}",
    "Insufficient time to drain CP HSM/FP.",
    "IPC_CP_HSM_SHUTDOWN_REQ {:#x}",
    "IPC_FP_SHUTDOWN_REQ {:#x}",
    "Ready in {} us",
    "iDFU Abort with OperationTimeout: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}",
    "iDFU Abort with OperationFailed: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}",
    "IPC_SP_SHUTDOWN_RSP {:#x}",
    "No reset detected.",
    "Spurious IPC_CP_HSM_SHUTDOWN_RSP after {}us",
    "Spurious IPC_FP_SHUTDOWN_RSP after {}us",
    "Failed to receive IPC message from HSM",
    "Failed to decode IPC message from HSM",
    "Failed to receive IPC message from FP",
    "Failed to decode IPC message from FP",
    "Fail to receive queue_delete_notification",
    "Mismatch tags: received ({:?}) vs expected ({:?})",
    "Received deferred action for a different controller: {:?}",
    "Error in enabling controller {:?}",
    "[cntrl] Failed to send IPC message to FP: {:?}",
    "[cntrl] Failed to send IPC message to HSM: {:?}",
    "[delete_sq] Invalid SQE: {:?}",
    "Get device Sq Failed {:?}",
    "Process HSM IPC failed {:?}",
    "[delete_sq] Delete device queue on_hsm_ipc_response: Delete submission queue failed {:?}",
    "Process FP IPC failed {:?}",
    "[delete_sq] Delete device queue on_fp_ipc_response: Delete submission queue failed {:?}",
    "[delete_sq] on_deferred_queue_deletion: Delete submission queue failed {:?}",
    "Failed to send FP IPC request {:?}",
    "Failed to send HSM IPC request {:?}",
    "Complete VFLR for {:?}",
    "[vflr] Failed to send IPC message to FP: {:?}",
    "[vflr] Failed to send IPC message to HSM: {:?}",
    "VF Prepare for PCIe function: {:?}",
    "[Vf Prepare] Unsupported event",
    "[tdisp_int] TdispIntFsm timer elapsed, state: {}",
    "[tdisp_int] Invalid interrupt source, source_mask: {}",
    "[tdisp_int] Failed to send IPC message to HSP: {:?}",
    "[tdisp_int] Failed to decode IPC message: {:?}",
    "[tdisp_int] Invalid IPC response with status {}",
    "[tdisp_int] Spurious Message",
    "VF Restore for PCIe function: {:?}",
    "[Vf Restore] Unsupported event",
    "VF Save for PCIe function: {:?}",
    "[Vf Save] Unsupported event",
    "VF Start for PCIe function: {:?}",
    "[Vf_Start] Unsupported event, expected StartCmd",
    "[get_features] Invalid SQE: {:?}",
    "[get_res] Invalid SQE: {:?}",
    "[stop_interface] Timer elapsed, state: {:?}",
    "[stop_interface] Failed to receive notification from queue",
    "[stop_interface] Message tag {} is not expected {}",
    "[stop_interface] Failed to send IPC message to FP: {:?}",
    "[stop_interface] Failed to send IPC message to HSM: {:?}",
    "[stop_interface] Failed to send response to HSP: {:?}",
    "[doe] on_rx_ready Failed to recv DOE message: {:?}",
    "[doe] on_doe_go: Failed to recv DOE message: {:?}",
    "Failed to end recv DOE message: {:?}",
    "Failed to decode IPC message: {:?}",
    "[doe] Invalid IPC response with status {}",
    "[doe] Spurious Message",
    "[doe] on_ipc_response: Failed to send DOE message: {:?}",
    "[doe] on_tx_ready: Failed to send DOE message: {:?}",
    "Failed to send IPC message to HSP: {:?}",
    "PCIE_PERST_UP Error: 0x{:08x}",
    "PF FLR complete",
    "Perst Down complete",
    "[set_res] Invalid SQE: {:?}",
    "Failed to set resource count: {:?}",
    "[set_res] Invalid IPC response with status {}",
    "Statemachine error {}",
    "AES-GCM Decrypt Tag Mismatch",
    "Failed to send AES GCM response entry to response queue",
    "Invalid state transition in AesGcmExtFsm. Current state = {}",
    "[delete_cq] Invalid SQE: {:?}",
    "Delete Cq Failed {:?}",
    "[set_features] Invalid SQE: {:?}",
    "Failed the self test {:?}",
    "Preop-Self test failed for {:?} with error {}",
    "Invalid IPC message header",
    "Cannot decode opcode. Invalid IPC message opcode",
    "Opcode does not match. Invalid IPC message opcode",
    "Invalid IPC message",
    "Invalid IPC message for Negative self test.",
    "Invalid test ID in negative self test IPC payload.",
    "Invalid Header found in IPC message",
    "Invalid Shutdown for reset request message",
    "handle_invalid_message_opcode: Invalid IPC message header {:?}",
    "Unhandled message with opcode {:#X}",
    "Failed to send response to HSP core {:?}",
    "send_hsm_ipc_response: Invalid IPC message header {:?}",
    "Failed to send response to hsm core {:?}",
    "DTCM error received. Fault code: {}",
    "ITCM error received. Fault code: {}",
    "GDMA Data Structure Error. Fault code: {}",
    "GDMA Data Access Error. Fault code: {}",
    "GDMA Delivery Queue Error. Fault code: {}",
    "GDMA Completion Queue Error. Fault code: {}",
];
