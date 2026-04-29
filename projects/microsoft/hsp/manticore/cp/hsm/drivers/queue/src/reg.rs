// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_registers::ucd::cpu_pf_nvme_ctrl;
use mcr_registers::ucd::ib_lg2phys;
use mcr_registers::ucd::inbound::core0_ib_iq;
use mcr_registers::ucd::inbound::core_ib_cmn;
use mcr_registers::ucd::ob_lg2phys;
use mcr_registers::ucd::outbound::core0_ob_oq;
use mcr_registers::ucd::outbound::core_ob_cmn;

use crate::*;

/// Function controller register access trait
pub(crate) trait QueueCtrlReg {
    /// Returns the register block for the specified Queue controller
    ///
    /// # Arguments
    ///
    /// * `ctrl_id` - Queue controller Id
    fn ctrl_reg(ctrl_id: QueueCntrlId) -> Self;
}

/// Queue group register access trait
pub(crate) trait QueueGroupReg {
    /// Returns the register block for the specified queue group
    ///
    /// # Arguments
    ///
    /// * `group` - Queue group number
    fn group_reg(group: QueueGroup) -> Self;
}

impl QueueCtrlReg for cpu_pf_nvme_ctrl::RegisterBlock {
    fn ctrl_reg(ctrl_id: QueueCntrlId) -> Self {
        const PF_CONTROLLER_REG_BASE_ADDR: u32 = 0xA1100000;
        const VF_CONTROLLER_REG_BASE_ADDR: u32 = 0xA1108000;
        const FUNCTION_CONTROLLER_REG_STRIDE: u32 = 0x400;

        let base_addr: u32 = if ctrl_id == QueueCntrlId::Pf {
            PF_CONTROLLER_REG_BASE_ADDR
        } else {
            VF_CONTROLLER_REG_BASE_ADDR + (FUNCTION_CONTROLLER_REG_STRIDE * (ctrl_id as u32 - 1))
        };

        unsafe { Self::new((base_addr) as *mut u32) }
    }
}

impl QueueGroupReg for core0_ib_iq::RegisterBlock {
    fn group_reg(group: QueueGroup) -> Self {
        const QUEUE_CONTROLLER_INBOUND_QUEUE_REG_BASE_ADDR: u32 = 0xA1281000;
        const QUEUE_CONTROLLER_REGISTER_STRIDE: u32 = 0x4000;

        let base_addr: u32 = QUEUE_CONTROLLER_INBOUND_QUEUE_REG_BASE_ADDR
            + (QUEUE_CONTROLLER_REGISTER_STRIDE * group as u32);
        unsafe { Self::new((base_addr) as *mut u32) }
    }
}

impl QueueGroupReg for core_ib_cmn::RegisterBlock {
    fn group_reg(group: QueueGroup) -> Self {
        const CORE0_INBOUND_BASE_ADDR: u32 = 0xA1280000;
        const IO_CORE_REGISTER_STRIDE: u32 = 0x4000;

        unsafe {
            Self::new(
                (CORE0_INBOUND_BASE_ADDR + (group as u32 * IO_CORE_REGISTER_STRIDE)) as *mut u32,
            )
        }
    }
}

impl QueueGroupReg for core0_ob_oq::RegisterBlock {
    fn group_reg(group: QueueGroup) -> Self {
        const QUEUE_CONTROLLER_OUTBOUND_QUEUE_REG_BASE_ADDR: u32 = 0xA12C1000;
        const QUEUE_CONTROLLER_REGISTER_STRIDE: u32 = 0x4000;

        let base_addr: u32 = QUEUE_CONTROLLER_OUTBOUND_QUEUE_REG_BASE_ADDR
            + (QUEUE_CONTROLLER_REGISTER_STRIDE * group as u32);
        unsafe { Self::new((base_addr) as *mut u32) }
    }
}

impl QueueGroupReg for core_ob_cmn::RegisterBlock {
    fn group_reg(group: QueueGroup) -> Self {
        const CORE0_OUTBOUND_BASE_ADDR: u32 = 0xA12C0000;
        const IO_CORE_REGISTER_STRIDE: u32 = 0x4000;

        unsafe {
            Self::new(
                (CORE0_OUTBOUND_BASE_ADDR + (group as u32 * IO_CORE_REGISTER_STRIDE)) as *mut u32,
            )
        }
    }
}

impl QueueGroupReg for ib_lg2phys::RegisterBlock {
    fn group_reg(group: QueueGroup) -> Self {
        const INBOUND_LOGICAL_TO_PHYSICAL_BASE_ADDR: u32 = 0xA1201000;
        const NUM_QUEUES_PER_GROUP: u32 = 132;
        const LOG_2_PHYS_REG_SIZE: u32 = 4;

        let base_addr: u32 = INBOUND_LOGICAL_TO_PHYSICAL_BASE_ADDR
            + (NUM_QUEUES_PER_GROUP * LOG_2_PHYS_REG_SIZE * group as u32);
        unsafe { Self::new((base_addr) as *mut u32) }
    }
}

impl QueueGroupReg for ob_lg2phys::RegisterBlock {
    fn group_reg(group: QueueGroup) -> Self {
        const OUTBOUND_LOGICAL_TO_PHYSICAL_BASE_ADDR: u32 = 0xA1202000;
        const NUM_QUEUES_PER_GROUP: u32 = 132;
        const LOG_2_PHYS_REG_SIZE: u32 = 4;

        let base_addr: u32 = OUTBOUND_LOGICAL_TO_PHYSICAL_BASE_ADDR
            + (NUM_QUEUES_PER_GROUP * LOG_2_PHYS_REG_SIZE * group as u32);
        unsafe { Self::new((base_addr) as *mut u32) }
    }
}
