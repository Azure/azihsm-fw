// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]
#![forbid(unsafe_code)]

//! File contains definition of types used as templates and parameters.

/// Template parameter
#[derive(Copy, Clone)]
pub struct TbsParam {
    pub name: &'static str,
    pub offset: usize,
    pub len: usize,
}

impl TbsParam {
    /// Create an instance `TbsParam`
    pub fn new(name: &'static str, offset: usize, len: usize) -> Self {
        Self { name, offset, len }
    }
}

/// Template
pub struct TbsTemplate {
    buf: Vec<u8>,
    params: Vec<TbsParam>,
}

impl TbsTemplate {
    /// Create an instance of `TbsTemplate`
    pub fn new(template: Vec<u8>, params: Vec<TbsParam>) -> Self {
        Self {
            buf: template,
            params,
        }
    }

    /// Retrieve template blob
    pub fn tbs(&self) -> &[u8] {
        &self.buf
    }

    /// Retrieve template parameters
    pub fn params(&self) -> &[TbsParam] {
        &self.params
    }
}
