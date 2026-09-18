# Cube

[![GitHub License](https://img.shields.io/github/license/coshz/cube?color=blue)](LICENSE)
[![GitHub Release](https://img.shields.io/github/v/release/coshz/cube?color=blue&logo=github)](https://github.com/coshz/cube/releases)
[![Hackage](https://img.shields.io/hackage/v/cube-hs.svg?logo=haskell&color=orange)](https://hackage.haskell.org/package/cube-hs)
[![Crates.io](https://img.shields.io/crates/v/cube-rust.svg)](https://crates.io/crates/cube-rust)
[![PyPI](https://img.shields.io/pypi/v/cube-python.svg)](https://pypi.org/project/cube-python/)
[![npm](https://img.shields.io/npm/v/@coshz/jscube.svg)](https://www.npmjs.com/package/@coshz/jscube)

An ultra-fast, cross-platform Rubik's Cube solver engine built with modern C++17, implementing Kociemba's Two-Phase Algorithm.

## ✨ Features

* **⚡ High Performance**: Fast and resource-efficient core implementation.
* **🌐 Multi-Language**: Native bindings for Python, Rust, Haskell, and WebAssembly/TypeScript.
* **📦 Cross-Platform SDK**: Ships C dynamic/static libraries, Swift module (`Cube.framework`), and interactive CLI (`icube`).
* **🛠️ Standalone**: Zero external third-party dependencies.


## 📦 Language Bindings

| Language | Directory & Guide | Package |
| :--- | :--- | :--- |
| **Python** | [bindings/python](bindings/python/README.md)| [cube-python](https://pypi.org/project/cube-python/) |
| **Rust** | [bindings/rust](bindings/rust/README.md)  | [cube-rust](https://crates.io/crates/cube-rust) |
| **Haskell** | [bindings/haskell](bindings/haskell/README.md) | [cube-hs](https://hackage.haskell.org/package/cube-hs) |
| **TypeScript** | [bindings/wasm](bindings/wasm/README.md) | [@coshz/jscube](https://www.npmjs.com/package/@coshz/jscube) |


## 🚀 Quick Start

### 1. Interactive CLI (`icube`)

```bash
# Launch interactive solver
icube

# Or solve directly via state string
icube solve "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"
```
<details>
  <summary>Click to view CLI screenshot (shotsnap)</summary>
  <img src="asset/icube-demo.png" alt="shotsnap-1">
</details>


### 2. SDK Usage

```c
#include <cube/cube.h>
#include <stdio.h>
#include <string.h>

int main() 
{
  char buf[CUBE_BS], sol[CUBE_BS];

  // apply maneuver to give cube (here, CUBE_ID)
  facecube(buf, "U F U' L2 R L' D2 B", CUBE_ID);

  // solve from buf to CUBE_ID
  SolveResult sr = solve(sol, buf, CUBE_ID, 30, true);

  if(sr == SolveResultSuccess) {
    puts(sol);
    // verify the solution 
    char solved[CUBE_BS];
    facecube(solved, sol, buf);
    if(strcmp(solved, CUBE_ID) == 0) {
      puts("solution verified");
    } else {
      puts("wrong solution");
    }
  } else {
    fprintf(stderr, "%s\n", solve_result_to_string(sr));
  }
  return 0;
}
```


## 🛠️ Build & Install

### Prerequisites
* CMake 3.15+
* C++17 compatible compiler (Clang, GCC, MSVC)
* Ninja (Optional, recommended)

### Instructions

```bash
# Configure & build core SDK
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Install C++ artifacts to system default path
cmake --build build --target install
```


## 📂 Repository Layout

```text
.
├── amalg/            # amalgamated C++ source
├── asset             # project static resources
├── bindings/         # Language bindings
├── cli/              # Source code for `icube` command-line tool
├── cmake/            # templates or modules for cmake use
├── dev/              # Scripts to eases development
├── include/          # Public C++ SDK headers
├── src/              # Core Two-Phase Algorithm implementation
└── test/             # Testing
```

## 📄 License

[MIT](LICENSE) © coshz

## 🔗 References

1. [http://kociemba.org/cube.htm](http://kociemba.org/cube.htm)
