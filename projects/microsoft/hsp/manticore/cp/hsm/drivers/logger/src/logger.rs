// Copyright (c) Microsoft Corporation. All rights reserved.

use core::sync::atomic::AtomicU32;
use core::sync::atomic::Ordering;

use log::*;
use mcr_mem_map::GsRamMemMap;

cfg_if::cfg_if! {
    if #[cfg(not(feature = "disable_output"))] {
        use core::fmt::Write;
    }
}

/// Structure containing configuration data for logger.
pub struct Logger {
    /// The verbosity level of logging.
    level: LevelFilter,
}

/// The Log crate requires a static object instance of logger defined.
pub static mut LOGGER: Logger = Logger {
    level: LevelFilter::Off,
};

impl Logger {
    /// Helper function to get a reference to the atomic lock variable.
    ///
    /// # Arguments
    ///
    /// * None
    ///
    /// # Returns
    ///
    /// * Atomic instance to the lock variable
    fn lock() -> &'static AtomicU32 {
        let x = GsRamMemMap::logger_lock().as_ptr();
        let lock: &AtomicU32 = unsafe { &*x.cast() };

        lock
    }

    /// Acquire the logger lock to ensure mutual exclusion.
    ///
    /// # Arguments
    ///
    /// * None
    ///
    /// # Returns
    ///
    /// * None
    fn acquire() {
        let lock = Self::lock();

        loop {
            let res = lock.compare_exchange(0, 1, Ordering::Acquire, Ordering::Relaxed);
            if let Ok(val) = res {
                mcr_cpu::dmb();
                if val == 0 {
                    break;
                }
            }
        }
    }

    /// Release the logger lock.
    ///
    /// # Arguments
    ///
    /// * None
    ///
    /// # Returns
    ///
    /// * None
    fn release() {
        let lock = Self::lock();

        lock.store(0, Ordering::Release);
        mcr_cpu::dmb();
    }

    /// Initialize the logger object.
    ///
    /// # Arguments
    ///
    /// * `log_level` - The verbosity level of logging.
    ///
    /// # Returns
    ///
    /// * None
    pub fn init(log_level: LevelFilter) {
        unsafe {
            LOGGER.level = log_level;
            #[allow(static_mut_refs)]
            set_logger(&LOGGER).unwrap();
            set_max_level(LOGGER.level);
        }
    }
}

impl log::Log for Logger {
    fn enabled(&self, metadata: &Metadata) -> bool {
        metadata.level() <= self.level
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            Logger::acquire();

            #[cfg(not(feature = "disable_output"))]
            mcr_uart::Uart::default()
                .write_fmt(format_args!(
                    "[{}:{}] [{}] {}\n",
                    mcr_cpu::cpu_id() as u8,
                    record.level(),
                    record.target(),
                    record.args(),
                ))
                .unwrap();

            Logger::release();
        }
    }

    fn flush(&self) {}
}
