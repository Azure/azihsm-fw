// Copyright (c) Microsoft Corporation. All rights reserved.

use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

/// Ucd query
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct UcdQueryInfo {
    /// Queue length
    pub queue_len: u32,

    /// Io Receive Free List Base Address
    pub rx_free_list_addr: u32,

    /// Io Transmit Free List Base Address
    pub tx_free_list_addr: u32,

    /// Rx Queue Base Address
    pub rx_queue_addr: u32,

    /// Tx Queue Base Address
    pub tx_queue_addr: u32,

    /// Rx Entry Base Address
    pub rx_entry_addr: u32,

    /// Receive Queue Producer Index Address
    pub rx_pi_addr: u32,

    /// Transmit Queue Producer Index Address
    pub tx_pi_addr: u32,

    /// Io Controller Id
    pub ctrl_id: IoControllerId,

    /// Io Channel Id
    pub channel_id: IoChannelId,

    /// Reserved padding
    pub _rsvd: u16,
}

impl Default for UcdQueryInfo {
    fn default() -> Self {
        Self {
            queue_len: Default::default(),
            rx_free_list_addr: Default::default(),
            tx_free_list_addr: Default::default(),
            rx_queue_addr: Default::default(),
            tx_queue_addr: Default::default(),
            rx_entry_addr: Default::default(),
            rx_pi_addr: Default::default(),
            tx_pi_addr: Default::default(),
            ctrl_id: IoControllerId::Core0,
            channel_id: IoChannelId::Channel0,
            _rsvd: Default::default(),
        }
    }
}

// Ucd Query message
/// Equivalent FP message code identifier: MSG_OP_UCD_QUERY
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageUcdQuery {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// Ucd Query Info
    pub info: UcdQueryInfo,

    /// Reserved padding
    // TODO: change this to PAYLOAD - LEN after finalyzing the data struct size
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageUcdQuery::LEN],
}
static_assertions::assert_eq_size!(IpcMessageUcdQuery, IpcMessage);

impl Default for IpcMessageUcdQuery {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::UcdQuery as u32)
                .with_length(IpcMessageUcdQuery::LEN as u32),
            info: Default::default(),
            _rsvd: [0; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageUcdQuery::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageUcdQuery {
    const OP: IpcMessageOpCode = IpcMessageOpCode::UcdQuery;
    // TODO: finalize this size after the ucd query message revision for send
    const LEN: usize = core::mem::size_of::<UcdQueryInfo>();

    fn validate(&self) -> McrResult<()> {
        if !matches!(
            self.info.ctrl_id,
            IoControllerId::Core0 | IoControllerId::Core1
        ) {
            return Err(IpcMessageErr::InvalidControllerId)?;
        }

        if !matches!(
            self.info.channel_id,
            IoChannelId::Channel0
                | IoChannelId::Channel1
                | IoChannelId::Channel2
                | IoChannelId::Channel3
                | IoChannelId::Channel4
        ) {
            return Err(IpcMessageErr::InvalidChannelId)?;
        }

        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageUcdQuery {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_ucd_query_ipc_message() {
        let message = IpcMessageUcdQuery {
            info: UcdQueryInfo {
                queue_len: 0x80u32,
                rx_free_list_addr: 0x12345678u32,
                tx_free_list_addr: 0x23456781u32,
                rx_queue_addr: 0x34567812u32,
                tx_queue_addr: 0x45678123u32,
                rx_entry_addr: 0x56781234u32,
                rx_pi_addr: 0x67812345u32,
                tx_pi_addr: 0x78123456u32,
                ctrl_id: IoControllerId::Core1,
                channel_id: IoChannelId::Channel2,
                _rsvd: 0,
            },
            ..Default::default()
        };

        let ipc_message = message.encode();
        assert_eq!(ipc_message.data[0], 0x2400000a);
        assert_eq!(ipc_message.data[1], 0x80);
        assert_eq!(ipc_message.data[2], 0x12345678);
        assert_eq!(ipc_message.data[3], 0x23456781);
        assert_eq!(ipc_message.data[4], 0x34567812);
        assert_eq!(ipc_message.data[5], 0x45678123);
        assert_eq!(ipc_message.data[6], 0x56781234);
        assert_eq!(ipc_message.data[7], 0x67812345);
        assert_eq!(ipc_message.data[8], 0x78123456);
        assert_eq!(ipc_message.data[9], 0x0201);
        assert_eq!(ipc_message.data[10..IPC_MESSAGE_LENGTH], [0; 6]);
    }

    #[test]
    fn decode_ucd_query_ipc_message() {
        let message = IpcMessageUcdQuery {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::UcdQuery as u32)
                .with_length(60)
                .with_response(false)
                .with_tag(0x30)
                .with_complete_map(0)
                .with_submit_map(0)
                .with_status(0),
            info: UcdQueryInfo {
                queue_len: 128u32,
                rx_free_list_addr: 0x12345678u32,
                tx_free_list_addr: 0x23456781u32,
                rx_queue_addr: 0x34567812u32,
                tx_queue_addr: 0x45678123u32,
                rx_entry_addr: 0x56781234u32,
                rx_pi_addr: 0x67812345u32,
                tx_pi_addr: 0x78123456u32,
                ctrl_id: IoControllerId::Core1,
                channel_id: IoChannelId::Channel0,
                _rsvd: 0,
            },
            _rsvd: [0; 24],
        };

        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        let slice = ipc_message.as_mut_bytes();
        slice.copy_from_slice(message.as_bytes());

        let result: McrResult<IpcMessageUcdQuery> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());

        let return_message = result.unwrap();

        assert_eq!(return_message.header.length(), message.header.length());
        assert_eq!(return_message.header.msg_op(), message.header.msg_op());
        assert_eq!(return_message.header.response(), message.header.response());
        assert_eq!(return_message.header.tag(), message.header.tag());
        assert_eq!(return_message.header.status(), message.header.status());
        assert_eq!(return_message.info.queue_len, message.info.queue_len);
        assert_eq!(
            return_message.info.rx_free_list_addr,
            message.info.rx_free_list_addr
        );
        assert_eq!(
            return_message.info.tx_free_list_addr,
            message.info.tx_free_list_addr
        );
        assert_eq!(
            return_message.info.rx_queue_addr,
            message.info.rx_queue_addr
        );
        assert_eq!(
            return_message.info.tx_queue_addr,
            message.info.tx_queue_addr
        );
        assert_eq!(
            return_message.info.rx_entry_addr,
            message.info.rx_entry_addr
        );
        assert_eq!(return_message.info.rx_pi_addr, message.info.rx_pi_addr);
        assert_eq!(return_message.info.tx_pi_addr, message.info.tx_pi_addr);
        assert!(return_message.info.ctrl_id == message.info.ctrl_id);
        assert!(return_message.info.channel_id == message.info.channel_id);
    }

    #[test]
    fn decode_ucd_query_ipc_message_with_invalid_controller_and_channel_id() {
        let message = IpcMessageUcdQuery {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::UcdQuery as u32)
                .with_length(60)
                .with_response(false)
                .with_tag(0x30)
                .with_complete_map(0)
                .with_submit_map(0)
                .with_status(0),
            info: UcdQueryInfo {
                queue_len: 128u32,
                rx_free_list_addr: 0x12345678u32,
                tx_free_list_addr: 0x23456781u32,
                rx_queue_addr: 0x34567812u32,
                tx_queue_addr: 0x45678123u32,
                rx_entry_addr: 0x56781234u32,
                rx_pi_addr: 0x67812345u32,
                tx_pi_addr: 0x78123456u32,
                ctrl_id: IoControllerId::Core1,
                channel_id: IoChannelId::Channel2,
                _rsvd: 0,
            },
            _rsvd: [0; 24],
        };

        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        let slice = ipc_message.as_mut_bytes();
        slice.copy_from_slice(message.as_bytes());

        // Insert error condition by adding invalid values to controller Id
        assert_eq!(ipc_message.data[9], 0x0201);
        ipc_message.data[9] = 0x0210;

        let result: McrResult<IpcMessageUcdQuery> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_err());

        // Insert error condition by adding invalid values to channel Id
        ipc_message.data[9] = 0x2001;

        let result: McrResult<IpcMessageUcdQuery> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_err());

        // Insert error condition by adding invalid values to controller and channel Id
        ipc_message.data[9] = 0xFFFF;

        let result: McrResult<IpcMessageUcdQuery> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_err());
    }

    #[test]
    fn decode_header_for_ucd_query_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        ipc_message.data[0] = 0x3C00508A;

        let result = IpcMessageDecoder::decode_header(&ipc_message);
        assert!(result.is_ok());

        let header = result.unwrap();

        assert_eq!(header.msg_op(), 0xA);
        assert!(header.response());
        assert_eq!(header.tag(), 0x50);
        assert_eq!(header.status(), 0x00);
        assert_eq!(header.submit_map(), 0x0);
        assert_eq!(header.complete_map(), 0x0);
        assert_eq!(header.length(), 0x3C);
    }

    #[test]
    fn decode_invalid_opcode() {
        let ipc_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageUcdQuery> = IpcMessageDecoder::decode(ipc_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
