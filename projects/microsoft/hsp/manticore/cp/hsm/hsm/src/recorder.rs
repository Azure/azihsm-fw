// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;

use super::*;
use crate::error::HsmErr;
use crate::event::HsmFsmEvent;

#[derive(Clone, Copy, Eq, PartialEq)]
pub(crate) enum HsmFsmEventId {
    StartEvent,
    EndEvent,
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub(crate) struct HsmFsmEventRecord {
    pub id: HsmFsmEventId,
    pub event: HsmFsmEvent,
    pub tag: TagId,
    pub result: Option<Result<(), HsmErr>>,
}

#[derive(Clone, Default)]
pub(crate) struct HsmFsmEventRecorder {
    rimpl: Rc<RefCell<HsmFsmEventRecorderImpl>>,
}

impl CmdFsmEventRecorder for HsmFsmEventRecorder {
    type Error = HsmErr;
    type Event = HsmFsmEvent;

    fn on_event_start(&self, event: Self::Event, tag: TagId) {
        self.rimpl.borrow_mut().on_event_start(event, tag)
    }

    fn on_event_end(&self, event: Self::Event, tag: TagId, result: Result<(), Self::Error>) {
        self.rimpl.borrow_mut().on_event_end(event, tag, result)
    }
}

#[allow(dead_code)]
impl HsmFsmEventRecorder {
    pub fn enable(&self) {
        self.rimpl.borrow_mut().enable()
    }

    pub fn disable(&self) {
        self.rimpl.borrow_mut().disable()
    }

    pub fn events(&self) -> Vec<HsmFsmEventRecord> {
        self.rimpl.borrow().vec.clone()
    }
}

#[derive(Default)]
struct HsmFsmEventRecorderImpl {
    vec: Vec<HsmFsmEventRecord>,
    enable: bool,
}

impl HsmFsmEventRecorderImpl {
    fn enable(&mut self) {
        self.enable = true
    }

    fn disable(&mut self) {
        self.enable = false
    }

    fn on_event_start(&mut self, event: HsmFsmEvent, tag: TagId) {
        if !self.enable {
            return;
        }

        self.vec.push(HsmFsmEventRecord {
            id: HsmFsmEventId::StartEvent,
            event,
            tag,
            result: None,
        })
    }

    fn on_event_end(&mut self, event: HsmFsmEvent, tag: TagId, result: Result<(), HsmErr>) {
        if !self.enable {
            return;
        }

        self.vec.push(HsmFsmEventRecord {
            id: HsmFsmEventId::EndEvent,
            event,
            tag,
            result: Some(result),
        })
    }
}
