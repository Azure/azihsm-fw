// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use core::cell::RefCell;
use zerocopy::IntoBytes;
use zeroize::Zeroize;

use mcr_error::McrResult;
use mcr_queue_controller::*;
use mcr_types::*;

use super::admin_queue::AdminQueue;
use super::*;
use crate::error::AdminErr;

/// PCIe Function
#[derive(Clone)]
pub(crate) struct Function<Q: QueueControllerTrait> {
    rimpl: Rc<RefCell<FunctionImpl<Q>>>,
}

impl<Q: QueueControllerTrait> FunctionTrait for Function<Q> {
    /// Query the controller state change
    fn query_state_change(&self) -> CntrlStateChangeAction {
        self.rimpl.borrow().query_state_change()
    }

    /// Get the Queue controller Id corresponding to this function
    fn cntrl_id(&self) -> QueueCntrlId {
        self.rimpl.borrow().cntrl_id()
    }

    /// Enable Pcie function
    fn enable(&self) -> McrResult<()> {
        self.rimpl.borrow_mut().enable()
    }

    /// Disable Pcie function
    fn disable(&self) {
        self.rimpl.borrow_mut().disable()
    }

    /// Clear the enable status of the function
    fn clear_enable(&self) {
        self.rimpl.borrow().cntrl.clear_enable();
    }

    /// Reset the function
    fn reset(&self) {
        self.rimpl.borrow_mut().reset()
    }

    /// Check if the function is ready
    fn ready(&self) -> bool {
        self.rimpl.borrow().ready()
    }

    /// Check if the function is enabled
    fn enabled(&self) -> bool {
        self.rimpl.borrow().cntrl.enabled()
    }

    /// set the allocated resource count to this function
    fn set_res_cnt(&self, cnt: u32) {
        self.rimpl.borrow_mut().set_res_cnt(cnt)
    }

    /// Get the resource counts allocated to this function
    fn res_cnt(&self) -> u32 {
        self.rimpl.borrow().res_cnt()
    }

    /// Get the resource allocation mask
    fn res_mask(&self) -> [u8; 16] {
        self.rimpl.borrow().res_mask()
    }

    /// Create device completion queue from host completion queue Id
    fn create_cq(
        &self,
        host_cq: HostCqId,
        mem: QueueMem,
        irq: Option<u16>,
    ) -> Result<(), AdminErr> {
        self.rimpl.borrow_mut().create_cq(host_cq, mem, irq)
    }

    /// Delete a device completion queue from host completion queue Id
    fn delete_cq(&self, host_cq: HostCqId) -> Result<(), AdminErr> {
        self.rimpl.borrow_mut().delete_cq(host_cq)
    }

    /// Create device submission queue from host submission queue Id
    fn create_sq(
        &self,
        host_sq: HostSqId,
        host_cq: HostCqId,
        mem: QueueMem,
    ) -> Result<(DevSqId, DevCqId), AdminErr> {
        self.rimpl.borrow_mut().create_sq(host_sq, host_cq, mem)
    }

    /// Get the device submission queue Id from host submission queue Id
    fn dev_sq(&self, host_sq: HostSqId) -> Result<DevSqId, AdminErr> {
        self.rimpl.borrow().dev_sq(host_sq)
    }

    /// Delete a device submission queue from host submission queue Id
    fn delete_sq(&self, host_sq: HostSqId) -> Result<(DevSqId, DevCqId), AdminErr> {
        self.rimpl.borrow_mut().delete_sq(host_sq)
    }

    /// Set Controller Fatal Status
    fn set_cfs(&self) {
        self.rimpl.borrow().set_cfs()
    }

    /// Get the instance of the admin queue
    fn admin_queue(&self) -> Option<AdminQueue> {
        self.rimpl.borrow().admin_queue()
    }

    /// Save the live migration context
    fn save_lm_context(
        &self,
        lm_info: &mut VmLiveMigrationInfo,
        session_allocation_mask: u8,
        masked_bk_boot: &MaskedBkBoot,
        sealed_bk3: &SealedBk3,
    ) {
        self.rimpl.borrow_mut().save_lm_context(
            lm_info,
            session_allocation_mask,
            masked_bk_boot,
            sealed_bk3,
        )
    }

    /// Restore the live migration context
    fn restore_lm_context(&self, lm_info: &mut VmLiveMigrationInfo) -> Result<(), AdminErr> {
        self.rimpl.borrow_mut().restore_lm_context(lm_info)
    }

    /// Get the enabled submission queues info
    fn get_enabled_sq_info(&self) -> Vec<(HostSqId, DevSqId, DevCqId)> {
        self.rimpl.borrow().get_enabled_sq_info()
    }

    /// Enable the device submission queue
    fn enable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId) {
        self.rimpl.borrow().cntrl.enable_sq(dev_sq, host_sq);
    }

    /// Disable the device submission queue
    fn disable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId) {
        self.rimpl.borrow().cntrl.disable_sq(dev_sq, host_sq);
    }

    /// Complete the live migration process
    fn complete_live_migration(&self) {
        self.rimpl.borrow().cntrl.set_enable();
        self.rimpl.borrow().cntrl.enable();
    }
}

impl<Q: QueueControllerTrait> Function<Q> {
    pub(crate) fn new(cntrl: Q, resources: Rc<RefCell<&'static mut [Resource]>>) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(FunctionImpl::new(cntrl, resources))),
        }
    }
}

/// PCIe Function
struct FunctionImpl<Q: QueueControllerTrait> {
    /// Queue Controller
    cntrl: Q,

    /// List of resources owned by this function
    resources: Rc<RefCell<&'static mut [Resource]>>,

    /// Admin queue instance
    admin_queue: Option<AdminQueue>,

    /// Current Resource count assigned to this function
    res_cnt: u32,

    /// PcieFunction
    pfn: PcieFunction,
}

impl<Q: QueueControllerTrait> FunctionImpl<Q> {
    /// Create a new instance of function implementation
    fn new(cntrl: Q, resources: Rc<RefCell<&'static mut [Resource]>>) -> Self {
        let pfn = cntrl.id().into();

        // Calculate current resource assignment to this function
        let res_cnt = resources
            .borrow()
            .iter()
            .filter(|item| item.pfn == Some(pfn))
            .count() as u32;

        let admin_queue = if cntrl.ready() {
            Some(AdminQueue::new(cntrl.id().into(), cntrl.id().into()))
        } else {
            None
        };

        Self {
            cntrl,
            resources,
            admin_queue,
            res_cnt,
            pfn,
        }
    }

    /// Query the controller state change
    fn query_state_change(&self) -> CntrlStateChangeAction {
        match (self.cntrl.ready(), self.cntrl.enabled()) {
            (false, true) => CntrlStateChangeAction::Enable,
            (true, false) => CntrlStateChangeAction::Disable,
            _ => CntrlStateChangeAction::Invalid,
        }
    }

    /// Get the Queue controller Id corresponding to this function
    fn cntrl_id(&self) -> QueueCntrlId {
        self.cntrl.id()
    }

    /// Enable Pcie function
    fn enable(&mut self) -> McrResult<()> {
        let sq_id = self.cntrl.id().into();
        let cq_id = self.cntrl.id().into();

        self.cntrl.create_acq(cq_id, HostCqId::Id0)?;

        self.cntrl.create_asq(sq_id, HostSqId::Id0).map_err(|_| {
            self.cntrl.delete_cq(self.cntrl.id().into(), HostCqId::Id0);
            AdminErr::AdminSqCreateFailed
        })?;

        self.cntrl.enable();

        self.admin_queue = Some(AdminQueue::new(sq_id, cq_id));

        Ok(())
    }

    /// Disable Pcie function
    fn disable(&mut self) {
        self.delete_io_queues();
        self.cntrl.delete_sq(self.cntrl.id().into(), HostSqId::Id0);
        self.cntrl.delete_cq(self.cntrl.id().into(), HostCqId::Id0);
        self.cntrl.disable();

        if let Some(queue) = self.admin_queue.take() {
            queue.invalidate();
        }
    }

    /// Reset the function
    fn reset(&mut self) {
        self.delete_io_queues();
        self.cntrl.delete_sq(self.cntrl.id().into(), HostSqId::Id0);
        self.cntrl.delete_cq(self.cntrl.id().into(), HostCqId::Id0);
        self.cntrl.reset();
        self.res_cnt = 0;
    }

    /// Check if the function is ready
    fn ready(&self) -> bool {
        self.cntrl.ready()
    }

    /// set the allocated resource count to this function
    fn set_res_cnt(&mut self, cnt: u32) {
        self.res_cnt = cnt
    }

    /// Get the resource counts allocated to this function
    fn res_cnt(&self) -> u32 {
        self.res_cnt
    }

    /// Get the resource allocation mask
    fn res_mask(&self) -> [u8; 16] {
        const INIT: u128 = 0;
        let pfn = self.cntrl_id().into();
        let mask: u128 = self
            .resources
            .borrow()
            .iter()
            .enumerate()
            .fold(
                INIT,
                |m, (i, r)| {
                    if r.pfn == Some(pfn) {
                        m | 1 << i
                    } else {
                        m
                    }
                },
            );

        mask.to_le_bytes()
    }

    /// Create device completion queue from host completion queue Id
    fn create_cq(
        &mut self,
        host_cq: HostCqId,
        mem: QueueMem,
        irq: Option<u16>,
    ) -> Result<(), AdminErr> {
        let res_index = self.resource_index(host_cq)?;

        let dev_cq = self.resources.borrow_mut().as_mut()[res_index]
            .alloc_cq(host_cq)
            .ok_or(AdminErr::CqAlreadyAllocated)?;

        self.cntrl
            .create_cq(dev_cq, host_cq, mem, irq)
            .map_err(|_| {
                self.resources.borrow_mut().as_mut()[res_index].free_cq(host_cq);
                AdminErr::CreateCqFailedByQueueController
            })
    }

    /// Delete a device completion queue from host completion queue Id
    fn delete_cq(&mut self, host_cq: HostCqId) -> Result<(), AdminErr> {
        let res_index = self.resource_index(host_cq)?;

        if self.resources.borrow().as_ref()[res_index].enabled(HostSqId::from(host_cq)) {
            Err(AdminErr::InvalidQueueDelete)?;
        }

        let dev_cq = self.resources.borrow_mut().as_mut()[res_index]
            .free_cq(host_cq)
            .ok_or(AdminErr::InvalidCompletionQueueIdInDelete)?;
        self.cntrl.delete_cq(dev_cq, host_cq);

        Ok(())
    }

    /// Create device submission queue from host submission queue Id
    fn create_sq(
        &mut self,
        host_sq: HostSqId,
        host_cq: HostCqId,
        mem: QueueMem,
    ) -> Result<(DevSqId, DevCqId), AdminErr> {
        let res_index = self.resource_index(host_cq)?;

        if !self.resources.borrow().as_ref()[res_index].enabled(host_cq) {
            Err(AdminErr::CqNotAvailable)?;
        }

        let res_index = self.resource_index(host_sq)?;

        let dev_sq = self.resources.borrow_mut().as_mut()[res_index]
            .alloc_sq(host_sq)
            .ok_or(AdminErr::SqAlreadyAllocated)?;

        self.cntrl.create_sq(dev_sq, host_sq, mem).map_err(|_| {
            self.resources.borrow_mut().as_mut()[res_index].free_sq(host_sq);
            AdminErr::CreateSqFailedByQueueController
        })?;

        Ok((dev_sq, dev_sq.into()))
    }

    /// Get the device submission queue Id from host submission queue Id
    fn dev_sq(&self, host_sq: HostSqId) -> Result<DevSqId, AdminErr> {
        let res_index = self.resource_index(host_sq)?;

        let dev_sq = self.resources.borrow().as_ref()[res_index]
            .dev_sq(host_sq)
            .ok_or(AdminErr::InvalidSqId)?;

        Ok(dev_sq)
    }

    /// Delete a device submission queue from host submission queue Id
    fn delete_sq(&mut self, host_sq: HostSqId) -> Result<(DevSqId, DevCqId), AdminErr> {
        let res_index = self.resource_index(host_sq)?;

        let dev_sq = self.resources.borrow_mut().as_mut()[res_index]
            .free_sq(host_sq)
            .ok_or(AdminErr::InvalidSqId)?;

        self.cntrl.delete_sq(dev_sq, host_sq);

        Ok((dev_sq, dev_sq.into()))
    }

    /// Delete all the IO queues belong to this function
    fn delete_io_queues(&mut self) {
        for (index, resource) in self
            .resources
            .borrow_mut()
            .iter_mut()
            .filter(|r| r.pfn == Some(self.pfn))
            .enumerate()
        {
            while let Some(host_sq) = resource.host_sq(index) {
                if let Some(dev_sq) = resource.free_sq(host_sq) {
                    self.cntrl.delete_sq(dev_sq, host_sq);
                }
            }

            while let Some(host_cq) = resource.host_cq(index) {
                if let Some(dev_cq) = resource.free_cq(host_cq) {
                    self.cntrl.delete_cq(dev_cq, host_cq);
                }
            }
        }
    }

    /// Set Controller Fatal Status
    fn set_cfs(&self) {
        self.cntrl.set_cfs()
    }

    /// Get the instance of the admin queue
    fn admin_queue(&self) -> Option<AdminQueue> {
        self.admin_queue.clone()
    }

    /// Save the live migration context
    fn save_lm_context(
        &self,
        lm_info: &mut VmLiveMigrationInfo,
        session_allocation_mask: u8,
        masked_bk_boot: &MaskedBkBoot,
        sealed_bk3: &SealedBk3,
    ) {
        const LM_INFO_VER_MAJ: u16 = 1;
        const LM_INFO_VER_MIN: u16 = 0;

        let mut sq_info_index = 0;
        let mut cq_info_index = 0;

        // Clear the LM info buffer before saving a new context
        lm_info.as_mut_bytes().zeroize();

        lm_info.version = LmVersionInfo {
            minor: LM_INFO_VER_MIN,
            major: LM_INFO_VER_MAJ,
        };

        // Save resource count allocated to this function
        lm_info.resource_cnt = self.res_cnt;

        // Admin queue information
        lm_info.admin_sq_info = self.cntrl.sq_info(self.cntrl.id().into(), HostSqId::Id0);
        lm_info.admin_cq_info = self.cntrl.cq_info(self.cntrl.id().into(), HostCqId::Id0);

        // Store the controller register information
        lm_info.cntrl_info = self.cntrl.host_register_info();

        // For all the valid resoruces in this function, store the submission and completion queue
        // info.
        for (index, resource) in self
            .resources
            .borrow_mut()
            .iter_mut()
            .filter(|r| r.pfn == Some(self.pfn))
            .enumerate()
        {
            let host_sq_list = resource.host_sq_list(index);

            for host_sq in host_sq_list.iter() {
                if let Some(dev_sq) = resource.dev_sq(*host_sq) {
                    lm_info.io_sq_info[sq_info_index] = self.cntrl.sq_info(dev_sq, *host_sq);
                    sq_info_index += 1;
                }
            }

            let host_cq_list = resource.host_cq_list(index);

            for host_cq in host_cq_list.iter() {
                if let Some(dev_cq) = resource.dev_cq(*host_cq) {
                    lm_info.io_cq_info[cq_info_index] = self.cntrl.cq_info(dev_cq, *host_cq);
                    cq_info_index += 1;
                }
            }
        }

        lm_info.sq_cnt = sq_info_index as u16;
        lm_info.cq_cnt = cq_info_index as u16;

        lm_info.masked_bk_boot = *masked_bk_boot;
        lm_info.sealed_bk3 = *sealed_bk3;

        lm_info.session_allocation_mask = session_allocation_mask;

        let mut hasher = TinyCrc32::new();
        lm_info.crc = hasher.checksum(lm_info.as_bytes());
    }

    /// Restore the live migration context
    fn restore_lm_context(&mut self, lm_info: &mut VmLiveMigrationInfo) -> Result<(), AdminErr> {
        // Validate the CRC-32 by extracting the CRC-32 from the LM info
        // and calculating the CRC-32 from the LM info buffer.
        // If the CRC-32 does not match, return an error.
        let crc = lm_info.crc;

        // Recalculate the CRC-32 by excluding the CRC-32 field
        lm_info.crc = 0;
        let mut hasher = TinyCrc32::new();
        if hasher.checksum(lm_info.as_bytes()) != crc {
            Err(AdminErr::InvalidLmContextToRestore)?
        }

        // Cannot restore the context if the resoruce count assignment does not match
        // between the back up and current function
        if lm_info.resource_cnt != self.res_cnt() {
            Err(AdminErr::ResourceLimitReached)?;
        }

        // Restore the controller register information
        // includig admin queue information
        self.cntrl.restore_host_register_info(&lm_info.cntrl_info);

        // Admin queue device IDs are the same as the controller ID
        let sq_id = self.cntrl.id().into();
        let cq_id = self.cntrl.id().into();

        // Restore the admin queues
        self.cntrl
            .restore_cq_info(cq_id, &lm_info.admin_cq_info)
            .map_err(|_| AdminErr::CreateCqFailedByQueueController)?;
        self.cntrl
            .restore_sq_info(sq_id, &lm_info.admin_sq_info)
            .map_err(|_| AdminErr::CreateSqFailedByQueueController)?;
        self.cntrl.enable_sq(sq_id, HostSqId(0));
        self.admin_queue = Some(AdminQueue::new(sq_id, cq_id));

        // Restore the IO completion queues
        for index in 0..lm_info.cq_cnt as usize {
            self.restore_cq_info(&lm_info.io_cq_info[index])?;
        }

        // Restore the IO submission queues
        for index in 0..lm_info.sq_cnt as usize {
            self.restore_sq_info(&lm_info.io_sq_info[index])?;
        }

        Ok(())
    }

    /// Restore device completion queue from LM info backup
    fn restore_cq_info(&mut self, info: &LmCqInfo) -> Result<(), AdminErr> {
        let host_cq = HostCqId(info.id);
        let res_index = self.resource_index(host_cq)?;

        let dev_cq = self.resources.borrow_mut().as_mut()[res_index]
            .alloc_cq(host_cq)
            .ok_or(AdminErr::CqAlreadyAllocated)?;

        self.cntrl.restore_cq_info(dev_cq, info).map_err(|_| {
            self.resources.borrow_mut().as_mut()[res_index].free_cq(host_cq);
            AdminErr::CreateCqFailedByQueueController
        })
    }

    /// Restore device submission queue from LM info backup
    fn restore_sq_info(&mut self, info: &LmSqInfo) -> Result<(), AdminErr> {
        let host_cq = HostCqId(info.cq_id);
        let res_index = self.resource_index(host_cq)?;

        if !self.resources.borrow().as_ref()[res_index].enabled(host_cq) {
            Err(AdminErr::CqNotAvailable)?;
        }

        let host_sq = HostSqId(info.id);
        let res_index = self.resource_index(host_sq)?;

        let dev_sq = self.resources.borrow_mut().as_mut()[res_index]
            .alloc_sq(host_sq)
            .ok_or(AdminErr::SqAlreadyAllocated)?;

        self.cntrl.restore_sq_info(dev_sq, info).map_err(|_| {
            self.resources.borrow_mut().as_mut()[res_index].free_sq(host_sq);
            AdminErr::CreateSqFailedByQueueController
        })
    }

    /// Get the enabled submission queues info
    fn get_enabled_sq_info(&self) -> Vec<(HostSqId, DevSqId, DevCqId)> {
        let mut sq_info = Vec::new();

        // For all the valid resoruces in this function, store the submission and completion queue
        // info.
        for (index, resource) in self
            .resources
            .borrow_mut()
            .iter_mut()
            .filter(|r| r.pfn == Some(self.pfn))
            .enumerate()
        {
            let host_sq_list = resource.host_sq_list(index);

            for host_sq in host_sq_list.iter() {
                let host_cq = HostCqId(host_sq.0);

                if let Some(dev_sq) = resource.dev_sq(*host_sq) {
                    if let Some(dev_cq) = resource.dev_cq(host_cq) {
                        sq_info.push((*host_sq, dev_sq, dev_cq));
                    }
                }
            }
        }

        sq_info
    }

    /// Get the resource index from host submission or completion queue Id
    fn resource_index<T: FunctionResourceLayout>(&self, queue_id: T) -> Result<usize, AdminErr> {
        let res_index = queue_id.res_index().ok_or(AdminErr::InvalidQueueId)?;
        if res_index >= self.res_cnt() as usize {
            Err(AdminErr::QueueIdOutOfRangeForFunction)?;
        }

        for (index, resource) in self
            .resources
            .borrow()
            .iter()
            .filter(|r| r.pfn == Some(self.pfn))
            .enumerate()
        {
            if index == res_index {
                let real_index = resource.id;

                return Ok(real_index as usize);
            }
        }

        Err(AdminErr::ResourceLimitReached)
    }
}
