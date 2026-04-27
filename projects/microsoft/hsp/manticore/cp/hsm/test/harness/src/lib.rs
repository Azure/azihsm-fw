// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

// Some panic handler needs to be included. This one halts the processor on panic.

//extern crate panic_halt;

use core::fmt;
use core::fmt::Write;

#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => ($crate::_print(format_args!($($arg)*)));
}

#[macro_export]
macro_rules! println {
    () => ($crate::print!("\n"));
    ($($arg:tt)*) => ($crate::print!("{}\n", format_args!($($arg)*)));
}

#[doc(hidden)]
pub fn _print(args: fmt::Arguments) {
    mcr_uart::Uart::default().write_fmt(args).unwrap();
}

#[macro_export]
macro_rules! test_suite {
    ($($test_case: ident,)*) => {
        #[panic_handler]
        pub fn panic(info: &core::panic::PanicInfo) -> ! {
            println!("[failed]");
            println!("Error: {}\n", info);
            loop{}
        }

        #[no_mangle]
        pub extern "C" fn exec() {
            $(
                $test_case.run();
            )*
        }

        extern crate alloc;
        use embedded_alloc::TlsfHeap as Heap;
        #[global_allocator]
        static HEAP: Heap = Heap::empty();

        #[entry]
        fn app_entry() -> ! {

            {
                use core::mem::MaybeUninit;
                const HEAP_SIZE: usize = 2048;
                static mut HEAP_MEM: [MaybeUninit<u8>; HEAP_SIZE] = [MaybeUninit::uninit(); HEAP_SIZE];
                unsafe { HEAP.init(HEAP_MEM.as_ptr() as usize, HEAP_SIZE) }
            }
            exec();
            loop {};
        }
    };
}

pub trait Testable {
    fn run(&self);
}

impl<T> Testable for T
where
    T: Fn(),
{
    fn run(&self) {
        print!("{}...\t", core::any::type_name::<T>());
        self();
        println!("[ok]");
    }
}
