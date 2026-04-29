// Copyright (c) Microsoft Corporation. All rights reserved.

use core::ops::Deref;

use super::AdminFsmResourceId;
use crate::CmdResourceInfo;

/// Cast resource
#[derive(Clone, Default)]
pub(crate) struct DoeIdle;

impl CmdResourceInfo for DoeIdle {
    type Id = AdminFsmResourceId;
    type Resource = ();
    type Context = ();

    fn id(&self) -> Self::Id {
        AdminFsmResourceId::DoeIdle
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

impl DoeIdle {
    pub fn new() -> Self {
        Self
    }
}

impl Deref for DoeIdle {
    type Target = ();

    fn deref(&self) -> &Self::Target {
        &()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_doe_idle() {
        let mut doe_idle = DoeIdle::new();
        assert_eq!(doe_idle.id(), AdminFsmResourceId::DoeIdle);
        assert_eq!(doe_idle.max_count(), 1);
        assert_eq!(doe_idle.set(()), Some(0));
        assert_eq!(doe_idle.find_ctx(|_| true), None);
    }
}
