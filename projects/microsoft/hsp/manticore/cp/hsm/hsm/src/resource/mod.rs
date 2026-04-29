// Copyright (c) Microsoft Corporation. All rights reserved.

mod ipc;
mod pka;

pub(crate) use ipc::FpIpcChannelResource;
pub(crate) use ipc::HsmToAdminIpcChannelResource;
pub(crate) use ipc::HspIpcChannelResource;
pub(crate) use pka::PkaResource;

/// Resource IDs supported by HSM FSM.
#[derive(Clone, Copy, Eq, PartialEq)]
pub(crate) enum HsmFsmResourceId {
    Pka,
    FpIpcChannel,
    HspIpcChannel,
    HsmToAdminIpcChannel,
}
