// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::vec;
use alloc::vec::Vec;

use mcr_doe::DoeEvents;
use mcr_doe::PcieDoe;
use mcr_event_loop::IrqGroup;
use mcr_event_loop::IrqInfo;
use mcr_event_loop::IrqSensitivity;
use mcr_event_loop_derive::Event;
use mcr_interrupt_controller::Interrupt;
use mcr_ipc_controller::IpcController;
use mcr_ipc_controller::IpcDescriptor;
use mcr_ipc_controller::IpcIntBlock;
use mcr_ipc_message::ShutdownInfo;
use mcr_pcie_controller::PcieController;
use mcr_pcie_controller::PcieEvent;
use mcr_pcie_controller::TdispIntInfo;
use mcr_queue_controller::QueueCntrlEvent;
use mcr_queue_controller::QueueController;
use mcr_self_test::SelfTest;

use crate::alloc::string::ToString;
use crate::resource::AdminFsmResourceId;

/// Enumeration to define the NVIC events supported by Admin core.
///
/// # Notes
///
/// List of NVIC events handled by NVIC event loop in the same priority as
/// the order in which the NVIC interrupts listed in this enum
#[derive(Event, Eq, PartialEq, Clone, Copy, Debug)]
pub(crate) enum NvicEvent {
    /// PCIE PERST down event is the highest priority event
    #[event(interrupt = Interrupt::PciePerstDownIrq)]
    PciePerstDown,

    /// Pcie PERST up event
    #[event(interrupt = Interrupt::PciePerstUpIrq)]
    PciePerstUp,

    /// PCIe summary event
    #[event(interrupt = Interrupt::PcieIrq)]
    PcieCommon,

    /// Common UCD Queue controller event
    #[event(interrupt = Interrupt::UcdIrq)]
    Ucd,

    /// INTC IPC summary event
    #[event(interrupt = Interrupt::IpcSgiCore0)]
    Ipc,

    /// UCD outbound completion queue event
    #[event(interrupt = Interrupt::UcdObcqRr1Irq)]
    UcdObcq,

    /// GDMA event
    #[event(interrupt = Interrupt::GdmaCq0Irq)]
    Gdma,

    /// UCD inbound completion queue event
    #[event(interrupt = Interrupt::UcdIbcqRr1Irq)]
    UcdIbcq,

    /// DOE event
    #[event(interrupt = Interrupt::PcieDoeIrq)]
    Doe,

    /// IDE event
    #[event(interrupt = Interrupt::PcieIdeIrq)]
    Ide,

    // TCON wakeup timer event
    #[event(interrupt = Interrupt::TconWakeup0Irq)]
    WakeupTimer0,
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub(crate) enum AdminFsmEvent {
    /// PCIe PERST up
    PciePerstUp,

    /// PCIe PERST down
    PciePerstDown,

    /// PCIe Function Level Reset
    PcieFlr,

    /// PCIe virtual function level reset
    PcieVflr(u64),

    /// DOE message
    Doe(DoeEvents),

    /// DOE FSM Init
    DoeFsmInit,

    /// IDE event (ide_io_irq_status)
    Ide(Option<u32>),

    /// Fastpath processor to Admin IPC response
    FpToAdminIpcResponse,

    /// Fastpath processor Reset complete event
    FpResetComplete,

    /// HSM processor IPC response
    HsmIpcResponse,

    /// HSP processor to Admin IPC response
    HspToAdminIpcResponse,

    /// HSP processor to Admin IPC request
    HspToAdminIpcRequest,

    /// HSM processor Reset complete event
    HsmResetComplete,

    /// HSM to Admin IPC request
    HsmToAdminIpcRequest,

    /// Controller state change event
    CntrlStateChange(u128),

    /// NVMe subsystem reset pending
    Nssr(u128),

    /// Request ready event
    RxReady,

    /// Response complete event
    TxComplete,

    /// DMA complete event
    DmaComplete,

    /// Resource ready
    ResourceReady(AdminFsmResourceId),

    /// Start Admin Cmd
    StartCmd,

    /// TCON timer elapsed
    TimerElapsed,

    /// Shutdown request includes drain-time
    ShutdownRequest((ShutdownInfo, u32)),

    /// Queue empty event
    SchedulerQueueEmptyEvent,

    /// Io Cancellation complete event from HSM
    IoCancellationComplete,

    /// SoftAes request from IO core
    SoftAesRequest,

    /// Self test complete event
    SelfTestResponse,

    /// Negative Self test event
    NegativeSelfTest(SelfTest),

    /// TDISP interrupt event
    TdispInt(TdispIntInfo),

    /// HSP processor to Admin Stop Interface IPC request
    HspToAdminStopInterfaceIpcRequest,

    /// Stop Interface request for internal process (mask, tag from sp)
    StopInterfaceRequest((u128, u32)),

    /// AES GCM Extension (workaround) request
    AesGcmExtRequest,

    /// Unknown
    Unknown,
}

impl From<NvicEvent> for AdminFsmEvent {
    fn from(value: NvicEvent) -> Self {
        match value {
            NvicEvent::PciePerstUp => {
                PcieController::clr_perst_up();
                AdminFsmEvent::PciePerstUp
            }
            NvicEvent::PciePerstDown => {
                PcieController::clr_perst_down();
                AdminFsmEvent::PciePerstDown
            }
            NvicEvent::PcieCommon => PcieController::event()
                .map(|e| match e {
                    PcieEvent::Flr => AdminFsmEvent::PcieFlr,
                    PcieEvent::VflrActive(pending_list) => AdminFsmEvent::PcieVflr(pending_list),
                    PcieEvent::TdispInterrupt(info) => AdminFsmEvent::TdispInt(info),
                })
                .unwrap_or(AdminFsmEvent::Unknown),
            NvicEvent::Ipc => IpcController::descriptor(IpcIntBlock::IntBlock0)
                .map(|e| match e {
                    IpcDescriptor::Descriptor1 => AdminFsmEvent::FpToAdminIpcResponse,
                    IpcDescriptor::Descriptor12 => AdminFsmEvent::HsmToAdminIpcRequest,
                    IpcDescriptor::Descriptor18 => AdminFsmEvent::FpResetComplete,
                    IpcDescriptor::Descriptor21 => AdminFsmEvent::HspToAdminIpcResponse,
                    IpcDescriptor::Descriptor22 => AdminFsmEvent::HspToAdminIpcRequest,
                    IpcDescriptor::Descriptor29 => AdminFsmEvent::HsmResetComplete,
                    IpcDescriptor::Descriptor31 => AdminFsmEvent::HsmIpcResponse,
                    IpcDescriptor::Descriptor8 => AdminFsmEvent::HspToAdminStopInterfaceIpcRequest,
                    _ => AdminFsmEvent::Unknown,
                })
                .unwrap_or(AdminFsmEvent::Unknown),
            NvicEvent::Ucd => QueueController::event()
                .map(|e| match e {
                    QueueCntrlEvent::StateChange(pending) => {
                        AdminFsmEvent::CntrlStateChange(pending)
                    }
                    QueueCntrlEvent::NssrPending(pending) => AdminFsmEvent::Nssr(pending),
                })
                .unwrap_or(AdminFsmEvent::Unknown),
            NvicEvent::UcdIbcq => AdminFsmEvent::RxReady,
            NvicEvent::UcdObcq => AdminFsmEvent::TxComplete,
            NvicEvent::Gdma => AdminFsmEvent::DmaComplete,
            NvicEvent::Doe => AdminFsmEvent::Doe(PcieDoe::event()),
            NvicEvent::Ide => AdminFsmEvent::Ide(None),
            NvicEvent::WakeupTimer0 => AdminFsmEvent::TimerElapsed,
        }
    }
}

impl From<AdminFsmEvent> for u32 {
    fn from(event: AdminFsmEvent) -> Self {
        match event {
            AdminFsmEvent::PciePerstUp => 0,
            AdminFsmEvent::PciePerstDown => 1,
            AdminFsmEvent::PcieFlr => 2,
            AdminFsmEvent::PcieVflr(_) => 3,
            AdminFsmEvent::Doe(_) => 4,
            AdminFsmEvent::DoeFsmInit => 5,
            AdminFsmEvent::Ide(_) => 6,
            AdminFsmEvent::FpToAdminIpcResponse => 7,
            AdminFsmEvent::FpResetComplete => 8,
            AdminFsmEvent::HsmIpcResponse => 9,
            AdminFsmEvent::HspToAdminIpcResponse => 10,
            AdminFsmEvent::HspToAdminIpcRequest => 11,
            AdminFsmEvent::HsmResetComplete => 12,
            AdminFsmEvent::HsmToAdminIpcRequest => 13,
            AdminFsmEvent::CntrlStateChange(_) => 14,
            AdminFsmEvent::RxReady => 15,
            AdminFsmEvent::TxComplete => 16,
            AdminFsmEvent::DmaComplete => 17,
            AdminFsmEvent::ResourceReady(_) => 18,
            AdminFsmEvent::StartCmd => 19,
            AdminFsmEvent::TimerElapsed => 20,
            AdminFsmEvent::ShutdownRequest((_, _)) => 21,
            AdminFsmEvent::SchedulerQueueEmptyEvent => 22,
            AdminFsmEvent::IoCancellationComplete => 23,
            AdminFsmEvent::SoftAesRequest => 24,
            AdminFsmEvent::SelfTestResponse => 25,
            AdminFsmEvent::NegativeSelfTest(_) => 26,
            AdminFsmEvent::TdispInt(_) => 27,
            AdminFsmEvent::HspToAdminStopInterfaceIpcRequest => 28,
            AdminFsmEvent::StopInterfaceRequest(_) => 29,
            AdminFsmEvent::Nssr(_) => 30,
            AdminFsmEvent::AesGcmExtRequest => 31,
            AdminFsmEvent::Unknown => 0xFFFFFFFF,
        }
    }
}
