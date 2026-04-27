// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;

use mcr_types::*;

use crate::env::HsmEnvTrait;
use crate::partition::*;

/// HSM Partition Manager
pub(crate) struct HsmPartitionMgr<E: HsmEnvTrait + 'static> {
    rimpl: Rc<RefCell<HsmPartitionMgrImpl<E>>>,
}

impl<E: HsmEnvTrait> Clone for HsmPartitionMgr<E> {
    /// Returns a copy of the value.
    fn clone(&self) -> Self {
        Self {
            rimpl: self.rimpl.clone(),
        }
    }
}

impl<E: HsmEnvTrait> HsmPartitionMgr<E> {
    /// Create an instnace of `HsmPartitionMgr`
    ///
    /// # Arguments
    ///
    /// * `func` - Function to create a new partition
    ///
    /// # Returns
    ///
    /// * Instance of `HsmPartitionMgr`
    pub(crate) fn new<F>(func: F) -> Self
    where
        F: Fn(PcieFunction) -> E::Partition,
    {
        Self {
            rimpl: Rc::new(RefCell::new(HsmPartitionMgrImpl::new(func))),
        }
    }

    /// Returns the partition associated with the given PCIe function
    ///
    /// # Arguments
    ///
    /// * `pfn` - PCIe function
    ///
    /// # Returns
    ///
    /// Retuns the partition
    pub fn partition(&self, pfn: PcieFunction) -> E::Partition {
        self.rimpl.borrow().partition(pfn)
    }

    /// Prepare for shutdown
    pub fn prepare_for_shutdown(&self) {
        self.rimpl.borrow().prepare_for_shutdown()
    }
}

/// HSM Function Manager
struct HsmPartitionMgrImpl<E: HsmEnvTrait + 'static> {
    partitions: Vec<E::Partition>,
}

impl<E: HsmEnvTrait> HsmPartitionMgrImpl<E> {
    /// Create an instnace of `HsmPartitionMgr`
    ///
    /// # Arguments
    ///
    /// * `func` - Function to create a new partition
    ///
    /// # Returns
    ///
    /// * Instance of `HsmFuncMgr`
    fn new<F>(func: F) -> Self
    where
        F: Fn(PcieFunction) -> E::Partition,
    {
        let mut partitions = Vec::with_capacity(MAX_PCIE_FUNCTIONS);
        for pfn in PcieFunction::iter() {
            partitions.push(func(pfn));
        }

        Self { partitions }
    }

    /// Returns the partition associated with the given PCIe function
    ///
    /// # Arguments
    ///
    /// * `pfn` - PCIe function
    ///
    /// # Returns
    ///
    /// Retuns the partition
    fn partition(&self, pfn: PcieFunction) -> E::Partition {
        let index: usize = pfn.into();
        self.partitions[index].clone()
    }

    /// Prepare for shutdown
    fn prepare_for_shutdown(&self) {
        for part in self.partitions.iter() {
            part.store_data()
        }
    }
}
