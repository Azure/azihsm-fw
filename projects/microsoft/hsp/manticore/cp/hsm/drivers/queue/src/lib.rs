// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod cntrl;
mod reg;

pub use cntrl::QueueController;
use mcr_error::*;
use mcr_types::*;

seq_macro::seq! {
    N in 0..64 {
        /// Queue controller Ids
        #[derive(PartialEq, Eq, PartialOrd, Ord, Clone, Copy)]
        pub enum QueueCntrlId {
            Pf = 0,
            #(
                #[allow(clippy::identity_op)]
                Vf~N = N + 1,
            )*
        }
    }
}

seq_macro::seq! {
    N in 0..64 {
        impl From<QueueCntrlId> for MemoryLocation {
            fn from(value: QueueCntrlId) -> Self {
                match value {
                    QueueCntrlId::Pf => MemoryLocation::Pf,
                    #(QueueCntrlId::Vf~N => MemoryLocation::Vf~N,)*
                }
            }
        }
    }
}

seq_macro::seq! {
    N in 0..64 {
        impl From<QueueCntrlId> for PcieFunction {
            fn from(value: QueueCntrlId) -> Self {
                match value {
                    QueueCntrlId::Pf => PcieFunction::Pf,
                    #(QueueCntrlId::Vf~N => PcieFunction::Vf~N,)*
                }
            }
        }
    }
}

seq_macro::seq! {
    N in 0..64 {
        impl TryFrom<PcieFunction> for QueueCntrlId {
            type Error = u32;

            fn try_from(value: PcieFunction) -> Result<Self, Self::Error> {
                match value {
                    PcieFunction::Pf => Ok(QueueCntrlId::Pf),
                    #(PcieFunction::Vf~N => Ok(QueueCntrlId::Vf~N),)*
                    _ => Err(QueueControllerErr::InvalidConversionFromPcieFunction.into()),
                }
            }
        }
    }
}

impl From<QueueCntrlId> for u8 {
    fn from(value: QueueCntrlId) -> Self {
        value as Self
    }
}

impl From<QueueCntrlId> for u16 {
    fn from(value: QueueCntrlId) -> Self {
        value as Self
    }
}

impl TryFrom<u32> for QueueCntrlId {
    type Error = u32;

    fn try_from(value: u32) -> Result<Self, Self::Error> {
        match value {
            0..=64 => Ok(unsafe { core::mem::transmute::<u8, QueueCntrlId>(value as u8) }),
            _ => Err(QueueControllerErr::InvalidConversionFromU32.into()),
        }
    }
}

impl From<QueueCntrlId> for DevSqId {
    fn from(value: QueueCntrlId) -> Self {
        unsafe { core::mem::transmute(value as u8) }
    }
}

impl From<QueueCntrlId> for DevCqId {
    fn from(value: QueueCntrlId) -> Self {
        unsafe { core::mem::transmute(value as u8) }
    }
}

/// Queue Memory
#[derive(Default, Clone, Copy)]
pub struct QueueMem {
    /// Queue Base Address
    pub addr: MemoryAddr,

    /// Queue Length
    pub len: u32,
}

/// Queue controller event
pub enum QueueCntrlEvent {
    /// Queue Controller Configuration Enable status change
    StateChange(u128),

    /// Queue Controller subsystem reset pending
    NssrPending(u128),
}

/// Function Controller Interface
pub trait QueueControllerTrait: Clone {
    /// Create an instance of Queue Controller with initialization
    ///
    /// # Arguments
    ///
    /// * `id` - Queue controller Id
    fn from_id_with_init(id: QueueCntrlId) -> Self;

    /// Create an instance of Queue Controller without initialization
    ///
    /// # Arguments
    ///
    /// * `id` - Queue controller Id
    fn from_id(id: QueueCntrlId) -> Self;

    /// Get the Queue controller Id
    fn id(&self) -> QueueCntrlId;

    /// Check if the controller is enabled
    ///
    /// # Returns
    ///
    /// * `bool` - true if controller is enabled , false otherwise
    fn enabled(&self) -> bool;

    /// Check if the Queue controller is ready
    ///
    /// # Returns
    ///
    /// * `bool` - true if controller is ready , false otherwise
    fn ready(&self) -> bool;

    /// Enable the controller
    fn enable(&self);

    /// Disable the controller
    fn disable(&self);

    /// Clear the Enable status of the controller
    ///
    /// # Notes
    ///
    /// This will trigger the controller disable sequence
    fn clear_enable(&self);

    /// Set the Enable status of the controller
    ///
    /// # Notes
    ///
    /// This will trigger the controller enable sequence
    fn set_enable(&self);

    /// Reset the controller
    fn reset(&self);

    /// Create admin submission queue
    ///
    /// # Arguments
    ///
    /// * `dev_sq`  - Device submission queue Id
    /// * `host_sq` - Host submission queue Id
    ///
    /// # Returns
    ///
    /// * `McrResult<())>` - Ok or an Err
    fn create_asq(&self, dev_sq: DevSqId, host_sq: HostSqId) -> McrResult<()>;

    /// Create admin completion queue
    ///
    /// # Arguments
    ///
    /// * `dev_cq` - Device completion queue Id
    /// * `host_cq` - Host completion queue Id
    ///
    /// # Returns
    ///
    /// * `McrResult<())>` - Ok or an Err
    fn create_acq(&self, dev_cq: DevCqId, host_cq: HostCqId) -> McrResult<()>;

    /// Create Io submission queue
    ///
    /// # Arguments
    ///
    /// * `dev_sq` - Device submission queue Id
    /// * `host_sq` - Host submission queue Id
    /// * `mem` - Queue Memory
    ///
    /// # Returns
    ///
    /// * `McrResult<())>` - Ok or an Err
    fn create_sq(&self, dev_sq: DevSqId, host_sq: HostSqId, mem: QueueMem) -> McrResult<()>;

    /// Delete Io submission queue
    ///
    /// # Arguments
    ///
    /// * `dev_sq` - Device submission queue Id
    /// * `host_sq` - Host submission queue Id
    ///
    fn delete_sq(&self, dev_sq: DevSqId, host_sq: HostSqId);

    /// Create Io completion queue
    ///
    /// # Arguments
    ///
    /// * `id` - Queue controller Id
    /// * `host_cq` - Host completion queue Id
    /// * `dev_cq` - Device completion queue Id
    /// * `mem` - Queue memory
    /// * `Option<irq>` - An optional irq vector
    ///
    /// # Returns
    ///
    /// * `McrResult<())>` - Ok or an Err
    fn create_cq(
        &self,
        dev_cq: DevCqId,
        host_cq: HostCqId,
        mem: QueueMem,
        irq: Option<u16>,
    ) -> McrResult<()>;

    /// Delete Io completion queue
    ///
    /// # Arguments
    ///
    /// * `dev_cq` - Device completion queue Id
    /// * `host_cq` - Host completion queue Id
    ///
    fn delete_cq(&self, dev_cq: DevCqId, host_cq: HostCqId);

    /// Set Controller Fatal Status
    fn set_cfs(&self);

    /// Get the controller host register programming information
    ///
    /// # Returns
    ///
    /// * `ControllerLmInfo` - Current controller host register programming information
    fn host_register_info(&self) -> ControllerLmInfo;

    /// Restore the controller host register programming information
    ///
    /// # Arguments
    ///
    /// * `info` - Controller host register programming information to be restored
    fn restore_host_register_info(&self, info: &ControllerLmInfo);

    /// Get the submission queue information programmed in queue controller
    ///
    /// # Arguments
    ///
    /// * `dev_sq` - Device submission queue Id
    /// * `host_sq` - Host submission queue Id
    ///
    /// # Returns
    ///
    /// * `LmSqInfo` - Submission queue information to be stored for Live Migration
    fn sq_info(&self, dev_sq: DevSqId, host_sq: HostSqId) -> LmSqInfo;

    /// Restore the submission queue information programmed in queue controller
    ///
    /// # Arguments
    ///
    /// * `dev_sq` - Device submission queue Id
    /// * `info` - Submission queue information to be restored
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok or an Err
    fn restore_sq_info(&self, dev_sq: DevSqId, info: &LmSqInfo) -> McrResult<()>;

    /// Get the completion queue information programmed in queue controller
    ///
    /// # Arguments
    ///
    /// * `dev_cq` - Device completion queue Id
    /// * `host_cq` - Host completion queue Id
    ///
    /// # Returns
    ///
    /// * `LmCqInfo` - Completion queue information to be stored for Live Migration
    fn cq_info(&self, dev_cq: DevCqId, host_cq: HostCqId) -> LmCqInfo;

    /// Restore the completion queue information programmed in queue controller
    ///
    /// # Arguments
    ///
    /// * `dev_cq` - Device completion queue Id
    /// * `info` - Completion queue information to be restored
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok or an Err
    fn restore_cq_info(&self, dev_cq: DevCqId, info: &LmCqInfo) -> McrResult<()>;

    /// Enable the device submission queue
    ///
    /// # Arguments
    ///
    /// * `dev_sq` - Device submission queue Id
    /// * `host_sq` - Host submission queue Id
    fn enable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId);

    /// Disable the device submission queue
    ///
    /// # Arguments
    ///
    /// * `dev_sq` - Device submission queue Id
    /// * `host_sq` - Host submission queue Id
    fn disable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId);
}

// Error code definitions for Queue Controller
mcr_err_decl! {
    QueueController,
    QueueControllerErr {
        // Device submission queue reset request timed out
        DeviceSqResetRequestTimeout = 0x1,

        // Device completion queue reset request timed out
        DeviceCqResetRequestTimeout = 0x2,

        // Invalid conversion from U32 to QueueCntrlId
        InvalidConversionFromU32 = 0x3,

        // Invalid conversion from PcieFunction to QueueCntrlId
        InvalidConversionFromPcieFunction = 0x4,
    }
}
