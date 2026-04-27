// Copyright (c) Microsoft Corporation. All rights reserved.

use core::ops::Deref;

use super::AdminFsmResourceId;
use crate::CmdResourceInfo;

/// TDISP idle resource
#[derive(Clone, Default)]
pub(crate) struct TdispIdle;

impl CmdResourceInfo for TdispIdle {
    type Id = AdminFsmResourceId;
    type Resource = ();
    type Context = ();

    fn id(&self) -> Self::Id {
        AdminFsmResourceId::TdispIdle
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

impl TdispIdle {
    pub fn new() -> Self {
        Self
    }
}

impl Deref for TdispIdle {
    type Target = ();

    fn deref(&self) -> &Self::Target {
        &()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tdisp_idle() {
        let mut tdisp_idle = TdispIdle::new();
        assert_eq!(tdisp_idle.id(), AdminFsmResourceId::TdispIdle);
        assert_eq!(tdisp_idle.max_count(), 1);
        assert_eq!(tdisp_idle.set(()), Some(0));
        assert_eq!(tdisp_idle.find_ctx(|_| true), None);
    }
}
