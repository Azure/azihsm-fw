// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;

use crate::error::AdminErr;
use crate::event::AdminFsmEvent;
use crate::CmdFsmEventRecorder;
use crate::TagId;

#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub(crate) enum AdminFsmEventId {
    StartEvent,
    EndEvent,
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub(crate) struct AdminFsmEventRecord {
    pub id: AdminFsmEventId,
    pub event: AdminFsmEvent,
    pub tag: TagId,
    pub result: Option<Result<(), AdminErr>>,
}

#[derive(Clone, Default)]
pub(crate) struct AdminFsmEventRecorder {
    rimpl: Rc<RefCell<AdminFsmEventRecorderImpl>>,
}

impl CmdFsmEventRecorder for AdminFsmEventRecorder {
    type Error = AdminErr;
    type Event = AdminFsmEvent;

    fn on_event_start(&self, event: Self::Event, tag: TagId) {
        self.rimpl.borrow_mut().on_event_start(event, tag)
    }

    fn on_event_end(&self, event: Self::Event, tag: TagId, result: Result<(), Self::Error>) {
        self.rimpl.borrow_mut().on_event_end(event, tag, result)
    }
}

#[allow(dead_code)]
impl AdminFsmEventRecorder {
    pub fn enable(&self) {
        self.rimpl.borrow_mut().enable()
    }

    pub fn disable(&self) {
        self.rimpl.borrow_mut().disable()
    }

    pub fn events(&self) -> Vec<AdminFsmEventRecord> {
        self.rimpl.borrow().vec.clone()
    }
}

#[derive(Default)]
struct AdminFsmEventRecorderImpl {
    vec: Vec<AdminFsmEventRecord>,
    enable: bool,
}

impl AdminFsmEventRecorderImpl {
    fn enable(&mut self) {
        self.enable = true
    }

    fn disable(&mut self) {
        self.enable = false
    }

    fn on_event_start(&mut self, event: AdminFsmEvent, tag: TagId) {
        if !self.enable {
            return;
        }

        self.vec.push(AdminFsmEventRecord {
            id: AdminFsmEventId::StartEvent,
            event,
            tag,
            result: None,
        })
    }

    fn on_event_end(&mut self, event: AdminFsmEvent, tag: TagId, result: Result<(), AdminErr>) {
        if !self.enable {
            return;
        }

        self.vec.push(AdminFsmEventRecord {
            id: AdminFsmEventId::EndEvent,
            event,
            tag,
            result: Some(result),
        })
    }
}
