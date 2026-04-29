// Copyright (c) Microsoft Corporation. All rights reserved.

mod controller {
    cfg_if::cfg_if! {
        if #[cfg(test)] {
            use mockall::*;
            use mockall::predicate::*;
        }
    }

    use mcr_interrupt_controller::Interrupt;
    use mcr_interrupt_controller::InterruptControllerTrait;

    #[derive(Default)]
    pub struct MockInterruptController {}

    #[cfg_attr(test, automock)]
    impl InterruptControllerTrait for MockInterruptController {
        fn pending(&self, _interrupt: Interrupt) -> bool {
            todo!()
        }

        fn clear(&self, _interrupt: Interrupt) {
            todo!()
        }

        fn enable(&self, _interrupt: Interrupt) {
            todo!()
        }

        fn disable(&self, _interrupt: Interrupt) {
            todo!()
        }
    }
}

#[mockall_double::double]
pub(crate) use controller::MockInterruptController;
