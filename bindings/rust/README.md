# cube-rust

[![Crates.io](https://img.shields.io/crates/v/cube-rust.svg)](https://crates.io/crates/cube-rust)
[![Documentation](https://docs.rs/cube-rust/badge.svg)](https://docs.rs/cube-rust)
[![License](https://img.shields.io/crates/l/cube-rust.svg)](https://github.com/coshz/cube/tree/master/bindings/rust#license)
[![GitHub](https://img.shields.io/badge/github-coshz%2Fcube-blue?logo=github)](https://github.com/coshz/cube/tree/master/bindings/rust)

A high-performance, memory-safe Rust FFI wrapper for a C++ Rubik's Cube solver engine (Two-Phase Algorithm).

## 🚀 Features

- **Idiomatic API Design**: Supports both functional chaining for immutable transformations and efficient in-place mutation.
- **Rich Error Diagnostics**: `CubeError` fully implements `std::error::Error` and `Display`, accurately exposing underlying C++ engine diagnostic messages.
- **Batteries Included**: Zero manual C++ configuration required. Automatically compiles C++17 source code and handles static linking via `cc` build script.

## 📦 Installation

Add `cube-rust` to your project using `cargo`:
```bash
cargo add cube-rust
```
Or add it manually to `Cargo.toml`: 
```toml
[dependencies]
cube-rust = "0.3.1"
```

## 💡 Quick Start

```rust
use cube_rust::{Cube, CubeError};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // 1. Initialize a cube in standard solved state
    let cube = Cube::default();

    // 2. Apply moves 
    cube.apply_maneuver_mut("R U F' D2").unwrap();

    // 3. Check if the scrambled state is solvable
    assert!(cube.is_solvable());

    // 4. Solve the cube safely
    match cube.solve() {
        Ok(solution) => println!("Solution: {solution}"),
        Err(err) => eprintln!("Failed to solve: {err}"),
    }

    Ok(())
}
```

