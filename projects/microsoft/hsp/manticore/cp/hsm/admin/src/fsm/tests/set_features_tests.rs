// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;

use super::helper::*;
use crate::error::AdminErr;
use crate::fsm::set_features::AdminSetFeaturesCmd;
use crate::fsm::types::*;
use crate::fsm::AdminCmdTrait;
use crate::AdminFsmEvent;

#[test]
fn test_set_features_cmd_unknown_event() {
    const ID: AdminFeatureId = AdminFeatureId::NumberOfQueues;
    const OP_CODE: AdminCommandOpCodes = AdminCommandOpCodes::SetFeatures;
    const NUM_RES: u32 = 1;
    const UNKNOWN_EVENT: bool = true;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_get_set_features(OP_CODE, ID, NUM_RES, UNKNOWN_EVENT);

    // Create Set Features command FSM
    let mut fsm = AdminSetFeaturesCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::Unknown, 0xFF),
        Err(AdminErr::Pending)
    );
}

#[test]
fn test_set_features_cmd_num_queues() {
    const ID: AdminFeatureId = AdminFeatureId::NumberOfQueues;
    const OP_CODE: AdminCommandOpCodes = AdminCommandOpCodes::SetFeatures;
    const NUM_RES: u32 = 1;
    const UNKNOWN_EVENT: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_get_set_features(OP_CODE, ID, NUM_RES, UNKNOWN_EVENT);

    // Create Set Features command FSM
    let mut fsm = AdminSetFeaturesCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}

#[test]
fn test_set_features_cmd_fp_num_queues() {
    const ID: AdminFeatureId = AdminFeatureId::FpNumberOfQueues;
    const OP_CODE: AdminCommandOpCodes = AdminCommandOpCodes::SetFeatures;
    const NUM_RES: u32 = 1;
    const UNKNOWN_EVENT: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_get_set_features(OP_CODE, ID, NUM_RES, UNKNOWN_EVENT);

    // Create Set Features command FSM
    let mut fsm = AdminSetFeaturesCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(fsm.on_event(AdminFsmEvent::StartCmd, 0xFF), Ok(()));
}

#[test]
fn test_set_features_cmd_with_zero_res_cnt() {
    const ID: AdminFeatureId = AdminFeatureId::FpNumberOfQueues;
    const OP_CODE: AdminCommandOpCodes = AdminCommandOpCodes::SetFeatures;
    const NUM_RES: u32 = 0;
    const UNKNOWN_EVENT: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_get_set_features(OP_CODE, ID, NUM_RES, UNKNOWN_EVENT);

    // Create Set Features command FSM
    let mut fsm = AdminSetFeaturesCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidFieldInGetSetFeaturesCmd)
    );
}

#[test]
fn test_set_features_cmd_with_invalid_feature_id() {
    const ID: AdminFeatureId = AdminFeatureId(200);
    const OP_CODE: AdminCommandOpCodes = AdminCommandOpCodes::SetFeatures;
    const NUM_RES: u32 = 1;
    const UNKNOWN_EVENT: bool = false;

    // Prepare the test environment
    let (ctx, sqe) = make_fsm_for_get_set_features(OP_CODE, ID, NUM_RES, UNKNOWN_EVENT);

    // Create Set Features command FSM
    let mut fsm = AdminSetFeaturesCmd::new(ctx, PcieFunction::Pf, sqe, None);

    // Execute test by passing events to the FSM
    assert_eq!(
        fsm.on_event(AdminFsmEvent::StartCmd, 0xFF),
        Err(AdminErr::InvalidFeatureId)
    );
}
