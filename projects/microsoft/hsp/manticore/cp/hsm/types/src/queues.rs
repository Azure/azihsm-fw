// Copyright (c) Microsoft Corporation. All rights reserved.

use open_enum::open_enum;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::IoChannelId;
use crate::PcieFunction;

/// Max HSM IO queues
pub const HSM_IO_QUEUE_SIZE: usize = 65;

/// HSM IO queues start from index 65.
pub const HSM_IO_QUEUE_BASE: usize = 65;

/// Valid host HSM queue start ID
pub const HSM_QUEUE_START_ID: u16 = 1;

/// Valid host HSM queue end ID
pub const HSM_QUEUE_END_ID: u16 = 255;

// Host Admin queue ID for each function or partition
const ADMIN_QUEUE_ID: u16 = 0;

/// FPS 65th resource hi priority device queue ID
const FP_65TH_RESOURCE_HI_PRIO_DEV_QUEUE: u8 = 64;

/// FPS 65th resource low priority device queue ID
const FP_65TH_RESOURCE_LO_PRIO_DEV_QUEUE: u8 = 129;

/// Queue Group
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum QueueGroup {
    /// Group 0
    Group0 = 0,

    /// Group 1
    Group1 = 1,
}

seq_macro::seq! {
    N in 0..511 {
        /// Host Submission Queue Ids
        #[repr(u16)]
        #[open_enum]
        #[derive(Clone, Copy, PartialOrd, PartialEq, Eq, FromBytes, IntoBytes, Immutable)]
        pub enum HostSqId {
            #(
                Id~N = N,
            )*
        }
    }
}

impl Default for HostSqId {
    fn default() -> Self {
        HostSqId::Id0
    }
}

impl HostSqId {
    pub fn credit(self) -> u32 {
        match self.into() {
            ADMIN_QUEUE_ID => 1,
            HSM_QUEUE_START_ID..=HSM_QUEUE_END_ID => 1,
            _ => 4,
        }
    }
}

impl From<HostSqId> for u16 {
    fn from(value: HostSqId) -> Self {
        value.0 as Self
    }
}

impl From<HostSqId> for u32 {
    fn from(value: HostSqId) -> Self {
        value.0 as Self
    }
}

impl From<HostSqId> for usize {
    fn from(value: HostSqId) -> Self {
        value.0 as Self
    }
}

impl From<HostCqId> for HostSqId {
    fn from(value: HostCqId) -> Self {
        unsafe { core::mem::transmute(value.0) }
    }
}

impl HostSqId {
    pub fn queue_group_and_channel(&self, dev_sq_id: DevSqId) -> (QueueGroup, IoChannelId) {
        match (*self).into() {
            ADMIN_QUEUE_ID => (QueueGroup::Group0, IoChannelId::Channel2),
            HSM_QUEUE_START_ID..=HSM_QUEUE_END_ID => (QueueGroup::Group0, IoChannelId::Channel3),
            _ => match dev_sq_id.into() {
                FP_65TH_RESOURCE_HI_PRIO_DEV_QUEUE | FP_65TH_RESOURCE_LO_PRIO_DEV_QUEUE => {
                    (QueueGroup::Group1, IoChannelId::Channel1)
                }
                _ => (QueueGroup::Group1, IoChannelId::Channel0),
            },
        }
    }
}

// Host Completion Queue Ids sequence macro
seq_macro::seq! {
    N in 0..511 {
        /// Host Completion Queue Ids
        #[repr(u16)]
        #[open_enum]
        #[derive(Clone, Copy, PartialOrd, PartialEq, Eq, FromBytes, IntoBytes, Immutable)]
        pub enum HostCqId {
            #(
                Id~N = N,
            )*
        }
    }
}

impl Default for HostCqId {
    fn default() -> Self {
        HostCqId::Id0
    }
}

impl From<HostCqId> for u16 {
    fn from(value: HostCqId) -> Self {
        value.0 as Self
    }
}

impl From<HostCqId> for u32 {
    fn from(value: HostCqId) -> Self {
        value.0 as Self
    }
}

impl From<HostCqId> for usize {
    fn from(value: HostCqId) -> Self {
        value.0 as Self
    }
}

impl HostCqId {
    pub fn queue_group_and_channel(&self, dev_cq_id: DevCqId) -> (QueueGroup, IoChannelId) {
        match (*self).into() {
            ADMIN_QUEUE_ID => (QueueGroup::Group0, IoChannelId::Channel2),
            HSM_QUEUE_START_ID..=HSM_QUEUE_END_ID => (QueueGroup::Group0, IoChannelId::Channel3),
            _ => match dev_cq_id.into() {
                FP_65TH_RESOURCE_HI_PRIO_DEV_QUEUE | FP_65TH_RESOURCE_LO_PRIO_DEV_QUEUE => {
                    (QueueGroup::Group1, IoChannelId::Channel1)
                }
                _ => (QueueGroup::Group1, IoChannelId::Channel0),
            },
        }
    }
}

seq_macro::seq! {
    N in 0..130 {
        /// Device Submission Queue Ids
        #[repr(u8)]
        #[open_enum]
        #[derive(Clone, Copy, PartialEq, Eq, FromBytes, IntoBytes, Immutable, Default)]
        pub enum DevSqId {
            #(
                Id~N = N,
            )*
        }
    }
}

impl From<DevSqId> for u8 {
    fn from(value: DevSqId) -> Self {
        value.0
    }
}

impl From<DevSqId> for u16 {
    fn from(value: DevSqId) -> Self {
        value.0 as Self
    }
}

impl From<DevSqId> for usize {
    fn from(value: DevSqId) -> Self {
        value.0 as Self
    }
}

impl From<u8> for DevSqId {
    fn from(value: u8) -> Self {
        unsafe { core::mem::transmute(value) }
    }
}

seq_macro::seq! {
    N in 0..130 {
        /// Device Completion Queue Ids
        #[repr(u8)]
        #[open_enum]
        #[derive(Clone, Copy, PartialEq, Eq, FromBytes, IntoBytes, Immutable, Default)]
        pub enum DevCqId {
            #(
                Id~N = N,
            )*
        }
    }
}

impl From<DevCqId> for u8 {
    fn from(value: DevCqId) -> Self {
        value.0
    }
}

impl From<DevCqId> for usize {
    fn from(value: DevCqId) -> Self {
        value.0 as Self
    }
}

impl From<u8> for DevCqId {
    fn from(value: u8) -> Self {
        unsafe { core::mem::transmute(value) }
    }
}

impl From<DevSqId> for DevCqId {
    fn from(value: DevSqId) -> Self {
        unsafe { core::mem::transmute(value.0) }
    }
}

/// Host Queue type
#[derive(PartialEq, Eq)]
pub enum HostQueueType {
    // Admin Queue
    Admin,

    /// Hsm Io Queue
    Hsm,

    /// Fastpath Io Queue
    Fp,
}

impl From<HostSqId> for HostQueueType {
    fn from(value: HostSqId) -> Self {
        match value.into() {
            ADMIN_QUEUE_ID => HostQueueType::Admin,
            HSM_QUEUE_START_ID..=HSM_QUEUE_END_ID => HostQueueType::Hsm,
            _ => HostQueueType::Fp,
        }
    }
}

impl From<HostCqId> for HostQueueType {
    fn from(value: HostCqId) -> Self {
        match value.into() {
            ADMIN_QUEUE_ID => HostQueueType::Admin,
            HSM_QUEUE_START_ID..=HSM_QUEUE_END_ID => HostQueueType::Hsm,
            _ => HostQueueType::Fp,
        }
    }
}

/// Host submission queue priority
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum HostQueuePriority {
    /// High priority queue
    High = 1,

    /// Low priority queue
    Low = 3,
}

impl From<HostQueuePriority> for u16 {
    fn from(value: HostQueuePriority) -> Self {
        value as u16
    }
}

/// Queue Delete Request and Response
#[repr(C)]
#[derive(Copy, Clone)]
pub struct QueueDeleteResponse {
    /// Originator's Tag ID the request or response belongs to
    pub tag: u16,

    /// PCIe function the queue belongs to
    pub pfn: PcieFunction,

    /// Reserved
    pub _rsvd: u8,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_devicesqid_from_devicesqid_for_usize() {
        assert_eq!(usize::from(DevSqId::Id0), 0usize);
    }

    #[test]
    fn test_devicecqid_from_devicecqid_for_usize() {
        assert_eq!(usize::from(DevCqId::Id0), 0usize);
    }

    #[test]
    fn test_hostcqid_from_hostcqid_for_u16() {
        assert_eq!(u16::from(HostCqId::Id1), 1u16);
    }

    #[test]
    fn test_hostsqid_from_hostsqid_for_u16() {
        assert_eq!(u16::from(HostSqId::Id1), 1u16);
    }

    #[test]
    fn test_host_cqid_to_group_and_channel_tuple() {
        let (group, channel): (QueueGroup, IoChannelId) =
            HostCqId::Id0.queue_group_and_channel(DevCqId::Id0);
        assert_eq!(group as usize, QueueGroup::Group0 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel2.into());

        let (group, channel): (QueueGroup, IoChannelId) =
            HostCqId::Id1.queue_group_and_channel(DevCqId::Id65);
        assert_eq!(group as usize, QueueGroup::Group0 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel3.into());

        let (group, channel): (QueueGroup, IoChannelId) =
            HostCqId::Id255.queue_group_and_channel(DevCqId::Id101);
        assert_eq!(group as usize, QueueGroup::Group0 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel3.into());

        let (group, channel): (QueueGroup, IoChannelId) =
            HostCqId::Id256.queue_group_and_channel(DevCqId::Id0);
        assert_eq!(group as usize, QueueGroup::Group1 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel0.into());

        let (group, channel): (QueueGroup, IoChannelId) =
            HostCqId::Id380.queue_group_and_channel(DevCqId::Id64);
        assert_eq!(group as usize, QueueGroup::Group1 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel1.into());

        let (group, channel): (QueueGroup, IoChannelId) =
            HostCqId::Id280.queue_group_and_channel(DevCqId::Id129);
        assert_eq!(group as usize, QueueGroup::Group1 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel1.into());
    }

    #[test]
    fn test_host_cqid_for_u32() {
        assert_eq!(u32::from(HostCqId::Id0), 0u32);
        assert_eq!(u32::from(HostCqId::Id1), 1u32);
        assert_eq!(u32::from(HostCqId::Id255), 255u32);
        assert_eq!(u32::from(HostCqId::Id256), 256u32);
        assert_eq!(u32::from(HostCqId::Id380), 380u32);
    }

    #[test]
    fn test_host_sqid_to_group_and_channel_tuple() {
        let (group, channel): (QueueGroup, IoChannelId) =
            HostSqId::Id0.queue_group_and_channel(DevSqId::Id0);
        assert_eq!(group as usize, QueueGroup::Group0 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel2.into());

        let (group, channel): (QueueGroup, IoChannelId) =
            HostSqId::Id1.queue_group_and_channel(DevSqId::Id65);
        assert_eq!(group as usize, QueueGroup::Group0 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel3.into());

        let (group, channel): (QueueGroup, IoChannelId) =
            HostSqId::Id255.queue_group_and_channel(DevSqId::Id66);
        assert_eq!(group as usize, QueueGroup::Group0 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel3.into());

        let (group, channel): (QueueGroup, IoChannelId) =
            HostSqId::Id256.queue_group_and_channel(DevSqId::Id0);
        assert_eq!(group as usize, QueueGroup::Group1 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel0.into());

        let (group, channel): (QueueGroup, IoChannelId) =
            HostSqId::Id380.queue_group_and_channel(DevSqId::Id64);
        assert_eq!(group as usize, QueueGroup::Group1 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel1.into());

        let (group, channel): (QueueGroup, IoChannelId) =
            HostSqId::Id270.queue_group_and_channel(DevSqId::Id129);
        assert_eq!(group as usize, QueueGroup::Group1 as usize);
        let channel_id: usize = channel.into();
        assert_eq!(channel_id, IoChannelId::Channel1.into());
    }

    #[test]
    fn test_host_sqid_for_u32() {
        assert_eq!(u32::from(HostSqId::Id0), 0u32);
        assert_eq!(u32::from(HostSqId::Id1), 1u32);
        assert_eq!(u32::from(HostSqId::Id255), 255u32);
        assert_eq!(u32::from(HostSqId::Id256), 256u32);
        assert_eq!(u32::from(HostSqId::Id380), 380u32);
    }

    #[test]
    fn test_host_sqid_to_io_flow_control_credit() {
        assert_eq!(HostSqId::Id0.credit(), 1);
        assert_eq!(HostSqId::Id1.credit(), 1);
        assert_eq!(HostSqId::Id255.credit(), 1);
        assert_eq!(HostSqId::Id256.credit(), 4);
        assert_eq!(HostSqId::Id380.credit(), 4);
    }

    #[test]
    fn test_hostsqid_to_host_queue_type() {
        assert!(matches!(
            HostQueueType::from(HostSqId::Id0),
            HostQueueType::Admin
        ));
        assert!(matches!(
            HostQueueType::from(HostSqId::Id1),
            HostQueueType::Hsm
        ));
        assert!(matches!(
            HostQueueType::from(HostSqId::Id255),
            HostQueueType::Hsm
        ));
        assert!(matches!(
            HostQueueType::from(HostSqId::Id256),
            HostQueueType::Fp
        ));
        assert!(matches!(
            HostQueueType::from(HostSqId::Id380),
            HostQueueType::Fp
        ));
    }

    #[test]
    fn test_hostcqid_to_host_queue_type() {
        assert!(matches!(
            HostQueueType::from(HostCqId::Id0),
            HostQueueType::Admin
        ));
        assert!(matches!(
            HostQueueType::from(HostCqId::Id1),
            HostQueueType::Hsm
        ));
        assert!(matches!(
            HostQueueType::from(HostCqId::Id255),
            HostQueueType::Hsm
        ));
        assert!(matches!(
            HostQueueType::from(HostCqId::Id256),
            HostQueueType::Fp
        ));
        assert!(matches!(
            HostQueueType::from(HostCqId::Id380),
            HostQueueType::Fp
        ));
    }
}
