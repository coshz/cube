use crate::ffi;

use std::error::Error;
use std::fmt;
use std::ffi::{CStr};

/// Errors that can occur during cube solving and state manipulation.
#[repr(i32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum CubeError {
    /// The cube state violates physical or state constraints and cannot be solved.
    Unsolvable = ffi::SolveResult::Unsolvable as i32,
    /// No solution was found within the specified maximum step limit.
    NotFound = ffi::SolveResult::NotFound as i32,
    /// The input source cube string format is invalid.
    InvalidSrc = ffi::SolveResult::InvalidSrc as i32,
    /// The input target cube string format is invalid.
    InvalidTgt = ffi::SolveResult::InvalidTgt as i32,
    /// An unknown internal error occurred.
    UnknownErr = ffi::SolveResult::UnknownErr as i32,
}

impl From<ffi::SolveResult> for CubeError {
    fn from(res: ffi::SolveResult) -> Self {
        match res {
            ffi::SolveResult::Unsolvable => CubeError::Unsolvable,
            ffi::SolveResult::NotFound => CubeError::NotFound,
            ffi::SolveResult::InvalidSrc => CubeError::InvalidSrc,
            ffi::SolveResult::InvalidTgt => CubeError::InvalidTgt,
            _ => CubeError::UnknownErr,
        }
    }
}

impl fmt::Display for CubeError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let c_code = match self {
            CubeError::Unsolvable => ffi::SolveResult::Unsolvable,
            CubeError::NotFound => ffi::SolveResult::NotFound,
            CubeError::InvalidSrc => ffi::SolveResult::InvalidSrc,
            CubeError::InvalidTgt => ffi::SolveResult::InvalidTgt,
            CubeError::UnknownErr => ffi::SolveResult::UnknownErr,
        };
        unsafe {
            let c_str_ptr = ffi::solve_result_to_string(c_code);
            let description = CStr::from_ptr(c_str_ptr).to_string_lossy();
            write!(f, "{}", description)
        }
    }
}

impl Error for CubeError {}

#[derive(Debug, PartialEq, Eq)]
pub enum CubeStringError {
    InvalidLength(usize),
    InvalidManeuver,
    InteriorNulByte,
}

impl fmt::Display for CubeStringError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            CubeStringError::InvalidLength(len) => write!(f, "cube string length must be 54, got {}", len),
            CubeStringError::InvalidManeuver => write!(f, "maneuver string is not valid"),
            CubeStringError::InteriorNulByte => write!(f, "the string contains interior NUL byte"),
        }
    }
}

impl Error for CubeStringError {}