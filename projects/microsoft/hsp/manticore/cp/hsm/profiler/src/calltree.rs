// Copyright (c) Microsoft Corporation. All rights reserved.

//! This module contains the call tree profiler.
//!
//! One should be careful when using this profiler in super high performance sensitive
//! code paths. This is because the library does look up existing sub tree nodes.
//!
//! You should fetch the log using the `get_log` method. You should call this method against
//! the root node to get the full log preferably outside of the performance sensitive
//! code path.
//!
//! The log has the following format:
//!   label:done_or_running:call_count/total_duration(min_duration,max_duration){child1_log;child2_log;...};
//!
//! # Example Log
//!    root:R:1/84(84,84){child1:D:2/15(5,10);child2:R:2/55(10,45){child2_1:D:1/10(10,10);};};
//!
//! This above log means:
//!   - root:R: The root node is running. This means `end_call` was not called on the root node.
//!     - 1/84: The root node was called once and the total duration is 84. The min duration is 84 and the max duration is 84.
//!     - child1:D: The child1 node is done. This means `end_call` was called on the child1 node.
//!         - 2/15: The child1 node was called twice and the total duration is 15. The min duration is 5 and the max duration is 10.
//!     - child2:R: The child2 node is running. This means `end_call` was not called on the child2 in the most recent call.
//!         - 2/55: The child2 node was called twice and the total duration is 55. The min duration is 10 and the max duration is 45.
//!         - child2_1:D: The child2_1 node is done. This means `end_call` was called on the child2_1 node.
//!             - 1/10: The child2_1 node was called once and the total duration is 10. The min duration is 10 and the max duration is 10.

extern crate alloc;

use alloc::format;
use alloc::rc::Rc;
use alloc::string::String;
use alloc::vec::Vec;
use core::cell::RefCell;

/// Represents a node in the call tree.
#[derive(Debug, Clone)]
pub struct CallTreeNode {
    rimpl: Rc<RefCell<CallTreeNodeImpl>>,
}

#[derive(Debug, Clone)]
struct CallTreeNodeImpl {
    call_count: u64,
    last_call_start_time: u64,
    total_duration: u64,
    min_duration: u64,
    max_duration: u64,
    label: &'static str,
    children: Vec<CallTreeNode>,
}

impl CallTreeNode {
    /// Create a new call tree.
    /// You will need to call this method only for the root node.
    ///
    /// # Arguments
    /// * `label` - The label of the call tree.
    /// * `call_time` - The current time at the call tree.
    ///
    /// # Returns
    /// The call tree node object.
    pub fn new(label: &'static str, call_time: u64) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(CallTreeNodeImpl::new(label, call_time))),
        }
    }

    /// Begins a child call.
    /// This method should be called on the parent call tree node object. If the child call
    /// already exists then it increments the call count and sets the last call start time.
    /// If it does not exist then it creates a new child.
    ///
    /// # Arguments
    /// * `label` - The label of the child call.
    /// * `call_time` - The current time at the child call.
    ///
    /// # Returns
    /// The child call tree node object.
    pub fn begin_child_call(&self, label: &'static str, call_time: u64) -> CallTreeNode {
        self.rimpl.borrow_mut().begin_child_call(label, call_time)
    }

    /// Ends the current call.
    ///
    /// # Arguments
    /// * `call_time` - The current time at the call tree.
    pub fn end_call(&self, call_time: u64) {
        self.rimpl.borrow_mut().end_call(call_time);
    }

    /// Gets the log of the current call sub tree.
    ///
    /// # Arguments
    /// * `call_time` - The current time at the call tree.
    ///
    /// # Returns
    /// The log string.
    ///
    /// The log has the following format:
    ///   label:done_or_running:call_count/total_duration(min_duration,max_duration){child1_log;child2_log;...};
    ///
    /// # Example Log
    ///    root:R:1/84(84,84){child1:D:2/15(5,10);child2:R:2/55(10,45){child2_1:D:1/10(10,10);};};
    ///
    /// This above log means:
    ///   - root:R: The root node is running. This means `end_call` was not called on the root node.
    ///     - 1/84: The root node was called once and the total duration is 84. The min duration is 84 and the max duration is 84.
    ///     - child1:D: The child1 node is done. This means `end_call` was called on the child1 node.
    ///         - 2/15: The child1 node was called twice and the total duration is 15. The min duration is 5 and the max duration is 10.
    ///     - child2:R: The child2 node is running. This means `end_call` was not called on the child2 in the most recent call.
    ///         - 2/55: The child2 node was called twice and the total duration is 55. The min duration is 10 and the max duration is 45.
    ///         - child2_1:D: The child2_1 node is done. This means `end_call` was called on the child2_1 node.
    ///             - 1/10: The child2_1 node was called once and the total duration is 10. The min duration is 10 and the max duration is 10.
    pub fn get_log(&self, call_time: u64) -> String {
        self.rimpl.borrow().get_log(call_time)
    }
}

impl CallTreeNodeImpl {
    fn new(label: &'static str, call_time: u64) -> Self {
        Self {
            call_count: 1,
            last_call_start_time: call_time,
            total_duration: 0,
            min_duration: u64::MAX,
            max_duration: 0,
            label,
            children: Vec::new(),
        }
    }

    fn begin_child_call(&mut self, label: &'static str, call_time: u64) -> CallTreeNode {
        let child = self
            .children
            .iter()
            .find(|c| c.rimpl.borrow().label == label);

        if let Some(child) = child {
            // If a child was found we need to increment the call count
            // and set the last_call_start_time.
            child.rimpl.borrow_mut().call_count += 1;
            child.rimpl.borrow_mut().last_call_start_time = call_time;

            child.clone()
        } else {
            // Add a new child
            let child = CallTreeNode::new(label, call_time);
            self.children.push(child.clone());

            child
        }
    }

    fn end_call(&mut self, call_time: u64) {
        let call_duration = call_time - self.last_call_start_time;
        self.total_duration += call_duration;
        self.min_duration = self.min_duration.min(call_duration);
        self.max_duration = self.max_duration.max(call_duration);
        self.last_call_start_time = 0;
    }

    pub fn get_log(&self, call_time: u64) -> String {
        let mut log = String::new();

        let mut total_duration = self.total_duration;
        let mut min_duration = self.min_duration;
        let mut max_duration = self.max_duration;
        let mut done_or_running = 'D';

        if self.last_call_start_time != 0 {
            done_or_running = 'R';
            let duration_since_last_call = call_time - self.last_call_start_time;
            total_duration += duration_since_last_call;
            min_duration = min_duration.min(duration_since_last_call);
            max_duration = max_duration.max(duration_since_last_call);
        }

        log.push_str(&format!(
            "{}:{}:{}/{}({},{})",
            self.label,
            done_or_running,
            self.call_count,
            total_duration,
            min_duration,
            max_duration
        ));

        if !self.children.is_empty() {
            log.push('{');
            for child in self.children.iter() {
                log.push_str(&child.get_log(call_time));
            }
            log.push('}');
        }

        log.push(';');

        log
    }
}
