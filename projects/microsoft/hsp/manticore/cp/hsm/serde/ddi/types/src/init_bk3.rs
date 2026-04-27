// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_ddi_derive::Ddi;

use crate::*;

/// DDI Initialize BK3 Request Structure
#[cfg_attr(feature = "fuzzing", derive(arbitrary::Arbitrary))]
#[derive(Ddi, Debug)]
#[ddi(map)]
pub struct DdiInitBk3Req {
    /// BK3
    #[ddi(id = 1)]
    pub bk3: MborByteArray<48>,
}

/// DDI Initialize BK3 Response Structure
#[cfg_attr(feature = "fuzzing", derive(arbitrary::Arbitrary))]
#[derive(Ddi, Debug)]
#[ddi(map)]
pub struct DdiInitBk3Resp {
    /// Output data (masked BK3)
    #[ddi(id = 1)]
    pub masked_bk3: MborByteArray<1024>,

    /// Launch ID for the partition
    #[ddi(id = 2)]
    pub vm_launch_guid: [u8; 16],
}

ddi_op_req_resp!(DdiInitBk3);
