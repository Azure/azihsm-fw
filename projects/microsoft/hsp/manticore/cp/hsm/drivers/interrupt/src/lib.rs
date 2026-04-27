// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod cntrl;
mod vector;

pub use cntrl::InterruptController;
pub use cntrl::InterruptControllerTrait;
use cortex_m::interrupt::InterruptNumber;
pub use cortex_m_rt::interrupt;
pub use Interrupt as interrupt;

extern "C" {
    fn ucd_irq();
    fn ucd_err_irq();
    fn ucd_ibcq_irq();
    fn ucd_ibcq_rr1_irq();
    fn ucd_ibcq_rr2_irq();
    fn ucd_obcq_rr1_irq();
    fn ucd_obcq_rr2_irq();
    fn pcie_irq();
    fn pcie_perst_up_irq();
    fn pcie_perst_down_irq();
    fn gdma_err_irq();
    fn gdma_cq0_irq();
    fn gdma_cq1_irq();
    fn ipc_sgi_core0();
    fn pcie_ide_irq();
    fn pcie_doe_irq();
    fn tcon_wakeup1_irq();
    fn rng_error_irq();
    fn cp0_dtcm_err_irq();
    fn cp1_dtcm_err_irq();
    fn gsram_irq();
    fn cp0_itcm_err_irq();
    fn cp1_itcm_err_irq();
}

/// Interrupt Enumeration
#[repr(u16)]
#[allow(non_camel_case_types)]
#[derive(Copy, Clone, PartialEq, Eq)]
pub enum Interrupt {
    /// UPKA0 Error IRQ
    upka0_error_irq = 0,

    /// UPKA1 Error IRQ
    upka1_error_irq = 1,

    /// UPKA0 Error IRQ
    upka2_error_irq = 2,

    /// UPKA3 Error IRQ
    upka3_error_irq = 3,

    /// UPKA4 Error IRQ
    upka4_error_irq = 4,

    /// UPKA5 Error IRQ
    upka5_error_irq = 5,

    /// UPKA6 Error IRQ
    upka6_error_irq = 6,

    /// UPKA7 Error IRQ
    upka7_error_irq = 7,

    /// UPKA8 Error IRQ
    upka8_error_irq = 8,

    /// UPKA9 Error IRQ
    upka9_error_irq = 9,

    /// UPKA10 Error IRQ
    upka10_error_irq = 10,

    /// UPKA11 Error IRQ
    upka11_error_irq = 11,

    /// UPKA12 Error IRQ
    upka12_error_irq = 12,

    /// UPKA13 Error IRQ
    upka13_error_irq = 13,

    /// UPKA14 Error IRQ
    upka14_error_irq = 14,

    /// UPKA15 Error IRQ
    upka15_error_irq = 15,

    /// AES Error IRQ
    aes_error_irq = 25,

    /// RNG Error IRQ
    rng_error_irq = 26,

    /// UPKA0 Done IRQ
    upka0_done_irq = 32,

    /// UPKA1 Done IRQ
    upka1_done_irq = 33,

    /// UPKA2 Done IRQ
    upka2_done_irq = 34,

    /// UPKA3 Done IRQ
    upka3_done_irq = 35,

    /// UPKA4 Done IRQ
    upka4_done_irq = 36,

    /// UPKA5 Done IRQ
    upka5_done_irq = 37,

    /// UPKA6 Done IRQ
    upka6_done_irq = 38,

    /// UPKA7 Done IRQ
    upka7_done_irq = 39,

    /// UPKA8 Done IRQ
    upka8_done_irq = 40,

    /// UPKA9 Done IRQ
    upka9_done_irq = 41,

    /// UPKA10 Done IRQ
    upka10_done_irq = 42,

    /// UPKA11 Done IRQ
    upka11_done_irq = 43,

    /// UPKA12 Done IRQ
    upka12_done_irq = 44,

    /// UPKA13 Done IRQ
    upka13_done_irq = 45,

    /// UPKA14 Done IRQ
    upka14_done_irq = 46,

    /// UPKA15 Done IRQ
    upka15_done_irq = 47,

    /// AES Done IRQ
    aes_done_irq = 57,

    /// RNG Done IRQ
    rng_done_irq = 58,

    /// GDMA Error IRQ
    gdma_err_irq = 64,

    /// GDMA CQ0 IRQ
    gdma_cq0_irq = 65,

    /// GDMA CQ1 IRQ
    gdma_cq1_irq = 66,

    /// GSRAM IRQ
    gsram_irq = 75,

    /// tcon_wakeup2_intr_o[0] IRQ
    tcon_wakeup0_irq = 88,

    /// tcon_wakeup2_intr_o[1] IRQ
    tcon_wakeup1_irq = 89,

    /// CP0_DTCM_ERR_INT
    cp0_dtcm_err_irq = 91,

    /// CP0_ITCM_ERR_INT
    cp0_itcm_err_irq = 92,

    /// CP1_DTCM_ERR_INT
    cp1_dtcm_err_irq = 93,

    /// CP1_ITCM_ERR_INT
    cp1_itcm_err_irq = 94,

    /// UCD Common IRQ
    ucd_irq = 96,

    /// UCD Error IRQ
    ucd_err_irq = 97,

    /// UCD Inbound Completion Queue IRQ
    ucd_ibcq_irq = 98,

    /// UCD Inbound Completion Queue Round Robin Priority 1 IRQ
    ucd_ibcq_rr1_irq = 102,

    /// UCD Inbound Completion Queue Round Robin Priority 2 IRQ
    ucd_ibcq_rr2_irq = 103,

    /// UCD Outbound Completion Queue Round Robin Priority 1 IRQ
    ucd_obcq_rr1_irq = 109,

    /// UCD Outbound Completion Queue Round Robin Priority 2 IRQ
    ucd_obcq_rr2_irq = 110,

    /// IPC SGI Core 0 IRQ
    ipc_sgi_core0 = 128,

    /// IPC SGI Core 1 IRQ
    ipc_sgi_core1 = 129,

    /// IPC SGI Core 2 IRQ
    ipc_sgi_core2 = 130,

    /// IPC SGI Core 3 IRQ
    ipc_sgi_core3 = 131,

    /// IPC SGI Core 4 IRQ
    ipc_sgi_core4 = 132,

    /// IPC SGI Core 5 IRQ
    ipc_sgi_core5 = 133,

    /// PCIe PERST Falling Edge IRQ
    pcie_perst_down_irq = 165,

    /// PCIe PERST Raising Edge IRQ
    pcie_perst_up_irq = 166,

    /// PCIe Common IRQ
    pcie_irq = 167,

    /// PCIe IDE IRQ
    pcie_ide_irq = 168,

    /// PCIe DOE IRQ
    pcie_doe_irq = 172,
}

#[allow(non_upper_case_globals)]
impl Interrupt {
    /// UPKA0 Error IRQ
    pub const Upka0ErrorIrq: Interrupt = Interrupt::upka0_error_irq;

    /// UPKA1 Error IRQ
    pub const Upka1ErrorIrq: Interrupt = Interrupt::upka1_error_irq;

    /// UPKA2 Error IRQ
    pub const Upka2ErrorIrq: Interrupt = Interrupt::upka2_error_irq;

    /// UPKA3 Error IRQ
    pub const Upka3ErrorIrq: Interrupt = Interrupt::upka3_error_irq;

    /// UPKA4 Error IRQ
    pub const Upka4ErrorIrq: Interrupt = Interrupt::upka4_error_irq;

    /// UPKA5 Error IRQ
    pub const Upka5ErrorIrq: Interrupt = Interrupt::upka5_error_irq;

    /// UPKA6 Error IRQ
    pub const Upka6ErrorIrq: Interrupt = Interrupt::upka6_error_irq;

    /// UPKA7 Error IRQ
    pub const Upka7ErrorIrq: Interrupt = Interrupt::upka7_error_irq;

    /// UPKA8 Error IRQ
    pub const Upka8ErrorIrq: Interrupt = Interrupt::upka8_error_irq;

    /// UPKA9 Error IRQ
    pub const Upka9ErrorIrq: Interrupt = Interrupt::upka9_error_irq;

    /// UPKA10 Error IRQ
    pub const Upka10ErrorIrq: Interrupt = Interrupt::upka10_error_irq;

    /// UPKA11 Error IRQ
    pub const Upka11ErrorIrq: Interrupt = Interrupt::upka11_error_irq;

    /// UPKA12 Error IRQ
    pub const Upka12ErrorIrq: Interrupt = Interrupt::upka12_error_irq;

    /// UPKA13 Error IRQ
    pub const Upka13ErrorIrq: Interrupt = Interrupt::upka13_error_irq;

    /// UPKA14 Error IRQ
    pub const Upka14ErrorIrq: Interrupt = Interrupt::upka14_error_irq;

    /// UPKA15 Error IRQ
    pub const Upka15ErrorIrq: Interrupt = Interrupt::upka15_error_irq;

    /// AES Error IRQ
    pub const AesErrorIrq: Interrupt = Interrupt::aes_error_irq;

    /// RNG Error IRQ
    pub const RngErrorIrq: Interrupt = Interrupt::rng_error_irq;

    /// UPKA0 Done IRQ
    pub const Upka0DoneIrq: Interrupt = Interrupt::upka0_done_irq;

    /// UPKA1 Done IRQ
    pub const Upka1DoneIrq: Interrupt = Interrupt::upka1_done_irq;

    /// UPKA2 Done IRQ
    pub const Upka2DoneIrq: Interrupt = Interrupt::upka2_done_irq;

    /// UPKA3 Done IRQ
    pub const Upka3DoneIrq: Interrupt = Interrupt::upka3_done_irq;

    /// UPKA4 Done IRQ
    pub const Upka4DoneIrq: Interrupt = Interrupt::upka4_done_irq;

    /// UPKA5 Done IRQ
    pub const Upka5DoneIrq: Interrupt = Interrupt::upka5_done_irq;

    /// UPKA6 Done IRQ
    pub const Upka6DoneIrq: Interrupt = Interrupt::upka6_done_irq;

    /// UPKA7 Done IRQ
    pub const Upka7DoneIrq: Interrupt = Interrupt::upka7_done_irq;

    /// UPKA8 Done IRQ
    pub const Upka8DoneIrq: Interrupt = Interrupt::upka8_done_irq;

    /// UPKA9 Done IRQ
    pub const Upka9DoneIrq: Interrupt = Interrupt::upka9_done_irq;

    /// UPKA10 Done IRQ
    pub const Upka10DoneIrq: Interrupt = Interrupt::upka10_done_irq;

    /// UPKA11 Done IRQ
    pub const Upka11DoneIrq: Interrupt = Interrupt::upka11_done_irq;

    /// UPKA12 Done IRQ
    pub const Upka12DoneIrq: Interrupt = Interrupt::upka12_done_irq;

    /// UPKA13 Done IRQ
    pub const Upka13DoneIrq: Interrupt = Interrupt::upka13_done_irq;

    /// UPKA14 Done IRQ
    pub const Upka14DoneIrq: Interrupt = Interrupt::upka14_done_irq;

    /// UPKA15 Done IRQ
    pub const Upka15DoneIrq: Interrupt = Interrupt::upka15_done_irq;

    /// AES Done IRQ
    pub const AesDoneIrq: Interrupt = Interrupt::aes_done_irq;

    /// RNG Done IRQ
    pub const RngDoneIrq: Interrupt = Interrupt::rng_done_irq;

    /// GDMA Err IRQ
    pub const GdmaErrIrq: Interrupt = Interrupt::gdma_err_irq;

    /// GDMA CQ0 IRQ
    pub const GdmaCq0Irq: Interrupt = Interrupt::gdma_cq0_irq;

    /// GDMA CQ1 IRQ
    pub const GdmaCq1Irq: Interrupt = Interrupt::gdma_cq1_irq;

    /// CP0 DTCM ERROR IRQ
    pub const Cp0DtcmErrIrq: Interrupt = Interrupt::cp0_dtcm_err_irq;

    /// CP1 DTCM ERROR IRQ
    pub const Cp1DtcmErrIrq: Interrupt = Interrupt::cp1_dtcm_err_irq;

    /// GSRAM IRQ
    pub const GsramDoubleBitError: Interrupt = Interrupt::gsram_irq;

    /// CP0 ITCM ERROR IRQ
    pub const Cp0ItcmErrIrq: Interrupt = Interrupt::cp0_itcm_err_irq;

    /// CP1 ITCM ERROR IRQ
    pub const Cp1ItcmErrIrq: Interrupt = Interrupt::cp1_itcm_err_irq;

    /// TCON Wakeup0 IRQ
    pub const TconWakeup0Irq: Interrupt = Interrupt::tcon_wakeup0_irq;

    /// TCON Wakeup1 IRQ
    pub const TconWakeup1Irq: Interrupt = Interrupt::tcon_wakeup1_irq;

    /// UCD Common IRQ
    pub const UcdIrq: Interrupt = Interrupt::ucd_irq;

    /// UCD Error IRQ
    pub const UcdErrIrq: Interrupt = Interrupt::ucd_err_irq;

    /// UCD Inbound Completion Queue IRQ
    pub const UcdIbcqIrq: Interrupt = Interrupt::ucd_ibcq_irq;

    /// UCD Inbound Completion Queue Round Robin Priority 1 IRQ
    pub const UcdIbcqRr1Irq: Interrupt = Interrupt::ucd_ibcq_rr1_irq;

    /// UCD Inbound Completion Queue Round Robin Priority 2 IRQ
    pub const UcdIbcqRr2Irq: Interrupt = Interrupt::ucd_ibcq_rr2_irq;

    /// UCD Outbound Completion Queue Round Robin Priority 1 IRQ
    pub const UcdObcqRr1Irq: Interrupt = Interrupt::ucd_obcq_rr1_irq;

    /// UCD Outbound Completion Queue Round Robin Priority 2 IRQ
    pub const UcdObcqRr2Irq: Interrupt = Interrupt::ucd_obcq_rr2_irq;

    /// IPC SGI Core 0 IRQ
    pub const IpcSgiCore0: Interrupt = Interrupt::ipc_sgi_core0;

    /// IPC SGI Core 1 IRQ
    pub const IpcSgiCore1: Interrupt = Interrupt::ipc_sgi_core1;

    /// IPC SGI Core 2 IRQ
    pub const IpcSgiCore2: Interrupt = Interrupt::ipc_sgi_core2;

    /// IPC SGI Core 3 IRQ
    pub const IpcSgiCore3: Interrupt = Interrupt::ipc_sgi_core3;

    /// IPC SGI Core 4 IRQ
    pub const IpcSgiCore4: Interrupt = Interrupt::ipc_sgi_core4;

    /// IPC SGI Core 5 IRQ
    pub const IpcSgiCore5: Interrupt = Interrupt::ipc_sgi_core5;

    /// PCIe PERST Falling Edge IRQ
    pub const PciePerstDownIrq: Interrupt = Interrupt::pcie_perst_down_irq;

    /// PCIe PERST Raising Edge IRQ
    pub const PciePerstUpIrq: Interrupt = Interrupt::pcie_perst_up_irq;

    /// PCIe Common IRQ
    pub const PcieIrq: Interrupt = Interrupt::pcie_irq;

    /// PCIe IDE Interrupt
    pub const PcieIdeIrq: Interrupt = Interrupt::pcie_ide_irq;

    /// PCIe DOE Interrupt
    pub const PcieDoeIrq: Interrupt = Interrupt::pcie_doe_irq;
}

/// Interrupt Number
unsafe impl InterruptNumber for Interrupt {
    /// Return the interrupt number associated with this variant.
    #[inline(always)]
    fn number(self) -> u16 {
        self as u16
    }
}
