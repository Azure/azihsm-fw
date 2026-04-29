/*
  Copyright (c) Microsoft Corporation. All rights reserved.
*/

PROVIDE(ucd_irq = DefaultHandler);
PROVIDE(ucd_err_irq = DefaultHandler);
PROVIDE(ucd_ibcq_irq = DefaultHandler);
PROVIDE(ucd_ibcq_rr1_irq = DefaultHandler);
PROVIDE(ucd_ibcq_rr2_irq = DefaultHandler);
PROVIDE(ucd_obcq_rr1_irq = DefaultHandler);
PROVIDE(ucd_obcq_rr2_irq = DefaultHandler);
PROVIDE(pcie_irq = DefaultHandler);
PROVIDE(pcie_perst_up_irq = DefaultHandler);
PROVIDE(pcie_perst_down_irq = DefaultHandler);
PROVIDE(gdma_err_irq = gdma_err_irq);
PROVIDE(gdma_cq0_irq = DefaultHandler);
PROVIDE(gdma_cq1_irq = DefaultHandler);
PROVIDE(ipc_sgi_core0 = DefaultHandler);
PROVIDE(pcie_doe_irq = DefaultHandler);
PROVIDE(pcie_ide_irq = DefaultHandler);
PROVIDE(tcon_wakeup1_irq = tcon_wakeup1_irq);
PROVIDE(rng_error_irq = rng_error_irq);
PROVIDE(cp0_dtcm_err_irq = cp0_dtcm_err_irq);
PROVIDE(cp1_dtcm_err_irq = cp1_dtcm_err_irq);
PROVIDE(gsram_irq = gsram_irq);
PROVIDE(cp0_itcm_err_irq = cp0_itcm_err_irq);
PROVIDE(cp1_itcm_err_irq = cp1_itcm_err_irq);
