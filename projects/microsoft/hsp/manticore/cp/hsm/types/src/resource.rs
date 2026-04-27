// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use alloc::vec::Vec;
use bitfield::Bit;
use bitfield::BitMut;

use crate::*;

pub const MAX_FUNCTION_RESOURCES: u8 = 65;

const HSM_HOST_QUEUE_START_ID: u16 = 1;
const HSM_HOST_QUEUE_END_ID: u16 = MAX_FUNCTION_RESOURCES as u16;
const FP_HOST_QUEUE_START_ID: u16 = 256;
const FP_HOST_QUEUE_END_ID: u16 = FP_HOST_QUEUE_START_ID + (MAX_FUNCTION_RESOURCES * 2) as u16 - 1;

/// Function Resource
#[repr(C)]
#[derive(Clone, Default)]
pub struct Resource {
    /// Pcie Function Id
    pub pfn: Option<PcieFunction>,

    /// Allocation mask
    pub alloc_map: u8,

    /// Resource ID of this resource
    pub id: u8,
}

impl Resource {
    /// Reset the resource
    pub fn reset(&mut self) {
        self.alloc_map = Default::default();
    }

    /// Allocate a completion queue from this resource
    ///
    /// # Arguments
    ///
    /// * `host_cq` - Host completion queue id
    ///
    /// # Returns
    ///
    /// * Device completion queue id
    pub fn alloc_cq(&mut self, queue_id: HostCqId) -> Option<DevCqId> {
        if self.alloc_map.bit(queue_id.offset()) {
            None
        } else {
            self.alloc_map.set_bit(queue_id.offset(), true);
            Some(queue_id.device_id(self.id))
        }
    }

    /// Free a completion queue from this resource
    ///
    /// # Arguments
    ///
    /// * `host_cq` - Host completion queue id
    ///
    /// # Returns
    ///
    /// * Device completion queue id
    pub fn free_cq(&mut self, host_cq: HostCqId) -> Option<DevCqId> {
        if self.alloc_map.bit(host_cq.offset()) {
            self.alloc_map.set_bit(host_cq.offset(), false);
            Some(host_cq.device_id(self.id))
        } else {
            None
        }
    }

    /// Get the first enabled host completion queue id
    ///
    /// # Arguments
    ///
    /// * `res_index` - Resource index belong to the function
    ///
    /// # Returns
    ///
    /// * `Option<HostCqId>` - Host completion queue id
    pub fn host_cq(&self, res_index: usize) -> Option<HostCqId> {
        for offset in HostCqId::OFFSET..HostCqId::OFFSET + HostCqId::LEN {
            if self.alloc_map.bit(offset) {
                return Some(HostCqId(HostCqId::host_queue_id(res_index, offset)));
            }
        }

        None
    }

    /// Get the list of all host completion queues that are enabled in this resource
    ///
    /// # Arguments
    ///
    /// * `res_index` - Resource index belong to the function
    ///
    /// # Returns
    ///
    /// * `Vec<HostCqId>` - List of Host completion queue ids enabled in this resource
    pub fn host_cq_list(&self, res_index: usize) -> Vec<HostCqId> {
        let mut host_cq_list = Vec::new();

        for offset in HostCqId::OFFSET..HostCqId::OFFSET + HostCqId::LEN {
            if self.alloc_map.bit(offset) {
                host_cq_list.push(HostCqId(HostCqId::host_queue_id(res_index, offset)));
            }
        }

        host_cq_list
    }

    /// Allocate a submission queue from this resource
    ///
    /// # Arguments
    ///
    /// * `host_sq` - Host submission queue id
    ///
    /// # Returns
    ///
    /// * Device submission queue id
    pub fn alloc_sq(&mut self, host_sq: HostSqId) -> Option<DevSqId> {
        if self.alloc_map.bit(host_sq.offset()) {
            None
        } else {
            self.alloc_map.set_bit(host_sq.offset(), true);
            Some(host_sq.device_id(self.id))
        }
    }

    /// Free a submission queue from this resource
    ///
    /// # Arguments
    ///
    /// * `host_sq` - Host submission queue id
    ///
    /// # Returns
    ///
    /// * Device submission queue id
    pub fn free_sq(&mut self, host_sq: HostSqId) -> Option<DevSqId> {
        if self.alloc_map.bit(host_sq.offset()) {
            self.alloc_map.set_bit(host_sq.offset(), false);
            Some(host_sq.device_id(self.id))
        } else {
            None
        }
    }

    /// Get the first enabled host submission queue id
    ///
    /// # Arguments
    ///
    /// * `res_index` - Resource index belong to the function
    ///
    /// # Returns
    ///
    /// * `Option<HostSqId>` - Host submission queue id
    pub fn host_sq(&self, res_index: usize) -> Option<HostSqId> {
        for offset in HostSqId::OFFSET..HostSqId::OFFSET + HostSqId::LEN {
            if self.alloc_map.bit(offset) {
                return Some(HostSqId(HostSqId::host_queue_id(res_index, offset)));
            }
        }

        None
    }

    /// Get the list of all host submission queues that are enabled in this resource
    ///
    /// # Arguments
    ///
    /// * `res_index` - Resource index belong to the function
    ///
    /// # Returns
    ///
    /// * `Vec<HostSqId>` - List of Host submission queue ids enabled in this resource
    pub fn host_sq_list(&self, res_index: usize) -> Vec<HostSqId> {
        let mut host_sq_list = Vec::new();

        for offset in HostSqId::OFFSET..HostSqId::OFFSET + HostSqId::LEN {
            if self.alloc_map.bit(offset) {
                host_sq_list.push(HostSqId(HostSqId::host_queue_id(res_index, offset)));
            }
        }

        host_sq_list
    }

    /// Get the device submission queue id from host submission queue id
    ///
    /// # Arguments
    ///
    /// * `host_sq` - Host submission queue id
    ///
    /// # Returns
    ///
    /// * `Option<DevSqId>` - Device submission queue id
    pub fn dev_sq(&self, host_sq: HostSqId) -> Option<DevSqId> {
        if self.alloc_map.bit(host_sq.offset()) {
            Some(host_sq.device_id(self.id))
        } else {
            None
        }
    }

    /// Get the device completion queue id from host completion queue id
    ///
    /// # Arguments
    ///
    /// * `host_cq` - Host submission queue id
    ///
    /// # Returns
    ///
    /// * `Option<DevCqId>` - Device completion queue id
    pub fn dev_cq(&self, host_cq: HostCqId) -> Option<DevCqId> {
        if self.alloc_map.bit(host_cq.offset()) {
            Some(host_cq.device_id(self.id))
        } else {
            None
        }
    }

    /// Check if the host queue is enabled
    ///
    /// # Arguments
    ///
    /// * `queue_id` - Host queue id
    ///
    /// # Returns
    ///
    /// * true if enabled, false otherwise
    pub fn enabled<T: FunctionResourceLayout>(&self, queue_id: T) -> bool {
        self.alloc_map.bit(queue_id.offset())
    }

    /// Get byte representation of the resource PFN
    ///
    /// # Returns
    ///
    /// * `Vec<u8>` - Vector containing series of bytes representing the resource PFN
    pub fn owner_pfn_as_bytes(&self) -> Vec<u8> {
        let mut bytes = Vec::new();

        match self.pfn {
            Some(value) => {
                bytes.push(1);
                bytes.push(value.0);
            }
            None => {
                bytes.push(0);
                bytes.push(0);
            }
        }

        bytes
    }
}

pub trait FunctionResourceLayout: Copy + Into<u16> + Into<usize> + Into<HostQueueType> {
    // The offset into the queue index
    const OFFSET: usize;

    // Number of queues per resource
    const LEN: usize = 3;

    // Device queue ID type
    type DevQueueId: From<u8>;

    /// Return the offset into queue index from queue type
    ///
    /// # Returns
    ///
    /// * `usize` - offset into the queue index
    fn offset(self) -> usize {
        match self.into() {
            HostQueueType::Hsm => Self::OFFSET,
            HostQueueType::Fp => {
                if <Self as Into<u16>>::into(self) % 2 == 0 {
                    Self::OFFSET + 1
                } else {
                    Self::OFFSET + 2
                }
            }
            HostQueueType::Admin => unreachable!(),
        }
    }

    /// Returns the device queue Id from host queue Id
    ///
    /// # Arguments
    ///
    /// * `resource_id` - Queue resource Id this queue belongs to
    ///
    /// # Returns
    ///
    /// * ` (QueueGroup, Self::DeviceQueueId)` - A tuple of queue group Id and device queue Id
    fn device_id(self, resource_id: u8) -> Self::DevQueueId {
        match self.into() {
            HostQueueType::Hsm => (resource_id + MAX_FUNCTION_RESOURCES).into(),
            HostQueueType::Fp => {
                if <Self as Into<u16>>::into(self) % 2 == 0 {
                    resource_id.into()
                } else {
                    (resource_id + MAX_FUNCTION_RESOURCES).into()
                }
            }
            HostQueueType::Admin => unreachable!(),
        }
    }

    /// Get the resource index
    ///
    /// # Returns
    ///
    /// * Resource index
    fn res_index(self) -> Option<usize> {
        match self.into() {
            HostQueueType::Hsm => {
                // HSM queue Ids start from 1 and ends at 65
                if HSM_HOST_QUEUE_START_ID <= self.into() && HSM_HOST_QUEUE_END_ID >= self.into() {
                    Some(<Self as Into<usize>>::into(self) - HSM_HOST_QUEUE_START_ID as usize)
                } else {
                    None
                }
            }
            HostQueueType::Fp => {
                // FP queue Ids start from 256 and ends at 385 for 65 resources with 2 queues per
                // resource
                if FP_HOST_QUEUE_START_ID <= self.into() && FP_HOST_QUEUE_END_ID >= self.into() {
                    Some((<Self as Into<usize>>::into(self) - FP_HOST_QUEUE_START_ID as usize) / 2)
                } else {
                    None
                }
            }
            HostQueueType::Admin => unreachable!(),
        }
    }

    /// Get the host queue id
    ///
    /// # Arguments
    ///
    /// * `res_index` - Resource index
    /// * `offset` - Offset into the queue resource bit map
    ///
    /// # Returns
    ///
    /// * Host queue id
    fn host_queue_id(res_index: usize, offset: usize) -> u16 {
        match offset {
            0 | 3 => HSM_HOST_QUEUE_START_ID + res_index as u16,
            1 | 4 => FP_HOST_QUEUE_START_ID + res_index as u16 * 2,
            2 | 5 => FP_HOST_QUEUE_START_ID + 1 + res_index as u16 * 2,
            _ => unreachable!(),
        }
    }
}

impl FunctionResourceLayout for HostSqId {
    const OFFSET: usize = 0;

    type DevQueueId = DevSqId;
}

impl FunctionResourceLayout for HostCqId {
    const OFFSET: usize = 3;

    type DevQueueId = DevCqId;
}

#[cfg(test)]
mod tests {
    use alloc::vec;

    use super::*;

    #[test]
    fn test_resource_owner_pfn_as_bytes_with_pfn() {
        let expected = vec![1, 5];
        let resource = Resource {
            pfn: Some(PcieFunction::Vf5),
            alloc_map: 4,
            id: 9,
        };

        let actual = resource.owner_pfn_as_bytes();
        assert_eq!(expected, actual);
    }

    #[test]
    fn test_resource_owner_pfn_as_bytes_without_pfn() {
        let expected = vec![0, 0];
        let resource = Resource {
            pfn: None,
            alloc_map: 0,
            id: 0,
        };

        let actual = resource.owner_pfn_as_bytes();
        assert_eq!(expected, actual);
    }
}
