// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::*;

/// Interrupt Vector
#[derive(Clone, Copy)]
union Vector {
    /// Interrupt Handler
    handler: unsafe extern "C" fn(),

    /// Reserved/Unused vector
    reserved: usize,
}

/// Cortex-M7 Interrupt Vector Table
#[link_section = ".vector_table.interrupts"]
#[no_mangle]
static __INTERRUPTS: [Vector; 177] = [
    // 0 - Reserved
    Vector { reserved: 0 },
    // 1 - Reserved
    Vector { reserved: 0 },
    // 2 - Reserved
    Vector { reserved: 0 },
    // 3 - Reserved
    Vector { reserved: 0 },
    // 4 - Reserved
    Vector { reserved: 0 },
    // 5 - Reserved
    Vector { reserved: 0 },
    // 6 - Reserved
    Vector { reserved: 0 },
    // 7 - Reserved
    Vector { reserved: 0 },
    // 8 - Reserved
    Vector { reserved: 0 },
    // 9 - Reserved
    Vector { reserved: 0 },
    // 10 - Reserved
    Vector { reserved: 0 },
    // 11 - Reserved
    Vector { reserved: 0 },
    // 12 - Reserved
    Vector { reserved: 0 },
    // 13 - Reserved
    Vector { reserved: 0 },
    // 14 - Reserved
    Vector { reserved: 0 },
    // 15 - Reserved
    Vector { reserved: 0 },
    // 16 - Reserved
    Vector { reserved: 0 },
    // 17 - Reserved
    Vector { reserved: 0 },
    // 18 - Reserved
    Vector { reserved: 0 },
    // 19 - Reserved
    Vector { reserved: 0 },
    // 20 - Reserved
    Vector { reserved: 0 },
    // 21 - Reserved
    Vector { reserved: 0 },
    // 22 - Reserved
    Vector { reserved: 0 },
    // 23 - Reserved
    Vector { reserved: 0 },
    // 24 - Reserved
    Vector { reserved: 0 },
    // 25 - Reserved
    Vector { reserved: 0 },
    // 26 - RNG Error Interrupt
    Vector {
        handler: rng_error_irq,
    },
    // 27 - Reserved
    Vector { reserved: 0 },
    // 28 - Reserved
    Vector { reserved: 0 },
    // 29 - Reserved
    Vector { reserved: 0 },
    // 30 - Reserved
    Vector { reserved: 0 },
    // 31 - Reserved
    Vector { reserved: 0 },
    // 32 - Reserved
    Vector { reserved: 0 },
    // 33 - Reserved
    Vector { reserved: 0 },
    // 34 - Reserved
    Vector { reserved: 0 },
    // 35 - Reserved
    Vector { reserved: 0 },
    // 36 - Reserved
    Vector { reserved: 0 },
    // 37 - Reserved
    Vector { reserved: 0 },
    // 38 - Reserved
    Vector { reserved: 0 },
    // 39 - Reserved
    Vector { reserved: 0 },
    // 40 - Reserved
    Vector { reserved: 0 },
    // 41 - Reserved
    Vector { reserved: 0 },
    // 42 - Reserved
    Vector { reserved: 0 },
    // 43 - Reserved
    Vector { reserved: 0 },
    // 44 - Reserved
    Vector { reserved: 0 },
    // 45 - Reserved
    Vector { reserved: 0 },
    // 46 - Reserved
    Vector { reserved: 0 },
    // 47 - Reserved
    Vector { reserved: 0 },
    // 48 - Reserved
    Vector { reserved: 0 },
    // 49 - Reserved
    Vector { reserved: 0 },
    // 50 - Reserved
    Vector { reserved: 0 },
    // 51 - Reserved
    Vector { reserved: 0 },
    // 52 - Reserved
    Vector { reserved: 0 },
    // 53 - Reserved
    Vector { reserved: 0 },
    // 54 - Reserved
    Vector { reserved: 0 },
    // 55 - Reserved
    Vector { reserved: 0 },
    // 56 - Reserved
    Vector { reserved: 0 },
    // 57 - Reserved
    Vector { reserved: 0 },
    // 58 - Reserved
    Vector { reserved: 0 },
    // 59 - Reserved
    Vector { reserved: 0 },
    // 60 - Reserved
    Vector { reserved: 0 },
    // 61 - Reserved
    Vector { reserved: 0 },
    // 62 - Reserved
    Vector { reserved: 0 },
    // 63 - Reserved
    Vector { reserved: 0 },
    // 64 - GDMA Error Interrupt
    Vector {
        handler: gdma_err_irq,
    },
    // 65 - GDMA Completion Queue 0 Interrupt
    Vector {
        handler: gdma_cq0_irq,
    },
    // 66 - GDMA Completion Queue 1 Interrupt
    Vector {
        handler: gdma_cq1_irq,
    },
    // 67 - Reserved
    Vector { reserved: 0 },
    // 68 - Reserved
    Vector { reserved: 0 },
    // 69 - Reserved
    Vector { reserved: 0 },
    // 70 - Reserved
    Vector { reserved: 0 },
    // 71 - Reserved
    Vector { reserved: 0 },
    // 72 - Reserved
    Vector { reserved: 0 },
    // 73 - Reserved
    Vector { reserved: 0 },
    // 74 - Reserved
    Vector { reserved: 0 },
    // 75 - GSRAM interrupt handler
    Vector { handler: gsram_irq },
    // 76 - Reserved
    Vector { reserved: 0 },
    // 77 - Reserved
    Vector { reserved: 0 },
    // 78 - Reserved
    Vector { reserved: 0 },
    // 79 - Reserved
    Vector { reserved: 0 },
    // 80 - Reserved
    Vector { reserved: 0 },
    // 81 - Reserved
    Vector { reserved: 0 },
    // 82 - Reserved
    Vector { reserved: 0 },
    // 83 - Reserved
    Vector { reserved: 0 },
    // 84 - Reserved
    Vector { reserved: 0 },
    // 85 - Reserved
    Vector { reserved: 0 },
    // 86 - Reserved
    Vector { reserved: 0 },
    // 87 - Reserved
    Vector { reserved: 0 },
    // 88 - Reserved
    Vector { reserved: 0 },
    // 89 - TCON1 handler
    Vector {
        handler: tcon_wakeup1_irq,
    },
    // 90 - Reserved
    Vector { reserved: 0 },
    // 91 - CP0 DTCM handler
    Vector {
        handler: cp0_dtcm_err_irq,
    },
    // 92 - CP0 ITCM handler
    Vector {
        handler: cp0_itcm_err_irq,
    },
    // 93 - CP1 DTCM handler
    Vector {
        handler: cp1_dtcm_err_irq,
    },
    // 94 - CP1 ITCM
    Vector {
        handler: cp1_itcm_err_irq,
    },
    // 95 - Reserved
    Vector { reserved: 0 },
    // 96 - UCD Common Interrupt
    Vector { handler: ucd_irq },
    // 97 - UCD Error Interrupt
    Vector {
        handler: ucd_err_irq,
    },
    // 98 - UCD Inbound Queue Common Irq
    Vector {
        handler: ucd_ibcq_irq,
    },
    // 99 - Reserved
    Vector { reserved: 0 },
    // 100 - Reserved
    Vector { reserved: 0 },
    // 101 - Reserved
    Vector { reserved: 0 },
    // 102 - UCD Inbound Queue Round Robin 1 Interrupt
    Vector {
        handler: ucd_ibcq_rr1_irq,
    },
    // 103 - UCD Inbound Queue Round Robin 2 Interrupt
    Vector {
        handler: ucd_ibcq_rr2_irq,
    },
    // 104 - Reserved
    Vector { reserved: 0 },
    // 105 - Reserved
    Vector { reserved: 0 },
    // 106 - Reserved
    Vector { reserved: 0 },
    // 107 - Reserved
    Vector { reserved: 0 },
    // 108 - Reserved
    Vector { reserved: 0 },
    // 109 - UCD Outbound Queue Round Robin 1 Interrupt
    Vector {
        handler: ucd_obcq_rr1_irq,
    },
    // 110 - UCD Outbound Queue Round Robin 2 Interrupt
    Vector {
        handler: ucd_obcq_rr2_irq,
    },
    // 111 - Reserved
    Vector { reserved: 0 },
    // 112 - Reserved
    Vector { reserved: 0 },
    // 113 - Reserved
    Vector { reserved: 0 },
    // 114 - Reserved
    Vector { reserved: 0 },
    // 115 - Reserved
    Vector { reserved: 0 },
    // 116 - Reserved
    Vector { reserved: 0 },
    // 117 - Reserved
    Vector { reserved: 0 },
    // 118 - Reserved
    Vector { reserved: 0 },
    // 119 - Reserved
    Vector { reserved: 0 },
    // 120 - Reserved
    Vector { reserved: 0 },
    // 121 - Reserved
    Vector { reserved: 0 },
    // 122 - Reserved
    Vector { reserved: 0 },
    // 123 - Reserved
    Vector { reserved: 0 },
    // 124 - Reserved
    Vector { reserved: 0 },
    // 125 - Reserved
    Vector { reserved: 0 },
    // 126 - Reserved
    Vector { reserved: 0 },
    // 127 - Reserved
    Vector { reserved: 0 },
    // 128 - Reserved
    Vector {
        handler: ipc_sgi_core0,
    },
    // 129 - Reserved
    Vector { reserved: 0 },
    // 130 - Reserved
    Vector { reserved: 0 },
    // 131 - Reserved
    Vector { reserved: 0 },
    // 132 - Reserved
    Vector { reserved: 0 },
    // 133 - Reserved
    Vector { reserved: 0 },
    // 134 - Reserved
    Vector { reserved: 0 },
    // 135 - Reserved
    Vector { reserved: 0 },
    // 136 - Reserved
    Vector { reserved: 0 },
    // 137 - Reserved
    Vector { reserved: 0 },
    // 138 - Reserved
    Vector { reserved: 0 },
    // 139 - Reserved
    Vector { reserved: 0 },
    // 140 - Reserved
    Vector { reserved: 0 },
    // 141 - Reserved
    Vector { reserved: 0 },
    // 142 - Reserved
    Vector { reserved: 0 },
    // 143 - Reserved
    Vector { reserved: 0 },
    // 144 - Reserved
    Vector { reserved: 0 },
    // 145 - Reserved
    Vector { reserved: 0 },
    // 146 - Reserved
    Vector { reserved: 0 },
    // 147 - Reserved
    Vector { reserved: 0 },
    // 148 - Reserved
    Vector { reserved: 0 },
    // 149 - Reserved
    Vector { reserved: 0 },
    // 150 - Reserved
    Vector { reserved: 0 },
    // 151 - Reserved
    Vector { reserved: 0 },
    // 152 - Reserved
    Vector { reserved: 0 },
    // 153 - Reserved
    Vector { reserved: 0 },
    // 154 - Reserved
    Vector { reserved: 0 },
    // 155 - Reserved
    Vector { reserved: 0 },
    // 156 - Reserved
    Vector { reserved: 0 },
    // 157 - Reserved
    Vector { reserved: 0 },
    // 158 - Reserved
    Vector { reserved: 0 },
    // 159 - Reserved
    Vector { reserved: 0 },
    // 160 - Reserved
    Vector { reserved: 0 },
    // 161 - Reserved
    Vector { reserved: 0 },
    // 162 - Reserved
    Vector { reserved: 0 },
    // 163 - Reserved
    Vector { reserved: 0 },
    // 164 - Reserved
    Vector { reserved: 0 },
    // 165 - PCIe PERST Falling Edge ISR
    Vector {
        handler: pcie_perst_down_irq,
    },
    // 166 - PCIe PERST Rising Edge ISR
    Vector {
        handler: pcie_perst_up_irq,
    },
    // 167 - PCIe ISR
    Vector { handler: pcie_irq },
    // 168 - IDE global ISR
    Vector {
        handler: pcie_ide_irq,
    },
    // 169 - Reserved
    Vector { reserved: 0 },
    // 170 - Reserved
    Vector { reserved: 0 },
    // 171 - Reserved
    Vector { reserved: 0 },
    // 172 - DOE ISR
    Vector {
        handler: pcie_doe_irq,
    },
    // 173 - Reserved
    Vector { reserved: 0 },
    // 174 - Reserved
    Vector { reserved: 0 },
    // 175 - Reserved
    Vector { reserved: 0 },
    // 176 - Reserved
    Vector { reserved: 0 },
];
