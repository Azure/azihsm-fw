// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_event_loop::*;
use mcr_interrupt_controller::Interrupt;

#[test]
fn test_mixed_events() {
    #[derive(mcr_event_loop_derive::Event, Copy, Clone, Eq, PartialEq)]
    pub enum TestEvent {
        /// Perst UP level interrupt with inferred sensitivity, default is Level
        #[event(interrupt = Interrupt::PciePerstUpIrq)]
        PerstUp,

        /// Perst Down level interrupt with specified sensitivity
        #[event(interrupt = Interrupt::PciePerstDownIrq, sensitivity = IrqSensitivity::Level)]
        PerstDown,

        /// UPKA 0 Done edge interrupt with specified sensitivity
        #[event(interrupt = Interrupt::Upka0DoneIrq, sensitivity = IrqSensitivity::Edge, group = "upka_done")]
        Upka0Done,

        /// UPKA 1 Done edge interrupt with specified sensitivity. Enum attributes are
        /// reordered purposefully to make sure the proc macro can detect right attributes
        /// regardless of its placement
        #[event(sensitivity = IrqSensitivity::Edge, group = "upka_done", interrupt = Interrupt::Upka1DoneIrq)]
        Upka1Done,

        /// UPKA 0 Error edge interrupt with specified sensitivity. Enum attributes are
        /// reordered purposefully to make sure the proc macro can detect right attributes
        /// regardless of its placement
        #[event(group = "upka_err", sensitivity = IrqSensitivity::Edge, interrupt = Interrupt::Upka0ErrorIrq)]
        Upka0Error,

        /// UCD Inbound completions level interrupt with inferred sensitivity, default is Level
        #[event(interrupt = Interrupt::UcdIbcqRr1Irq)]
        UcdIbcq,

        /// UPKA 1 Error edge interrupt with specified sensitivity
        #[event(interrupt = Interrupt::Upka1ErrorIrq, sensitivity = IrqSensitivity::Edge, group = "upka_err")]
        Upka1Error,
    }

    let irq_group = TestEvent::interrupts();

    assert_eq!(irq_group.len(), 5);

    assert_eq!(irq_group[0].id, None);
    assert_eq!(irq_group[0].interrupts.len(), 1);
    assert!(matches!(
        irq_group[0].interrupts[0].num,
        Interrupt::PciePerstUpIrq
    ));
    assert!(matches!(
        irq_group[0].interrupts[0].sensitivity,
        IrqSensitivity::Level
    ));
    assert_eq!(irq_group[0].interrupts[0].event_num, 0);
    assert_eq!(irq_group[0].next, 0);

    assert_eq!(irq_group[1].id, None);
    assert_eq!(irq_group[1].interrupts.len(), 1);
    assert!(matches!(
        irq_group[1].interrupts[0].num,
        Interrupt::PciePerstDownIrq
    ));
    assert!(matches!(
        irq_group[1].interrupts[0].sensitivity,
        IrqSensitivity::Level
    ));
    assert_eq!(irq_group[1].interrupts[0].event_num, 1);
    assert_eq!(irq_group[1].next, 0);

    assert_eq!(irq_group[2].id, Some("upka_done".to_string()));
    assert_eq!(irq_group[2].interrupts.len(), 2);
    assert!(matches!(
        irq_group[2].interrupts[0].num,
        Interrupt::Upka0DoneIrq
    ));
    assert!(matches!(
        irq_group[2].interrupts[0].sensitivity,
        IrqSensitivity::Edge
    ));
    assert_eq!(irq_group[2].interrupts[0].event_num, 2);
    assert_eq!(irq_group[2].next, 0);

    assert!(matches!(
        irq_group[2].interrupts[1].num,
        Interrupt::Upka1DoneIrq
    ));
    assert!(matches!(
        irq_group[2].interrupts[1].sensitivity,
        IrqSensitivity::Edge
    ));
    assert!(matches!(irq_group[2].interrupts[1].event_num, 3));

    assert_eq!(irq_group[3].id, Some("upka_err".to_string()));
    assert_eq!(irq_group[3].interrupts.len(), 2);
    assert!(matches!(
        irq_group[3].interrupts[0].num,
        Interrupt::Upka0ErrorIrq
    ));
    assert!(matches!(
        irq_group[3].interrupts[0].sensitivity,
        IrqSensitivity::Edge
    ));
    assert_eq!(irq_group[3].interrupts[0].event_num, 4);
    assert_eq!(irq_group[3].next, 0);

    assert!(matches!(
        irq_group[3].interrupts[1].num,
        Interrupt::Upka1ErrorIrq
    ));
    assert!(matches!(
        irq_group[3].interrupts[1].sensitivity,
        IrqSensitivity::Edge
    ));
    assert_eq!(irq_group[3].interrupts[1].event_num, 6);

    assert_eq!(irq_group[4].id, None);
    assert_eq!(irq_group[4].interrupts.len(), 1);
    assert!(matches!(
        irq_group[4].interrupts[0].num,
        Interrupt::UcdIbcqRr1Irq
    ));
    assert!(matches!(
        irq_group[4].interrupts[0].sensitivity,
        IrqSensitivity::Level
    ));
    assert_eq!(irq_group[4].interrupts[0].event_num, 5);
    assert_eq!(irq_group[4].next, 0);
}

#[test]
fn test_level_only_events() {
    #[derive(mcr_event_loop_derive::Event, Copy, Clone, Eq, PartialEq)]
    pub enum TestEvent {
        #[event(interrupt = Interrupt::PciePerstUpIrq)]
        PerstUp,

        #[event(interrupt = Interrupt::PciePerstDownIrq, sensitivity = IrqSensitivity::Level)]
        PerstDown,

        #[event(interrupt = Interrupt::UcdIbcqRr1Irq)]
        UcdIbcq,
    }

    let irq_group = TestEvent::interrupts();

    assert_eq!(irq_group.len(), 3);

    assert_eq!(irq_group[0].id, None);
    assert_eq!(irq_group[0].interrupts.len(), 1);
    assert!(matches!(
        irq_group[0].interrupts[0].num,
        Interrupt::PciePerstUpIrq
    ));
    assert!(matches!(
        irq_group[0].interrupts[0].sensitivity,
        IrqSensitivity::Level
    ));
    assert_eq!(irq_group[0].interrupts[0].event_num, 0);
    assert_eq!(irq_group[0].next, 0);

    assert_eq!(irq_group[1].id, None);
    assert_eq!(irq_group[1].interrupts.len(), 1);
    assert!(matches!(
        irq_group[1].interrupts[0].num,
        Interrupt::PciePerstDownIrq
    ));
    assert!(matches!(
        irq_group[1].interrupts[0].sensitivity,
        IrqSensitivity::Level
    ));
    assert_eq!(irq_group[1].interrupts[0].event_num, 1);
    assert_eq!(irq_group[1].next, 0);

    assert_eq!(irq_group[2].id, None);
    assert_eq!(irq_group[2].interrupts.len(), 1);
    assert!(matches!(
        irq_group[2].interrupts[0].num,
        Interrupt::UcdIbcqRr1Irq
    ));
    assert!(matches!(
        irq_group[2].interrupts[0].sensitivity,
        IrqSensitivity::Level
    ));
    assert_eq!(irq_group[2].interrupts[0].event_num, 2);
    assert_eq!(irq_group[2].next, 0);
}

#[test]
fn test_level_only_events_with_inferred_sensitivity() {
    #[derive(mcr_event_loop_derive::Event, Copy, Clone, Eq, PartialEq)]
    pub enum TestEvent {
        #[event(interrupt = Interrupt::PciePerstUpIrq)]
        PerstUp,

        #[event(interrupt = Interrupt::PciePerstDownIrq)]
        PerstDown,

        #[event(interrupt = Interrupt::UcdIbcqRr1Irq)]
        UcdIbcq,
    }

    let irq_group = TestEvent::interrupts();

    assert_eq!(irq_group.len(), 3);

    assert_eq!(irq_group[0].id, None);
    assert_eq!(irq_group[0].interrupts.len(), 1);
    assert!(matches!(
        irq_group[0].interrupts[0].num,
        Interrupt::PciePerstUpIrq
    ));
    assert!(matches!(
        irq_group[0].interrupts[0].sensitivity,
        IrqSensitivity::Level
    ));
    assert_eq!(irq_group[0].interrupts[0].event_num, 0);
    assert_eq!(irq_group[0].next, 0);

    assert_eq!(irq_group[1].id, None);
    assert_eq!(irq_group[1].interrupts.len(), 1);
    assert!(matches!(
        irq_group[1].interrupts[0].num,
        Interrupt::PciePerstDownIrq
    ));
    assert!(matches!(
        irq_group[1].interrupts[0].sensitivity,
        IrqSensitivity::Level
    ));
    assert_eq!(irq_group[1].interrupts[0].event_num, 1);
    assert_eq!(irq_group[1].next, 0);

    assert_eq!(irq_group[2].id, None);
    assert_eq!(irq_group[2].interrupts.len(), 1);
    assert!(matches!(
        irq_group[2].interrupts[0].num,
        Interrupt::UcdIbcqRr1Irq
    ));
    assert!(matches!(
        irq_group[2].interrupts[0].sensitivity,
        IrqSensitivity::Level
    ));
    assert_eq!(irq_group[2].interrupts[0].event_num, 2);
    assert_eq!(irq_group[2].next, 0);
}

#[test]
fn test_edge_only_events() {
    #[derive(mcr_event_loop_derive::Event, Copy, Clone, Eq, PartialEq)]
    pub enum TestEvent {
        #[event(interrupt = Interrupt::Upka0DoneIrq, group = "upka_done", sensitivity = IrqSensitivity::Edge)]
        Upka0Done,

        #[event(group = "upka_done", sensitivity = IrqSensitivity::Edge, interrupt = Interrupt::Upka1DoneIrq)]
        Upka1Done,

        #[event(sensitivity = IrqSensitivity::Edge, interrupt = Interrupt::Upka0ErrorIrq, group = "upka_err")]
        Upka0Error,

        #[event(interrupt = Interrupt::Upka1ErrorIrq, group = "upka_err", sensitivity = IrqSensitivity::Edge)]
        Upka1Error,
    }

    let irq_group = TestEvent::interrupts();

    assert_eq!(irq_group.len(), 2);

    assert_eq!(irq_group[0].id, Some("upka_done".to_string()));
    assert_eq!(irq_group[0].interrupts.len(), 2);
    assert!(matches!(
        irq_group[0].interrupts[0].num,
        Interrupt::Upka0DoneIrq
    ));
    assert!(matches!(
        irq_group[0].interrupts[0].sensitivity,
        IrqSensitivity::Edge
    ));
    assert_eq!(irq_group[0].interrupts[0].event_num, 0);
    assert_eq!(irq_group[0].next, 0);

    assert!(matches!(
        irq_group[0].interrupts[1].num,
        Interrupt::Upka1DoneIrq
    ));
    assert!(matches!(
        irq_group[0].interrupts[1].sensitivity,
        IrqSensitivity::Edge
    ));
    assert_eq!(irq_group[0].interrupts[1].event_num, 1);

    assert_eq!(irq_group[1].id, Some("upka_err".to_string()));
    assert_eq!(irq_group[1].interrupts.len(), 2);
    assert!(matches!(
        irq_group[1].interrupts[0].num,
        Interrupt::Upka0ErrorIrq
    ));
    assert!(matches!(
        irq_group[1].interrupts[0].sensitivity,
        IrqSensitivity::Edge
    ));
    assert_eq!(irq_group[1].interrupts[0].event_num, 2);
    assert_eq!(irq_group[1].next, 0);

    assert!(matches!(
        irq_group[1].interrupts[1].num,
        Interrupt::Upka1ErrorIrq
    ));
    assert!(matches!(
        irq_group[1].interrupts[1].sensitivity,
        IrqSensitivity::Edge
    ));
    assert_eq!(irq_group[1].interrupts[1].event_num, 3);
}
