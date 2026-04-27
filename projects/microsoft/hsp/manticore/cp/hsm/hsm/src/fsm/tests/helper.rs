// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::{
    key_usage::{KeyUsage, KeyUsageTrait},
    key_vault::entry::EntryFlags,
};
use mcr_crypto_aes::{AesCompletionDesc, AesCompletionStatus};
use mcr_ddi_types::*;
use mcr_gdma_controller::DmaTxnCompletionDesc;
use mcr_io_controller::{IoRxDesc, IoTxCompleteDesc, IoTxCompleteStatus};
use mcr_types::{DevCqId, DevSqId, MemoryAddr, PcieFunction};
use mockall::predicate::eq;
use zerocopy::{FromBytes, IntoBytes};

use crate::{
    error::{HostStatusCode, HsmErr},
    event::HsmFsmEvent,
    fsm::{
        get_api_rev::SUPPORTED_API_REV, HsmCqe, HsmSessionControlKind, HsmSessionFlags, HsmSqe,
        HsmSqeCmd, HsmSqeCmdOpcode, HsmSqeDmaDesc,
    },
    function::{Credential, IoQueue, Role, Session, SessionKind},
    key_vault::{entry::EntryKind, vault::KeyVault},
    mock::*,
};

use super::*;

#[derive(PartialEq)]
pub enum AesEncryptDecryptAllocType {
    Msg,
    Iv,
    Result,
    DmaRespBuf,
}

/// Create mock AES.
fn create_mock_resources(test: &mut HsmFsmTest) {
    test.aes().expect_clone().times(2).returning(MockAes::new);

    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);

    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);
}

/// Create a mock HSM FSM.
pub(crate) fn make_default_hsm_fsm() -> HsmFsmTest {
    let mut test = HsmFsmTest::default();
    create_mock_resources(&mut test);

    test
}

pub(crate) fn make_dma_desc(len: u32, prp1_lo: u32, prp2_lo: u32) -> HsmSqeDmaDesc {
    HsmSqeDmaDesc {
        len,
        prp1: MemoryAddr {
            lo: prp1_lo,
            ..Default::default()
        },
        prp2: MemoryAddr {
            lo: prp2_lo,
            ..Default::default()
        },
    }
}

pub(crate) fn make_sqe_cmd() -> HsmSqeCmd {
    HsmSqeCmd::default()
        .with_op(HsmSqeCmdOpcode::Generic)
        .with_id(0xB0)
}

pub(crate) fn make_sqe() -> HsmSqe {
    HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: 1,
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: 1,
            ..Default::default()
        },
        ..Default::default()
    }
}

pub(crate) fn make_flush_sqe_cmd() -> HsmSqeCmd {
    HsmSqeCmd::default()
        .with_op(HsmSqeCmdOpcode::Flush)
        .with_id(0xB0)
}

pub(crate) fn make_flush_sqe(sess_id_valid: bool, sess_ctrl_kind: HsmSessionControlKind) -> HsmSqe {
    HsmSqe {
        cmd: make_flush_sqe_cmd(),
        src: HsmSqeDmaDesc::default(),
        dst: HsmSqeDmaDesc::default(),
        session_flags: HsmSessionFlags::new()
            .with_id_valid(sess_id_valid)
            .with_ctrl(sess_ctrl_kind.into()),
        session_id: 0,
        ..Default::default()
    }
}

pub(crate) fn make_rx_desc(sqe: HsmSqe, status: bool) -> Option<IoRxDesc> {
    let mut entry = [0u8; 64];
    entry.copy_from_slice(sqe.as_bytes());
    Some(IoRxDesc {
        sq_id: DevSqId(0),
        addr: 0,
        entry,
        pfn: PcieFunction::Pf,
        status,
    })
}

pub(crate) fn make_pfn() -> MockFunction {
    let mut pfn = MockFunction::default();

    pfn.expect_io_queue()
        .with(eq(DevSqId::Id0))
        .times(1)
        .returning(|_| Some(IoQueue::new(DevSqId::Id0, DevCqId::Id0)));
    pfn.expect_enabled().times(1).return_const(true);

    pfn
}

pub(crate) fn make_fsm_with_pfn(sqe: HsmSqe) -> HsmFsmTest {
    let mut test = make_default_hsm_fsm();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .return_once(|_| make_pfn());
    test
}

pub(crate) fn make_fsm_with_cqe_err(sqe: HsmSqe, status: HostStatusCode) -> HsmFsmTest {
    let mut test = make_fsm_with_pfn(sqe);
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test
}

pub(crate) fn make_fsm_with_dma_err(
    desc: Option<DmaTxnCompletionDesc>,
    status: HostStatusCode,
) -> HsmFsmTest {
    let mut test = make_fsm_with_cqe_err(make_sqe(), status);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .return_once(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Ok(()));
    test.dma_channel()
        .expect_peek_tag()
        .times(1)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(1)
        .return_once(|| desc);
    test
}

pub(crate) fn make_fsm_with_dma(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = make_fsm_with_pfn(sqe);

    // 1. Allocate inbound DMA buffer
    // 2. Allocate outbound DMA buffer
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    // 1. Begin inbound DMA operation
    // 2. Begin outbound DMA operation
    test.dma_channel()
        .expect_begin_txn()
        .times(2)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });

    // 1. DmaComplete handler
    // 2. DmaComplete handler
    test.dma_channel()
        .expect_peek_tag()
        .times(2)
        .return_const(0);

    // 1. Complete inbound DMA operation
    // 2. Complete outbound DMA operation
    test.dma_channel().expect_end_txn().times(2).returning(|| {
        Some(DmaTxnCompletionDesc {
            success: true,
            tag: 0,
        })
    });

    // TxComplete handler
    test.io_channel().expect_peek_tag().times(1).return_const(0);

    // Send completion queue entry
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });

    // Receive submission queue entry
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));

    // End the receive operation
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());

    test
}

pub(crate) fn make_fsm_with_dma_with_sqe_session_id_incorrect(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32 + 1,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = make_fsm_with_pfn(sqe);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(2)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(2)
        .return_const(0);
    test.dma_channel().expect_end_txn().times(2).returning(|| {
        Some(DmaTxnCompletionDesc {
            success: true,
            tag: 0,
        })
    });
    test.io_channel().expect_peek_tag().times(1).return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test
}

pub(crate) fn make_fsm_with_dma_with_sqe_session_ctrl_flags_mismatch(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    session_flags: HsmSessionFlags,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32 + 1,
        session_flags,
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = make_fsm_with_pfn(sqe);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(2)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(2)
        .return_const(0);
    test.dma_channel().expect_end_txn().times(2).returning(|| {
        Some(DmaTxnCompletionDesc {
            success: true,
            tag: 0,
        })
    });
    test.io_channel().expect_peek_tag().times(1).return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test
}

pub(crate) fn make_fsm_with_dma_begin_send_err(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };

    let mut test = make_fsm_with_pfn(sqe);
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(2)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(2)
        .return_const(0);
    test.dma_channel().expect_end_txn().times(2).returning(|| {
        Some(DmaTxnCompletionDesc {
            success: true,
            tag: 0,
        })
    });
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .returning(|_| Err(u32::MAX));
    test
}

pub(crate) fn make_fsm_with_dma_begin_txn_err(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };

    let mut test = make_fsm_with_pfn(sqe);
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });

    test.dma_channel()
        .expect_peek_tag()
        .times(1)
        .return_const(0);
    test.dma_channel().expect_end_txn().times(1).returning(|| {
        Some(DmaTxnCompletionDesc {
            success: true,
            tag: 0,
        })
    });

    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .returning(|_| Err(u32::MAX));

    test.io_channel()
        .expect_begin_send()
        .times(1)
        .returning(|_| Err(u32::MAX));

    test
}

pub(crate) fn test_cmd<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    resp_page: &'a mut Page,
    session_expected: bool,
    status: HostStatusCode,
) -> T::OpResp {
    let req_page = ddi_encode_page(req);

    let mut test = make_fsm_with_dma(req_hdr, &req_page, resp_page, status);
    test.pfn_mgr()
        .expect_function()
        .times(0..=1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            if session_expected {
                pfn.expect_is_app_session().times(1).returning(|_| true);
            }
            pfn
        });
    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);

    ddi_decode_page::<T::OpResp>(resp_page)
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_aes_generate_fsm(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    func_enable: bool,
    peek_tag_cnt: usize,
    dma_alloc_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    aes_clone_cnt: usize,
    rng_gen_cnt: usize,
    buffer_u32_ptr: usize,
    kv_cnt: usize,
    session_type: SessionKind,
    dummy_entry: bool,
    entry_kind: Option<EntryKind>,
    entry_flags: Option<EntryFlags>,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(func_enable);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            let cred = Credential::new([0u8; 16usize], [0u8; 16usize], Role::App, 0u8);

            pfn.expect_session()
                .times(1)
                .returning(move |_| Session::new(0, cred, Some(session_type)));
            pfn.expect_key_vault().times(kv_cnt).returning(move || {
                let mut keyvault = KeyVault::new(buffer_u32_ptr, 0b1010110);
                if dummy_entry {
                    let mut flags = EntryFlags::default();
                    if let Some(entry_flags_val) = entry_flags {
                        flags = entry_flags_val;
                    } else {
                        flags.set_sign(true);
                        flags.set_verify(true);
                    }
                    let size = entry_kind.unwrap().raw_size();
                    let slice: &mut [u8] = &mut vec![0; size];
                    let _ = keyvault.add_entry(flags, 123, entry_kind.unwrap(), 0, slice);
                }

                keyvault
            });
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(dma_alloc_cnt)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes()
        .expect_clone()
        .times(aes_clone_cnt)
        .returning(move || {
            let mut aes = MockAes::new();
            aes.expect_peek_tag()
                .times(peek_tag_cnt)
                .returning(|| Some(0));
            aes
        });
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn test_cmd_open_session<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    resp_page: &'a mut Page,
    decode_err: bool,
    vault_err: bool,
    open_session_err_code: Option<HsmErr>,
    encode_err: bool,
    close_session_err: bool,
) {
    let mut req_page = ddi_encode_page(req);
    if decode_err {
        // Set incorrect request page length to trigger decode error.
        req_page.set_len(req_page.len() - 1);
    }

    let mut test = match encode_err {
        true => make_fsm_with_dma_heap_err(req_hdr, &req_page, resp_page, HostStatusCode::Success),
        false => make_fsm_with_dma(req_hdr, &req_page, resp_page, HostStatusCode::Success),
    };

    // Request header validation
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn
        });

    // FSM on_start logic
    if !decode_err && !vault_err {
        test.pfn_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::default();

                if !vault_err {
                    function
                        .expect_open_session()
                        .times(1)
                        .returning(move |_, _| {
                            if let Some(err) = open_session_err_code {
                                Err(err)
                            } else {
                                Ok(0u16)
                            }
                        });
                }

                function
            });
    }

    // Close session in error scenarios for which one was opened
    if encode_err {
        test.pfn_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::default();
                function
                    .expect_close_session()
                    .times(1)
                    .returning(move |_| {
                        if close_session_err {
                            Err(HsmErr::CmdError)
                        } else {
                            Ok(())
                        }
                    });
                function
            });
    }

    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);
}

pub(crate) fn test_cmd_close_manager_session<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    resp_page: &'a mut Page,
    decode_err: bool,
    session_err: bool,
    close_session_err_code: Option<HsmErr>,
    encode_err: bool,
) {
    let mut req_page = ddi_encode_page(req);
    if decode_err {
        // Set incorrect request page length to trigger decode error.
        req_page.set_len(req_page.len() - 1);
    }

    let mut test = match encode_err {
        true => make_fsm_with_dma_heap_err(req_hdr, &req_page, resp_page, HostStatusCode::Success),
        false => make_fsm_with_dma(req_hdr, &req_page, resp_page, HostStatusCode::Success),
    };

    // Request header validation
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            if !session_err {
                pfn.expect_is_manager_session()
                    .times(1)
                    .returning(move |_| true);
            }
            pfn
        });

    // FSM on_start logic
    if !session_err && !decode_err {
        test.pfn_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::default();
                function
                    .expect_close_session()
                    .times(1)
                    .returning(move |_| {
                        if let Some(err) = close_session_err_code {
                            Err(err)
                        } else {
                            Ok(())
                        }
                    });
                function
            });
    }

    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);
}

pub(crate) fn test_cmd_close_app_session<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    resp_page: &'a mut Page,
    decode_err: bool,
    session_err: bool,
    close_session_err_code: Option<HsmErr>,
    encode_err: bool,
) {
    let mut req_page = ddi_encode_page(req);
    if decode_err {
        // Set incorrect request page length to trigger decode error.
        req_page.set_len(req_page.len() - 1);
    }

    let mut test = match encode_err {
        true => make_fsm_with_dma_heap_err(req_hdr, &req_page, resp_page, HostStatusCode::Success),
        false => make_fsm_with_dma(req_hdr, &req_page, resp_page, HostStatusCode::Success),
    };

    // Request header validation
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            if !session_err {
                pfn.expect_is_app_session()
                    .times(1)
                    .returning(move |_| true);
            }
            pfn
        });

    // FSM on_start logic
    if !session_err && !decode_err {
        test.pfn_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::default();
                function
                    .expect_close_session()
                    .times(1)
                    .returning(move |_| {
                        if let Some(err) = close_session_err_code {
                            Err(err)
                        } else {
                            Ok(())
                        }
                    });
                function
            });
    }

    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);
}

pub(crate) fn test_cmd_create_app<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    resp_page: &'a mut Page,
    create_app_err_code: Option<HsmErr>,
    encode_err: bool,
) {
    let req_page = ddi_encode_page(req);

    let mut test = match encode_err {
        true => make_fsm_with_dma_heap_err(req_hdr, &req_page, resp_page, HostStatusCode::Success),
        false => make_fsm_with_dma(req_hdr, &req_page, resp_page, HostStatusCode::Success),
    };

    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_manager_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut function = MockFunction::default();

            function
                .expect_create_app_credential()
                .times(1)
                .returning(move |_| {
                    if let Some(err) = create_app_err_code {
                        Err(err)
                    } else {
                        Ok(())
                    }
                });

            function
        });

    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);
}

pub(crate) fn test_cmd_delete_key<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    resp_page: &'a mut Page,
    decode_err: bool,
    session_err: bool,
    delete_key_err_code: Option<HsmErr>,
    encode_err: bool,
) {
    let mut req_page = ddi_encode_page(req);
    if decode_err {
        // Set incorrect request page length to trigger decode error.
        req_page.set_len(req_page.len() - 1);
    }

    let mut test = match encode_err {
        true => make_fsm_with_dma_heap_err(req_hdr, &req_page, resp_page, HostStatusCode::Success),
        false => make_fsm_with_dma(req_hdr, &req_page, resp_page, HostStatusCode::Success),
    };

    // Request header validation
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            if !session_err {
                pfn.expect_is_app_session()
                    .times(1)
                    .returning(move |_| true);
            }
            pfn
        });

    // FSM on_start logic
    if !session_err && !decode_err {
        test.pfn_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::default();

                function
                    .expect_delete_key()
                    .times(1)
                    .returning(move |_, _| {
                        if let Some(err) = delete_key_err_code {
                            Err(err)
                        } else {
                            Ok(())
                        }
                    });

                function
            });
    }

    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);
}

pub(crate) fn test_cmd_delete_app<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    resp_page: &'a mut Page,
    decode_err: bool,
    session_err: bool,
    delete_app_err_code: Option<HsmErr>,
    encode_err: bool,
) {
    let mut req_page = ddi_encode_page(req);
    if decode_err {
        // Set incorrect request page length to trigger decode error.
        req_page.set_len(req_page.len() - 1);
    }

    let mut test = match encode_err {
        true => make_fsm_with_dma_heap_err(req_hdr, &req_page, resp_page, HostStatusCode::Success),
        false => make_fsm_with_dma(req_hdr, &req_page, resp_page, HostStatusCode::Success),
    };

    // Request header validation
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_manager_session()
                .times(1)
                .returning(move |_| !session_err);
            pfn
        });

    // FSM on_start logic
    if !session_err && !decode_err {
        test.pfn_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::default();

                function.expect_delete_app().times(1).returning(move |_| {
                    if let Some(err) = delete_app_err_code {
                        Err(err)
                    } else {
                        Ok(())
                    }
                });

                function
            });
    }

    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn test_cmd_change_manager_credential<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    resp_page: &'a mut Page,
    decode_err: bool,
    session_err: bool,
    session_id_err: bool,
    change_manager_session_err_code: Option<HsmErr>,
    encode_err: bool,
) {
    let mut req_page = ddi_encode_page(req);
    if decode_err {
        // Set incorrect request page length to trigger decode error.
        req_page.set_len(req_page.len() - 1);
    }

    let mut test = match encode_err {
        true => make_fsm_with_dma_heap_err(req_hdr, &req_page, resp_page, HostStatusCode::Success),
        false => make_fsm_with_dma(req_hdr, &req_page, resp_page, HostStatusCode::Success),
    };

    // Request header validation
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            if !session_err {
                pfn.expect_is_manager_session()
                    .times(1)
                    .returning(move |_| !session_id_err);
            }
            pfn
        });

    // FSM on_start logic
    if !session_err && !session_id_err && !decode_err {
        test.pfn_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::default();
                function
                    .expect_change_mgr_credential()
                    .times(1)
                    .returning(move |_| {
                        if let Some(err) = change_manager_session_err_code {
                            Err(err)
                        } else {
                            Ok(())
                        }
                    });
                function
            });
    }

    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);
}

pub(crate) fn make_fsm_with_dma_heap_err(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = make_fsm_with_pfn(sqe);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    test.dma_channel()
        .expect_begin_txn()
        .times(2)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(2)
        .return_const(0);
    test.dma_channel().expect_end_txn().times(2).returning(|| {
        Some(DmaTxnCompletionDesc {
            success: true,
            tag: 0,
        })
    });
    test.io_channel().expect_peek_tag().times(1).return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_aes_generate_fsm_with_dma_heap_err(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    peek_tag_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    aes_clone_cnt: usize,
    rng_gen_cnt: usize,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes()
        .expect_clone()
        .times(aes_clone_cnt)
        .returning(move || {
            let mut aes = MockAes::new();
            aes.expect_peek_tag()
                .times(peek_tag_cnt)
                .returning(|| Some(0));
            aes
        });
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

pub(crate) fn test_cmd_on_err<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    resp_page: &'a mut Page,
    session_expected: bool,
    host_status_code: HostStatusCode,
) {
    let req_page = ddi_encode_page(req);

    let mut test = make_fsm_with_dma(req_hdr, &req_page, resp_page, host_status_code);
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            if session_expected {
                pfn.expect_is_app_session().times(1).returning(|_| true);
            }
            pfn
        });
    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_aes_fsm_with_dma_heap_encode_err(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    peek_tag_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    aes_clone_cnt: usize,
    rng_gen_cnt: usize,
) -> HsmFsmTest {
    let buffer = [0u32; (17 * 1024 * 65) / 4];
    let buffer_u32_ptr = buffer.as_ptr() as usize;
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            let cred = Credential::new([0u8; 16usize], [0u8; 16usize], Role::App, 0u8);
            pfn.expect_session()
                .times(1)
                .returning(move |_| Session::new(0, cred, Some(SessionKind::Persistent)));
            pfn.expect_key_vault()
                .times(1)
                .returning(move || KeyVault::new(buffer_u32_ptr, 0b1010110));
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .returning(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes()
        .expect_clone()
        .times(aes_clone_cnt)
        .returning(move || {
            let mut aes = MockAes::new();
            aes.expect_peek_tag()
                .times(peek_tag_cnt)
                .returning(|| Some(0));
            aes
        });
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_aes_fsm_with_keybuf_alloc_fail(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    peek_tag_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    aes_clone_cnt: usize,
    rng_gen_cnt: usize,
) -> HsmFsmTest {
    let buffer = [0u32; (17 * 1024 * 65) / 4];
    let buffer_u32_ptr = buffer.as_ptr() as usize;
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            let cred = Credential::new([0u8; 16usize], [0u8; 16usize], Role::App, 0u8);
            pfn.expect_session()
                .times(1)
                .returning(move |_| Session::new(0, cred, Some(SessionKind::Persistent)));
            pfn.expect_key_vault()
                .times(1)
                .returning(move || KeyVault::new(buffer_u32_ptr, 0b1010110));
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .returning(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes()
        .expect_clone()
        .times(aes_clone_cnt)
        .returning(move || {
            let mut aes = MockAes::new();
            aes.expect_peek_tag()
                .times(peek_tag_cnt)
                .returning(|| Some(0));
            aes
        });
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_aes_fsm_session_not_found(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    peek_tag_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    aes_clone_cnt: usize,
    rng_gen_cnt: usize,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_session()
                .times(1)
                .returning(move |_| Err(HsmErr::SessionNotFound)?);
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .returning(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes()
        .expect_clone()
        .times(aes_clone_cnt)
        .returning(move || {
            let mut aes = MockAes::new();
            aes.expect_peek_tag()
                .times(peek_tag_cnt)
                .returning(|| Some(0));
            aes
        });
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_aes_fsm_app_kv_id_none(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    peek_tag_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    aes_clone_cnt: usize,
    rng_gen_cnt: usize,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            let cred = Credential::new([0u8; 16usize], [0u8; 16usize], Role::Manager, 0u8);
            pfn.expect_session()
                .times(1)
                .returning(move |_| Session::new(0, cred, None));
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .returning(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes()
        .expect_clone()
        .times(aes_clone_cnt)
        .returning(move || {
            let mut aes = MockAes::new();
            aes.expect_peek_tag()
                .times(peek_tag_cnt)
                .returning(|| Some(0));
            aes
        });
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_aes_fsm_with_on_err_encode_err(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    peek_tag_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    aes_clone_cnt: usize,
    rng_gen_cnt: usize,
) -> HsmFsmTest {
    let buffer = [0u32; (17 * 1024 * 65) / 4];
    let buffer_u32_ptr = buffer.as_ptr() as usize;
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            let cred = Credential::new([0u8; 16usize], [0u8; 16usize], Role::App, 0u8);
            pfn.expect_session()
                .times(1)
                .returning(move |_| Session::new(0, cred, Some(SessionKind::Persistent)));
            pfn.expect_key_vault()
                .times(1)
                .returning(move || KeyVault::new(buffer_u32_ptr, 0b1010110));
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);

    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .returning(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes()
        .expect_clone()
        .times(aes_clone_cnt)
        .returning(move || {
            let mut aes = MockAes::new();
            aes.expect_peek_tag()
                .times(peek_tag_cnt)
                .returning(|| Some(0));
            aes
        });
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_fsm_aes_encrypt_decrypt(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    func_enable: bool,
    peek_tag_cnt: usize,
    dma_alloc_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    rng_gen_cnt: usize,
    buffer_u32_ptr: usize,
    kv_cnt: usize,
    complete_status: Option<AesCompletionDesc>,
    err: Result<(), u32>,
    aes_complete_cnt: usize,
    key_usage_begin_status_ok: bool,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(func_enable);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_key_vault()
                .times(kv_cnt)
                .returning(move || KeyVault::new(buffer_u32_ptr, 0b1010110));
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_key_vault()
                .times(kv_cnt)
                .returning(move || KeyVault::new(buffer_u32_ptr, 0b1010110));
            pfn.expect_begin_key_usage()
                .times(1)
                .returning(move |r, i, k| {
                    if key_usage_begin_status_ok {
                        let key_usage = KeyUsage::new();
                        key_usage.begin(r, i, k)
                    } else {
                        Err(HsmErr::AnotherKeyInUse)
                    }
                });
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(dma_alloc_cnt)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes().expect_clone().times(1).returning(move || {
        let mut aes = MockAes::new();
        aes.expect_peek_tag()
            .times(peek_tag_cnt)
            .returning(|| Some(0));

        aes
    });
    test.aes().expect_clone().times(1).returning(move || {
        let mut aes = MockAes::new();
        aes.expect_encrypt_decrypt().once().returning(move |_| err);
        aes.expect_complete()
            .times(aes_complete_cnt)
            .returning(move || complete_status);
        aes
    });
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_fsm_aes_encrypt_decrypt_key_usage_begin(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    func_enable: bool,
    peek_tag_cnt: usize,
    dma_alloc_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    rng_gen_cnt: usize,
    buffer_u32_ptr: usize,
    kv_cnt: usize,
    key_usage_begin_status_ok: bool,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(func_enable);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_key_vault()
                .times(kv_cnt)
                .returning(move || KeyVault::new(buffer_u32_ptr, 0b1010110));
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_key_vault()
                .times(kv_cnt)
                .returning(move || KeyVault::new(buffer_u32_ptr, 0b1010110));
            pfn.expect_begin_key_usage()
                .times(1)
                .returning(move |r, i, k| {
                    if key_usage_begin_status_ok {
                        let key_usage = KeyUsage::new();
                        key_usage.begin(r, i, k)
                    } else {
                        Err(HsmErr::AnotherKeyInUse)
                    }
                });
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(dma_alloc_cnt)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes().expect_clone().times(1).returning(move || {
        let mut aes = MockAes::new();
        aes.expect_peek_tag()
            .times(peek_tag_cnt)
            .returning(|| Some(0));

        aes
    });
    test.aes().expect_clone().times(1).returning(MockAes::new);
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_fsm_aes_encrypt_decrypt_msg_len_not_aligned(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    peek_tag_cnt: usize,
    dma_alloc_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    rng_gen_cnt: usize,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(dma_alloc_cnt)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes().expect_clone().times(1).returning(move || {
        let mut aes = MockAes::new();
        aes.expect_peek_tag()
            .times(peek_tag_cnt)
            .returning(|| Some(0));

        aes
    });
    test.aes().expect_clone().times(1).returning(MockAes::new);
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_fsm_aes_encrypt_decrypt_buf_alloc_fail(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    peek_tag_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    rng_gen_cnt: usize,
    buffer_u32_ptr: usize,
    alloc_type: AesEncryptDecryptAllocType,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_key_vault()
                .times(1)
                .returning(move || KeyVault::new(buffer_u32_ptr, 0b1010110));
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    match alloc_type {
        AesEncryptDecryptAllocType::Msg => {
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|_| None);
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|s| Some(MockDmaAlloc::new(s)));
        }
        AesEncryptDecryptAllocType::Iv => {
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|s| Some(MockDmaAlloc::new(s)));
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|_| None);
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|s| Some(MockDmaAlloc::new(s)));
        }
        AesEncryptDecryptAllocType::Result => {
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|s| Some(MockDmaAlloc::new(s)));
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|s| Some(MockDmaAlloc::new(s)));
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|_| None);
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|s| Some(MockDmaAlloc::new(s)));
        }
        AesEncryptDecryptAllocType::DmaRespBuf => {
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|s| Some(MockDmaAlloc::new(s)));
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|s| Some(MockDmaAlloc::new(s)));
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|s| Some(MockDmaAlloc::new(s)));
            test.pfn_mgr()
                .expect_function()
                .times(1)
                .returning(move |_| {
                    let mut pfn = MockFunction::default();
                    pfn.expect_key_vault()
                        .times(1)
                        .returning(move || KeyVault::new(buffer_u32_ptr, 0b1010110));
                    pfn.expect_begin_key_usage()
                        .times(1)
                        .returning(move |r, i, k| {
                            let key_usage = KeyUsage::new();
                            key_usage.begin(r, i, k)
                        });
                    pfn
                });
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|_| None);
            test.dma_heap()
                .expect_allocate_from_pool()
                .times(1)
                .returning(|s| Some(MockDmaAlloc::new(s)));
        }
    }

    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes().expect_clone().times(1).returning(move || {
        let mut aes = MockAes::new();
        aes.expect_peek_tag()
            .times(peek_tag_cnt)
            .returning(|| Some(0));

        aes
    });
    test.aes().expect_clone().times(1).returning(move || {
        let mut aes = MockAes::new();
        if alloc_type == AesEncryptDecryptAllocType::DmaRespBuf {
            aes.expect_encrypt_decrypt()
                .once()
                .returning(move |_| Ok(()));
            aes.expect_complete().times(1).returning(move || {
                Some(AesCompletionDesc {
                    status: AesCompletionStatus::Complete,
                    tag: 123,
                })
            });
        }
        aes
    });
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn make_fsm_aes_encrypt_decrypt_misc_failures(
    req_hdr: &DdiReqHdr,
    req: &Page,
    resp: &mut Page,
    status: HostStatusCode,
    func_enable: bool,
    peek_tag_cnt: usize,
    dma_alloc_cnt: usize,
    begin_send_cnt: usize,
    io_channel_peek_tag_cnt: usize,
    dma_channel_begin_txn_cnt: usize,
    dma_channel_peek_tag_cnt: usize,
    dma_channel_end_txn_cnt: usize,
    rng_gen_cnt: usize,
    buffer_u32_ptr: usize,
    kv_cnt: usize,
) -> HsmFsmTest {
    let sqe = HsmSqe {
        cmd: make_sqe_cmd(),
        src: HsmSqeDmaDesc {
            len: req.len() as u32,
            prp1: req.addr(),
            ..Default::default()
        },
        dst: HsmSqeDmaDesc {
            len: resp.len() as u32,
            prp1: resp.addr(),
            ..Default::default()
        },
        session_id: req_hdr.sess_id.unwrap_or(0) as u32,
        session_flags: HsmSessionFlags::default()
            .with_ctrl(HsmSessionControlKind::from(req_hdr.op).into())
            .with_id_valid(req_hdr.sess_id.is_some()),
        ..Default::default()
    };
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = HsmFsmTest::default();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(|_| make_pfn());
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(func_enable);
            pfn.expect_is_app_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_key_vault()
                .times(kv_cnt)
                .returning(move || KeyVault::new(buffer_u32_ptr, 0b1010110));
            pfn
        });
    test.dma_heap()
        .expect_allocate_from_pool()
        .times(dma_alloc_cnt)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    test.dma_channel()
        .expect_begin_txn()
        .times(dma_channel_begin_txn_cnt)
        .returning(|d| {
            d.dst_fst
                .addr
                .slice_mut(d.len as usize)
                .copy_from_slice(d.src_fst.addr.slice(d.len as usize));
            Ok(())
        });
    test.dma_channel()
        .expect_peek_tag()
        .times(dma_channel_peek_tag_cnt)
        .return_const(0);
    test.dma_channel()
        .expect_end_txn()
        .times(dma_channel_end_txn_cnt)
        .returning(|| {
            Some(DmaTxnCompletionDesc {
                success: true,
                tag: 0,
            })
        });
    test.io_channel()
        .expect_peek_tag()
        .times(io_channel_peek_tag_cnt)
        .return_const(0);
    test.io_channel()
        .expect_begin_send()
        .times(begin_send_cnt)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            Ok(())
        });
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_const(());
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(io_tx_complete_desc));
    test.aes().expect_clone().times(1).returning(move || {
        let mut aes = MockAes::new();
        aes.expect_peek_tag()
            .times(peek_tag_cnt)
            .returning(|| Some(0));

        aes
    });
    test.aes().expect_clone().times(1).returning(MockAes::new);
    test.pka()
        .get_mut(0)
        .unwrap()
        .expect_clone()
        .times(2)
        .returning(MockPka::new);
    test.fp_ipc_channel()
        .expect_clone()
        .times(2)
        .returning(MockIpcMessageChannel::new);

    test.rng()
        .expect_bytes()
        .times(rng_gen_cnt)
        .returning(|_| ());

    test
}

pub(crate) fn test_cmd_reset_function<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    resp_page: &'a mut Page,
    encode_err: bool,
    clear_context_err: bool,
) {
    let req_page = ddi_encode_page(&req);

    let mut test = match encode_err {
        true => make_fsm_with_dma_heap_err(req_hdr, &req_page, resp_page, HostStatusCode::Success),
        false => make_fsm_with_dma(req_hdr, &req_page, resp_page, HostStatusCode::Success),
    };

    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut pfn = MockFunction::default();
            pfn.expect_enabled().times(1).return_const(true);
            pfn.expect_is_manager_session().times(1).returning(|_| true);
            pfn
        });
    test.pfn_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut function = MockFunction::default();
            function.expect_clear_context().times(1).returning(move || {
                if clear_context_err {
                    Err(HsmErr::InvalidKeyIndex)
                } else {
                    Ok(())
                }
            });
            function
        });

    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);
}

pub(crate) fn make_flush_cmd(
    status: HostStatusCode,
    sess_ctrl_kind: HsmSessionControlKind,
    is_session_valid: bool,
    io_send_error: bool,
    sqe_sess_id_valid: bool,
) -> HsmFsmTest {
    let local_session_control_kind = match &sess_ctrl_kind {
        HsmSessionControlKind::NoSession => HsmSessionControlKind::NoSession,
        HsmSessionControlKind::Open => HsmSessionControlKind::Open,
        HsmSessionControlKind::Close => HsmSessionControlKind::Close,
        HsmSessionControlKind::InSession => HsmSessionControlKind::InSession,
    };
    let sqe = make_flush_sqe(sqe_sess_id_valid, sess_ctrl_kind);
    let io_tx_complete_desc = IoTxCompleteDesc {
        queue_id: DevCqId::Id65.into(),
        queue_index: 0,
        tag: 0,
        status: IoTxCompleteStatus::IoTxCompleteStateSuccess,
    };

    let mut test = make_default_hsm_fsm();
    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(|| make_rx_desc(sqe, true));
    test.pfn_mgr()
        .expect_function()
        .once()
        .return_once(|_| make_pfn());
    if sqe_sess_id_valid && local_session_control_kind == HsmSessionControlKind::Close {
        test.pfn_mgr()
            .expect_function()
            .once()
            .return_once(move |_| {
                let mut function = MockFunction::new();

                let cred = Credential::new([0u8; 16usize], [0u8; 16usize], Role::App, 0u8);

                function.expect_session().times(1).returning(move |_| {
                    if is_session_valid {
                        Ok(Session::new(0, cred, Some(SessionKind::Persistent))?)
                    } else {
                        Err(HsmErr::SessionNotFound)
                    }
                });

                if is_session_valid {
                    function
                        .expect_close_session()
                        .times(1)
                        .returning(|_| Ok(()));
                }

                function
            });
    }
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(move |desc| {
            let cqe = HsmCqe::read_from_bytes(&desc.entry[..]).unwrap();
            assert_eq!(cqe.psf.status(), status);
            if io_send_error {
                Err(HsmErr::IoChannelSendError)?
            } else {
                Ok(())
            }
        });
    if !io_send_error {
        test.io_channel().expect_peek_tag().times(1).return_const(0);
        test.io_channel()
            .expect_end_send()
            .times(1)
            .return_once(|| Some(io_tx_complete_desc));
        test.io_channel()
            .expect_end_recv()
            .times(1)
            .return_const(());
    }
    test
}

pub(crate) fn create_aes_key(
    key_vault_buffer_u32_ptr: usize,
    aes_key_size: DdiAesKeySize,
    key_usage: DdiKeyUsage,
    dummy_key_required: bool,
    entry_kind: Option<EntryKind>,
    entry_flags: Option<EntryFlags>,
) {
    const FUNC_BOOL: bool = true;
    const PEEK_TAG_CNT: usize = 0;
    const ALLOC_CNT: usize = 3;
    const BEGIN_SEND_CNT: usize = 1;
    const IO_CHANNEL_PEEK_TAG_CNT: usize = 1;
    const DMA_CHANNEL_BEGIN_TXN_CNT: usize = 2;
    const DMA_CHANNEL_PEEK_TAG_CNT: usize = 2;
    const DMA_CHANNEL_END_TXN_CNT: usize = 2;
    const AES_CLONE_CNT: usize = 2;
    const RNG_GEN_CNT: usize = 1;
    const KV_CNT: usize = 1;

    let req = DdiAesGenerateKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(SUPPORTED_API_REV),
            sess_id: Some(1024),
            op: DdiOp::AesGenerateKey,
        },
        data: DdiAesGenerateKeyReq {
            key_size: aes_key_size,
            key_tag: None,
            key_properties: DdiKeyProperties { key_usage },
        },
    };

    let mut page = Page::new().unwrap();
    let req_page = ddi_encode_page(&req);
    let mut test = make_aes_generate_fsm(
        &req.hdr,
        &req_page,
        &mut page,
        HostStatusCode::Success,
        FUNC_BOOL,
        PEEK_TAG_CNT,
        ALLOC_CNT,
        BEGIN_SEND_CNT,
        IO_CHANNEL_PEEK_TAG_CNT,
        DMA_CHANNEL_BEGIN_TXN_CNT,
        DMA_CHANNEL_PEEK_TAG_CNT,
        DMA_CHANNEL_END_TXN_CNT,
        AES_CLONE_CNT,
        RNG_GEN_CNT,
        key_vault_buffer_u32_ptr,
        KV_CNT,
        SessionKind::Persistent,
        dummy_key_required,
        entry_kind,
        entry_flags,
    );

    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);

    let resp = ddi_decode_page::<
        <mcr_ddi_types::DdiAesGenerateKeyCmdReq as mcr_ddi_types::DdiOpReq>::OpResp,
    >(&page);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.hdr.sess_id, Some(1024));
    if dummy_key_required {
        assert_eq!(resp.data.key_id, 257);
    } else {
        assert_eq!(resp.data.key_id, 256);
    }
}

pub(crate) fn test_aes_done_fsm_small_message<'a, T: DdiOpReq<'a>>(
    req_hdr: &DdiReqHdr,
    req: &T,
    key_size: DdiAesKeySize,
) -> Page {
    const FUNC_BOOL: bool = true;
    const PEEK_TAG_CNT: usize = 1;
    const ALLOC_CNT: usize = 5;
    const BEGIN_SEND_CNT: usize = 1;
    const IO_CHANNEL_PEEK_TAG_CNT: usize = 1;
    const DMA_CHANNEL_BEGIN_TXN_CNT: usize = 2;
    const DMA_CHANNEL_PEEK_TAG_CNT: usize = 2;
    const DMA_CHANNEL_END_TXN_CNT: usize = 2;
    const RNG_GEN_CNT: usize = 0;
    const KV_CNT: usize = 1;
    const AES_COMPLETE_CNT: usize = 1;
    let key_usage_begin_status_ok: bool = true;

    let key_vault_buffer = [0u32; 1024 * 17 * 65 / 4];
    let key_vault_buffer_u32_ptr = key_vault_buffer.as_ptr() as *mut u32 as usize;

    create_aes_key(
        key_vault_buffer_u32_ptr,
        key_size,
        DdiKeyUsage::EncryptDecrypt,
        false,
        None,
        None,
    );

    let mut page = Page::new().unwrap();
    let req_page = ddi_encode_page(req);
    let mut test = make_fsm_aes_encrypt_decrypt(
        req_hdr,
        &req_page,
        &mut page,
        HostStatusCode::Success,
        FUNC_BOOL,
        PEEK_TAG_CNT,
        ALLOC_CNT,
        BEGIN_SEND_CNT,
        IO_CHANNEL_PEEK_TAG_CNT,
        DMA_CHANNEL_BEGIN_TXN_CNT,
        DMA_CHANNEL_PEEK_TAG_CNT,
        DMA_CHANNEL_END_TXN_CNT,
        RNG_GEN_CNT,
        key_vault_buffer_u32_ptr,
        KV_CNT,
        Some(AesCompletionDesc {
            status: mcr_crypto_aes::AesCompletionStatus::Complete,
            tag: 123,
        }),
        Ok(()),
        AES_COMPLETE_CNT,
        key_usage_begin_status_ok,
    );

    test.run(vec![
        (HsmFsmEvent::RxReady, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::AesDone, Err(HsmErr::Pending)),
        (HsmFsmEvent::DmaComplete, Err(HsmErr::Pending)),
        (HsmFsmEvent::TxComplete, Ok(())),
    ]);

    page
}
