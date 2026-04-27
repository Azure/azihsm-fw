// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;
use core::cmp::Ordering;

use mcr_queue_controller::*;
use mcr_types::*;

use super::*;
use crate::error::AdminErr;

/// Function Manager
#[derive(Clone)]
pub(crate) struct FunctionMgr<Q: QueueControllerTrait> {
    rimpl: Rc<RefCell<FunctionMgrImpl<Q>>>,
}

impl<Q: QueueControllerTrait> FunctionMgr<Q> {
    /// Create an instance of `FunctionMgr`
    ///
    /// # Returns
    ///
    /// * Instance of `FunctionMgr`
    pub fn new<F>(
        factory: F,
        resources: &'static mut [Resource],
        crc: &'static VolatileCell<u32>,
    ) -> McrResult<Self>
    where
        F: Fn(QueueCntrlId) -> Q,
    {
        let function_mgr_impl = FunctionMgrImpl::new(factory, resources, crc)?;
        Ok(Self {
            rimpl: Rc::new(RefCell::new(function_mgr_impl)),
        })
    }
}

impl<Q: QueueControllerTrait> FunctionMgrTrait for FunctionMgr<Q> {
    type Function = Function<Q>;

    /// Get the specified PCIe function
    fn function(&self, func: PcieFunction) -> Self::Function {
        self.rimpl.borrow().function(func)
    }

    /// Reset all the Pcie functions managed by this Pcie function manager
    fn reset(&self) {
        self.rimpl.borrow_mut().reset()
    }

    /// Set resource count for the function
    fn set_res_cnt(&self, func: PcieFunction, cnt: u32) -> Result<u32, AdminErr> {
        self.rimpl.borrow_mut().set_res_cnt(func, cnt)
    }

    /// Prepare the PCIe function manager for warm boot
    fn prepare_for_warm_boot(&self) {
        self.rimpl.borrow_mut().prepare_for_warm_boot();
    }
}

/// Function Manager Implementation
struct FunctionMgrImpl<Q: QueueControllerTrait> {
    /// List of all PCie functions supported
    funcs: Vec<Function<Q>>,

    /// List of all resources
    resources: Rc<RefCell<&'static mut [Resource]>>,

    /// Resources cyclic redundancy check
    crc: &'static VolatileCell<u32>,

    /// Free Resource Count
    free_resource_cnt: u32,
}

impl<Q: QueueControllerTrait> FunctionMgrImpl<Q> {
    /// Create an instance of `FunctionMgr`
    fn new<F>(
        factory: F,
        resources: &'static mut [Resource],
        crc: &'static VolatileCell<u32>,
    ) -> McrResult<Self>
    where
        F: Fn(QueueCntrlId) -> Q,
    {
        let mut funcs: Vec<Function<Q>> = Default::default();

        for (index, item) in resources.iter_mut().enumerate() {
            item.id = index as u8;
        }

        // Pre-calculate the initial free resource count across all resouce maps
        let free_resource_cnt = resources.iter().filter(|r| r.pfn.is_none()).count() as u32;

        let rc_resources = Rc::new(RefCell::new(resources));

        for func in PcieFunction::iter() {
            let cntrl_id =
                QueueCntrlId::try_from(func).map_err(|_| AdminErr::InvalidQueueControllerId)?;
            funcs.push(Function::new(factory(cntrl_id), rc_resources.clone()));
        }

        let mut calculated_crc = Self::get_crc(rc_resources.clone());
        if calculated_crc != crc.get() {
            // The given CRC does not match with the CRC of the resource table, so the resource
            // table is invalid. Zeroize the resource table and recalculate the CRC.
            for resource in rc_resources.borrow_mut().iter_mut() {
                resource.pfn = None;
                resource.reset();
            }
            calculated_crc = Self::get_crc(rc_resources.clone());
        }
        crc.set(calculated_crc);

        Ok(Self {
            funcs,
            resources: rc_resources,
            crc,
            free_resource_cnt,
        })
    }

    /// Reset all the Pcie functions managed by this Pcie function manager
    fn reset(&mut self) {
        for func in self.funcs.iter() {
            func.reset()
        }

        for resource in self.resources.borrow_mut().iter_mut() {
            resource.pfn = None;
            resource.reset();
        }

        self.free_resource_cnt = self.resources.borrow().len() as u32;
    }

    /// Prepare the PCIe function manager for warm boot
    fn prepare_for_warm_boot(&self) {
        for func in self.funcs.iter() {
            func.set_cfs();
        }

        // Reset queue allocations for each resource
        for res in self.resources.borrow_mut().iter_mut() {
            res.reset();
        }
    }

    /// Get the specified PCIe function
    fn function(&self, func: PcieFunction) -> Function<Q> {
        let fn_idx: usize = func.into();
        self.funcs[fn_idx].clone()
    }

    /// Set resource count for the function, if the resoruce allocation is successful for the
    /// the requested count, this function will return resource count owned by the function prior
    /// to this assignment.
    fn set_res_cnt(&mut self, pfn: PcieFunction, cnt: u32) -> Result<u32, AdminErr> {
        let func = self.function(pfn);
        // When a set resource count request is received for a previously enabled Virtual Function
        // (VF), it indicates the VF was unassigned from a VM. The Physical Function (PF) driver
        // then cleans it up by issuing set resource count with 0 resource for the VF. Firmware will
        // take this input and triggering the disable sequence to clear all the previously VM  data
        // before this VF can be reassigned to a new VM.
        //
        // Disclaimar: This step will trigger the CC_EN disable sequence from within the firmware,
        // to complete self cleanup on behalf of a VM that was shutdown abruptly.
        if pfn != PcieFunction::Pf && func.enabled() {
            func.clear_enable();
        }

        let current_res_cnt = func.res_cnt();

        match current_res_cnt.cmp(&cnt) {
            Ordering::Less => {
                // Request to increase existing resource count of the function
                let new_res_cnt = cnt - current_res_cnt;

                // Fail if the resource allocation cannot be made possible
                if self.free_resource_cnt < new_res_cnt {
                    Err(AdminErr::SetResCountLimitExceeded)?;
                }

                // Allocate the delta resources
                for resource in self
                    .resources
                    .borrow_mut()
                    .iter_mut()
                    .filter(|r| r.pfn.is_none())
                    .take(new_res_cnt as usize)
                {
                    resource.pfn = Some(pfn);
                    resource.reset();
                }
                // Update the function with newly assigned resource count
                self.free_resource_cnt -= new_res_cnt;
                func.set_res_cnt(cnt);

                self.update_crc();
            }
            Ordering::Greater => {
                let delete_res_cnt = current_res_cnt - cnt;
                self.free_resource_cnt += delete_res_cnt;

                // Reduce existing resource count of the function
                self.resources
                    .borrow_mut()
                    .iter_mut()
                    .filter(|r| r.pfn == Some(pfn))
                    .take(delete_res_cnt as usize)
                    .for_each(|r| r.pfn = None);

                // Update the function with newly assigned resource count
                func.set_res_cnt(cnt);

                self.update_crc();
            }
            Ordering::Equal => (),
        }

        Ok(current_res_cnt)
    }

    /// Get CRC of resources
    fn get_crc(resources: Rc<RefCell<&'static mut [Resource]>>) -> u32 {
        let mut hasher = TinyCrc32::new();
        for resource in resources.borrow().iter() {
            hasher.update(resource.owner_pfn_as_bytes().as_slice());
        }

        hasher.finalize()
    }

    /// Update CRC of resources
    fn update_crc(&mut self) {
        let new_crc = Self::get_crc(self.resources.clone());
        self.crc.set(new_crc);
    }
}
