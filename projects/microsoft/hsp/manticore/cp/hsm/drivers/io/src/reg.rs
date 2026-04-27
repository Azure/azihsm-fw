// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield::BitMut;
use mcr_registers::ucd::inbound::core0_ib_iq;
use mcr_registers::ucd::inbound::core0_ib_iq::RegisterBlock as InboundQueueReg;
use mcr_registers::ucd::inbound::core_ib_cmn;
use mcr_registers::ucd::inbound::core_ib_cmn::ib_cmn_cq;
use mcr_registers::ucd::inbound::core_ib_cmn::ib_cmn_cq::RegisterBlock as InboundCompletionQueueReg;
use mcr_registers::ucd::inbound::core_ib_cmn::ib_cmn_dfl;
use mcr_registers::ucd::inbound::core_ib_cmn::ib_cmn_dfl::RegisterBlock as DestFreeListRegs;
use mcr_registers::ucd::inbound::core_ib_cmn::RegisterBlock as InboundCommonReg;
use mcr_registers::ucd::outbound::core_ob_cmn;
use mcr_registers::ucd::outbound::core_ob_cmn::ob_cmn_cq;
use mcr_registers::ucd::outbound::core_ob_cmn::ob_cmn_cq::RegisterBlock as OutBoundCompletionQueueReg;
use mcr_registers::ucd::outbound::core_ob_cmn::ob_cmn_osl;
use mcr_registers::ucd::outbound::core_ob_cmn::ob_cmn_osl::RegisterBlock as OutBoundSourceListRegs;
use mcr_registers::ucd::outbound::core_ob_cmn::RegisterBlock as OutboundCommonReg;
use mcr_types::IoChannelId;
use mcr_types::IoControllerId;

/// Controller register trait to get the controller, channel and queue register block based on
/// controller instance.
pub(crate) trait IoCntrlReg {
    /// Return the register block of a given Io Controller for an implemented object
    ///
    /// # Arguments
    ///
    /// * `cntrl_id` - Io Controller ID of type IoControllerId
    fn cntrl_reg(cntrl_id: IoControllerId) -> Self;
}

impl IoCntrlReg for core0_ib_iq::RegisterBlock {
    fn cntrl_reg(cntrl_id: IoControllerId) -> Self {
        const CORE0_INBOUND_QUEUE_REG_BASE_ADDR: u32 = 0xA1281000;
        const IO_CORE_REGISTER_STRIDE: u32 = 0x4000;

        let base_addr: u32 =
            CORE0_INBOUND_QUEUE_REG_BASE_ADDR + (IO_CORE_REGISTER_STRIDE * cntrl_id.0 as u32);
        unsafe { Self::new((base_addr) as *mut u32) }
    }
}

impl IoCntrlReg for core_ib_cmn::RegisterBlock {
    fn cntrl_reg(cntrl_id: IoControllerId) -> Self {
        const CORE0_INBOUND_BASE_ADDR: u32 = 0xA1280000;
        const IO_CORE_REGISTER_STRIDE: u32 = 0x4000;

        unsafe {
            Self::new(
                (CORE0_INBOUND_BASE_ADDR + (cntrl_id.0 as u32 * IO_CORE_REGISTER_STRIDE))
                    as *mut u32,
            )
        }
    }
}

impl IoCntrlReg for ib_cmn_cq::RegisterBlock {
    fn cntrl_reg(cntrl_id: IoControllerId) -> Self {
        const CORE0_INBOUND_COMPLETION_QUEUE_BASE_ADDR: u32 = 0xA1280120;
        const IO_CORE_REGISTER_STRIDE: u32 = 0x4000;

        unsafe {
            Self::new(
                (CORE0_INBOUND_COMPLETION_QUEUE_BASE_ADDR
                    + (cntrl_id.0 as u32 * IO_CORE_REGISTER_STRIDE)) as *mut u32,
            )
        }
    }
}

impl IoCntrlReg for ib_cmn_dfl::RegisterBlock {
    fn cntrl_reg(cntrl_id: IoControllerId) -> Self {
        const CORE0_INBOUND_DESTINATION_FREE_LIST_BASE_ADDR: u32 = 0xA1280060;
        const IO_CORE_REGISTER_STRIDE: u32 = 0x4000;

        unsafe {
            Self::new(
                (CORE0_INBOUND_DESTINATION_FREE_LIST_BASE_ADDR
                    + (cntrl_id.0 as u32 * IO_CORE_REGISTER_STRIDE)) as *mut u32,
            )
        }
    }
}

impl IoCntrlReg for core_ob_cmn::RegisterBlock {
    fn cntrl_reg(cntrl_id: IoControllerId) -> Self {
        const CORE0_OUTBOUND_BASE_ADDR: u32 = 0xA12C0000;
        const IO_CORE_REGISTER_STRIDE: u32 = 0x4000;

        unsafe {
            Self::new(
                (CORE0_OUTBOUND_BASE_ADDR + (cntrl_id.0 as u32 * IO_CORE_REGISTER_STRIDE))
                    as *mut u32,
            )
        }
    }
}

impl IoCntrlReg for ob_cmn_cq::RegisterBlock {
    fn cntrl_reg(cntrl_id: IoControllerId) -> Self {
        const CORE0_OUTBOUND_COMPLETION_QUEUE_BASE_ADDR: u32 = 0xA12C0120;
        const IO_CORE_REGISTER_STRIDE: u32 = 0x4000;

        unsafe {
            Self::new(
                (CORE0_OUTBOUND_COMPLETION_QUEUE_BASE_ADDR
                    + (cntrl_id.0 as u32 * IO_CORE_REGISTER_STRIDE)) as *mut u32,
            )
        }
    }
}

impl IoCntrlReg for ob_cmn_osl::RegisterBlock {
    fn cntrl_reg(cntrl_id: IoControllerId) -> Self {
        const CORE0_OUTBOUND_SOURCE_LIST_BASE_ADDR: u32 = 0xA12C0070;
        const IO_CORE_REGISTER_STRIDE: u32 = 0x4000;

        unsafe {
            Self::new(
                (CORE0_OUTBOUND_SOURCE_LIST_BASE_ADDR
                    + (cntrl_id.0 as u32 * IO_CORE_REGISTER_STRIDE)) as *mut u32,
            )
        }
    }
}

/// Inbound Completion queue register programming interface
pub(crate) enum InboundCompletionQueue {}

impl InboundCompletionQueue {
    /// Enable the Inbound Completion Queue
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    /// * `enable_irq` - Enable interrupt for the queue
    /// * `addr` - Base address of the queue
    /// * `len` - Length of the queue
    /// * `pi_addr` - Base address of the shadow producer index
    ///
    /// # Behavior
    ///
    /// Calling this interface while the queue is already enabled will result in undefined behavior.
    pub(crate) fn enable(
        cntrl: IoControllerId,
        channel: IoChannelId,
        enable_irq: bool,
        addr: u32,
        len: usize,
        pi_addr: u32,
    ) {
        let reg = InboundCompletionQueueReg::cntrl_reg(cntrl);
        let cmn_reg = InboundCommonReg::cntrl_reg(cntrl);
        let queue_reg = reg.at(channel.into());

        // Populate the queue address
        queue_reg.base_addr_lo().write(|_| (addr).into());
        queue_reg.base_addr_hi().write(|_| 0);

        // Populate shadow producer index address
        queue_reg
            .pi_shadow_base_addr_lo()
            .write(|_| (pi_addr).into());
        queue_reg.pi_shadow_base_addr_hi().write(|_| 0);

        // Set the produce and consumer index to zero to indicate
        // the queue is empty
        queue_reg.pi().write(|w| w.cmpltn_q_pi(0));
        queue_reg.ci().write(|w| w.cmpltn_q_ci(0));

        if enable_irq {
            // TODO: Fix Register XML for all interrupt enable registers
            let mut int_en = 0u32;
            int_en.set_bit(channel.into(), true);

            cmn_reg
                .interrupt_0_enable_set()
                .read_and_modify(|r, _| r | int_en);
        }

        // Configure the queue size, interface select to SOC, enable
        // shadow
        let queue_size = len.trailing_zeros() - 5;
        queue_reg.configuration_control().read_and_modify(|_, w| {
            w.cmpltn_q_size(queue_size)
                .cmpltn_q_ifc_slct(0)
                .cmpltn_q_shdw_en(true)
        });

        // Enable the queue
        queue_reg
            .configuration_control()
            .read_and_modify(|_, w| w.cmpltn_q_en(true));
    }

    /// Disable the Inbound Completion Queue
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    pub(crate) fn disable(cntrl: IoControllerId, channel: IoChannelId) {
        let reg = InboundCompletionQueueReg::cntrl_reg(cntrl);
        let queue_reg = reg.at(channel.into());

        queue_reg
            .configuration_control()
            .read_and_modify(|_, w| w.cmpltn_q_en(false));
    }

    /// Check if the queue is enabled
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    ///
    /// # Return
    ///
    /// * `true` - If the queue is enabled
    pub(crate) fn enabled(cntrl: IoControllerId, channel: IoChannelId) -> bool {
        let reg = InboundCompletionQueueReg::cntrl_reg(cntrl);
        let queue_reg = reg.at(channel.into());

        queue_reg.configuration_control().read().cmpltn_q_en()
    }
}

/// Destination Free List register programming interface
pub(crate) enum DestinationFreeList {}

impl DestinationFreeList {
    /// Enable destination free list for a given channel
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    /// * `addr` - Base address of the free list
    /// * `len` - Length of the free list
    /// * `entry_size` - Size of each entry in the free list
    ///
    /// # Behavior
    ///
    /// Calling this interface while the free list is already enabled will result in
    /// undefined behavior.
    pub(crate) fn enable(
        cntrl: IoControllerId,
        channel: IoChannelId,
        addr: u32,
        len: u32,
        entry_size: u32,
    ) {
        let reg = DestFreeListRegs::cntrl_reg(cntrl);
        let list_reg = reg.at(channel.into());

        // Set the interface select to indicate the free list existing in SOC
        // memory
        list_reg.configuration_1().write(|w| {
            w.ib_dest_free_list_bffr_ifc_slct(0)
                .ib_dest_free_list_list_ifc_slct(0)
        });

        // Set the base address of the free list in hardware
        list_reg.base_addr_lo().write(|_| (addr).into());
        list_reg.base_addr_hi().write(|_| 0);

        // Clear the empty status bit in status register
        list_reg
            .status()
            .read_and_modify(|_, w| w.ib_dest_free_list_empty(true));

        // Set the producer index value to indicate the free list is fully populated
        list_reg.pi().write(|w| w.ib_dest_free_list_pi(len - 1));
        // Set the consumer index to zero
        list_reg.ci().write(|w| w.ib_dest_free_list_ci(0));

        // Encoded value, refer the documentation of ib_dest_free_list_size field in
        // configuration_0 register
        let list_size = len.trailing_zeros() - 5;

        // Encoded value, refer the documentation of ib_dest_free_list_bffr_lngth field in
        // configuration_0 register
        let entry_size = entry_size >> 4;

        // Configure the free list size, entry size
        list_reg.configuration_0().read_and_modify(|_, w| {
            w.ib_dest_free_list_size(list_size)
                .ib_dest_free_list_bffr_lngth(entry_size)
        });

        // Enable the Free List
        list_reg
            .configuration_0()
            .read_and_modify(|_, w| w.ib_dest_free_list_en(true));
    }

    /// Disable the destination free list for a given channel
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    pub(crate) fn disable(cntrl: IoControllerId, channel: IoChannelId) {
        let reg = DestFreeListRegs::cntrl_reg(cntrl);
        let list_reg = reg.at(channel.into());

        // Disable the list
        list_reg
            .configuration_0()
            .read_and_modify(|_, w| w.ib_dest_free_list_en(false));
    }

    /// Check if the destination free list is enabled for a given channel
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    ///
    /// # Return
    ///
    /// * `true` - If the free list is enabled
    pub(crate) fn enabled(cntrl: IoControllerId, channel: IoChannelId) -> bool {
        let reg = DestFreeListRegs::cntrl_reg(cntrl);
        let list_reg = reg.at(channel.into());

        list_reg.configuration_0().read().ib_dest_free_list_en()
    }
}

/// Inbound Queue register programming interface
pub(crate) enum InboundQueue {}

impl InboundQueue {
    /// Release a ExEntry credit back to the inbound queue
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `queue_id` - INbound queue ID
    pub(crate) fn release_credit(cntrl: IoControllerId, queue_id: u8) {
        const NUMBER_OF_CREDITS_TO_RELEASE_PER_IO: u32 = 1;
        let reg = InboundQueueReg::cntrl_reg(cntrl);
        let queue_reg = reg.at(queue_id as usize);

        // release one credit back to the inbound queue
        queue_reg.credit_count().read_and_modify(|r, w| {
            let mut credit_count = r.iq_credit_count();
            credit_count += NUMBER_OF_CREDITS_TO_RELEASE_PER_IO;
            w.iq_credit_count(credit_count)
        });
    }
}

/// Outbound Completion Queue register programming interface
pub(crate) enum OutboundCompletionQueue {}

impl OutboundCompletionQueue {
    /// Enable the Outbound Completion Queue
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    /// * `enable_irq` - Enable interrupt for the queue
    /// * `addr` - Base address of the queue
    /// * `len` - Length of the queue
    /// * `pi_addr` - Base address of the shadow producer index
    ///
    /// # Behavior
    /// Calling this interface while the queue is already enabled will result in undefined behavior.
    pub(crate) fn enable(
        cntrl: IoControllerId,
        channel: IoChannelId,
        enable_irq: bool,
        addr: u32,
        len: usize,
        pi_addr: u32,
    ) {
        let reg = OutBoundCompletionQueueReg::cntrl_reg(cntrl);
        let cmn_reg = OutboundCommonReg::cntrl_reg(cntrl);
        let queue_reg = reg.at(channel.into());

        // Populate base address of the queue
        queue_reg.base_addr_lo().write(|_| (addr).into());
        queue_reg.base_addr_hi().write(|_| 0);

        // Populate shadow producer index address
        queue_reg
            .pi_shadow_base_addr_lo()
            .write(|_| (pi_addr).into());
        queue_reg.pi_shadow_base_addr_hi().write(|_| 0);

        // Set the hardware producer and consumer index to zero to indicate
        // the queue is empty
        queue_reg.pi().write(|w| w.ob_cmpltn_q_pi(0));
        queue_reg.ci().write(|w| w.ob_cmpltn_q_ci(0));

        if enable_irq {
            // TODO: Fix Register XML for all interrupt enable registers
            let mut int_en = 0u32;
            int_en.set_bit(channel.into(), true);

            cmn_reg
                .interrupt_0_enable_set()
                .read_and_modify(|r, _| r | int_en);
        }

        // Configure the queue size, interface select to SoC, set shadow enable
        let queue_size = len.trailing_zeros() - 5;
        queue_reg.configuration_control().read_and_modify(|_, w| {
            w.ob_cmpltn_q_size(queue_size)
                .ob_cmpltn_q_ifc_slct(0)
                .ob_cmpltn_q_shdw_en(true)
        });

        // Enable the Transmit Queue
        queue_reg
            .configuration_control()
            .read_and_modify(|_, w| w.ob_cmpltn_q_en(true));
    }

    /// Disable the Outbound Completion Queue
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    pub(crate) fn disable(cntrl: IoControllerId, channel: IoChannelId) {
        let reg = OutBoundCompletionQueueReg::cntrl_reg(cntrl);
        let queue_reg = reg.at(channel.into());

        //Disable the queue
        queue_reg
            .configuration_control()
            .write(|w| w.ob_cmpltn_q_en(false));
    }

    /// Check if the Outbound completion queue is enabled
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    ///
    /// # Return
    ///
    /// * `true` - If the queue is enabled
    pub(crate) fn enabled(cntrl: IoControllerId, channel: IoChannelId) -> bool {
        let reg = OutBoundCompletionQueueReg::cntrl_reg(cntrl);
        let queue_reg = reg.at(channel.into());

        queue_reg.configuration_control().read().ob_cmpltn_q_en()
    }
}

/// Outbound Source List register programming interface
pub(crate) enum OutboundSourceList {}

impl OutboundSourceList {
    /// Enable the Outbound Source List
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    /// * `addr` - Base address of the queue
    /// * `len` - Length of the queue
    ///
    /// # Behavior
    ///
    /// Calling this interface while the queue is already enabled will result in undefined behavior.
    pub(crate) fn enable(cntrl: IoControllerId, channel: IoChannelId, addr: u32, len: usize) {
        let reg = OutBoundSourceListRegs::cntrl_reg(cntrl);
        let list_reg = reg.at(channel.into());

        list_reg.base_addr_lo().write(|_| (addr).into());
        list_reg.base_addr_hi().write(|_| 0);

        list_reg.pi().write(|w| w.ob_src_list_pi(0));
        list_reg.ci().write(|w| w.ob_src_list_ci(0));

        // Encoded value: refer to ob_src_list_size field of configuration_0 register
        let queue_size = len.trailing_zeros() - 5;

        // Program Tx Free List size and its interface select
        list_reg
            .configuration_0()
            .read_and_modify(|_, w| w.ob_src_list_size(queue_size).ob_src_list_ifc_slct(0));

        // Enable the Tx Free List
        list_reg
            .configuration_0()
            .read_and_modify(|_, w| w.ob_src_list_en(true));
    }

    /// Disable the Outbound Source List
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    pub(crate) fn disable(cntrl: IoControllerId, channel: IoChannelId) {
        let reg = OutBoundSourceListRegs::cntrl_reg(cntrl);
        let list_reg = reg.at(channel.into());

        list_reg
            .configuration_0()
            .read_and_modify(|_, w| w.ob_src_list_en(false));
    }

    /// Check if the Outbound Source List is enabled
    ///
    /// # Arguments
    ///
    /// * `cntrl` - Io Controller ID of type IoControllerId
    /// * `channel` - Io Channel ID of type IoChannelId
    ///
    /// # Return
    ///
    /// * `true` - If the queue is enabled
    pub(crate) fn enabled(cntrl: IoControllerId, channel: IoChannelId) -> bool {
        let reg = OutBoundSourceListRegs::cntrl_reg(cntrl);
        let list_reg = reg.at(channel.into());

        list_reg.configuration_0().read().ob_src_list_en()
    }
}
