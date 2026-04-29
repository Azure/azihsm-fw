// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::error::HsmResult;

#[cfg(any(feature = "mcr_test_hooks", test))]
use mcr_ddi_types::DdiTestActionPinPolicyConfig;

use mcr_tcon::*;

/// The time in seconds the pin policy manager will increment the delay factor
pub const DELAY_FACTOR_INCREMENT_IN_SEC: u16 = 60;

/// The time in minutes that the pin policy manager will delay for before rolling over
pub const MAX_DELAY_FACTOR_IN_MINS: u16 = 32;

/// Pin policy manager
pub struct PinPolicyMgr<E: HsmEnvTrait + 'static> {
    /// Partition Env
    part_env: PartEnv<E>,

    /// Pin policy context
    pin_policy_ref: &'static mut PinPolicy,

    /// Delay increment override (sec) for DDI testing
    #[cfg(any(feature = "mcr_test_hooks", test))]
    delay_increment_override: Option<u16>,
}

impl<E: HsmEnvTrait + 'static> PinPolicyMgr<E> {
    pub fn new(env: PartEnv<E>, pfn: PcieFunction) -> Self {
        let persistent_store = env.part_persistent_store_ref(pfn.into());

        let pin_policy_ref = &mut persistent_store.pin_policy;

        Self {
            part_env: env,
            pin_policy_ref,
            #[cfg(any(feature = "mcr_test_hooks", test))]
            delay_increment_override: None,
        }
    }

    /// Enforce the pin policy
    pub fn enforce_pin_policy(&mut self, valid_attempt: bool) -> HsmResult<()> {
        if !self.can_login() {
            Err(HsmErr::LoginFailed)?;
        }

        if valid_attempt {
            // Successful login resets the state machine
            if self.pin_policy_ref.state == PinPolicyState::Lockout {
                self.reset_context();
            }

            Ok(())
        } else {
            if self.pin_policy_ref.state == PinPolicyState::Ready {
                // invalid login attempt and in ready state, transition to lockout state
                self.pin_policy_ref.state = PinPolicyState::Lockout;
            }
            // invalid login attempt and lockout state
            self.lockout();
            Err(HsmErr::InvalidUserCredential)
        }
    }

    /// Check if pin policy manager allows a login attempt
    pub fn can_login(&self) -> bool {
        self.pin_policy_ref.state == PinPolicyState::Ready
            || self.part_env.tcon_tsc() >= u64::from_le_bytes(self.pin_policy_ref.lockout_time)
    }

    /// Helper function to update the pin policy context (except the state)
    fn lockout(&mut self) {
        self.increment_allowed_attempts();

        self.set_lockout_time();
    }

    /// Helper function to update the pin policy context allowed attempts
    fn increment_allowed_attempts(&mut self) {
        self.pin_policy_ref.allowed_attempts += 1;

        if self.pin_policy_ref.allowed_attempts == MAX_ALLOWED_PIN_AUTH_ATTEMPTS {
            // Increment the delay factor
            self.increment_delay_factor();

            // Reset allowed_attempts
            self.pin_policy_ref.allowed_attempts = 0;
        }
    }

    /// Helper function to increment the pin policy context delay factor
    fn increment_delay_factor(&mut self) {
        self.pin_policy_ref.delay_factor += 1;

        if self.pin_policy_ref.delay_factor > MAX_DELAY_FACTOR_IN_MINS {
            self.reset_context();
        }
    }

    /// Helper function to reset the pin policy context (except the state)
    fn reset_context(&mut self) {
        self.pin_policy_ref.allowed_attempts = 0;
        self.pin_policy_ref.delay_factor = 0;
        self.pin_policy_ref.lockout_time = [0u8; 8];
    }

    /// Helper function to set the pin policy context lockout time
    fn set_lockout_time(&mut self) {
        #[cfg(not(feature = "mcr_test_hooks"))]
        let delay_increment = DELAY_FACTOR_INCREMENT_IN_SEC as u64;

        #[cfg(feature = "mcr_test_hooks")]
        let delay_increment = match self.delay_increment_override {
            Some(increment) => increment as u64,
            None => DELAY_FACTOR_INCREMENT_IN_SEC as u64,
        };

        let delay = u64::from(self.pin_policy_ref.delay_factor)
            * delay_increment
            * u64::from(Tcon::tsc_freq_hz());

        let lockout_time = self.part_env.tcon_tsc() + delay;

        self.pin_policy_ref.lockout_time = lockout_time.to_le_bytes();
    }

    #[cfg(test)]
    /// Return the current pin policy context
    pub fn pin_policy(&self) -> PinPolicy {
        *self.pin_policy_ref
    }

    #[cfg(test)]
    /// Helper function for unit testing to reset the lockout time to 0 ticks
    pub fn reset_lockout_time(&mut self) {
        self.pin_policy_ref.lockout_time = [0u8; 8];
    }

    #[cfg(any(feature = "mcr_test_hooks", test))]
    /// Override the pin policy context values for unit and ddi testing
    pub fn override_pin_policy_context(&mut self, pin_policy_config: DdiTestActionPinPolicyConfig) {
        if pin_policy_config.delay_increment.is_some() {
            self.delay_increment_override = pin_policy_config.delay_increment;
            self.set_lockout_time();
        }

        if let Some(state) = pin_policy_config.state {
            if state {
                self.pin_policy_ref.state = PinPolicyState::Ready;
            } else {
                self.pin_policy_ref.state = PinPolicyState::Lockout;
            }
        };

        if let Some(delay) = pin_policy_config.delay {
            self.pin_policy_ref.delay_factor = delay;
            self.set_lockout_time();
        };

        if let Some(allowed_attempts) = pin_policy_config.allowed_attempts {
            self.pin_policy_ref.allowed_attempts = allowed_attempts;
        };

        if let Some(lockout_delay) = pin_policy_config.lockout_delay {
            let lockout_time = self.part_env.tcon_tsc()
                + u64::from(lockout_delay) * u64::from(Tcon::tsc_freq_hz());
            self.pin_policy_ref.lockout_time = lockout_time.to_le_bytes();
        };
    }

    #[cfg(feature = "mcr_test_hooks")]
    /// Clear the pin policy manager
    pub fn clear(&mut self) {
        *self.pin_policy_ref = PinPolicy::default();
        self.delay_increment_override = None;
    }
}
