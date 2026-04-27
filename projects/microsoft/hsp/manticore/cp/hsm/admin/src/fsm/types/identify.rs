// Copyright (c) Microsoft Corporation. All rights reserved.

use zerocopy::Immutable;
use zerocopy::IntoBytes;

/// Admin Identify controller buffer to be transferred to the host
#[repr(C, packed)]
#[derive(IntoBytes, Immutable)]
pub(crate) struct McrCntrlIdentify {
    /// Vendor Id
    pub vid: u16,

    /// Subsystem Vendor Id
    pub ssvid: u16,

    /// Serial Number
    pub sn: [u8; 32],

    /// Firmware Revision
    pub fr: [u8; 32],

    /// Reserved
    pub rsvd1: [u8; 4],

    /// HSM Io Maximum Data Transfer Size
    pub cp_mdts: u8,

    /// Reserved
    pub rsvd2: u8,

    /// Manticore controller Id
    pub cntrl_id: u16,

    /// Abort Command Limit
    pub acl: u8,

    /// HSM Io Submission Queue Entry Size
    pub hsm_sqes: u8,

    /// HSM Io Completion Queue Entry Size
    pub hsm_cqes: u8,

    /// Reserved
    pub rsvd3: u8,

    /// HSM Io Maximum Outstanding Commands
    pub hsm_maxcmd: u16,

    /// Aes Bulk Io Crypto Maximum Data Transfer Size
    pub fp_mdts: u8,

    /// Aes Bulk Crypto Io Submission Queue Entry Size
    pub fp_sqes: u8,

    /// Aes Bulk Crypto Io Completion Queue Entry Size
    pub fp_cqes: u8,

    /// Reserved
    pub rsvd4: u8,

    /// Aes Bulk Crypto Io Maximum Outstanding Commands
    pub fp_maxcmd: u16,

    /// Firmware SVN
    pub svn: [u8; 8],

    /// Version
    pub ver: u32,

    /// Controller Type
    pub cntrltype: u8,

    /// Firmware Updates
    pub frmw: u8,
}

const MDTS_8KB: u8 = 13;
const MSFT_VID: u16 = 0x1414;
const MCR_HSM_SQES: u8 = 0x66;
const MCR_HSM_CQES: u8 = 0x44;
const MCR_HSM_MAX_CMD: u16 = 2;
const MCR_FP_SQES: u8 = 0x77;
const MCR_FP_CQES: u8 = 0x66;
const MCR_FP_MAX_CMD: u16 = 4;
const ABORT_COMMAND_LIMIT: u8 = 0;

impl McrCntrlIdentify {
    pub(crate) fn new(cntrl_id: u16, fw_version: [u8; 32], soc_id: [u8; 32], svn: [u8; 8]) -> Self {
        Self {
            vid: MSFT_VID,
            ssvid: MSFT_VID,
            sn: soc_id,
            fr: fw_version,
            cp_mdts: MDTS_8KB,
            cntrl_id,
            acl: ABORT_COMMAND_LIMIT,
            hsm_sqes: MCR_HSM_SQES,
            hsm_cqes: MCR_HSM_CQES,
            hsm_maxcmd: MCR_HSM_MAX_CMD,
            fp_mdts: MDTS_8KB,
            fp_sqes: MCR_FP_SQES,
            fp_cqes: MCR_FP_CQES,
            fp_maxcmd: MCR_FP_MAX_CMD,
            svn,
            ver: 0x00010400,
            cntrltype: 1,
            frmw: 0,
            rsvd1: [0x00; 4],
            rsvd2: 0,
            rsvd3: 0,
            rsvd4: 0,
        }
    }
}
