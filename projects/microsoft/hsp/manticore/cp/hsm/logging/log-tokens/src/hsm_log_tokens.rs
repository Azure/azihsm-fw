// Copyright (c) Microsoft Corporation. All rights reserved.
// This is an auto-generated file. Please do not modify manually.
// To regenerate use command: `cargo xtask telemetry-tokenize`

use hashbrown::HashMap;

lazy_static::lazy_static! {
    pub static ref MANTICORE_HSM_LOG_TOKENS_MAP: HashMap<&'static str, u8> = {
        let mut m = HashMap::new();
        m.insert("Starting HSM event loop...", 0);
        m.insert("[delete_key] Invalid event, state:{:?}, event: {:?}", 1);
        m.insert("[delete_key], begin_delete_aesbulk256_key returned err: {:?}", 2);
        m.insert("GetUnwrappingKey command timed out. {:?}", 3);
        m.insert("[ecdh_key_exchange] Invalid State, state:{:?} event: {:?}", 4);
        m.insert("[ecdh_key_exchange] Invalid Event, state:{:?} event: {:?}", 5);
        m.insert("[ecc_gen_key] [op state: {:?}] PCT Validation Error: {:?}.", 6);
        m.insert("[get_establish_cred_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.", 7);
        m.insert("[attest_key] Invalid State, state:{:?}, event: {:?}", 8);
        m.insert("[attest_key] Invalid Event, state:{:?}, event: {:?}", 9);
        m.insert("[unmask_key] Invalid Event, state:{:?}, event: {:?}", 10);
        m.insert("[part_init] Unexpected event {} ", 11);
        m.insert("[part_init] Gen ID phase - expect to find UPKA engine on res ready", 12);
        m.insert("[part_init] Failed begin part id generation {}", 13);
        m.insert("[part_init] No context available for ending partition identifier generation.", 14);
        m.insert("[part_init] Failed Continue part id generation: {}", 15);
        m.insert("[part_init] Failed begin PCT validation: {}", 16);
        m.insert("[part_init] Failed PCT final validation", 17);
        m.insert("[part_init] Failed end PCT validation: {}", 18);
        m.insert("[part_init] Failed continue PCT validation: {}", 19);
        m.insert("[establish_credential] on_engine_ready begin_establish_credential err: {:?}", 20);
        m.insert("[hsm_fsm] IO Timed out while in {:?}", 21);
        m.insert("[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_in_dma", 22);
        m.insert("[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_out_dma", 23);
        m.insert("send_err_cqe: IO channel send error. (0x{:08x})", 24);
        m.insert("send_cqe: DMA buffer not found", 25);
        m.insert("send_cqe: IO channel send error. (0x{:08x})", 26);
        m.insert("IO channel send error. (0x{:08x})", 27);
        m.insert("[close_session] Invalid Event, state:{:?}, event: {:?}", 28);
        m.insert("[close_session] begin_close_user_session err: {:?}", 29);
        m.insert("[open_session] on_engine_ready: begin_open_session err: {:?}", 30);
        m.insert("[rsa_mod_exp] Invalid State, state:{:?} event: {:?}", 31);
        m.insert("[rsa_mod_exp] Invalid Event, state:{:?} event: {:?}", 32);
        m.insert("[test_action] Missing negative self test ID", 33);
        m.insert("[test_action] Missing pin policy config", 34);
        m.insert("Invalid rng failure test id received", 35);
        m.insert("[test_action] trigger_crashdump_local: Missing crash info", 36);
        m.insert("[test_action] send_crashdump_request: Missing crash info", 37);
        m.insert("[test_action] Invalid negative self test ID", 38);
        m.insert("[flush_session] Invalid Event, state:{:?}, event: {:?}", 39);
        m.insert("[flush_session] begin_close_user_session returned err: {:?}", 40);
        m.insert("[get_session_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.", 41);
        m.insert("[aes_gen_key] Invalid Event, state:{:?}, event: {:?}", 42);
        m.insert("[open_key] Invalid State, state:{:?} event: {:?}", 43);
        m.insert("[open_key] Invalid Event, state:{:?} event: {:?}", 44);
        m.insert("[ecc_sign] Invalid State, state:{:?} event: {:?}", 45);
        m.insert("[ecc_sign] Invalid Event, state:{:?} event: {:?}", 46);
        m.insert("Unexpected: Resource cleanup for an already cleaned up resource {}.", 47);
        m.insert("Unexpected: Resource cleanup completed for resource {}.", 48);
        m.insert("PKA instance {} cleanup error", 49);
        m.insert("[der_key_import] Invalid State, state:{:?}, event: {:?}", 50);
        m.insert("[der_key_import] Invalid Event, state:{:?}, event: {:?}", 51);
        m.insert("[rsa_unwrap] Invalid State, state:{:?} event: {:?}", 52);
        m.insert("[rsa_unwrap] Invalid Event, state:{:?} event: {:?}", 53);
        m.insert("Timeout!! State: {:?} Res Op State: {:?}", 54);
        m.insert("[rsa2k_mod] Failed to send self test response", 55);
        m.insert("[rsa2k_crt] Failed to send self test response", 56);
        m.insert("[ecdsa_engine] Failed to send self test response", 57);
        m.insert("[ecdh_engine] Failed to send self test response", 58);
        m.insert("[rng] Failed to send self test response", 59);
        m.insert("Failed to send self test response", 60);
        m.insert("[ecc] ECC structural validation Public key length mismatch", 61);
        m.insert("[ecc] ECC structural validation Public key mismatch", 62);
        m.insert("[ecc] ECC structural validation Private key length mismatch", 63);
        m.insert("ECC structural validation Scalar d is not in the range 0 < d < n", 64);
        m.insert("send_crashdump_request: Failed to send IPC message to Admin: {:?}", 65);
        m.insert("[mod] Failed to send IPC message to FP: {:?}", 66);
        m.insert("Invalid core id", 67);
        m.insert("begin_neg_self_test_req: Failed to send IPC message to Admin: {:?}", 68);
        m.insert("[mod] Failed to decode IPC message header: {:?}", 69);
        m.insert("Negative self test failed with status: {}", 70);
        m.insert("[aes] begin_import_der_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}", 71);
        m.insert("[aes] end_import_der_aesbulk256_key_inner: Failed to decode IPC message header: {:?}", 72);
        m.insert("[aes] Import der key: Invalid IPC response with status {}", 73);
        m.insert("[aes] end_import_der_aesbulk256_key_inner: Spurious Message", 74);
        m.insert("[aes] begin_delete_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}", 75);
        m.insert("[aes] end_delete_aesbulk256_key_inner: Failed to decode IPC message header: {:?}", 76);
        m.insert("[aes] Delete Key Operation: Invalid IPC response with status {}", 77);
        m.insert("[aes] end_delete_aesbulk256_key_inner: Spurious Message", 78);
        m.insert("[aes] begin_rollback_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}", 79);
        m.insert("[aes] end_rollback_aesbulk256_key_inner: Failed to decode IPC message header: {:?}", 80);
        m.insert("[aes] Delete key operation rollback: Invalid IPC response with status {}", 81);
        m.insert("[aes] end_rollback_aesbulk256_key_inner: Spurious Message", 82);
        m.insert("[part] delete_session_keys: failed to delete key key_id={:?}, Error: {:?}", 83);
        m.insert("[part] Failed to send IPC message to FP: {}", 84);
        m.insert("[part] Failed to decode IPC message header: {:?}", 85);
        m.insert("[part] Invalid IPC response with status {}", 86);
        m.insert("[part] Spurious Message", 87);
        m.insert("Establish cred key deletion failed {:?}. Ignoring error.", 88);
        m.insert("Failed the self test {:?}", 89);
        m.insert("PCT Validation Failure: {:?}. System Reset Required! ", 90);
        m.insert("Preop-Self test failed for {:?} with error {}", 91);
        m.insert("on_request_ready: spurious event", 92);
        m.insert("on_response_complete: spurious event", 93);
        m.insert("on_dma_complete: spurious event", 94);
        m.insert("Failed to send FLR complete event", 95);
        m.insert("on_flr: spurious event", 96);
        m.insert("Invalid Header found in IPC message", 97);
        m.insert("Invalid PcieFunction enable/disable message", 98);
        m.insert("Invalid Set Resource message {:?}", 99);
        m.insert("Failed to allocate partition init FSM", 100);
        m.insert("Invalid IPC Create Delete Submission Queue Message {:?}", 101);
        m.insert("{:?} not Enabled", 102);
        m.insert("Invalid Shutdown for reset request message", 103);
        m.insert("iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err", 104);
        m.insert("Failed to send prepare shutdown response to Admin core {:?}", 105);
        m.insert("Schedule is busy in draining IOs", 106);
        m.insert("iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err", 107);
        m.insert("iDFU Fault Injected: idfu_fault_pre_reset_hsm_drain_timeout", 108);
        m.insert("Failed to get CDMA vault key entry", 109);
        m.insert("Invalid IPC message header {:?}", 110);
        m.insert("Unhandled message with opcode {:#X}", 111);
        m.insert("Failed to send response to Admin core {:?}", 112);
        m.insert("on_pka: spurious event. Index: {}", 113);
        m.insert("Invalid Header found for fp to hsm IPC message", 114);
        m.insert("Correctable ECC errors have exceeded threshold, Key vault memory reloaded", 115);
        m.insert("Failed to send response to FP core {:?}", 116);
        m.insert("Invalid op code found in IPC message", 117);
        m.insert("on_fp_ipc_response: spurious event", 118);
        m.insert("on_hsp_ipc_response: spurious event", 119);
        m.insert("on_admin_ipc_response: spurious event", 120);
        m.insert("on_aes_unwrap_resp: spurious event", 121);
        m.insert("Failed to receive self test request", 122);
        m.insert("RNG Hardware error received. Fault code: {}", 123);
        m.insert("DTCM error received. Fault code: {}", 124);
        m.insert("ITCM error received. Fault code: {}", 125);
        m
    };
}

lazy_static::lazy_static! {
    pub static ref MANTICORE_HSM_LOG_TOKENS_INDEX_TO_MESSAGE_MAP: HashMap<u8, &'static str> = {
        let mut m = HashMap::new();
        m.insert(0, "Starting HSM event loop...");
        m.insert(1, "[delete_key] Invalid event, state:{:?}, event: {:?}");
        m.insert(2, "[delete_key], begin_delete_aesbulk256_key returned err: {:?}");
        m.insert(3, "GetUnwrappingKey command timed out. {:?}");
        m.insert(4, "[ecdh_key_exchange] Invalid State, state:{:?} event: {:?}");
        m.insert(5, "[ecdh_key_exchange] Invalid Event, state:{:?} event: {:?}");
        m.insert(6, "[ecc_gen_key] [op state: {:?}] PCT Validation Error: {:?}.");
        m.insert(7, "[get_establish_cred_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.");
        m.insert(8, "[attest_key] Invalid State, state:{:?}, event: {:?}");
        m.insert(9, "[attest_key] Invalid Event, state:{:?}, event: {:?}");
        m.insert(10, "[unmask_key] Invalid Event, state:{:?}, event: {:?}");
        m.insert(11, "[part_init] Unexpected event {} ");
        m.insert(12, "[part_init] Gen ID phase - expect to find UPKA engine on res ready");
        m.insert(13, "[part_init] Failed begin part id generation {}");
        m.insert(14, "[part_init] No context available for ending partition identifier generation.");
        m.insert(15, "[part_init] Failed Continue part id generation: {}");
        m.insert(16, "[part_init] Failed begin PCT validation: {}");
        m.insert(17, "[part_init] Failed PCT final validation");
        m.insert(18, "[part_init] Failed end PCT validation: {}");
        m.insert(19, "[part_init] Failed continue PCT validation: {}");
        m.insert(20, "[establish_credential] on_engine_ready begin_establish_credential err: {:?}");
        m.insert(21, "[hsm_fsm] IO Timed out while in {:?}");
        m.insert(22, "[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_in_dma");
        m.insert(23, "[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_out_dma");
        m.insert(24, "send_err_cqe: IO channel send error. (0x{:08x})");
        m.insert(25, "send_cqe: DMA buffer not found");
        m.insert(26, "send_cqe: IO channel send error. (0x{:08x})");
        m.insert(27, "IO channel send error. (0x{:08x})");
        m.insert(28, "[close_session] Invalid Event, state:{:?}, event: {:?}");
        m.insert(29, "[close_session] begin_close_user_session err: {:?}");
        m.insert(30, "[open_session] on_engine_ready: begin_open_session err: {:?}");
        m.insert(31, "[rsa_mod_exp] Invalid State, state:{:?} event: {:?}");
        m.insert(32, "[rsa_mod_exp] Invalid Event, state:{:?} event: {:?}");
        m.insert(33, "[test_action] Missing negative self test ID");
        m.insert(34, "[test_action] Missing pin policy config");
        m.insert(35, "Invalid rng failure test id received");
        m.insert(36, "[test_action] trigger_crashdump_local: Missing crash info");
        m.insert(37, "[test_action] send_crashdump_request: Missing crash info");
        m.insert(38, "[test_action] Invalid negative self test ID");
        m.insert(39, "[flush_session] Invalid Event, state:{:?}, event: {:?}");
        m.insert(40, "[flush_session] begin_close_user_session returned err: {:?}");
        m.insert(41, "[get_session_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.");
        m.insert(42, "[aes_gen_key] Invalid Event, state:{:?}, event: {:?}");
        m.insert(43, "[open_key] Invalid State, state:{:?} event: {:?}");
        m.insert(44, "[open_key] Invalid Event, state:{:?} event: {:?}");
        m.insert(45, "[ecc_sign] Invalid State, state:{:?} event: {:?}");
        m.insert(46, "[ecc_sign] Invalid Event, state:{:?} event: {:?}");
        m.insert(47, "Unexpected: Resource cleanup for an already cleaned up resource {}.");
        m.insert(48, "Unexpected: Resource cleanup completed for resource {}.");
        m.insert(49, "PKA instance {} cleanup error");
        m.insert(50, "[der_key_import] Invalid State, state:{:?}, event: {:?}");
        m.insert(51, "[der_key_import] Invalid Event, state:{:?}, event: {:?}");
        m.insert(52, "[rsa_unwrap] Invalid State, state:{:?} event: {:?}");
        m.insert(53, "[rsa_unwrap] Invalid Event, state:{:?} event: {:?}");
        m.insert(54, "Timeout!! State: {:?} Res Op State: {:?}");
        m.insert(55, "[rsa2k_mod] Failed to send self test response");
        m.insert(56, "[rsa2k_crt] Failed to send self test response");
        m.insert(57, "[ecdsa_engine] Failed to send self test response");
        m.insert(58, "[ecdh_engine] Failed to send self test response");
        m.insert(59, "[rng] Failed to send self test response");
        m.insert(60, "Failed to send self test response");
        m.insert(61, "[ecc] ECC structural validation Public key length mismatch");
        m.insert(62, "[ecc] ECC structural validation Public key mismatch");
        m.insert(63, "[ecc] ECC structural validation Private key length mismatch");
        m.insert(64, "ECC structural validation Scalar d is not in the range 0 < d < n");
        m.insert(65, "send_crashdump_request: Failed to send IPC message to Admin: {:?}");
        m.insert(66, "[mod] Failed to send IPC message to FP: {:?}");
        m.insert(67, "Invalid core id");
        m.insert(68, "begin_neg_self_test_req: Failed to send IPC message to Admin: {:?}");
        m.insert(69, "[mod] Failed to decode IPC message header: {:?}");
        m.insert(70, "Negative self test failed with status: {}");
        m.insert(71, "[aes] begin_import_der_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}");
        m.insert(72, "[aes] end_import_der_aesbulk256_key_inner: Failed to decode IPC message header: {:?}");
        m.insert(73, "[aes] Import der key: Invalid IPC response with status {}");
        m.insert(74, "[aes] end_import_der_aesbulk256_key_inner: Spurious Message");
        m.insert(75, "[aes] begin_delete_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}");
        m.insert(76, "[aes] end_delete_aesbulk256_key_inner: Failed to decode IPC message header: {:?}");
        m.insert(77, "[aes] Delete Key Operation: Invalid IPC response with status {}");
        m.insert(78, "[aes] end_delete_aesbulk256_key_inner: Spurious Message");
        m.insert(79, "[aes] begin_rollback_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}");
        m.insert(80, "[aes] end_rollback_aesbulk256_key_inner: Failed to decode IPC message header: {:?}");
        m.insert(81, "[aes] Delete key operation rollback: Invalid IPC response with status {}");
        m.insert(82, "[aes] end_rollback_aesbulk256_key_inner: Spurious Message");
        m.insert(83, "[part] delete_session_keys: failed to delete key key_id={:?}, Error: {:?}");
        m.insert(84, "[part] Failed to send IPC message to FP: {}");
        m.insert(85, "[part] Failed to decode IPC message header: {:?}");
        m.insert(86, "[part] Invalid IPC response with status {}");
        m.insert(87, "[part] Spurious Message");
        m.insert(88, "Establish cred key deletion failed {:?}. Ignoring error.");
        m.insert(89, "Failed the self test {:?}");
        m.insert(90, "PCT Validation Failure: {:?}. System Reset Required! ");
        m.insert(91, "Preop-Self test failed for {:?} with error {}");
        m.insert(92, "on_request_ready: spurious event");
        m.insert(93, "on_response_complete: spurious event");
        m.insert(94, "on_dma_complete: spurious event");
        m.insert(95, "Failed to send FLR complete event");
        m.insert(96, "on_flr: spurious event");
        m.insert(97, "Invalid Header found in IPC message");
        m.insert(98, "Invalid PcieFunction enable/disable message");
        m.insert(99, "Invalid Set Resource message {:?}");
        m.insert(100, "Failed to allocate partition init FSM");
        m.insert(101, "Invalid IPC Create Delete Submission Queue Message {:?}");
        m.insert(102, "{:?} not Enabled");
        m.insert(103, "Invalid Shutdown for reset request message");
        m.insert(104, "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err");
        m.insert(105, "Failed to send prepare shutdown response to Admin core {:?}");
        m.insert(106, "Schedule is busy in draining IOs");
        m.insert(107, "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err");
        m.insert(108, "iDFU Fault Injected: idfu_fault_pre_reset_hsm_drain_timeout");
        m.insert(109, "Failed to get CDMA vault key entry");
        m.insert(110, "Invalid IPC message header {:?}");
        m.insert(111, "Unhandled message with opcode {:#X}");
        m.insert(112, "Failed to send response to Admin core {:?}");
        m.insert(113, "on_pka: spurious event. Index: {}");
        m.insert(114, "Invalid Header found for fp to hsm IPC message");
        m.insert(115, "Correctable ECC errors have exceeded threshold, Key vault memory reloaded");
        m.insert(116, "Failed to send response to FP core {:?}");
        m.insert(117, "Invalid op code found in IPC message");
        m.insert(118, "on_fp_ipc_response: spurious event");
        m.insert(119, "on_hsp_ipc_response: spurious event");
        m.insert(120, "on_admin_ipc_response: spurious event");
        m.insert(121, "on_aes_unwrap_resp: spurious event");
        m.insert(122, "Failed to receive self test request");
        m.insert(123, "RNG Hardware error received. Fault code: {}");
        m.insert(124, "DTCM error received. Fault code: {}");
        m.insert(125, "ITCM error received. Fault code: {}");
        m
    };
}

lazy_static::lazy_static! {
    pub static ref MANTICORE_HSM_LOG_TOKENS_INDEX_TO_PREFIX_MAP: HashMap<u8, &'static str> = {
        let mut m = HashMap::new();
        m.insert(0, "[mcr_hsm::main]");
        m.insert(1, "[mcr_hsm::fsm::delete_key]");
        m.insert(2, "[mcr_hsm::fsm::delete_key]");
        m.insert(3, "[mcr_hsm::fsm::get_unwrapping_key]");
        m.insert(4, "[mcr_hsm::fsm::ecdh_key_exchange]");
        m.insert(5, "[mcr_hsm::fsm::ecdh_key_exchange]");
        m.insert(6, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(7, "[mcr_hsm::fsm::get_establish_cred_encryption_key]");
        m.insert(8, "[mcr_hsm::fsm::attest_key]");
        m.insert(9, "[mcr_hsm::fsm::attest_key]");
        m.insert(10, "[mcr_hsm::fsm::unmask_key]");
        m.insert(11, "[mcr_hsm::fsm::part_init]");
        m.insert(12, "[mcr_hsm::fsm::part_init]");
        m.insert(13, "[mcr_hsm::fsm::part_init]");
        m.insert(14, "[mcr_hsm::fsm::part_init]");
        m.insert(15, "[mcr_hsm::fsm::part_init]");
        m.insert(16, "[mcr_hsm::fsm::part_init]");
        m.insert(17, "[mcr_hsm::fsm::part_init]");
        m.insert(18, "[mcr_hsm::fsm::part_init]");
        m.insert(19, "[mcr_hsm::fsm::part_init]");
        m.insert(20, "[mcr_hsm::fsm::establish_credential]");
        m.insert(21, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(22, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(23, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(24, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(25, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(26, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(27, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(28, "[mcr_hsm::fsm::close_session]");
        m.insert(29, "[mcr_hsm::fsm::close_session]");
        m.insert(30, "[mcr_hsm::fsm::open_session]");
        m.insert(31, "[mcr_hsm::fsm::rsa_mod_exp]");
        m.insert(32, "[mcr_hsm::fsm::rsa_mod_exp]");
        m.insert(33, "[mcr_hsm::fsm::test_action]");
        m.insert(34, "[mcr_hsm::fsm::test_action]");
        m.insert(35, "[mcr_hsm::fsm::test_action]");
        m.insert(36, "[mcr_hsm::fsm::test_action]");
        m.insert(37, "[mcr_hsm::fsm::test_action]");
        m.insert(38, "[mcr_hsm::fsm::test_action]");
        m.insert(39, "[mcr_hsm::fsm::flush_session]");
        m.insert(40, "[mcr_hsm::fsm::flush_session]");
        m.insert(41, "[mcr_hsm::fsm::get_session_encryption_key]");
        m.insert(42, "[mcr_hsm::fsm::aes_gen_key]");
        m.insert(43, "[mcr_hsm::fsm::open_key]");
        m.insert(44, "[mcr_hsm::fsm::open_key]");
        m.insert(45, "[mcr_hsm::fsm::ecc_sign]");
        m.insert(46, "[mcr_hsm::fsm::ecc_sign]");
        m.insert(47, "[mcr_hsm::fsm::res_cleanup_fsm]");
        m.insert(48, "[mcr_hsm::fsm::res_cleanup_fsm]");
        m.insert(49, "[mcr_hsm::fsm::res_cleanup_fsm]");
        m.insert(50, "[mcr_hsm::fsm::der_key_import]");
        m.insert(51, "[mcr_hsm::fsm::der_key_import]");
        m.insert(52, "[mcr_hsm::fsm::rsa_unwrap]");
        m.insert(53, "[mcr_hsm::fsm::rsa_unwrap]");
        m.insert(54, "[mcr_hsm::fsm::rsa_unwrap]");
        m.insert(55, "[mcr_hsm::self_test_handler]");
        m.insert(56, "[mcr_hsm::self_test_handler]");
        m.insert(57, "[mcr_hsm::self_test_handler]");
        m.insert(58, "[mcr_hsm::self_test_handler]");
        m.insert(59, "[mcr_hsm::self_test_handler]");
        m.insert(60, "[mcr_hsm::self_test_handler]");
        m.insert(61, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(62, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(63, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(64, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(65, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(66, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(67, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(68, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(69, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(70, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(71, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(72, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(73, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(74, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(75, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(76, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(77, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(78, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(79, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(80, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(81, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(82, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(83, "[mcr_hsm::partition::part]");
        m.insert(84, "[mcr_hsm::partition::part]");
        m.insert(85, "[mcr_hsm::partition::part]");
        m.insert(86, "[mcr_hsm::partition::part]");
        m.insert(87, "[mcr_hsm::partition::part]");
        m.insert(88, "[mcr_hsm::partition::part]");
        m.insert(89, "[mcr_hsm::env]");
        m.insert(90, "[mcr_hsm::env]");
        m.insert(91, "[mcr_hsm::env]");
        m.insert(92, "[mcr_hsm::handler]");
        m.insert(93, "[mcr_hsm::handler]");
        m.insert(94, "[mcr_hsm::handler]");
        m.insert(95, "[mcr_hsm::handler]");
        m.insert(96, "[mcr_hsm::handler]");
        m.insert(97, "[mcr_hsm::handler]");
        m.insert(98, "[mcr_hsm::handler]");
        m.insert(99, "[mcr_hsm::handler]");
        m.insert(100, "[mcr_hsm::handler]");
        m.insert(101, "[mcr_hsm::handler]");
        m.insert(102, "[mcr_hsm::handler]");
        m.insert(103, "[mcr_hsm::handler]");
        m.insert(104, "[mcr_hsm::handler]");
        m.insert(105, "[mcr_hsm::handler]");
        m.insert(106, "[mcr_hsm::handler]");
        m.insert(107, "[mcr_hsm::handler]");
        m.insert(108, "[mcr_hsm::handler]");
        m.insert(109, "[mcr_hsm::handler]");
        m.insert(110, "[mcr_hsm::handler]");
        m.insert(111, "[mcr_hsm::handler]");
        m.insert(112, "[mcr_hsm::handler]");
        m.insert(113, "[mcr_hsm::handler]");
        m.insert(114, "[mcr_hsm::handler]");
        m.insert(115, "[mcr_hsm::handler]");
        m.insert(116, "[mcr_hsm::handler]");
        m.insert(117, "[mcr_hsm::handler]");
        m.insert(118, "[mcr_hsm::handler]");
        m.insert(119, "[mcr_hsm::handler]");
        m.insert(120, "[mcr_hsm::handler]");
        m.insert(121, "[mcr_hsm::handler]");
        m.insert(122, "[mcr_hsm::handler]");
        m.insert(123, "[app::exception_handlers]");
        m.insert(124, "[app::exception_handlers]");
        m.insert(125, "[app::exception_handlers]");
        m
    };
}

#[allow(dead_code)]
pub const MANTICORE_HSM_LOG_TOKENS: [&str; 126] = [
    "Starting HSM event loop...",
    "[delete_key] Invalid event, state:{:?}, event: {:?}",
    "[delete_key], begin_delete_aesbulk256_key returned err: {:?}",
    "GetUnwrappingKey command timed out. {:?}",
    "[ecdh_key_exchange] Invalid State, state:{:?} event: {:?}",
    "[ecdh_key_exchange] Invalid Event, state:{:?} event: {:?}",
    "[ecc_gen_key] [op state: {:?}] PCT Validation Error: {:?}.",
    "[get_establish_cred_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.",
    "[attest_key] Invalid State, state:{:?}, event: {:?}",
    "[attest_key] Invalid Event, state:{:?}, event: {:?}",
    "[unmask_key] Invalid Event, state:{:?}, event: {:?}",
    "[part_init] Unexpected event {} ",
    "[part_init] Gen ID phase - expect to find UPKA engine on res ready",
    "[part_init] Failed begin part id generation {}",
    "[part_init] No context available for ending partition identifier generation.",
    "[part_init] Failed Continue part id generation: {}",
    "[part_init] Failed begin PCT validation: {}",
    "[part_init] Failed PCT final validation",
    "[part_init] Failed end PCT validation: {}",
    "[part_init] Failed continue PCT validation: {}",
    "[establish_credential] on_engine_ready begin_establish_credential err: {:?}",
    "[hsm_fsm] IO Timed out while in {:?}",
    "[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_in_dma",
    "[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_out_dma",
    "send_err_cqe: IO channel send error. (0x{:08x})",
    "send_cqe: DMA buffer not found",
    "send_cqe: IO channel send error. (0x{:08x})",
    "IO channel send error. (0x{:08x})",
    "[close_session] Invalid Event, state:{:?}, event: {:?}",
    "[close_session] begin_close_user_session err: {:?}",
    "[open_session] on_engine_ready: begin_open_session err: {:?}",
    "[rsa_mod_exp] Invalid State, state:{:?} event: {:?}",
    "[rsa_mod_exp] Invalid Event, state:{:?} event: {:?}",
    "[test_action] Missing negative self test ID",
    "[test_action] Missing pin policy config",
    "Invalid rng failure test id received",
    "[test_action] trigger_crashdump_local: Missing crash info",
    "[test_action] send_crashdump_request: Missing crash info",
    "[test_action] Invalid negative self test ID",
    "[flush_session] Invalid Event, state:{:?}, event: {:?}",
    "[flush_session] begin_close_user_session returned err: {:?}",
    "[get_session_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.",
    "[aes_gen_key] Invalid Event, state:{:?}, event: {:?}",
    "[open_key] Invalid State, state:{:?} event: {:?}",
    "[open_key] Invalid Event, state:{:?} event: {:?}",
    "[ecc_sign] Invalid State, state:{:?} event: {:?}",
    "[ecc_sign] Invalid Event, state:{:?} event: {:?}",
    "Unexpected: Resource cleanup for an already cleaned up resource {}.",
    "Unexpected: Resource cleanup completed for resource {}.",
    "PKA instance {} cleanup error",
    "[der_key_import] Invalid State, state:{:?}, event: {:?}",
    "[der_key_import] Invalid Event, state:{:?}, event: {:?}",
    "[rsa_unwrap] Invalid State, state:{:?} event: {:?}",
    "[rsa_unwrap] Invalid Event, state:{:?} event: {:?}",
    "Timeout!! State: {:?} Res Op State: {:?}",
    "[rsa2k_mod] Failed to send self test response",
    "[rsa2k_crt] Failed to send self test response",
    "[ecdsa_engine] Failed to send self test response",
    "[ecdh_engine] Failed to send self test response",
    "[rng] Failed to send self test response",
    "Failed to send self test response",
    "[ecc] ECC structural validation Public key length mismatch",
    "[ecc] ECC structural validation Public key mismatch",
    "[ecc] ECC structural validation Private key length mismatch",
    "ECC structural validation Scalar d is not in the range 0 < d < n",
    "send_crashdump_request: Failed to send IPC message to Admin: {:?}",
    "[mod] Failed to send IPC message to FP: {:?}",
    "Invalid core id",
    "begin_neg_self_test_req: Failed to send IPC message to Admin: {:?}",
    "[mod] Failed to decode IPC message header: {:?}",
    "Negative self test failed with status: {}",
    "[aes] begin_import_der_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}",
    "[aes] end_import_der_aesbulk256_key_inner: Failed to decode IPC message header: {:?}",
    "[aes] Import der key: Invalid IPC response with status {}",
    "[aes] end_import_der_aesbulk256_key_inner: Spurious Message",
    "[aes] begin_delete_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}",
    "[aes] end_delete_aesbulk256_key_inner: Failed to decode IPC message header: {:?}",
    "[aes] Delete Key Operation: Invalid IPC response with status {}",
    "[aes] end_delete_aesbulk256_key_inner: Spurious Message",
    "[aes] begin_rollback_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}",
    "[aes] end_rollback_aesbulk256_key_inner: Failed to decode IPC message header: {:?}",
    "[aes] Delete key operation rollback: Invalid IPC response with status {}",
    "[aes] end_rollback_aesbulk256_key_inner: Spurious Message",
    "[part] delete_session_keys: failed to delete key key_id={:?}, Error: {:?}",
    "[part] Failed to send IPC message to FP: {}",
    "[part] Failed to decode IPC message header: {:?}",
    "[part] Invalid IPC response with status {}",
    "[part] Spurious Message",
    "Establish cred key deletion failed {:?}. Ignoring error.",
    "Failed the self test {:?}",
    "PCT Validation Failure: {:?}. System Reset Required! ",
    "Preop-Self test failed for {:?} with error {}",
    "on_request_ready: spurious event",
    "on_response_complete: spurious event",
    "on_dma_complete: spurious event",
    "Failed to send FLR complete event",
    "on_flr: spurious event",
    "Invalid Header found in IPC message",
    "Invalid PcieFunction enable/disable message",
    "Invalid Set Resource message {:?}",
    "Failed to allocate partition init FSM",
    "Invalid IPC Create Delete Submission Queue Message {:?}",
    "{:?} not Enabled",
    "Invalid Shutdown for reset request message",
    "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err",
    "Failed to send prepare shutdown response to Admin core {:?}",
    "Schedule is busy in draining IOs",
    "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err",
    "iDFU Fault Injected: idfu_fault_pre_reset_hsm_drain_timeout",
    "Failed to get CDMA vault key entry",
    "Invalid IPC message header {:?}",
    "Unhandled message with opcode {:#X}",
    "Failed to send response to Admin core {:?}",
    "on_pka: spurious event. Index: {}",
    "Invalid Header found for fp to hsm IPC message",
    "Correctable ECC errors have exceeded threshold, Key vault memory reloaded",
    "Failed to send response to FP core {:?}",
    "Invalid op code found in IPC message",
    "on_fp_ipc_response: spurious event",
    "on_hsp_ipc_response: spurious event",
    "on_admin_ipc_response: spurious event",
    "on_aes_unwrap_resp: spurious event",
    "Failed to receive self test request",
    "RNG Hardware error received. Fault code: {}",
    "DTCM error received. Fault code: {}",
    "ITCM error received. Fault code: {}",
];
