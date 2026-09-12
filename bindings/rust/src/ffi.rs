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
    /// Converts a result code into a human-readable C-string description from C++.
    pub fn solve_result_to_string(sr: SolveResult) -> *const c_char;

    /// Core solve function accepting source state, target state, buffer, step limit, and search options.
    pub fn solve_ultimate(
        src: *const c_char,
        tgt: *const c_char,
        solution_buffer: *mut c_char,
        step: c_int,
        best: bool,
        formated: c_int,
    ) -> SolveResult;

    /// Checks if a given facelet configuration string represents a valid, solvable cube.
    pub fn solvable(color_cube: *const c_char) -> bool;

    /// Applies a maneuver sequence to a facelet configuration and writes the resulting state to the buffer.
    pub fn facecube(cube: *const c_char, maneuver: *const c_char, cube_buffer: *mut c_char);

    /// Converts a maneuver sequence into its structural permutation string representation.
    pub fn permutation(maneuver: *const c_char, perm_buffer: *mut c_char);
}