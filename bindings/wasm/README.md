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
import { createCubeApi } from '@coshz/jscube';

async function main() {
  // 1. Initialize API instance
  const cube = await createCubeApi();

  // 2. Check state validity
  const scrambled = cube.facecube("U F U' L2 R L' D2 B")
  console.log(cube.solvable(scrambled)); // true

  // 3. Solve the cube
  try {
    const solution = cube.solve(scrambled, {target:cube.cubeId,goodEnough:20});
    console.log("Solution: ", solution);
    // Verify solution
    const endState = cube.facecube(solution, scrambled);
    console.log("Is solved:", endState === cube.cubeId); // true
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
import { createCubeApi, type CubeApi, type SolveOptions, type SolveResult, PermFormat } from '@coshz/jscube';

// Initialize the API instance
const api: CubeApi = await createCubeApi();
```

### Methods

| Method | Description |
| :--- | :--- |
| `solve(src?, options?)` | Solves the cube state. Returns a solution string or throws an error. |
| `trySolve(src?, options?)` | Safe solver. Returns a `SolveResult` object. |
| `solvable(src)` | Returns `true` if the 54-character state string is physically solvable. |
| `facecube(maneuver, cube?)` | Applies move sequence (e.g., `"R U R' U'"`) and returns the new state. |
| `permutation(ms_or_cube, fmt?)` | Returns the permutation array/string for a move sequence or a cube. |

### Constants & Types

```typescript
interface CubeApi {
  readonly cubeId: string;
  solvable: (src: string) => boolean;
  facecube: (maneuver: string, cube?: string) => string;
  permutation: (maneuver: string, fmt?: PermFormat) => string;
  trySolve: (src: string, options?: SolveOptions) => SolveResult; // no exception
  solve: (src: string, options?: SolveOptions) => string; // with exception
}

interface SolveOptions {
  target?: string;      // Target state (default: CubeID)
  goodEnough?: number;  // Acceptable solution length (default: 22)
}

type SolveResult =
  | { ok: true; solution: string }
  | { ok: false; error: string }
;

// Permutation output format (default: PermFormat.Cycle)
const PermFormat = { Face: 0, Cubie: 1, Cycle: 2 } as const;
type PermFormat = typeof PermFormat[keyof typeof PermFormat];

// Solved state constant (54 chars)
// cubeId = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"
```
---

## 📄 License

[MIT](LICENSE) © coshz