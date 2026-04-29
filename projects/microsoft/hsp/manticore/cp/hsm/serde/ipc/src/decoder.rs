// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]

use mcr_error::McrResult;
use mcr_ipc_controller::IpcMessage;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

/// IPC Message decoder to convert raw message to an expected type
pub struct IpcMessageDecoder {}

impl IpcMessageDecoder {
    /// Decode raw IPC message to a expected type. This function will fail if the size of the
    /// raw message is not the same as the expected type.
    ///
    /// # Arguments
    ///
    /// * `ipc_message` - Raw IPC messsage
    ///
    /// # Retruns
    ///
    /// * `McrResult<T>` - Ok with decoded message of an expected type or an appropriate error
    ///   code.
    pub fn decode<T: IpcMessageType + IntoBytes + Immutable + FromBytes>(
        ipc_message: IpcMessage,
    ) -> McrResult<T> {
        let header = Self::decode_header(&ipc_message)?;
        if header.msg_op() != T::OP as u32 {
            Err(IpcMessageErr::InvalidOpcodeConversion)?
        }

        let Some(message) = T::read_from_bytes(ipc_message.as_bytes()).ok() else {
            Err(IpcMessageErr::InvalidInputMessageForDecode)?
        };

        message.validate()?;

        Ok(message)
    }

    /// Decode IPC message header from a raw IPC message
    ///
    /// # Arguments
    ///
    /// * `ipc_message` - Raw IPC message
    ///
    /// # Returns
    ///
    /// * `McrResult<IpcMessageHeader>` - Ok with Decoded message header or an appropriate error
    ///   code.
    pub fn decode_header(ipc_message: &IpcMessage) -> McrResult<IpcMessageHeader> {
        let Some(header) = IpcMessageHeader::read_from_bytes(ipc_message.data[0].as_bytes()).ok()
        else {
            Err(IpcMessageErr::InvalidMessageHeaderDecode)?
        };

        Ok(header)
    }
}
