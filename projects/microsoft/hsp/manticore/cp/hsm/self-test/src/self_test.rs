// Copyright (c) Microsoft Corporation. All rights reserved.

macro_rules! define_self_tests {
    ($name:ident { $($variant:ident),* $(,)? }) => {
        /// List of all Cryptographic Algorithmic Self Test
        #[repr(C)]
        #[derive(Copy, Clone, PartialEq, Eq)]
        pub enum $name {
            $(
                #[doc = concat!("Self test for ", stringify!($variant))]
                $variant
            ),*
        }

        impl $name {
            /// Return all the self tests
            pub fn all() -> &'static [$name] {
                &[$($name::$variant),*]
            }

            /// Get the engine instance for the self test
            pub fn get_engine_instance(&self) -> Option<usize> {
                match self {
                    $($name::$variant => stringify!($variant)
                        .chars()
                        .rev()
                        .take_while(|c| c.is_digit(10))
                        .collect::<String>()
                        .chars()
                        .rev()
                        .collect::<String>()
                        .parse::<usize>()
                        .ok(),)*
                }
            }

            /// Check if the test matches given test base value and instance
            pub fn is_matching_test(&self, matching_id: Self, instance: usize) -> bool {
                let class_name = match matching_id {
                    $($name::$variant => stringify!($variant)
                        .trim_end_matches(|c: char| c.is_numeric()),)*
                };

                match self {
                    $($name::$variant if stringify!($variant).starts_with(class_name) => {
                        self.get_engine_instance() == Some(instance)
                    })*
                    _ => false,
                }
            }
        }

        impl From<usize> for $name {
            fn from(value: usize) -> Self {
                match value {
                    $(x if x == $name::$variant as usize => $name::$variant),*,
                    _ => $name::SelfTestCompleted
                }
            }
        }

        impl TryFrom<u32> for $name {
            type Error = ();

            fn try_from(value: u32) -> Result<Self, Self::Error> {
                match value {
                    $(x if x == $name::$variant as u32 => Ok($name::$variant)),*,
                    _ => Err(())
                }
            }
        }

        impl TryInto<u32> for $name {
            type Error = ();

            fn try_into(self) -> Result<u32, Self::Error> {
                match self {
                    $(SelfTest::$variant => Ok($name::$variant as u32),)*
                }
            }
        }

        impl TryFrom<u16> for $name {
            type Error = ();

            fn try_from(value: u16) -> Result<Self, Self::Error> {
                match value {
                    $(x if x == $name::$variant as u16 => Ok($name::$variant)),*,
                    _ => Err(())
                }
            }
        }
    };
}
