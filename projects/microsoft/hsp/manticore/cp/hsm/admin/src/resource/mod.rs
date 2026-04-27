// Copyright (c) Microsoft Corporation. All rights reserved.

mod cast_idle;
mod doe_idle;
mod ipc_channel;
mod tdisp_idle;

pub(crate) use cast_idle::CastIdle;
pub(crate) use doe_idle::DoeIdle;
pub(crate) use ipc_channel::AdminToFpIpcChannel;
pub(crate) use ipc_channel::HsmIpcChannel;
pub(crate) use ipc_channel::HspIpcChannel;
pub(crate) use tdisp_idle::TdispIdle;

#[allow(clippy::enum_variant_names)]
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub(crate) enum AdminFsmResourceId {
    AdminToFpIpcChannel,
    HsmIpcChannel,
    HspIpcChannel,
    CastIdle,
    DoeIdle,
    TdispIdle,
}
