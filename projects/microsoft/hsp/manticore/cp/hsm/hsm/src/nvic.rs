// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_interrupt_controller::Interrupt;
use mcr_interrupt_controller::InterruptControllerTrait;
use mcr_ipc_controller::IpcController;
use mcr_ipc_controller::IpcDescriptor;
use mcr_ipc_controller::IpcIntBlock;
use mcr_mem_map::HsmDtcmMemMap;
use mcr_registers::cortexm7::systemcontrol;

use crate::handler::HsmEventHandler;
use crate::HsmEnvTrait;
use crate::HsmFsmEvent;

pub fn handle_nvic_ipc_event<E: HsmEnvTrait, I: InterruptControllerTrait>(
    reg: &systemcontrol::RegisterBlock,
    handler: &mut HsmEventHandler<E>,
    intc: &I,
) {
    const MASK_IPC_SGI_CORE1: u32 = 0x2;

    let int_ipc = reg.nvic_ispr_at(4).nvic_ispr().read() & MASK_IPC_SGI_CORE1;
    if int_ipc != 0 {
        let event = IpcController::descriptor(IpcIntBlock::IntBlock1)
            .map(|e| match e {
                IpcDescriptor::Descriptor10 => HsmFsmEvent::FpToHsmIpcRequest,
                #[cfg(any(feature = "mcr_test_hooks", feature = "mcr_manual_test_hooks"))]
                IpcDescriptor::Descriptor13 => HsmFsmEvent::AdminToHsmIpcResponse,
                IpcDescriptor::Descriptor16 => HsmFsmEvent::FpToHsmIpcResponse,
                IpcDescriptor::Descriptor28 => HsmFsmEvent::Flr,
                IpcDescriptor::Descriptor30 => HsmFsmEvent::AdminToHsmIpcRequest,
                IpcDescriptor::Descriptor25 => HsmFsmEvent::HspToHsmIpcResponse,
                _ => HsmFsmEvent::Unknown,
            })
            .unwrap_or(HsmFsmEvent::Unknown);
        handler.on_event(event);
        // IPC interrupts are level so we will clear here after processing
        intc.clear(Interrupt::IpcSgiCore1);
    }
}

pub fn handle_nvic_ucd_event<E: HsmEnvTrait, I: InterruptControllerTrait>(
    reg: &systemcontrol::RegisterBlock,
    handler: &mut HsmEventHandler<E>,
    intc: &I,
) {
    const MASK_UCD_OBCQ: u32 = 0x4000;
    const MASK_UCD_IBCQ: u32 = 0x80;

    let int_ucd = reg.nvic_ispr_at(3).nvic_ispr().read() & (MASK_UCD_OBCQ | MASK_UCD_IBCQ);
    if int_ucd != 0 {
        if int_ucd & MASK_UCD_OBCQ != 0 {
            handler.on_event(HsmFsmEvent::TxComplete);
            // UCD interrupts are level so we will clear here after processing
            intc.clear(Interrupt::UcdObcqRr2Irq);
        } else {
            handler.on_event(HsmFsmEvent::RxReady);
            // UCD interrupts are level so we will clear here after processing
            intc.clear(Interrupt::UcdIbcqRr2Irq);
        }
    }
}

pub fn handle_nvic_gdma_timer_event<E: HsmEnvTrait, I: InterruptControllerTrait>(
    reg: &systemcontrol::RegisterBlock,
    handler: &mut HsmEventHandler<E>,
    intc: &I,
) {
    const MASK_GDMA_CQ1: u32 = 0x4;
    const MASK_TCON_WAKEUP0: u32 = 0x100_0000;

    let int_gdma_timer =
        reg.nvic_ispr_at(2).nvic_ispr().read() & (MASK_GDMA_CQ1 | MASK_TCON_WAKEUP0);
    if int_gdma_timer != 0 {
        if int_gdma_timer & MASK_TCON_WAKEUP0 != 0 {
            // Timer interrupts are edge interrupts. We will clear it here before we process
            // the event as it does not read any register etc.
            handler.on_event(HsmFsmEvent::TimerElapsed);
            intc.clear(Interrupt::TconWakeup0Irq);
        } else {
            handler.on_event(HsmFsmEvent::DmaComplete);
            // GDMA interrupts are level so we will clear here after processing
            intc.clear(Interrupt::GdmaCq1Irq);
        }
    }
}

pub fn handle_nvic_crypto_done_event<E: HsmEnvTrait>(
    reg: &systemcontrol::RegisterBlock,
    handler: &mut HsmEventHandler<E>,
    pka_start_index: u32,
) {
    const MASK_PKA_DONE_0_15: u32 = 0xFFFF;

    let int_done = reg.nvic_ispr_at(1).nvic_ispr().read() & (MASK_PKA_DONE_0_15);
    if int_done != 0 {
        // PKA Interrupts are pulse and clearing them is handled by the PKA driver as soon as
        // PKA finishes reading the register.
        let index = find_pka_index(int_done, pka_start_index);
        handler.on_event(HsmFsmEvent::PkaDone(index));
    }
}

pub fn handle_nvic_crypto_error_event<E: HsmEnvTrait>(
    reg: &systemcontrol::RegisterBlock,
    handler: &mut HsmEventHandler<E>,
    pka_start_index: u32,
) {
    const MASK_PKA_ERROR_0_15: u32 = 0xFFFF;

    let int_error = reg.nvic_ispr_at(0).nvic_ispr().read() & (MASK_PKA_ERROR_0_15);
    if int_error != 0 {
        // PKA Interrupts are pulse and clearing them is handled by the PKA driver as soon as
        // PKA finishes reading the register.
        let index = find_pka_index(int_error, pka_start_index);
        handler.on_event(HsmFsmEvent::PkaError(index));
    }
}

pub fn handle_soft_interrupt<E: HsmEnvTrait>(handler: &mut HsmEventHandler<E>) {
    // TODO: Optimized to read the PI and CI directly to improve performance.
    // Find a way to do this without breaking the abstraction.
    if HsmDtcmMemMap::soft_aes_resp_ci().get() != HsmDtcmMemMap::soft_aes_resp_pi().get() {
        handler.on_event(HsmFsmEvent::SoftAesResp);
    }

    if HsmDtcmMemMap::self_test_req_ci().get() != HsmDtcmMemMap::self_test_req_pi().get() {
        handler.on_event(HsmFsmEvent::SelfTestRequest);
    }

    if HsmDtcmMemMap::get_bulk_key_req_ci().get() != HsmDtcmMemMap::get_bulk_key_req_pi().get() {
        handler.on_event(HsmFsmEvent::GetBulkKeyRequest);
    }
}

fn find_pka_index(int: u32, start_index: u32) -> usize {
    ((int.rotate_right(start_index).trailing_zeros() + start_index) & 0xF) as usize
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_find_pka_index() {
        let int_done = 0b110101u32;

        let index = find_pka_index(int_done, 0);
        assert_eq!(index, 0);

        let index = find_pka_index(int_done, 1);
        assert_eq!(index, 2);

        let index = find_pka_index(int_done, 2);
        assert_eq!(index, 2);

        let index = find_pka_index(int_done, 3);
        assert_eq!(index, 4);
        let index = find_pka_index(int_done, 4);
        assert_eq!(index, 4);

        let index = find_pka_index(int_done, 5);
        assert_eq!(index, 5);

        for start_index in 6..15 {
            let index = find_pka_index(int_done, start_index);
            assert_eq!(index, 0);
        }
    }
}
