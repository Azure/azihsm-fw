// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield_struct::bitfield;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

use crate::*;

// CDMA IO message data
/// DW0 of CDMA IO message data
#[bitfield(u32)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct CdmaIoMsgDataReqDw0 {
    /// VF ID that this IO belongs to
    #[bits(8)]
    pub vfid: u8,

    /// Reserved
    #[bits(24)]
    pub rsvd0: u32,
}

/// DW1 of CDMA IO message data
#[bitfield(u32)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct CdmaIoMsgDataReqDw1 {
    /// Interface select of PRP/SGL data
    #[bits(8)]
    pub src_desc_inter_sel: u8,

    /// Interface select of PRP/SGL data
    #[bits(8)]
    pub src_data_inter_sel: u8,

    /// Interface select of PRP/SGL data
    #[bits(8)]
    pub dst_desc_inter_sel: u8,

    /// Interface select of PRP/SGL data
    #[bits(8)]
    pub dst_data_inter_sel: u8,
}

/// CDMA IO message data
#[repr(C, align(4))]
#[derive(Clone, IntoBytes, Immutable, FromBytes)]
pub struct CdmaIoMsgDataReq {
    /// CDMA IO message data DW0; VFID info
    pub dw0: CdmaIoMsgDataReqDw0,

    /// CDMA IO message data DW1; Interface select
    pub dw1: CdmaIoMsgDataReqDw1,
}

impl Default for CdmaIoMsgDataReq {
    fn default() -> Self {
        Self {
            dw0: CdmaIoMsgDataReqDw0::new(),
            dw1: CdmaIoMsgDataReqDw1::new(),
        }
    }
}

static_assertions::const_assert_eq!(size_of::<CdmaIoMsgDataReq>(), 0x8);

/// Admin to FP CDMA IO Ipc Message Request
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageCdmaIoReq {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// CDMA IO info
    pub info: CdmaIoMsgDataReq,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageCdmaIoReq::LEN],
}

static_assertions::assert_eq_size!(IpcMessageCdmaIoReq, IpcMessage);

impl Default for IpcMessageCdmaIoReq {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::CdmaIo as u32)
                .with_response(false)
                .with_length(IpcMessageCdmaIoReq::LEN as u32),
            info: Default::default(),
            _rsvd: [0; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageCdmaIoReq::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageCdmaIoReq {
    const OP: IpcMessageOpCode = IpcMessageOpCode::CdmaIo;
    const LEN: usize = core::mem::size_of::<CdmaIoMsgDataReq>();

    fn validate(&self) -> McrResult<()> {
        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageCdmaIoReq {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

/// Admin to FP CDMA IO Info
#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub struct CdmaIoMsgDataResp {
    /// Command list index
    pub cmd_list_idx: u8,

    /// Command list number
    pub cmd_list_num: u8,

    /// Command status
    pub cmd_status: u8,
}

/// Admin to FP CDMA IO Ipc Message Resp
#[repr(C)]
#[derive(IntoBytes, Immutable, FromBytes)]
pub struct IpcMessageCdmaIoResp {
    /// IPC header fields
    pub header: IpcMessageHeader,

    /// CDMA IO info
    pub info: CdmaIoMsgDataResp,

    /// Reserved padding
    pub _rsvd: [u8; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageCdmaIoResp::LEN],
}

static_assertions::assert_eq_size!(IpcMessageCdmaIoResp, IpcMessage);

impl Default for IpcMessageCdmaIoResp {
    fn default() -> Self {
        Self {
            header: IpcMessageHeader::new()
                .with_msg_op(IpcMessageOpCode::CdmaIo as u32)
                .with_response(true)
                .with_length(IpcMessageCdmaIoResp::LEN as u32),
            info: Default::default(),
            _rsvd: [0; IPC_MESSAGE_PAYLOAD_LEN - IpcMessageCdmaIoResp::LEN],
        }
    }
}

impl IpcMessageType for IpcMessageCdmaIoResp {
    const OP: IpcMessageOpCode = IpcMessageOpCode::CdmaIo;
    const LEN: usize = core::mem::size_of::<CdmaIoMsgDataResp>();

    fn validate(&self) -> McrResult<()> {
        Ok(())
    }
}

impl IpcMessageEncoderTrait for IpcMessageCdmaIoResp {
    fn encode(self) -> IpcMessage {
        IpcMessageEncoder::encode(self)
    }
}

#[cfg(test)]
mod cfg_test {
    use mcr_ipc_controller::IPC_MESSAGE_LENGTH;

    use super::*;

    #[test]
    fn encode_cdma_io_req_ipc_message() {
        let message = IpcMessageCdmaIoReq {
            info: CdmaIoMsgDataReq {
                // 0x00000041
                dw0: CdmaIoMsgDataReqDw0::new().with_vfid(0x41).with_rsvd0(0),

                // 0xDEADBEEF
                dw1: CdmaIoMsgDataReqDw1::new()
                    .with_src_desc_inter_sel(0xEF)
                    .with_src_data_inter_sel(0xBE)
                    .with_dst_desc_inter_sel(0xAD)
                    .with_dst_data_inter_sel(0xDE),
            },
            ..Default::default()
        };

        let ipc_message = message.encode();

        // Message header; Message len of 8 (0x8) bytes, IpcMessageCdmaIo opcode is 0x6
        assert_eq!(ipc_message.data[0], 0x08000006);

        // Message data
        assert_eq!(ipc_message.data[1], 0x00000041);
        assert_eq!(ipc_message.data[2], 0xDEADBEEF);

        // Message body should be all 0x0 by default
        assert_eq!(ipc_message.data[3..IPC_MESSAGE_LENGTH], [0; 13]);
    }

    #[test]
    fn decode_cdma_io_req_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        // message header
        ipc_message.data[0] = 0x08011206;

        // message body
        ipc_message.data[1] = 0x00000041;
        ipc_message.data[2] = 0xDEADBEEF;

        let result: McrResult<IpcMessageCdmaIoReq> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());

        let message = result.unwrap();

        // Check message header values
        assert_eq!(message.header.msg_op(), 0x6);
        assert_eq!(message.header.length(), 0x8);
        assert!(!message.header.response());
        assert_eq!(message.header.tag(), 0x12);
        assert_eq!(message.header.status(), 0x1);

        // Check message body
        assert_eq!(message.info.dw0.vfid(), 0x41);
        assert_eq!(message.info.dw1.src_desc_inter_sel(), 0xEF);
        assert_eq!(message.info.dw1.src_data_inter_sel(), 0xBE);
        assert_eq!(message.info.dw1.dst_desc_inter_sel(), 0xAD);
        assert_eq!(message.info.dw1.dst_data_inter_sel(), 0xDE);
    }

    #[test]
    fn encode_cdma_io_resp_ipc_message() {
        let resp_message = IpcMessageCdmaIoResp {
            info: CdmaIoMsgDataResp {
                cmd_list_idx: 0x0,
                cmd_list_num: 0x2,
                cmd_status: 0x0,
            },
            ..Default::default()
        };

        let ipc_message = resp_message.encode();

        // Message header; Message len of 3 bytes, Response == true, IpcMessageCdmaIo opcode is 0x6
        assert_eq!(ipc_message.data[0], 0x03000086);

        // Message data
        assert_eq!(ipc_message.data[1], 0x00000200);

        // Message body should be all 0x0 by default
        assert_eq!(ipc_message.data[2..IPC_MESSAGE_LENGTH], [0; 14]);
    }

    #[test]
    fn decode_cdma_io_resp_ipc_message() {
        let mut ipc_message = IpcMessage {
            data: [0x00; IPC_MESSAGE_LENGTH],
        };

        // message header
        ipc_message.data[0] = 0x03011206;

        // message body
        ipc_message.data[1] = 0x00ABCDEF;

        let result: McrResult<IpcMessageCdmaIoResp> = IpcMessageDecoder::decode(ipc_message);
        assert!(result.is_ok());

        let message = result.unwrap();

        // Check message header values
        assert_eq!(message.header.msg_op(), 0x6);
        assert_eq!(message.header.length(), 0x3);
        assert!(!message.header.response());
        assert_eq!(message.header.tag(), 0x12);
        assert_eq!(message.header.status(), 0x1);

        // Check message body
        assert_eq!(message.info.cmd_list_idx, 0xEF);
        assert_eq!(message.info.cmd_list_num, 0xCD);
        assert_eq!(message.info.cmd_status, 0xAB);
    }

    #[test]
    fn decode_invalid_opcode() {
        // request message
        let ipc_req_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageCdmaIoReq> = IpcMessageDecoder::decode(ipc_req_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );

        // response message
        let ipc_resp_message = IpcMessage {
            data: [0xFF; IPC_MESSAGE_LENGTH],
        };

        let result: McrResult<IpcMessageCdmaIoResp> = IpcMessageDecoder::decode(ipc_resp_message);

        assert_eq!(
            result.err(),
            Some(IpcMessageErr::InvalidOpcodeConversion as u32)
        );
    }
}
