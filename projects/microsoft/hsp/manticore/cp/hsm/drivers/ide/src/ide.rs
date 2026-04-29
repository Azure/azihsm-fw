// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use mcr_registers::pcie_ide::dwc_pcie_ide_apb::id_ide_cfg::RegisterBlock as IdeRegisters;

/// Mask for each of the Posted, Non-Posted, and Completion substreams.
const IDE_ALL_SUBSTREAMS_MASK: u32 = 0x7;

/// Mask for TX_KBIT_TOGGLED_IRQ_GLBL status in IDE_IO_IRQ_STATUS register.
const IDE_IO_IRQ_STATUS_TX_KBIT_TOGGLED: u32 = 0x80; // 1 << 7

/// Mask for RX_KBIT_TOGGLED_IRQ_GLBL status in IDE_IO_IRQ_STATUS register.
const IDE_IO_IRQ_STATUS_RX_KBIT_TOGGLED: u32 = 0x100; // 1 << 8

/// Mask for INSEC_STREAM_IRQ_GLBL status in IDE_IO_IRQ_STATUS register.
const IDE_IO_IRQ_STATUS_INSEC_STREAM: u32 = 0x1000; // 1 << 12

pub struct PcieIde {}

/// PCIe IDE
impl PcieIde {
    pub fn event_handler() -> Option<u32> {
        let mut report_irq_status = None;

        let reg = IdeRegisters::block();
        let status = reg.ide_io_irq_status().read();

        if status.rx_kbit_toggled_irq_glbl() {
            // Check which Rx stream(s) were toggled
            let rx_lnk_toggled: u32 = reg.rx_lnk_kbit_toggled().read().into();
            let rx_slt_toggled: u32 = reg.rx_slt_kbit_toggled().read().into();

            // Clear interrupt
            reg.ide_io_irq_status()
                .write(|_| IDE_IO_IRQ_STATUS_RX_KBIT_TOGGLED.into());

            // Execute key swap for the Tx stream(s) which are analogous to the Rx stream(s) which
            // were key swapped
            if rx_lnk_toggled != 0 {
                let kbit: u32 = reg.rx_lnk_kbit_current().read().into();
                reg.tx_lnk_kbit_cfg()
                    .write(|_| (kbit & IDE_ALL_SUBSTREAMS_MASK).into());
            }

            if rx_slt_toggled != 0 {
                let kbit: u32 = reg.rx_slt_kbit_current().read().into();
                reg.tx_slt_kbit1_cfg()
                    .write(|_| (kbit & IDE_ALL_SUBSTREAMS_MASK).into());
            }
        } else if status.tx_kbit_toggled_irq_glbl() {
            // Clear interrupt
            reg.ide_io_irq_status()
                .write(|_| IDE_IO_IRQ_STATUS_TX_KBIT_TOGGLED.into());

            // No further action is needed, as the Tx K bit toggling has occurred following the
            // Rx K bit toggling.
        } else if status.insec_stream_irq_glbl() {
            report_irq_status = Some(status.into());

            // Clear interrupt
            reg.ide_io_irq_status()
                .write(|_| IDE_IO_IRQ_STATUS_INSEC_STREAM.into());

            // No further action is needed, as TDIs which are in LOCKED/RUN state will be transited to ERROR.
            // So we just need to handle the TDISP interrupts
        } else {
            // Clear all the interrupt
            reg.ide_io_irq_status().write(|_| u32::from(status).into());
        }

        report_irq_status
    }
}
