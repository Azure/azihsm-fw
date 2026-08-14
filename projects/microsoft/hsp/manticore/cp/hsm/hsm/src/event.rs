// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::resource::HsmFsmResourceId;
use mcr_ipc_message::IpcMessageSetRes;

/// Enumeration to define the HSM events.
#[derive(Clone, Copy, PartialEq, Eq)]
pub(crate) enum HsmFsmEvent {
    /// Request ready event
    RxReady,

    /// Response complete event
    TxComplete,

    /// DMA complete event
    DmaComplete,

    /// Start DDI Cmd
    StartCmd,

    /// FLR event
    Flr,

    /// Admin to HSM IPC request
    AdminToHsmIpcRequest,

    /// FP to HSM IPC response
    FpToHsmIpcResponse,

    /// HSP to HSM IPC response
    HspToHsmIpcResponse,

    #[allow(dead_code)]
    /// Admin to HSM IPC response
    AdminToHsmIpcResponse,

    /// PKA done for one of the UPKA engines.
    PkaDone(usize),

    /// PKA error for the one of the UPKA engines.
    PkaError(usize),

    /// Resource ready
    ResourceReady(HsmFsmResourceId),

    /// TCON timer elapsed
    TimerElapsed,

    /// Check if FSM is still alive
    CheckAlive,

    /// SoftAes Response
    SoftAesResp,

    /// Self Test Request
    SelfTestRequest,

    /// Resource Cleanup
    ResourceCleanup(HsmFsmResourceId, usize),

    /// Start partition initialization
    InitPartition(IpcMessageSetRes),

    /// FP to HSM IPC request
    FpToHsmIpcRequest,

    /// Unknown event
    Unknown,
}

impl From<HsmFsmEvent> for u32 {
    fn from(event: HsmFsmEvent) -> Self {
        match event {
            HsmFsmEvent::RxReady => 0,
            HsmFsmEvent::TxComplete => 1,
            HsmFsmEvent::DmaComplete => 2,
            HsmFsmEvent::StartCmd => 3,
            HsmFsmEvent::Flr => 4,
            HsmFsmEvent::AdminToHsmIpcRequest => 5,
            HsmFsmEvent::FpToHsmIpcRequest => 6,
            HsmFsmEvent::HspToHsmIpcResponse => 7,
            HsmFsmEvent::AdminToHsmIpcResponse => 8,
            HsmFsmEvent::PkaDone(_idx) => 9,
            HsmFsmEvent::PkaError(_idx) => 10,
            HsmFsmEvent::ResourceReady(_res_id) => 11,
            HsmFsmEvent::TimerElapsed => 12,
            HsmFsmEvent::CheckAlive => 13,
            HsmFsmEvent::SoftAesResp => 14,
            HsmFsmEvent::SelfTestRequest => 15,
            HsmFsmEvent::ResourceCleanup(_res_id, _idx) => 16,
            HsmFsmEvent::InitPartition(_ipc_message) => 17,
            HsmFsmEvent::FpToHsmIpcResponse => 18,
            HsmFsmEvent::Unknown => 0xFFFFFFFF,
        }
    }
}
