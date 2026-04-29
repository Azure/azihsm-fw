// Copyright (c) Microsoft Corporation. All rights reserved.

use core::ops::Deref;

use mcr_ipc_controller::IpcMessageChannelTrait;

use super::HsmFsmResourceId;
use crate::{cmd_scheduler::CmdResourceInfo, HsmFsmEvent};

/// FP IPC message channel resource
pub(crate) struct FpIpcChannelResource<T: IpcMessageChannelTrait> {
    /// Resource object
    res: T,

    /// Resource count
    count: usize,
}

impl<T: IpcMessageChannelTrait> CmdResourceInfo for FpIpcChannelResource<T> {
    type Id = HsmFsmResourceId;
    type Resource = T;
    type Event = HsmFsmEvent;
    type Context = ();

    /// Resource ID
    fn id(&self) -> Self::Id {
        HsmFsmResourceId::FpIpcChannel
    }

    /// Get the current resource count
    fn count(&self) -> usize {
        self.count
    }

    fn set(&mut self, _ctx: Self::Context) -> Option<usize> {
        self.count -= 1;
        debug_assert!(self.count == 0);

        Some(0)
    }

    fn clear(&mut self, _idx: usize) {
        self.count += 1;
        debug_assert!(self.count == 1);
    }

    fn resource(&self, _idx: usize) -> &Self::Resource {
        &self.res
    }

    fn find_ctx<F>(&self, _predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        None
    }
}

impl<T: IpcMessageChannelTrait> FpIpcChannelResource<T> {
    pub fn new(res: T) -> Self {
        Self { res, count: 1 }
    }
}

impl<T: IpcMessageChannelTrait> Deref for FpIpcChannelResource<T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.res
    }
}

/// HSP IPC message channel resource
pub(crate) struct HspIpcChannelResource<T: IpcMessageChannelTrait> {
    /// Resource object
    res: T,

    /// Resource count
    count: usize,
}

impl<T: IpcMessageChannelTrait> CmdResourceInfo for HspIpcChannelResource<T> {
    type Id = HsmFsmResourceId;
    type Resource = T;
    type Event = HsmFsmEvent;
    type Context = ();

    /// Resource ID
    fn id(&self) -> Self::Id {
        HsmFsmResourceId::HspIpcChannel
    }

    /// Get the current resource count
    fn count(&self) -> usize {
        self.count
    }

    fn set(&mut self, _ctx: Self::Context) -> Option<usize> {
        self.count -= 1;
        debug_assert!(self.count == 0);

        Some(0)
    }

    fn clear(&mut self, _idx: usize) {
        self.count += 1;
        debug_assert!(self.count == 1);
    }

    fn resource(&self, _idx: usize) -> &Self::Resource {
        &self.res
    }

    fn find_ctx<F>(&self, _predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        None
    }
}

impl<T: IpcMessageChannelTrait> HspIpcChannelResource<T> {
    pub fn new(res: T) -> Self {
        Self { res, count: 1 }
    }
}

impl<T: IpcMessageChannelTrait> Deref for HspIpcChannelResource<T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.res
    }
}

/// Admin IPC message channel resource
pub(crate) struct HsmToAdminIpcChannelResource<T: IpcMessageChannelTrait> {
    /// Resource object
    res: T,

    /// Resource count
    count: usize,
}

impl<T: IpcMessageChannelTrait> CmdResourceInfo for HsmToAdminIpcChannelResource<T> {
    type Id = HsmFsmResourceId;
    type Resource = T;
    type Event = HsmFsmEvent;
    type Context = ();

    /// Resource ID
    fn id(&self) -> Self::Id {
        HsmFsmResourceId::HsmToAdminIpcChannel
    }

    /// Get the current resource count
    fn count(&self) -> usize {
        self.count
    }

    fn set(&mut self, _ctx: Self::Context) -> Option<usize> {
        self.count -= 1;
        debug_assert!(self.count == 0);

        Some(0)
    }

    fn clear(&mut self, _idx: usize) {
        self.count += 1;
        debug_assert!(self.count == 1);
    }

    fn resource(&self, _idx: usize) -> &Self::Resource {
        &self.res
    }

    fn find_ctx<F>(&self, _predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        None
    }
}

impl<T: IpcMessageChannelTrait> HsmToAdminIpcChannelResource<T> {
    pub fn new(res: T) -> Self {
        Self { res, count: 1 }
    }
}

impl<T: IpcMessageChannelTrait> Deref for HsmToAdminIpcChannelResource<T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.res
    }
}

#[cfg(test)]
mod tests {
    use mcr_error::McrResult;
    use mcr_ipc_controller::*;

    use super::*;

    struct TestResource {}
    impl IpcMessageChannelTrait for TestResource {
        fn send_request(&self, _tag: u16, _message: IpcMessage) -> McrResult<()> {
            Ok(())
        }

        fn send_response(&self, _message: IpcMessage) -> McrResult<()> {
            Ok(())
        }

        fn peek_tag(&self) -> Option<u16> {
            None
        }

        fn receive_message(&self) -> Option<IpcMessage> {
            None
        }

        fn poll_message(&self) -> Option<IpcMessage> {
            None
        }
    }
    #[test]
    fn test_aes_new() {
        let test_resource = super::FpIpcChannelResource::new(TestResource {});
        assert!(test_resource.id() == HsmFsmResourceId::FpIpcChannel);
        assert_eq!(test_resource.count(), 1);
        assert_eq!(test_resource.peek_tag(), None);
    }

    #[test]
    fn test_hsp_ipc_new() {
        let test_resource = super::HspIpcChannelResource::new(TestResource {});
        assert!(test_resource.id() == HsmFsmResourceId::HspIpcChannel);
        assert_eq!(test_resource.count(), 1);
        assert_eq!(test_resource.peek_tag(), None);
    }
}
