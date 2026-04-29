// Copyright (c) Microsoft Corporation. All rights reserved.

//! This file defines a simple resource that is used as a mutex for periodic CAST operation.
//! The resource is used to ensure that only one of CAST FSM or IDFU FSM can run at a time.
use core::ops::Deref;

use super::AdminFsmResourceId;
use crate::CmdResourceInfo;

/// Cast resource
#[derive(Clone, Default)]
pub(crate) struct CastIdle;

impl CmdResourceInfo for CastIdle {
    type Id = AdminFsmResourceId;
    type Resource = ();
    type Context = ();

    fn id(&self) -> Self::Id {
        AdminFsmResourceId::CastIdle
    }

    fn max_count(&self) -> usize {
        const MAX_COUNT: usize = 1;
        MAX_COUNT
    }

    fn set(&mut self, _ctx: Self::Context) -> Option<usize> {
        Some(0)
    }

    fn clear(&mut self, _idx: usize) {}

    fn resource(&self, _idx: usize) -> &Self::Resource {
        &()
    }

    fn find_ctx<F>(&self, _predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        None
    }
}

impl CastIdle {
    pub fn new() -> Self {
        Self
    }
}

impl Deref for CastIdle {
    type Target = ();

    fn deref(&self) -> &Self::Target {
        &()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_cast_idle() {
        let mut cast_idle = CastIdle::new();
        assert_eq!(cast_idle.id(), AdminFsmResourceId::CastIdle);
        assert_eq!(cast_idle.max_count(), 1);
        assert_eq!(cast_idle.set(()), Some(0));
        assert_eq!(cast_idle.find_ctx(|_| true), None);
    }
}
