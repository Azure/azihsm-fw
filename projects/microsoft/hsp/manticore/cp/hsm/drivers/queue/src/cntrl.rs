// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield_struct::bitfield;
use mcr_error::McrResult;
use mcr_registers::ucd::cpu_pf_nvme_ctrl::RegisterBlock as ControllerReg;
use mcr_registers::ucd::gen_cmn;
use mcr_registers::ucd::ib_lg2phys as HostToDeviceSqDbMapReg;
use mcr_registers::ucd::inbound::core0_ib_iq as InboundGroupReg;
use mcr_registers::ucd::inbound::core_ib_cmn::RegisterBlock as InboundCommonReg;
use mcr_registers::ucd::ob_lg2phys as HostToDeviceCqDbMapReg;
use mcr_registers::ucd::outbound::core0_ob_oq as OutboundGroupReg;
use mcr_registers::ucd::outbound::core_ob_cmn::RegisterBlock as OutboundCommonReg;

use crate::reg::*;
use crate::*;

/// Device queue reset retry count
const DEVICE_QUEUE_RESET_RETRY_COUNT: usize = 1000;

/// Submission queue processing credit policy
enum DevSqCreditPolicy {
    /// Queue based credit policy
    QueueBasedCreditPolicy = 1,
}

impl From<DevSqCreditPolicy> for u32 {
    fn from(value: DevSqCreditPolicy) -> Self {
        value as Self
    }
}

/// Interupt1 Enable bitfield
#[bitfield(u32)]
#[derive(Default)]
struct Interrupt1Enable {
    /// Reserved
    #[bits(23)]
    rsvd1: u32,

    /// Host Doorbell access error interrupt enable
    host_doorbell_access_err: bool,

    #[bits(5)]
    rsvd2: u8,

    /// AXI Moinitor error interrupt enable
    axi_monitor_err_enable: bool,

    /// AXI Slave Parity error interrupt enable
    axi_slave_parity_err_enable: bool,

    /// AXI Master Parity error interrupt enable
    axi_master_parity_err_enable: bool,
}

/// Queue controller
#[derive(Clone)]
pub struct QueueController {
    /// Controller Identifier
    id: QueueCntrlId,

    /// Queue Controller register interface
    reg: ControllerReg,
}

impl QueueController {
    /// Returns controller state change bit mask
    pub fn event() -> Option<QueueCntrlEvent> {
        let mut state_change = 0u128;

        let reg = gen_cmn::RegisterBlock::block();

        // PF controller state change
        if reg
            .nvme_controller_configuration_en_field_updated_pf()
            .read()
            .hiu_nvme_cc_en_updtd_65()
        {
            state_change |= 1 << QueueCntrlId::Pf as u128;
            reg.nvme_controller_configuration_en_field_updated_pf()
                .write(|w| w.hiu_nvme_cc_en_updtd_65(true));
        }

        // VF 0 - 31 controller state change
        let val = reg
            .nvme_controller_configuration_en_field_updated_0_vf()
            .read();
        if val != 0 {
            state_change |= (val as u128) << QueueCntrlId::Vf0 as u128;
            reg.nvme_controller_configuration_en_field_updated_0_vf()
                .write(|_| val);
        }

        // VF 32 - 63 controller state change
        let val = reg
            .nvme_controller_configuration_en_field_updated_1_vf()
            .read();
        if val != 0 {
            state_change |= (val as u128) << QueueCntrlId::Vf32 as u128;
            reg.nvme_controller_configuration_en_field_updated_1_vf()
                .write(|_| val);
        }

        if state_change != 0 {
            return Some(QueueCntrlEvent::StateChange(state_change));
        }

        let mut nssr_req = 0u128;

        // PF NVMe subsystem reset pending
        if reg
            .nvme_subsystem_reset_received_pf()
            .read()
            .hiu_nvme_reset_rcvd_65()
        {
            nssr_req |= 1 << QueueCntrlId::Pf as u128;
            reg.nvme_subsystem_reset_received_pf()
                .write(|w| w.hiu_nvme_reset_rcvd_65(true));
        }

        // VF 0 - 31 NVMe subsystem reset pending
        let val = reg.nvme_subsystem_reset_received_0_vf().read();
        if val != 0 {
            nssr_req |= (val as u128) << QueueCntrlId::Vf0 as u128;
            reg.nvme_subsystem_reset_received_0_vf().write(|_| val);
        }

        // VF 32 - 63 controller state change
        let val = reg.nvme_subsystem_reset_received_1_vf().read();
        if val != 0 {
            nssr_req |= (val as u128) << QueueCntrlId::Vf32 as u128;
            reg.nvme_subsystem_reset_received_1_vf().write(|_| val);
        }

        if nssr_req != 0 {
            return Some(QueueCntrlEvent::NssrPending(nssr_req));
        }

        None
    }

    /// Queue controller global pause by disabling all interrupts.
    pub fn pause() {
        let reg = gen_cmn::RegisterBlock::block();

        // Disable the cc_en and NSSR for PF and all VFs
        reg.common_ucd_interrupt_0_enable().write(|_| 0x0000);
    }

    /// Queue controller global resume by enabling all interrupts.
    pub fn resume() {
        let reg = gen_cmn::RegisterBlock::block();

        // Enable the cc_en and NSSR for PF and all VFs
        reg.common_ucd_interrupt_0_enable().write(|_| 0x3300);
    }

    /// Queue Controller global init
    pub fn global_init() {
        let reg = gen_cmn::RegisterBlock::block();

        // Enable the cc_en and NSSR for PF and all VFs
        reg.common_ucd_interrupt_0_enable().write(|_| 0x3300);

        // TODO: Global Error interrupts enable
        reg.common_ucd_interrupt_1_enable().write(|_| {
            Interrupt1Enable::new()
                .with_axi_slave_parity_err_enable(true)
                .with_axi_master_parity_err_enable(true)
                .into()
        });
    }

    ///
    /// Helper function to calculate the logical address of doorbell address mapping
    ///
    /// # Arguments
    ///
    /// * `id` - Queue controller ID
    /// * `host_queue_id` - Host Submission or Completion Queue ID
    ///
    /// # Returns
    ///
    /// * `u32` - Logical doorbell address mapping
    fn doorbell_addr(&self, host_queue: u32) -> u32 {
        const NVME_BAR23_ADDR_PF: u32 = 0xA130_0000;
        const NVME_BAR23_ADDR_VF: u32 = 0xA140_1000;

        if self.id == QueueCntrlId::Pf {
            ((NVME_BAR23_ADDR_PF & 0x3fc000u32) >> 2) + (8 * host_queue)
        } else {
            (((NVME_BAR23_ADDR_VF + (0x4000u32 * (PcieFunction::from(self.id).0 as u32)))
                & 0x3fc000u32)
                >> 2)
                + (8 * host_queue)
        }
    }

    /// Queue controller init
    fn init(&self) {
        const CNTRL_CAP_WIGHTED_RR_ARBITRATION: u32 = 0x1;
        const CNTRL_CAP_MAX_SQ_CQ_SIZE: u32 = 1023;
        const CNTRL_CAP_CCEN_TIMEOUT_IN_500MS_UNIT: u32 = 20;

        self.reg.controller_capabilities_lo().write(|w| {
            w.cap_lo_ams(CNTRL_CAP_WIGHTED_RR_ARBITRATION)
                .cap_lo_cap_lo_cqr(true)
                .cap_lo_mqes(CNTRL_CAP_MAX_SQ_CQ_SIZE)
                .cap_lo_to(CNTRL_CAP_CCEN_TIMEOUT_IN_500MS_UNIT)
        });
        self.reg
            .controller_capabilities_hi()
            .read_and_modify(|_, w| w.cap_hi_nssrs(true));
        self.reg
            .controller_status()
            .read_and_modify(|_, w| w.csts_rdy(false));
    }
}

impl QueueControllerTrait for QueueController {
    /// Create an instance of Queue Controller with initialization
    fn from_id_with_init(id: QueueCntrlId) -> Self {
        let cntrl = Self {
            id,
            reg: ControllerReg::ctrl_reg(id),
        };

        // Initialize a new controller instance by letting go of previous context
        cntrl.init();

        // Set the firmware capabilities
        cntrl
            .reg
            .reserved_1()
            .write(|_| FwCapabilities::default().into());

        cntrl
    }

    /// Create an instance of Queue Controller without initialization
    fn from_id(id: QueueCntrlId) -> Self {
        let cntrl = Self {
            id,
            reg: ControllerReg::ctrl_reg(id),
        };

        // Set the firmware capabilities
        cntrl
            .reg
            .reserved_1()
            .write(|_| FwCapabilities::default().into());

        cntrl
    }

    /// Get the Queue Controller Id
    fn id(&self) -> QueueCntrlId {
        self.id
    }

    /// Check if the controller is enabled
    fn enabled(&self) -> bool {
        self.reg.controller_configuration().read().cc_en()
    }

    /// Check if the controller is ready
    fn ready(&self) -> bool {
        self.reg.controller_status().read().csts_rdy()
    }

    /// Enable the controller
    fn enable(&self) {
        self.reg
            .controller_status()
            .read_and_modify(|_, w| w.csts_rdy(true).csts_cfs(false));
    }

    /// Disable the controller
    fn disable(&self) {
        self.reg
            .controller_status()
            .read_and_modify(|_, w| w.csts_rdy(false).csts_cfs(false));
    }

    /// Clear the Enable status of the controller
    fn clear_enable(&self) {
        self.reg
            .controller_configuration()
            .read_and_modify(|_, w| w.cc_en(false));
    }

    /// Set the Enable status of the controller
    fn set_enable(&self) {
        self.reg
            .controller_configuration()
            .read_and_modify(|_, w| w.cc_en(true));
    }

    /// Reset the controller
    fn reset(&self) {
        let cmn_reg = mcr_registers::ucd::gen_cmn::RegisterBlock::block();
        match self.id {
            QueueCntrlId::Pf => {
                cmn_reg
                    .nvme_register_set_reset_pf()
                    .read_and_modify(|_, w| w.hiu_nvme_regstr_set_rst_65(true));
                cmn_reg
                    .nvme_register_set_reset_pf()
                    .read_and_modify(|_, w| w.hiu_nvme_regstr_set_rst_65(false));
            }
            ctrl_id if ctrl_id >= QueueCntrlId::Vf0 && ctrl_id <= QueueCntrlId::Vf31 => {
                let mask = 1 << (ctrl_id as u32 - 1);
                cmn_reg
                    .nvme_register_set_reset_0_vf()
                    .read_and_modify(|r, _| r | mask);
                cmn_reg
                    .nvme_register_set_reset_0_vf()
                    .read_and_modify(|r, _| r & !mask);
            }
            ctrl_id if ctrl_id >= QueueCntrlId::Vf32 && ctrl_id <= QueueCntrlId::Vf63 => {
                let mask = 1 << (ctrl_id as u32 - 33);
                cmn_reg
                    .nvme_register_set_reset_1_vf()
                    .read_and_modify(|r, _| r | mask);
                cmn_reg
                    .nvme_register_set_reset_1_vf()
                    .read_and_modify(|r, _| r & !mask);
            }
            _ => (),
        }

        self.init()
    }

    /// Create admin submission queue
    fn create_asq(&self, dev_sq: DevSqId, host_sq: HostSqId) -> McrResult<()> {
        let mem = QueueMem {
            addr: MemoryAddr {
                lo: self
                    .reg
                    .admin_submission_queue_base_address_lo()
                    .read()
                    .into(),
                hi: self.reg.admin_submission_queue_base_address_hi().read(),
            },
            len: self.reg.admin_queue_attributes().read().aqa_asqs(),
        };
        self.create_sq(dev_sq, host_sq, mem)?;

        Ok(())
    }

    /// Create admin completion queue
    fn create_acq(&self, dev_cq: DevCqId, host_cq: HostCqId) -> McrResult<()> {
        const ADMIN_CQ_IRQ: u16 = 0;

        let mem = QueueMem {
            addr: MemoryAddr {
                lo: self
                    .reg
                    .admin_completion_queue_base_address_lo()
                    .read()
                    .into(),
                hi: self.reg.admin_completion_queue_base_address_hi().read(),
            },
            len: self.reg.admin_queue_attributes().read().aqa_acqs(),
        };

        self.create_cq(dev_cq, host_cq, mem, Some(ADMIN_CQ_IRQ))?;

        Ok(())
    }

    /// Create Io submission queue
    fn create_sq(&self, dev_sq: DevSqId, host_sq: HostSqId, mem: QueueMem) -> McrResult<()> {
        let (group, channel) = host_sq.queue_group_and_channel(dev_sq);
        let grp_reg = InboundGroupReg::RegisterBlock::group_reg(group);
        let queue_reg = grp_reg.at(dev_sq.into());
        let binding = HostToDeviceSqDbMapReg::RegisterBlock::group_reg(group);
        let db_map_reg = binding.at(dev_sq.into());

        // Reset the queue before enabling, iq_rst() is a self clearing bit after reset is
        // complete, poll for the reset to complete with a timeout
        queue_reg.configuration_0().write(|w| w.iq_rst(true));
        let mut retry_cnt = DEVICE_QUEUE_RESET_RETRY_COUNT;
        while retry_cnt > 0 {
            if !queue_reg.configuration_0().read().iq_rst() {
                break;
            }
            retry_cnt -= 1;
        }
        if retry_cnt == 0 {
            Err(QueueControllerErr::DeviceSqResetRequestTimeout)?;
        }

        queue_reg.configuration_1().read_and_modify(|_, w| {
            w.iq_ifc_slct(MemoryLocation::from(self.id) as u32)
                .iq_host_logical_id(host_sq.into())
        });

        queue_reg.base_address_low().write(|_| mem.addr.lo.into());
        queue_reg.base_address_high().write(|_| mem.addr.hi);

        // Program the Submission queue Doorbell map register
        db_map_reg.logical_to_physical_assignment().write(|w| {
            w.logical_addr_of_iq_producer_indx(self.doorbell_addr(host_sq.into()))
                .logical_addr_invalid(false)
        });

        queue_reg
            .credit_count()
            .write(|w| w.iq_credit_count(host_sq.credit()));

        queue_reg.configuration_0().read_and_modify(|_, w| {
            w.iq_nm_elmnts(mem.len)
                .iq_credit_policy_en(DevSqCreditPolicy::QueueBasedCreditPolicy.into())
                .iq_free_list_slct(channel.into())
                .iq_priority(channel.into())
        });

        // Finally enable the queue once all the queue configuration is programmed
        queue_reg
            .configuration_0()
            .read_and_modify(|_, w| w.iq_en(true));

        Ok(())
    }

    /// Delete Io submission queue
    fn delete_sq(&self, dev_sq: DevSqId, host_sq: HostSqId) {
        let (group, _) = host_sq.queue_group_and_channel(dev_sq);
        let grp_reg = InboundGroupReg::RegisterBlock::group_reg(group);
        let queue_reg = grp_reg.at(dev_sq.into());
        let mapping_regs = HostToDeviceSqDbMapReg::RegisterBlock::group_reg(group);
        let mapping_reg = mapping_regs.at(dev_sq.into());

        //TODO: Add a check to make sure dev_sq to host_sq pair is valid

        mapping_reg
            .logical_to_physical_assignment()
            .write(|w| w.logical_addr_invalid(true));

        queue_reg.configuration_0().write(|w| w.iq_en(false));
    }

    /// Create Io completion queue
    fn create_cq(
        &self,
        dev_cq: DevCqId,
        host_cq: HostCqId,
        mem: QueueMem,
        irq: Option<u16>,
    ) -> McrResult<()> {
        let (group, _) = host_cq.queue_group_and_channel(dev_cq);
        let grp_reg = OutboundGroupReg::RegisterBlock::group_reg(group);
        let queue_reg = grp_reg.at(dev_cq.into());
        let mapping_regs = HostToDeviceCqDbMapReg::RegisterBlock::group_reg(group);
        let mapping_reg = mapping_regs.at(dev_cq.into());

        queue_reg.configuration_0().write(|w| w.oq_rst(true));
        let mut retry_cnt = DEVICE_QUEUE_RESET_RETRY_COUNT;
        while retry_cnt > 0 {
            if !queue_reg.configuration_0().read().oq_rst() {
                break;
            }
            retry_cnt -= 1;
        }
        if retry_cnt == 0 {
            Err(QueueControllerErr::DeviceCqResetRequestTimeout)?;
        }

        queue_reg
            .configuration_1()
            .read_and_modify(|_, w| w.oq_ifc_slct(MemoryLocation::from(self.id) as u32));

        mapping_reg
            .lgc2phys_reg_oq_logical_to_physical_assignment()
            .write(|w| {
                w.logical_addr_of_oq_consumer_indx(self.doorbell_addr(host_cq.into()) + 4)
                    .logical_addr_invalid(false)
            });

        queue_reg.base_addr_lo().write(|_| mem.addr.lo.into());
        queue_reg.base_addr_hi().write(|_| mem.addr.hi);

        // Enable MSI interrupts if it is requested to be enabled
        if let Some(msix_vec) = irq {
            // todo()! implement it part of the type?
            let msix_vec_sel = if group == QueueGroup::Group0 {
                msix_vec
            } else {
                // Second Queue group controller MSI vectors are wired from 0x10 to 0x1F
                msix_vec - 0x10
            };
            queue_reg
                .interrupt_configuration_2()
                .read_and_modify(|_, w| {
                    w.oq_msi_x_vctr_slct(msix_vec_sel.into())
                        .oq_en_gen_msi_x(true)
                        .oq_en_extrnl_tmr_rstrt(true)
                        .oq_msi_x_tbl_slct(PcieFunction::from(self.id).0 as u32)
                });
        } else {
            queue_reg
                .interrupt_configuration_2()
                .read_and_modify(|_, w| w.oq_en_gen_msi_x(false));
        }

        queue_reg.configuration_0().read_and_modify(|_, w| {
            w.oq_phs_bit_en(true)
                .oq_elmnt_sz(0)
                .oq_nm_elmnts(mem.len)
                .oq_iq_ci_updt_en(true)
                .oq_iq_id_updt_en(true)
        });

        // Finally enable the queue once all the queue configuration is programmed
        queue_reg
            .configuration_0()
            .read_and_modify(|_, w| w.oq_en(true));

        Ok(())
    }

    /// Create Io completion queue
    fn delete_cq(&self, dev_cq: DevCqId, host_cq: HostCqId) {
        let (group, _) = host_cq.queue_group_and_channel(dev_cq);
        let grp_reg = OutboundGroupReg::RegisterBlock::group_reg(group);
        let queue_reg = grp_reg.at(dev_cq.into());
        let binding = HostToDeviceCqDbMapReg::RegisterBlock::group_reg(group);
        let db_map_reg = binding.at(dev_cq.into());

        //TODO: Add a check to make sure dev_cq to host_cq pair is valid

        db_map_reg
            .lgc2phys_reg_oq_logical_to_physical_assignment()
            .read_and_modify(|_, w| w.logical_addr_invalid(true));

        queue_reg.configuration_0().write(|w| w.oq_en(false));
    }

    /// Set Controller Fatal Status
    fn set_cfs(&self) {
        self.reg
            .controller_status()
            .read_and_modify(|_, w| w.csts_cfs(true));
    }

    /// Get the controller host register programming information
    fn host_register_info(&self) -> ControllerLmInfo {
        ControllerLmInfo {
            version: self.reg.version().read().into(),
            ivms: self.reg.interrupt_mask_clear().read().into(),
            aq_attr: self.reg.admin_queue_attributes().read().into(),
            asq_addr_lo: self
                .reg
                .admin_submission_queue_base_address_lo()
                .read()
                .into(),
            asq_addr_hi: self.reg.admin_submission_queue_base_address_hi().read(),
            acq_addr_lo: self
                .reg
                .admin_completion_queue_base_address_lo()
                .read()
                .into(),
            acq_addr_hi: self.reg.admin_completion_queue_base_address_hi().read(),
            memory_buffer_location: self.reg.controller_memory_buffer_location().read().into(),
            memory_buffer_size: self.reg.controller_memory_buffer_size().read().into(),
            ..Default::default()
        }
    }

    /// Restore the controller host register programming information
    fn restore_host_register_info(&self, info: &ControllerLmInfo) {
        self.reg.version().write(|w| w.vs_mjr(info.version));
        self.reg.version().write(|w| w.vs_mnr(info.version));
        self.reg
            .interrupt_mask_clear()
            .write(|w| w.intmc_ivmc(info.ivms));
        self.reg
            .admin_queue_attributes()
            .write(|w| w.aqa_asqs(info.aq_attr).aqa_acqs(info.aq_attr));
        self.reg
            .admin_submission_queue_base_address_lo()
            .write(|w| w.asqb_lo(info.asq_addr_lo));
        self.reg
            .admin_submission_queue_base_address_hi()
            .write(|_| info.asq_addr_hi);
        self.reg
            .admin_completion_queue_base_address_lo()
            .write(|w| w.acqb_lo(info.acq_addr_lo));
        self.reg
            .admin_completion_queue_base_address_hi()
            .write(|_| info.acq_addr_hi);
        self.reg
            .controller_memory_buffer_location()
            .write(|_| info.memory_buffer_location.into());
        self.reg
            .controller_memory_buffer_size()
            .write(|_| info.memory_buffer_size.into());
    }

    /// Get the submission queue information programmed in queue controller
    fn sq_info(&self, dev_sq: DevSqId, host_sq: HostSqId) -> LmSqInfo {
        let (group, _) = host_sq.queue_group_and_channel(dev_sq);
        let grp_reg = InboundGroupReg::RegisterBlock::group_reg(group);
        let queue_reg = grp_reg.at(dev_sq.into());

        LmSqInfo {
            id: host_sq.into(),
            len: queue_reg.configuration_0().read().iq_nm_elmnts() as u16,
            cq_id: host_sq.into(),
            rsvd: 0,
            addr: MemoryAddr {
                lo: queue_reg.base_address_low().read().into(),
                hi: queue_reg.base_address_high().read(),
            },
            head: queue_reg.ci().read().iq_cnsmr_indx() as u16,
            tail: queue_reg.pi().read().iq_prdcr_indx() as u16,
        }
    }

    /// Restore the submission queue information programmed in queue controller
    fn restore_sq_info(&self, dev_sq: DevSqId, info: &LmSqInfo) -> McrResult<()> {
        let host_sq = HostSqId(info.id);
        let (group, channel) = host_sq.queue_group_and_channel(dev_sq);
        let grp_reg = InboundGroupReg::RegisterBlock::group_reg(group);
        let queue_reg = grp_reg.at(dev_sq.into());
        let binding = HostToDeviceSqDbMapReg::RegisterBlock::group_reg(group);
        let db_map_reg = binding.at(dev_sq.into());
        let ib_cmn_reg = InboundCommonReg::group_reg(group);

        // Reset the queue before enabling, iq_rst() is a self clearing bit after reset is
        // complete, poll for the reset to complete with a timeout
        queue_reg.configuration_0().write(|w| w.iq_rst(true));
        let mut retry_cnt = DEVICE_QUEUE_RESET_RETRY_COUNT;
        while retry_cnt > 0 {
            if !queue_reg.configuration_0().read().iq_rst() {
                break;
            }
            retry_cnt -= 1;
        }
        if retry_cnt == 0 {
            Err(QueueControllerErr::DeviceSqResetRequestTimeout)?;
        }

        queue_reg.configuration_1().read_and_modify(|_, w| {
            w.iq_ifc_slct(MemoryLocation::from(self.id) as u32)
                .iq_host_logical_id(host_sq.into())
        });

        queue_reg.base_address_low().write(|_| info.addr.lo.into());
        queue_reg.base_address_high().write(|_| info.addr.hi);

        // Program the Submission queue Doorbell map register
        db_map_reg.logical_to_physical_assignment().write(|w| {
            w.logical_addr_of_iq_producer_indx(self.doorbell_addr(host_sq.into()))
                .logical_addr_invalid(false)
        });

        queue_reg
            .credit_count()
            .write(|w| w.iq_credit_count(host_sq.credit()));

        queue_reg.configuration_0().read_and_modify(|_, w| {
            w.iq_nm_elmnts(info.len as u32)
                .iq_credit_policy_en(DevSqCreditPolicy::QueueBasedCreditPolicy.into())
                .iq_free_list_slct(channel.into())
                .iq_priority(channel.into())
        });

        // Restore the head and tail pointers
        ib_cmn_reg
            .miscellaneous_control()
            .read_and_modify(|_, w| w.dsbl_dbell_compliance_chk_err(7u32));
        queue_reg.ci().write(|w| w.iq_cnsmr_indx(info.head as u32));
        queue_reg.pi().write(|w| w.iq_prdcr_indx(info.tail as u32));
        ib_cmn_reg
            .miscellaneous_control()
            .read_and_modify(|_, w| w.dsbl_dbell_compliance_chk_err(0u32));

        // Skip Enabling the queues we are only restoring the information

        Ok(())
    }

    /// Get the completion queue information programmed in queue controller
    fn cq_info(&self, dev_cq: DevCqId, host_cq: HostCqId) -> LmCqInfo {
        let (group, _) = host_cq.queue_group_and_channel(dev_cq);
        let grp_reg = OutboundGroupReg::RegisterBlock::group_reg(group);
        let queue_reg = grp_reg.at(dev_cq.into());

        // MSI-x vector for queue controller 2 are programmed with value that is adjusted to 0x10
        let msi_vec_sel = queue_reg
            .interrupt_configuration_2()
            .read()
            .oq_msi_x_vctr_slct() as u16;

        LmCqInfo {
            id: host_cq.into(),
            len: queue_reg.configuration_0().read().oq_nm_elmnts() as u16,
            addr: MemoryAddr {
                lo: queue_reg.base_addr_lo().read().into(),
                hi: queue_reg.base_addr_hi().read(),
            },
            attr: LmCqAttributes::new()
                .with_ien(
                    queue_reg
                        .interrupt_configuration_2()
                        .read()
                        .oq_en_gen_msi_x(),
                )
                .with_ph(queue_reg.pi().read().oq_phase()),
            iv: msi_vec_sel,
            head: queue_reg.ci().read().oq_cnsmr_indx() as u16,
            tail: queue_reg.pi().read().oq_prdcr_indx() as u16,
        }
    }

    /// Restore the completion queue information programmed in queue controller
    fn restore_cq_info(&self, dev_cq: DevCqId, info: &LmCqInfo) -> McrResult<()> {
        let host_cq = HostCqId(info.id);
        let (group, _) = host_cq.queue_group_and_channel(dev_cq);
        let grp_reg = OutboundGroupReg::RegisterBlock::group_reg(group);
        let queue_reg = grp_reg.at(dev_cq.into());
        let mapping_regs = HostToDeviceCqDbMapReg::RegisterBlock::group_reg(group);
        let mapping_reg = mapping_regs.at(dev_cq.into());
        let ob_cmn_reg = OutboundCommonReg::group_reg(group);

        queue_reg.configuration_0().write(|w| w.oq_rst(true));

        let mut retry_cnt = DEVICE_QUEUE_RESET_RETRY_COUNT;
        while retry_cnt > 0 {
            if !queue_reg.configuration_0().read().oq_rst() {
                break;
            }
            retry_cnt -= 1;
        }
        if retry_cnt == 0 {
            Err(QueueControllerErr::DeviceCqResetRequestTimeout)?;
        }

        queue_reg
            .configuration_1()
            .read_and_modify(|_, w| w.oq_ifc_slct(MemoryLocation::from(self.id) as u32));

        mapping_reg
            .lgc2phys_reg_oq_logical_to_physical_assignment()
            .write(|w| {
                w.logical_addr_of_oq_consumer_indx(self.doorbell_addr(host_cq.into()) + 4)
                    .logical_addr_invalid(false)
            });

        queue_reg.base_addr_lo().write(|_| info.addr.lo.into());
        queue_reg.base_addr_hi().write(|_| info.addr.hi);

        // Enable MSI interrupts if it is requested to be enabled
        if info.attr.ien() {
            queue_reg
                .interrupt_configuration_2()
                .read_and_modify(|_, w| {
                    w.oq_msi_x_vctr_slct(info.iv.into())
                        .oq_en_gen_msi_x(true)
                        .oq_en_extrnl_tmr_rstrt(true)
                        .oq_msi_x_tbl_slct(PcieFunction::from(self.id).0 as u32)
                });
        } else {
            queue_reg
                .interrupt_configuration_2()
                .read_and_modify(|_, w| w.oq_en_gen_msi_x(false));
        }

        queue_reg.configuration_0().read_and_modify(|_, w| {
            w.oq_phs_bit_en(true)
                .oq_elmnt_sz(0)
                .oq_nm_elmnts(info.len.into())
                .oq_iq_ci_updt_en(true)
                .oq_iq_id_updt_en(true)
        });

        // Restore the head and tail pointers
        ob_cmn_reg
            .miscellaneous_control()
            .read_and_modify(|_, w| w.dsbl_dbell_compliance_chk_err(7u32));
        queue_reg.ci().write(|w| w.oq_cnsmr_indx(info.head as u32));
        queue_reg.pi().write(|w| w.oq_prdcr_indx(info.tail as u32));
        ob_cmn_reg
            .miscellaneous_control()
            .read_and_modify(|_, w| w.dsbl_dbell_compliance_chk_err(0u32));

        // Restore the phase bit
        // Deafult value of phase bit in pi reg is 1, if info.attr.ph() is false
        // then we need to write 1 to clear the phase bit to 0
        if !info.attr.ph() {
            queue_reg.pi().read_and_modify(|_, w| w.oq_phase(true));
        }

        // Finally enable the queue once all the queue configuration is programmed
        queue_reg
            .configuration_0()
            .read_and_modify(|_, w| w.oq_en(true));

        Ok(())
    }

    /// Enable the device submission queue
    fn enable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId) {
        let (group, _) = host_sq.queue_group_and_channel(dev_sq);
        let grp_reg = InboundGroupReg::RegisterBlock::group_reg(group);
        let queue_reg = grp_reg.at(dev_sq.into());

        queue_reg
            .configuration_0()
            .read_and_modify(|_, w| w.iq_en(true));
    }

    /// Disable the device submission queue
    fn disable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId) {
        let (group, _) = host_sq.queue_group_and_channel(dev_sq);
        let grp_reg = InboundGroupReg::RegisterBlock::group_reg(group);
        let queue_reg = grp_reg.at(dev_sq.into());

        queue_reg
            .configuration_0()
            .read_and_modify(|_, w| w.iq_en(false));
    }
}
