//! # cube-rust
//!
//! A fast, self-contained Rust library for Rubik's Cube solving and state manipulation.
//!
//! ## Quick Start
//!
//! ```rust
//! use cube_rust::{Cube, CubeSolver};
//!
//! // Construct a solved cube and apply a scramble maneuver sequence
//! let cube = Cube::default().apply_maneuver("R U R' U'");
//!
//! // Verify solvability
//! assert!(cube.is_solvable());
//!
//! // Solve the cube
//! match cube.solve() {
//!     Ok(sol) => println!("Solution: {sol}"),
//!     Err(err) => eprintln!("Error: {err}"),
//! }
//! ```

pub mod ffi;

use std::error::Error;
use std::ffi::{CStr, CString};
use std::fmt;
use std::ops::Deref;
use std::os::raw::c_char;

use ffi::CUBE_BS;
pub use ffi::CUBE_ID;

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

/// An opaque domain type wrapping a 54-facelet cube configuration string.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Cube(CString);

impl Cube {
    /// Creates a new `Cube` from a string representation.
    ///
    /// # Panics
    /// Panics if the input string contains an interior NUL byte (`\0`).
    pub fn new(s: impl AsRef<str>) -> Self {
        let c_str = CString::new(s.as_ref()).expect("cube string contains interior NUL byte");
        Cube(c_str)
    }

    pub(crate) fn as_ptr(&self) -> *const c_char {
        self.0.as_ptr()
    }

    pub(crate) fn as_str(&self) -> &str {
        self.0.to_str().expect("Cube string is valid UTF-8")
    }

    /// Checks if the cube configuration is valid and solvable.
    pub fn is_solvable(&self) -> bool {
        unsafe { ffi::solvable(self.as_ptr()) }
    }

    /// Modifies `self` in place by applying a maneuver sequence (e.g., `"R U R' U'"`).
    pub fn apply_maneuver_mut(&mut self, maneuver: impl AsRef<str>) {
        let c_maneuver = CString::new(maneuver.as_ref()).expect("maneuver contains interior NUL byte");
        let mut buffer = [0u8; CUBE_BS];
        unsafe {
            ffi::facecube(
                self.as_ptr(),
                c_maneuver.as_ptr(),
                buffer.as_mut_ptr() as *mut c_char,
            );
            let c_str = CStr::from_ptr(buffer.as_ptr() as *const c_char);
            self.0 = c_str.to_owned();
        }

    }

    /// Returns a new `Cube` after applying a maneuver sequence (immutable version).
    pub fn apply_maneuver(&self, maneuver: impl AsRef<str>) -> Cube {
        let mut cloned = self.clone();
        cloned.apply_maneuver_mut(maneuver);
        cloned
    }

    /// Solves the current cube using default solver configuration (30 max steps, optimal search).
    ///
    /// # Errors
    /// Returns [`CubeError`] if the cube is unsolvable or if no solution is found within the step limit.
    pub fn solve(&self) -> Result<String, CubeError> {
        CubeSolver::new().src(self.clone()).solve()
    }
}

/// Returns a `Cube` in the default solved state
impl Default for Cube {
    fn default() -> Self {
        Cube::new(CUBE_ID)
    }
}

impl Deref for Cube {
    type Target = str;

    fn deref(&self) -> &Self::Target {
        self.as_str()
    }
}

impl fmt::Display for Cube {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.as_str())
    }
}

impl From<&str> for Cube {
    fn from(s: &str) -> Self {
        Cube::new(s)
    }
}

impl From<String> for Cube {
    fn from(s: String) -> Self {
        Cube::new(s)
    }
}

/// Converts a move sequence (e.g., `"R U R' U'"`) into its structural permutation string representation.
pub fn permutation(maneuver: impl AsRef<str>) -> String {
    let c_maneuver = CString::new(maneuver.as_ref()).expect("maneuver contains interior NUL byte");
    let mut buffer = [0u8; CUBE_BS];
    unsafe {
        ffi::permutation(c_maneuver.as_ptr(), buffer.as_mut_ptr() as *mut c_char);
        let c_str = CStr::from_ptr(buffer.as_ptr() as *const c_char);
        c_str.to_string_lossy().into_owned()
    }
}

/// A builder for configuring and executing Rubik's Cube solving operations.
#[derive(Debug, Clone)]
pub struct CubeSolver {
    src: Option<Cube>,
    tgt: Option<Cube>,
    step: i32,
    best: bool,
}

impl CubeSolver {
    /// Creates a new solver builder with default parameters (30 max steps, optimal search enabled).
    pub fn new() -> Self {
        Self {
            src: None,
            tgt: None,
            step: 30,
            best: true,
        }
    }

    /// Sets the source cube state to solve from (defaults to `None`, representing solved state).
    pub fn src(mut self, src: Cube) -> Self {
        self.src = Some(src);
        self
    }

    /// Sets the target cube state to solve towards (defaults to `None`, representing solved state `CUBE_ID`).
    pub fn tgt(mut self, tgt: Cube) -> Self {
        self.tgt = Some(tgt);
        self
    }

    /// Sets the maximum search depth limit in steps (default: `30`).
    pub fn max_steps(mut self, step: i32) -> Self {
        self.step = step;
        self
    }

    /// Toggles optimal solution search mode (default: `true`).
    pub fn best(mut self, best: bool) -> Self {
        self.best = best;
        self
    }

    /// Executes the solving algorithm.
    ///
    /// # Errors
    /// Returns [`CubeError`] if no solution is found or if the cube state is invalid.
    pub fn solve(self) -> Result<String, CubeError> {
        let src_ptr = self.src.as_ref().map_or(std::ptr::null(), |s| s.as_ptr());
        let tgt_ptr = self.tgt.as_ref().map_or(std::ptr::null(), |s| s.as_ptr());

        let mut buffer = [0u8; CUBE_BS];

        let res = unsafe {
            ffi::solve_ultimate(
                src_ptr,
                tgt_ptr,
                buffer.as_mut_ptr() as *mut c_char,
                self.step,
                self.best,
                1,
            )
        };

        if res == ffi::SolveResult::Success {
            let c_str = unsafe { CStr::from_ptr(buffer.as_ptr() as *const c_char) };
            Ok(c_str.to_string_lossy().into_owned())
        } else {
            Err(res.into())
        }
    }
}

impl Default for CubeSolver {
    fn default() -> Self {
        Self::new()
    }
}