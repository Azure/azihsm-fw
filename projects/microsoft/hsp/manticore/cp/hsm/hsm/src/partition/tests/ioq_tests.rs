// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::DevCqId;
use mcr_types::DevSqId;

use super::queue::IoQueueDeleteContext;
use crate::partition::IoQueue;

#[test]
fn test_sq_id() {
    let sq_id = DevSqId::Id0;
    let ioq = IoQueue::new(sq_id, DevCqId::Id0);

    assert!(sq_id == ioq.sq_id());
}

#[test]
fn test_cq_id() {
    let cq_id = DevCqId::Id0;
    let ioq = IoQueue::new(DevSqId::Id0, cq_id);

    assert!(cq_id == ioq.cq_id());
}

#[test]
fn test_valid() {
    let ioq = IoQueue::new(DevSqId::Id0, DevCqId::Id0);

    assert!(ioq.valid());
}

#[test]
fn test_invalidate() {
    let mut ioq = IoQueue::new(DevSqId::Id0, DevCqId::Id0);

    ioq.invalidate();
    assert!(!ioq.valid());
}

#[test]
fn test_ref_cnt() {
    let ioq = IoQueue::new(DevSqId::Id1, DevCqId::Id1);

    assert_eq!(ioq.ref_cnt(), 1);

    let _ioq_clone1 = Some(ioq.clone());
    assert_eq!(ioq.ref_cnt(), 2);
    {
        let _ioq_clone2 = Some(ioq.clone());
        assert_eq!(ioq.ref_cnt(), 3);
    }
    assert_eq!(ioq.ref_cnt(), 2);
    let _ioq_clone3 = Some(ioq.clone());
    assert_eq!(ioq.ref_cnt(), 3);
}

#[test]
fn test_add_delete_context() {
    let ioq = IoQueue::new(DevSqId::Id1, DevCqId::Id1);
    {
        let delete_ctx = IoQueueDeleteContext::new(10, false);
        ioq.set_delete_context(Some(delete_ctx.clone()));
    }

    assert_eq!(ioq.ref_cnt(), 1);

    let delete_ctx = ioq.take_delete_ctx();
    assert!(delete_ctx.is_some());
    assert_eq!(delete_ctx.as_ref().unwrap().ref_cnt(), 1);
    assert_eq!(delete_ctx.as_ref().unwrap().tag(), 10);
}

#[test]
fn test_add_delete_context_to_multiple_queues() {
    let ioq1 = IoQueue::new(DevSqId::Id1, DevCqId::Id1);
    let ioq2 = IoQueue::new(DevSqId::Id2, DevCqId::Id2);
    let ioq3 = IoQueue::new(DevSqId::Id3, DevCqId::Id3);
    let ioq4 = IoQueue::new(DevSqId::Id4, DevCqId::Id4);
    let ioq5 = IoQueue::new(DevSqId::Id5, DevCqId::Id5);
    let ioq6 = IoQueue::new(DevSqId::Id6, DevCqId::Id6);
    let ioq7 = IoQueue::new(DevSqId::Id7, DevCqId::Id7);
    let ioq8 = IoQueue::new(DevSqId::Id8, DevCqId::Id8);

    // Create a delete context and put it in all the queues
    {
        let delete_ctx = IoQueueDeleteContext::new(10, false);
        ioq1.set_delete_context(Some(delete_ctx.clone()));
        ioq2.set_delete_context(Some(delete_ctx.clone()));
        ioq3.set_delete_context(Some(delete_ctx.clone()));
        ioq4.set_delete_context(Some(delete_ctx.clone()));
        ioq5.set_delete_context(Some(delete_ctx.clone()));
        ioq6.set_delete_context(Some(delete_ctx.clone()));
        ioq7.set_delete_context(Some(delete_ctx.clone()));
        ioq8.set_delete_context(Some(delete_ctx.clone()));
    }

    assert_eq!(ioq1.ref_cnt(), 1);
    assert_eq!(ioq2.ref_cnt(), 1);
    assert_eq!(ioq3.ref_cnt(), 1);
    assert_eq!(ioq4.ref_cnt(), 1);
    assert_eq!(ioq5.ref_cnt(), 1);
    assert_eq!(ioq6.ref_cnt(), 1);
    assert_eq!(ioq7.ref_cnt(), 1);
    assert_eq!(ioq8.ref_cnt(), 1);

    let delete_ctx = ioq1.take_delete_ctx();
    assert!(delete_ctx.is_some());
    assert_eq!(delete_ctx.as_ref().unwrap().ref_cnt(), 8);
    assert_eq!(delete_ctx.as_ref().unwrap().tag(), 10);

    drop(delete_ctx);

    let delete_ctx = ioq1.take_delete_ctx();
    assert!(delete_ctx.is_none());

    let delete_ctx = ioq2.take_delete_ctx();
    assert!(delete_ctx.is_some());
    assert_eq!(delete_ctx.as_ref().unwrap().ref_cnt(), 7);
    assert_eq!(delete_ctx.as_ref().unwrap().tag(), 10);

    drop(delete_ctx);

    let delete_ctx = ioq2.take_delete_ctx();
    assert!(delete_ctx.is_none());

    let delete_ctx = ioq3.take_delete_ctx();
    assert!(delete_ctx.is_some());
    assert_eq!(delete_ctx.as_ref().unwrap().ref_cnt(), 6);
    assert_eq!(delete_ctx.as_ref().unwrap().tag(), 10);

    drop(delete_ctx);

    let delete_ctx = ioq3.take_delete_ctx();
    assert!(delete_ctx.is_none());

    let delete_ctx = ioq4.take_delete_ctx();
    assert!(delete_ctx.is_some());
    assert_eq!(delete_ctx.as_ref().unwrap().ref_cnt(), 5);
    assert_eq!(delete_ctx.as_ref().unwrap().tag(), 10);

    drop(delete_ctx);

    let delete_ctx = ioq4.take_delete_ctx();
    assert!(delete_ctx.is_none());

    let delete_ctx = ioq5.take_delete_ctx();
    assert!(delete_ctx.is_some());
    assert_eq!(delete_ctx.as_ref().unwrap().ref_cnt(), 4);
    assert_eq!(delete_ctx.as_ref().unwrap().tag(), 10);

    drop(delete_ctx);

    let delete_ctx = ioq5.take_delete_ctx();
    assert!(delete_ctx.is_none());

    let delete_ctx = ioq6.take_delete_ctx();
    assert!(delete_ctx.is_some());
    assert_eq!(delete_ctx.as_ref().unwrap().ref_cnt(), 3);
    assert_eq!(delete_ctx.as_ref().unwrap().tag(), 10);

    drop(delete_ctx);

    let delete_ctx = ioq6.take_delete_ctx();
    assert!(delete_ctx.is_none());

    let delete_ctx = ioq7.take_delete_ctx();
    assert!(delete_ctx.is_some());
    assert_eq!(delete_ctx.as_ref().unwrap().ref_cnt(), 2);
    assert_eq!(delete_ctx.as_ref().unwrap().tag(), 10);

    drop(delete_ctx);

    let delete_ctx = ioq7.take_delete_ctx();
    assert!(delete_ctx.is_none());

    let delete_ctx = ioq8.take_delete_ctx();
    assert!(delete_ctx.is_some());
    assert_eq!(delete_ctx.as_ref().unwrap().ref_cnt(), 1);
    assert_eq!(delete_ctx.as_ref().unwrap().tag(), 10);

    drop(delete_ctx);

    let delete_ctx = ioq8.take_delete_ctx();
    assert!(delete_ctx.is_none());
}

#[test]
fn test_delete_context_count_on_disabled_queue() {
    let ioq = IoQueue::new(DevSqId::Id1, DevCqId::Id1);
    let delete_ctx = IoQueueDeleteContext::new(10, false);
    ioq.set_delete_context(Some(delete_ctx.clone()));

    assert_eq!(delete_ctx.ref_cnt(), 2);

    assert_eq!(ioq.ref_cnt(), 1);

    drop(ioq);

    assert_eq!(delete_ctx.ref_cnt(), 1);
    assert_eq!(delete_ctx.tag(), 10);
}
