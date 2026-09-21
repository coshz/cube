<div align="center">

# @coshz/jscube

![npm version](https://img.shields.io/npm/v/@coshz/jscube?style=flat-square&color=blue)
![license](https://img.shields.io/npm/l/@coshz/jscube?style=flat-square)
[![GitHub](https://img.shields.io/badge/github-coshz%2Fcube-blue?logo=github)](https://github.com/coshz/cube/tree/HEAD/bindings/wasm)

**High-performance, cross-platform WebAssembly Rubik's Cube core algorithm engine.**

[Submit Issue](https://github.com/coshz/cube/issues)

</div>

---

## ✨ Features

* **⚡ High Performance**: Powered by C++ compiled to WebAssembly for lightning-fast solving and state calculations.
* **🛡️ Type-Safe**: Out-of-the-box TypeScript definitions included.
* **🌐 Cross-Platform**: Seamlessly runs across both Node.js and browser environments.
* **🎯 Exception-Safe**: Supports both `solve` (throws on failure) and `trySolve` (safe result pattern) APIs.

---

## 📦 Installation

```bash
npm install @coshz/jscube
```

---

## 🚀 Quick Start

```typescript
import { createCubeAPI, CubeID } from '@coshz/jscube';

async function main() {
  // 1. Initialize API instance
  const cube = await createCubeAPI();

  // 2. Check state validity
  const scrambled = cube.facecube("U F U' L2 R L' D2 B")
  console.log(cube.solvable(scrambled)); // true

  // 3. Solve the cube
  try {
    const solution = cube.solve(scrambled, {target: CubeID});
    console.log("Solution: ", solution);
    // Verify solution
    const endState = cube.facecube(solution, scrambled);
    console.log("Is solved:", endState === CubeID); // true
  } catch (err) {
    console.error("Solving failed:", (err as Error).message);
    return;
  }
}

await main();
```

---
## 📖 API Reference

```typescript
import { createCubeAPI, CubeID, type CubeAPI, type SolveOptions, type SolveResult } from '@coshz/jscube';

// Initialize the API instance
const api: CubeAPI = await createCubeAPI();
```

### Methods

| Method | Description |
| :--- | :--- |
| `solve(src?, options?)` | Solves the cube state. Returns solution string or throws an error. |
| `trySolve(src?, options?)` | Safe solver. Returns `{ ok: true, solution }` or `{ ok: false, error }`. |
| `solvable(src)` | Returns `true` if the 54-character state string is physically solvable. |
| `facecube(maneuver, cube?)` | Applies move sequence (e.g., `"R U R' U'"`) and returns the new state. |
| `permutation(mc, fmt?)` | Returns the permutation array/string for a move sequence or a cube. |

### Types

```typescript
interface SolveOptions {
  target?: string;    // Target state (default: CubeID)
  maxSteps?: number;  // Max search depth (default: 30)
  best?: boolean;     // Search for optimal solution (default: true)
}

// Solved state constant (54 chars)
// CubeID = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"
```
---

## 📄 License

[MIT](LICENSE) © coshz