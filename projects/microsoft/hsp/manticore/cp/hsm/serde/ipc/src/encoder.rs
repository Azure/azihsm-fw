// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]

use mcr_ipc_controller::IpcMessage;
use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

use crate::*;

/// IPC Message encoder trait
pub trait IpcMessageEncoderTrait {
    /// Encode given IPC message type to raw message. This function will return error if
    /// the size of the IPC message type is not the same as raw message.
    ///
    /// # Retruns
    ///
    /// * `IpcMessage` - Ok with encoded raw message or an appropriate error code.
    fn encode(self) -> IpcMessage;
}

/// IPC Message encoder to convert from an IPC message type to raw message
pub struct IpcMessageEncoder {}

impl IpcMessageEncoder {
    /// Encode given IPC message type to raw message. This function will return error if
    /// the size of the IPC message type is not the same as raw message.
    ///
    /// # Arguments
    ///
    /// * `message` - An IPC messsage type
    ///
    /// # Retruns
    ///
    /// * `IpcMessage` - Ok with encoded raw message or an appropriate error code.
    pub fn encode<T: IpcMessageType + IntoBytes + Immutable>(message: T) -> IpcMessage {
        let mut ipc_message = IpcMessage {
            data: [0; IPC_MESSAGE_LENGTH],
        };

        let dst = ipc_message.as_mut_bytes();
        let src = message.as_bytes();
        dst.copy_from_slice(src);

        ipc_message
    }
}
