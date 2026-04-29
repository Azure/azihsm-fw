// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::DevCqId;
use mcr_types::DevSqId;

use super::IoQueueDeleteContext;
use crate::partition::queue::IoQueueMgr;

#[test]
fn test_enable_io_queue() {
    let mut ioq_mgr = IoQueueMgr::default();

    ioq_mgr.enable_io_queue(DevSqId::Id65, DevCqId::Id65);
    ioq_mgr.enable_io_queue(DevSqId::Id65, DevCqId::Id65);
}

#[test]
fn test_disable_io_queue() {
    let mut ioq_mgr = IoQueueMgr::default();
    let delete_ctx = Some(IoQueueDeleteContext::new(10, false));

    ioq_mgr.enable_io_queue(DevSqId::Id65, DevCqId::Id65);
    ioq_mgr.enable_io_queue(DevSqId::Id66, DevCqId::Id66);

    assert!(!ioq_mgr.disable_io_queue(DevSqId::Id65, delete_ctx.clone()));
    assert!(!ioq_mgr.disable_io_queue(DevSqId::Id66, delete_ctx.clone()));
}

#[test]
fn test_disable_io_queue_none() {
    let mut ioq_mgr = IoQueueMgr::default();
    let delete_ctx = Some(IoQueueDeleteContext::new(10, false));

    assert!(!ioq_mgr.disable_io_queue(DevSqId::Id65, delete_ctx));
}

#[test]
fn test_io_queue() {
    let mut ioq_mgr = IoQueueMgr::default();

    ioq_mgr.enable_io_queue(DevSqId::Id65, DevCqId::Id65);

    let result = ioq_mgr.io_queue(DevSqId::Id65);
    assert!(result.is_some());
}

#[test]
fn test_io_queue_none() {
    let ioq_mgr = IoQueueMgr::default();

    let result = ioq_mgr.io_queue(DevSqId::Id65);
    assert!(result.is_none());
}

#[test]
fn test_disable_all_io_queues() {
    let mut ioq_mgr = IoQueueMgr::default();
    let delete_ctx = Some(IoQueueDeleteContext::new(10, false));

    ioq_mgr.enable_io_queue(DevSqId::Id65, DevCqId::Id65);
    ioq_mgr.enable_io_queue(DevSqId::Id66, DevCqId::Id66);

    assert!(!ioq_mgr.disable_all_io_queues(delete_ctx));

    assert!(ioq_mgr.io_queue(DevSqId::Id65).is_none());
    assert!(ioq_mgr.io_queue(DevSqId::Id66).is_none());
}

#[test]
fn test_disable_all_io_queues_with_pending_queue_deletions() {
    let mut ioq_mgr = IoQueueMgr::default();
    let delete_ctx = Some(IoQueueDeleteContext::new(10, false));

    ioq_mgr.enable_io_queue(DevSqId::Id65, DevCqId::Id65);
    ioq_mgr.enable_io_queue(DevSqId::Id66, DevCqId::Id66);

    let _ioq = ioq_mgr.io_queue(DevSqId::Id65);

    assert!(ioq_mgr.disable_all_io_queues(delete_ctx));

    assert!(ioq_mgr.io_queue(DevSqId::Id65).is_none());
    assert!(ioq_mgr.io_queue(DevSqId::Id66).is_none());
}

#[test]
fn test_io_ref_cnt() {
    let mut ioq_mgr = IoQueueMgr::default();
    let delete_ctx = Some(IoQueueDeleteContext::new(10, false));

    ioq_mgr.enable_io_queue(DevSqId::Id65, DevCqId::Id65);
    ioq_mgr.enable_io_queue(DevSqId::Id66, DevCqId::Id66);

    let ioq = ioq_mgr.io_queue(DevSqId::Id65);

    assert_eq!(ioq.as_ref().unwrap().ref_cnt(), 2);

    assert!(ioq_mgr.disable_io_queue(DevSqId::Id65, delete_ctx));

    assert_eq!(ioq.as_ref().unwrap().ref_cnt(), 1);
}

#[test]
fn test_io_queue_ref_cnt_before_disable() {
    let mut ioq_mgr = IoQueueMgr::default();
    let delete_ctx = Some(IoQueueDeleteContext::new(10, false));

    ioq_mgr.enable_io_queue(DevSqId::Id65, DevCqId::Id65);

    // HSM FSM contains a reference to the IO Queue
    let _ioq_fsm = ioq_mgr.io_queue(DevSqId::Id65);

    assert_eq!(_ioq_fsm.as_ref().unwrap().ref_cnt(), 2);

    let ioq = ioq_mgr.io_queue(DevSqId::Id65);

    assert_eq!(ioq.as_ref().unwrap().ref_cnt(), 3);

    // Since we hold a reference to the IO Queue, attempting to delete the the IO Queue should
    // result in pending response
    assert!(ioq_mgr.disable_io_queue(DevSqId::Id65, delete_ctx));

    assert_eq!(ioq.as_ref().unwrap().ref_cnt(), 2);
}
