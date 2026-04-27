// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod channel;
mod cntrl;

pub use channel::IpcEventChannel;
pub use channel::IpcMessageChannel;
pub use cntrl::IpcController;
use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
use mcr_types::VolatileCell;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

/// Ipc message length in DWORDS
pub const IPC_MESSAGE_LENGTH: usize = 16;

// Ipc Descriptor sequence macro
seq_macro::seq! {
    N in 0..32 {
        /// IpcDescriptor::Descriptor0 through IpcDescriptor::Descriptor31
        #[repr(u8)]
        #[derive(Clone, Copy, PartialEq, Eq)]
        pub enum IpcDescriptor {
            #(
                Descriptor~N = N,
            )*
        }
    }
}

impl TryFrom<u8> for IpcDescriptor {
    type Error = ();
    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            0..=31 => Ok(unsafe { core::mem::transmute::<u8, IpcDescriptor>(value) }),
            _ => Err(()),
        }
    }
}

/// IPC Interrupt Register Block
#[derive(Clone, Copy)]
pub enum IpcIntBlock {
    /// Interrupt Block 0
    IntBlock0 = 0,

    /// Interrupt Block 1
    IntBlock1 = 1,

    /// Interrupt Block 2
    IntBlock2 = 2,

    /// Interrupt Block 3
    IntBlock3 = 3,

    /// Interrupt Block 4
    IntBlock4 = 4,

    /// Interrupt Block 5
    IntBlock5 = 5,
}

impl From<IpcIntBlock> for usize {
    fn from(value: IpcIntBlock) -> usize {
        value as Self
    }
}

/// IPC Channel Identifier
#[derive(Clone, Copy)]
pub enum IpcChannelId {
    /// Request Channel from Admin to FP Io Core
    AdminToFpIoCore,

    /// Request Channel from Hsm Io Core to FP Io Core
    HsmIoCoreToFpIoCore,

    /// Request Channel from Admin to HSM Io Core
    AdminToHsmIoCore,

    /// Request Channel from Admin to HSP core
    AdminToHsp,

    /// Request Channel from HSM Io Core to HSP core
    HsmIoCoreToHsp,

    /// Request Channel from HSP to Admin core
    HspToAdmin,

    /// Request Channel from HSM Io Core to Admin core
    HsmIoCoreToAdmin,

    /// Request Channel from FP to HSM core
    FpToHsm,
}

/// IPC Queue Descriptor
#[repr(C)]
#[derive(Clone, IntoBytes, Immutable, Copy, FromBytes)]
pub struct IpcMessage {
    /// Fixed length IPCMessage data.
    pub data: [u32; IPC_MESSAGE_LENGTH],
}

/// IPC Message Queue configuration
pub struct IpcMessageQueueConfig {
    /// Queue
    pub queue: &'static mut [IpcMessage],

    /// Queue consumer index
    pub ci: &'static VolatileCell<u32>,

    /// Queue producer index
    pub pi: &'static VolatileCell<u32>,
}

/// IPC Message Channel configuration
pub struct IpcMessageChannelConfig {
    /// Interrupt Block
    pub int_block: IpcIntBlock,

    /// Transmit queue
    pub tx_queue: IpcMessageQueueConfig,

    /// Receive queue
    pub rx_queue: IpcMessageQueueConfig,

    /// Send message descriptor
    pub send_message_descriptor: IpcDescriptor,

    /// Receive message descriptor
    pub receive_message_descriptor: IpcDescriptor,
}

/// IPC Event Channel configuration
pub struct IpcEventChannelConfig {
    /// Interrupt Block
    pub int_block: IpcIntBlock,

    /// Send event mask
    pub send_event_mask: u32,

    /// Receive event mask
    pub receive_event_mask: u32,
}

/// Ipc Message Channel Trait
pub trait IpcMessageChannelTrait {
    /// Send a request message of type `IpcMessage` using this Ipc channel
    ///
    /// # Arguments
    ///
    /// * `tag` - Ipc message tag
    /// * `message` - Ipc message to be sent of type `IpcMessage`
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok() or an appropriate error code
    fn send_request(&self, tag: u16, message: IpcMessage) -> McrResult<()>;

    /// Send a response message of type `IpcMessage` using this Ipc channel
    ///
    /// # Arguments
    ///
    /// * `message` - Ipc message to be sent of type `IpcMessage`
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok() or an appropriate error code
    fn send_response(&self, message: IpcMessage) -> McrResult<()>;

    /// Retrieve the tag for pending IPC request
    fn peek_tag(&self) -> Option<u16>;

    /// Receive a message of type `IpcMessage` using this Ipc channel
    ///
    /// # Returns
    ///
    /// * `Option<IpcMessage>` - Optional message if it is available
    fn receive_message(&self) -> Option<IpcMessage>;

    /// Poll for a new message of type `IpcMessage` using this Ipc channel
    ///
    /// # Returns
    ///
    /// * `Option<IpcMessage>` - Optional message if it is available
    fn poll_message(&self) -> Option<IpcMessage>;
}

/// Ipc Event Channel Trait
pub trait IpcEventChannelTrait {
    /// Begin event
    ///
    /// # Arguments
    ///
    /// * `tag` - Ipc event tag
    /// * `event_id` - Ipc event Id of type IpcDescriptor to be used to send an event
    /// * `event` - Ipc event to be sent through this event channel
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok() or an appropriate error code
    fn begin_event(&self, tag: u16, event_id: IpcDescriptor, event: u32) -> McrResult<()>;

    /// End event
    ///
    /// # Arguments
    ///
    /// * `event_id` - Ipc event Id of type IpcDescriptor to be used to send an event
    /// * `event` - Ipc event to be sent through this event channel
    ///
    /// # Returns
    ///
    /// * `McrResult<()>` - Ok() or an appropriate error code
    fn end_event(&self, event_id: IpcDescriptor, event: u32) -> McrResult<()>;

    /// Retrieve the tag for pending IPC request
    ///
    /// # Returns
    ///
    /// * `Option<u16>` - Optional tag Id that currently owns this event channel
    fn peek_tag(&self) -> Option<u16>;

    /// Receive a event for given `IpcDescriptor` using this Ipc event channel
    ///
    /// # Arguments
    ///
    /// * `event_id` - Ipc event Id of type IpcDescriptor to be used to send an event
    ///
    /// # Returns
    ///
    /// * `Option<u32>` - Optional event received through the descriptor
    ///   appropriate error code
    fn receive_event(&self, event_id: IpcDescriptor) -> Option<u32>;
}

mcr_err_decl! {
    IpcController,
    IpcControllerErr
    {
        // Sending an Ipc event on a Ipc event channel that is not configured
        EventNotConfigured = 1,

        // Ipc Tx Message queue full
        MessageQueueFull = 2,

        // Ipc Channel busy
        ChannelBusy = 3,

        // Invalid IPC received
        InvalidIpc = 4,
    }
}
