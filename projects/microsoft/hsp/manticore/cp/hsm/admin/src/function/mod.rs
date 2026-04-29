// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::vec::Vec;

mod admin_queue;
mod func;
mod mgr;

#[cfg(test)]
mod tests;

pub(crate) use admin_queue::*;
pub(crate) use func::*;
use mcr_error::McrResult;
use mcr_queue_controller::*;
use mcr_types::*;
pub(crate) use mgr::*;

use crate::error::AdminErr;

/// Controller state change actions
#[derive(Debug, PartialEq, Eq)]
pub(crate) enum CntrlStateChangeAction {
    /// Controller is due to be enabled
    Enable,

    /// Controller is due to be disabled
    Disable,

    /// Controller Migrate
    Migrate,

    /// Invalid state change
    Invalid,
}

/// Function Manager Trait
pub(crate) trait FunctionMgrTrait {
    /// Function interface
    type Function: FunctionTrait + Clone;

    /// Reset all the Pcie functions managed by this Pcie function manager
    fn reset(&self);

    /// Get the specified PCIe function
    ///
    /// # Arguments
    ///
    /// * `func` - PcieFunction
    ///
    /// # Returns
    ///
    /// * The `Function`
    fn function(&self, func: PcieFunction) -> Self::Function;

    /// Set resource count for the function, if the resource allocation is successful for the
    /// the requested count, this function will return resource count owned by the function prior
    /// to this assignment.
    ///
    /// # Arguments
    ///
    /// * `func` - PcieFunction
    /// * `cnt` - Number of resources to be assigned to the function
    ///
    /// # Returns
    ///
    /// * `Result<u32, AdminErr>` - Ok(resources owned by this function before this assignment) or
    ///   AdminErr
    fn set_res_cnt(&self, func: PcieFunction, cnt: u32) -> Result<u32, AdminErr>;

    /// Prepare the PCIe function manager for warm boot
    fn prepare_for_warm_boot(&self);
}

pub(crate) trait FunctionTrait {
    /// Query the controller state change
    ///
    /// # Returns
    ///
    /// * `CntrlStateChange` - Requested state change
    fn query_state_change(&self) -> CntrlStateChangeAction;

    /// Get the Queue controller Id corresponding to this function
    ///
    /// # Returns
    ///
    /// * `QueueCntrlId` - Queue controller Id
    fn cntrl_id(&self) -> QueueCntrlId;

    /// Enable Pcie function
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok or an Err
    fn enable(&self) -> McrResult<()>;

    /// Disable Pcie function
    fn disable(&self);

    /// Clear the enable status of the function
    ///
    /// After Effects# This will trigger function disable sequence
    fn clear_enable(&self);

    /// Reset the function
    fn reset(&self);

    /// Check if the function is ready
    ///
    /// # Returns
    ///
    /// * `bool` - true if function is ready , false otherwise
    fn ready(&self) -> bool;

    /// Check if the function is enabled
    ///
    /// # Returns
    ///
    /// * `bool` - true if function is enabled , false otherwise
    fn enabled(&self) -> bool;

    /// set the allocated resource count to this function
    ///
    /// # Arguments
    ///
    /// * `cnt` - Allocated resource count to this function
    fn set_res_cnt(&self, cnt: u32);

    /// Get the resource counts allocated to this function
    ///
    /// # Returns
    ///
    /// * `u32` - number of resources assigned to the function
    fn res_cnt(&self) -> u32;

    /// Get the resource allocation mask
    ///
    /// # Returns
    ///
    /// * `[u8; 16]` - Little Endian resource allocation mask
    fn res_mask(&self) -> [u8; 16];

    /// Create device completion queue from host completion queue Id
    ///
    /// # Arguments
    ///
    /// * `host_cq` - Host completion queue Id
    /// * `mem` - Queue memory
    /// * `irq` - Interrupt number
    ///
    /// # Returns
    ///
    /// * `Result<(), AdminErr>` - Ok or an Err
    fn create_cq(&self, host_cq: HostCqId, mem: QueueMem, irq: Option<u16>)
        -> Result<(), AdminErr>;

    /// Delete a device completion queue from host completion queue Id
    ///
    /// # Arguments
    ///
    /// * `host_cq` - Host completion queue Id
    ///
    /// # Returns
    ///
    /// * `Result<(), AdminErr>` - Ok or an Err
    fn delete_cq(&self, host_cq: HostCqId) -> Result<(), AdminErr>;

    /// Create device submission queue from host submission queue Id
    ///
    /// # Arguments
    ///
    /// * `host_sq` - Host submission queue Id
    /// * `host_cq` - Host completion queue Id
    /// * `mem` - Queue memory
    ///
    /// # Returns
    ///
    /// * `Result<(DevSqId, DevCqId), AdminErr>` - Ok with an tuple of device submission and
    ///   completion queue Id or an Err
    fn create_sq(
        &self,
        host_sq: HostSqId,
        host_cq: HostCqId,
        mem: QueueMem,
    ) -> Result<(DevSqId, DevCqId), AdminErr>;

    /// Delete a device submission queue from host submission queue Id
    ///
    /// # Arguments
    ///
    /// * `host_cq` - Host submission queue Id
    ///
    /// # Returns
    ///
    /// * `Result<(DevSqId, DevCqId), AdminErr>` - Ok with an tuple of device submission and
    ///   completion queue Id or an Err
    fn delete_sq(&self, host_sq: HostSqId) -> Result<(DevSqId, DevCqId), AdminErr>;

    /// Set Controller Fatal Status
    fn set_cfs(&self);

    /// Get the device submission queue Id from host submission queue Id
    ///
    /// # Arguments
    ///
    /// * `host_sq` - Host submission queue Id
    ///
    /// # Returns
    ///
    /// * `Result<DevSqId, AdminErr>` - Ok with device submission queue Id or an Err
    fn dev_sq(&self, host_sq: HostSqId) -> Result<DevSqId, AdminErr>;

    /// Get the instance of the admin queue
    ///
    /// # Returns
    ///
    /// * `AdminQueue` - Instance of the admin queue
    fn admin_queue(&self) -> Option<AdminQueue>;

    /// Save the live migration context
    ///
    /// # Arguments
    ///
    /// * `buf` - Buffer to save the live migration context
    /// * `session_allocation_mask` - Session allocation mask
    /// * `masked_bk_boot` - Masked Boot Key
    /// * `sealed_bk3` - Sealed Backup Key 3
    fn save_lm_context(
        &self,
        buf: &mut VmLiveMigrationInfo,
        session_allocation_mask: u8,
        masked_bk_boot: &MaskedBkBoot,
        sealed_bk3: &SealedBk3,
    );

    /// Restore the live migration context
    ///
    /// # Arguments
    ///
    /// * `buf` - Buffer containing the live migration context
    ///
    /// # Returns
    ///
    /// * `Result<(), AdminErr>` - Ok or an Err
    fn restore_lm_context(&self, buf: &mut VmLiveMigrationInfo) -> Result<(), AdminErr>;

    /// Get the enabled submission queue information
    ///
    /// # Returns
    ///
    /// * `Vec<(HostSqId, DevSqId, DevCqId)>` - Vector of tuples containing Host submission queue
    ///   Id, Device submission queue Id, and Device completion queue Id
    fn get_enabled_sq_info(&self) -> Vec<(HostSqId, DevSqId, DevCqId)>;

    /// Enable the device submission queue
    ///
    /// # Arguments
    ///
    /// * `dev_sq` - Device submission queue Id
    /// * `host_sq` - Host submission queue Id
    fn enable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId);

    /// Disable all device submission queue
    ///
    /// # Arguments
    ///
    /// * `dev_sq` - Device submission queue Id
    /// * `host_sq` - Host submission queue Id
    fn disable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId);

    /// Complete the live migration process
    ///
    /// # This function is called after all the live migration context has been restored
    /// and the function is ready to be used.
    fn complete_live_migration(&self);
}

/// Streaming CRC32 implementation for constrained firmware
/// Allows processing data in chunks with update/finalize pattern
pub(crate) struct TinyCrc32 {
    crc: u32,
}

impl TinyCrc32 {
    /// Initialize a new CRC32 calculation
    /// with init crc as 0xFFFFFFFF
    pub(crate) fn new() -> Self {
        Self { crc: !0u32 }
    }

    /// Update CRC32 with new data chunk
    ///
    /// # Arguments
    /// * `data` - New data slice to process  
    pub(crate) fn update(&mut self, data: &[u8]) {
        for &byte in data {
            self.crc ^= byte as u32;
            for _ in 0..8 {
                self.crc = if self.crc & 1 != 0 {
                    (self.crc >> 1) ^ 0xEDB88320
                } else {
                    self.crc >> 1
                };
            }
        }
    }

    /// Finalize CRC32 calculation
    ///
    /// # Arguments
    /// * `crc` - Final CRC state from last update
    ///
    /// # Returns
    /// Final CRC32 checksum
    pub fn finalize(&self) -> u32 {
        !self.crc
    }

    /// One-shot CRC32 calculation (convenience function)
    ///
    /// # Arguments
    /// * `data` - Data slice to process
    ///
    /// # Returns
    /// Final CRC32 checksum
    pub fn checksum(&mut self, data: &[u8]) -> u32 {
        self.update(data);

        self.finalize()
    }
}
