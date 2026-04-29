// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::cmd_scheduler::TagId;

#[test]
fn test_unsupported_cmd() {
    let mut cmd = UnsupportedCmd::<MockEnv>::new();
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedCmd)
    );
    assert!(cmd.session_id().is_none());
    assert!(cmd.take_response().is_none());
}
