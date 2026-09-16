//! # cube-rust
//!
//! A fast, self-contained Rust library for Rubik's Cube solving and state manipulation.
//!
//! ## Quick Start
//!
//! ```rust
//! use cube_rust::{Cube, CubeSolver};
//! 
//! // Initialize a cube in standard solved state
//! let cube = Cube::default();
//! 
//! // Apply moves
//! cube.apply_maneuver("R U R' U'").unwrap();
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

pub(crate) mod ffi;
pub(crate) mod err;

use err::{CubeError, CubeStringError};
use ffi::CUBE_BS;
pub use ffi::CUBE_ID;

use std::ffi::{CStr, CString};
use std::fmt;
use std::ops::Deref;
use std::os::raw::c_char;

/// An opaque domain type wrapping a 54-facelet cube configuration string.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Cube(CString);

impl Cube {
    pub fn new(s: impl AsRef<str>) -> Result<Self, CubeStringError> {
        let str_ref = s.as_ref();
        if str_ref.len() != 54 {
            return Err(CubeStringError::InvalidLength(str_ref.len()))
        }
        let c_str = CString::new(s.as_ref()).map_err(|_| CubeStringError::InteriorNulByte)?;
        Ok(Cube(c_str))
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
    pub fn apply_maneuver_mut(&mut self, maneuver: impl AsRef<str>) -> Result<(), CubeStringError>{
        let c_maneuver = CString::new(maneuver.as_ref()).map_err(|_| CubeStringError::InteriorNulByte)?;
        let mut buffer = [0u8; CUBE_BS];
        
        let success = unsafe {
            ffi::facecube(
                buffer.as_mut_ptr() as *mut c_char,
                c_maneuver.as_ptr(),
                self.as_ptr()
            )
        }; 

        if !success {
            return Err(CubeStringError::InvalidManeuver);
        }

        let c_str = unsafe { CStr::from_ptr(buffer.as_ptr() as *const c_char) };
        self.0 = c_str.to_owned();
        Ok(())
    }

    /// Returns a new `Cube` after applying a maneuver sequence (immutable version).
    pub fn apply_maneuver(&self, maneuver: impl AsRef<str>) -> Result<Cube, CubeStringError> {
        let mut cloned = self.clone();
        cloned.apply_maneuver_mut(maneuver)?;
        Ok(cloned)
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
        Cube::new(CUBE_ID).unwrap()
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

/// Converts a color cube or move sequence into its structural permutation string representation.
pub fn permutation(ms_or_cube: impl AsRef<str>, fmt: i32) -> String {
    let Ok(ms_or_cube) = CString::new(ms_or_cube.as_ref()) else {
        return "???".to_string();
    };

    let mut buffer = [0u8; CUBE_BS];
    unsafe {
        let success = ffi::permutation(
            buffer.as_mut_ptr() as *mut c_char,
            ms_or_cube.as_ptr(), 
            fmt
        );
        if !success {
            return "???".to_string();
        }
        CStr::from_ptr(buffer.as_ptr() as *const c_char)
            .to_string_lossy()
            .into_owned()
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
            ffi::solve(
                buffer.as_mut_ptr() as *mut c_char,
                src_ptr,
                tgt_ptr,
                self.step,
                self.best,
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