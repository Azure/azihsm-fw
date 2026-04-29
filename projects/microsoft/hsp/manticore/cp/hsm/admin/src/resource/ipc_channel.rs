// Copyright (c) Microsoft Corporation. All rights reserved.

use core::ops::Deref;

use mcr_ipc_controller::IpcMessageChannelTrait;

use super::AdminFsmResourceId;
use crate::CmdResourceInfo;

/// FP IPC Channel resource
pub(crate) struct AdminToFpIpcChannel<T: IpcMessageChannelTrait>(T);

impl<T: IpcMessageChannelTrait> CmdResourceInfo for AdminToFpIpcChannel<T> {
    type Id = AdminFsmResourceId;
    type Resource = T;
    type Context = ();

    /// Resource ID
    fn id(&self) -> Self::Id {
        AdminFsmResourceId::AdminToFpIpcChannel
    }

    /// Maximum number of commands
    fn max_count(&self) -> usize {
        const MAX_COUNT: usize = 1;
        MAX_COUNT
    }

    fn set(&mut self, _ctx: Self::Context) -> Option<usize> {
        Some(0)
    }

    fn clear(&mut self, _idx: usize) {}

    fn resource(&self, _idx: usize) -> &Self::Resource {
        &self.0
    }

    fn find_ctx<F>(&self, _predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        None
    }
}

impl<T: IpcMessageChannelTrait> AdminToFpIpcChannel<T> {
    pub fn new(value: T) -> Self {
        Self(value)
    }
}

impl<T: IpcMessageChannelTrait> Deref for AdminToFpIpcChannel<T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

/// HSM IPC Channel resource
pub(crate) struct HsmIpcChannel<T: IpcMessageChannelTrait>(T);

impl<T: IpcMessageChannelTrait> CmdResourceInfo for HsmIpcChannel<T> {
    type Id = AdminFsmResourceId;
    type Resource = T;
    type Context = ();

    /// Resource ID
    fn id(&self) -> Self::Id {
        AdminFsmResourceId::HsmIpcChannel
    }

    /// Maximum number of commands
    fn max_count(&self) -> usize {
        const MAX_COUNT: usize = 1;
        MAX_COUNT
    }

    fn set(&mut self, _ctx: Self::Context) -> Option<usize> {
        Some(0)
    }

    fn clear(&mut self, _idx: usize) {}

    fn resource(&self, _idx: usize) -> &Self::Resource {
        &self.0
    }

    fn find_ctx<F>(&self, _predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        None
    }
}

impl<T: IpcMessageChannelTrait> HsmIpcChannel<T> {
    pub fn new(value: T) -> Self {
        Self(value)
    }
}

impl<T: IpcMessageChannelTrait> Deref for HsmIpcChannel<T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

/// HSP IPC Channel resource
pub(crate) struct HspIpcChannel<T: IpcMessageChannelTrait>(T);

impl<T: IpcMessageChannelTrait> CmdResourceInfo for HspIpcChannel<T> {
    type Id = AdminFsmResourceId;
    type Resource = T;
    type Context = ();

    /// Resource ID
    fn id(&self) -> Self::Id {
        AdminFsmResourceId::HspIpcChannel
    }

    /// Maximum number of commands
    fn max_count(&self) -> usize {
        const MAX_COUNT: usize = 1;
        MAX_COUNT
    }

    fn set(&mut self, _ctx: Self::Context) -> Option<usize> {
        Some(0)
    }

    fn clear(&mut self, _idx: usize) {}

    fn resource(&self, _idx: usize) -> &Self::Resource {
        &self.0
    }

    fn find_ctx<F>(&self, _predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        None
    }
}

impl<T: IpcMessageChannelTrait> HspIpcChannel<T> {
    pub fn new(value: T) -> Self {
        Self(value)
    }
}

impl<T: IpcMessageChannelTrait> Deref for HspIpcChannel<T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}
