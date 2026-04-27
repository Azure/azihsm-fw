// Copyright (c) Microsoft Corporation. All rights reserved.

#![allow(unexpected_cfgs)]
#![cfg_attr(not(feature = "std"), no_std)]
#![cfg_attr(not(feature = "std"), no_main)]

cfg_if::cfg_if! {
    if #[cfg(not(feature = "std"))] {
        // extern crate alloc;

        #[allow(unused_imports)]
        use core::mem::MaybeUninit;
        use cortex_m_rt::entry;
        use embedded_alloc::TlsfHeap as Heap;

        #[global_allocator]
        #[allow(static_mut_refs)]
        static HEAP: Heap = Heap::empty();

        use mcr_exception_handlers as _;
    }
}

extern crate alloc;

mod cmd_scheduler;
mod context;
mod env;
mod error;
mod event;
mod fsm;
mod function;
mod handler;
mod preop_cdma_io;
mod recorder;
mod resource;

#[cfg(test)]
mod mock;

#[macro_use]
mod admin_logging_macros;

use crate::recorder::AdminFsmEventRecorder;
pub(crate) use cmd_scheduler::*;
pub(crate) use context::AdminFsmContext;
pub(crate) use env::AdminEnv;
pub(crate) use env::AdminEnvTrait;
pub(crate) use event::AdminFsmEvent;
pub(crate) use event::NvicEvent;
pub(crate) use handler::AdminEventHandler;
use mcr_event_loop::EventLoopInterface;
use mcr_event_loop_nvic::NvicEventLoop;
use mcr_interrupt_controller::InterruptController;
use mcr_logging::*;
use mcr_types::DebugLogComponent;
use mcr_types::DebugLogEntryParameters;
use mcr_types::DebugLogSeverity;

pub fn run() -> ! {
    let event_loop = NvicEventLoop::<NvicEvent, InterruptController>::default();
    event_loop.run(|mut events| {
        // Intialize Debug log sender
        AdminEnv::initialize_debug_log_sender();

        let env = match AdminEnv::new() {
            Ok(env) => env,
            Err(err) => panic!("Failed to initialize Admin environment: {:#x}", err),
        };

        info!("Starting Admin event loop...");

        let mut handler = AdminEventHandler::new(env, AdminFsmEventRecorder::default());
        loop {
            for event in &mut events {
                handler.on_event(event.into());
            }

            if let Some(soft_event) = handler.check_soft_events() {
                handler.on_event(soft_event)
            }

            // Populate AES GCM IVs
            handler.fill_aes_gcm_iv_queue();
        }
    });

    unreachable!()
}

#[cfg(not(feature = "std"))]
#[entry]
fn app_entry() -> ! {
    // Initialize heap
    const HEAP_SIZE: usize = 72 * 1024;
    static mut HEAP_MEM: [core::mem::MaybeUninit<u8>; HEAP_SIZE] =
        [core::mem::MaybeUninit::uninit(); HEAP_SIZE];
    #[allow(static_mut_refs)]
    unsafe {
        HEAP.init(HEAP_MEM.as_ptr() as usize, HEAP_SIZE);
    }
    mcr_logger::Logger::init(log::LevelFilter::Info);
    run()
}

#[cfg(feature = "std")]
fn main() {}
