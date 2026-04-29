// Copyright (c) Microsoft Corporation. All rights reserved.

use std::env;

// RAII guard to ensure environment variable(s) cleanup
#[derive(Default)]
pub(crate) struct EnvVarGuard {
    vars: Vec<String>,
}

impl EnvVarGuard {
    pub fn set(&mut self, var_name: &'static str, value: &str) {
        env::set_var(var_name, value);
        self.vars.push(var_name.to_string());
    }
}

impl Drop for EnvVarGuard {
    fn drop(&mut self) {
        for var in self.vars.drain(..) {
            env::remove_var(var);
        }
    }
}
