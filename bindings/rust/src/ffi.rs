//! Raw FFI bindings to the underlying C++ Rubik's Cube solver library.

use std::os::raw::{c_char, c_int};

/// Default buffer byte size for C++ string outputs.
pub const CUBE_BS: usize = 128;

/// The 54-facelet string representation of a solved Rubik's Cube (U, R, F, D, L, B).
pub const CUBE_ID: &str = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

/// Raw result codes returned by the C++ solver.
#[repr(i32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum SolveResult {
    Success = 0,
    Unsolvable = 1,
    NotFound = 2,
    InvalidSrc = 3,
    InvalidTgt = 4,
    UnknownErr = 5,
}

extern "C" {
    /// Converts a result code into a human-readable C-string description
    pub fn solve_result_to_string(sr: SolveResult) -> *const c_char;

    /// Core solve function
    pub fn solve(
        buf: *mut c_char,
        src: *const c_char,
        tgt: *const c_char,
        step: c_int,
        best: bool,
    ) -> SolveResult;

    /// Checks if a given color cube represents a valid, solvable cube
    pub fn solvable(color_cube: *const c_char) -> bool;

    /// Applies a maneuver string to a color cube and writes the resulting state to the buffer
    pub fn facecube(
        buf: *mut c_char, 
        maneuver: *const c_char,
        cube: *const c_char
    ) -> bool;

    /// Converts a color cube OR a maneuver string into its string representation
    pub fn permutation(
        buf: *mut c_char, 
        ms_or_cube: *const c_char, 
        format: c_int
    ) -> bool;
}