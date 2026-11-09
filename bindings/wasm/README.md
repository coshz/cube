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
- **🎯 Result-based errors** — `solve` returns a discriminated `SolveResult`; narrow with `result.ok` instead of catching exceptions.

---

## 📦 Installation

```bash
npm install @coshz/jscube
```

---

## 🚀 Quick Start

```typescript
import { createNodeCubeApi } from '@coshz/jscube';

async function main() {
  const cube = await createNodeCubeApi();

  // Scramble & check legality
  const scrambled = cube.facecube("U F U' L2 R L' D2 B")
  console.log("scrambled: ", scrambled);
  console.log("solvable:", cube.solvable(scrambled));

  // useSymmetry=true: faster solving, but ~260 MB of tables. 
  // Build once on first run (a few minutes), then cached via TableManager. 
  await cube.init(true);
  
  // Solve & verify
  const solution = cube.solve(scrambled, { target: cube.cubeId, goodEnough: 22 });
  if (solution.ok) {
    console.log("solution: ", solution.solution);
    const endState = cube.facecube(solution.solution, scrambled);
    console.log("verified:", endState === cube.cubeId);
  } else {
    console.error("solving failed:", solution.error)
  }
}
await main();
```

Examples: See [demo-node](./examples/demo-node/), [demo-web](./examples/demo-web/) .

## 📖 API Reference

### Environments

```typescript
// Node.js
import type { NodeCubeApi } from '@coshz/jscube';
import { createNodeCubeApi, SolveResult, SolveOptions, PermFormat } from '@coshz/jscube';

const api: NodeCubeApi = await createNodeCubeApi()

// Browser (runs in a Web Worker)
import type { WorkerCubeApi } from '@coshz/jscube';
import { createWorkerCubeApi, SolveResult, SolveOptions, PermFormat } from '@coshz/jscube';

const api: WorkerCubeApi = await createWorkerCubeApi();
```

#### Usage order

| Method | Order |
| :--- | :--- |
| api . { solvable / permutation / facecube } | available immediately |
| api . init(useSymmetry)                     | load or build tables |
| api . solve(cube)                           | requires init() |

#### Trade-off

| Value | Table Size | Build time | Solve speed |
| :--- | :--- | :--- | :--- |
| `useSymmetry=false` | < 10 MB | fast | slower |
| `useSymmetry=true` | ~260 MB | a few minutes | faster |`

### Methods

| Method | Description |
| :--- | :--- |
| `init(useSymmetry)` | Loads or build tables; `useSymmetry` selects the table variant. |
| `solve(src, options?)` | Solver. Returns a `SolveResult` object. |
| `solvable(src)` | Returns `true` if the state string is physically solvable. |
| `facecube(maneuver, cube?)` | Applies move sequence (e.g., `"R U R' U'"`) and returns the new state. |
| `permutation(mOrc, fmt?)` | Returns the permutation array/string for a move sequence or a cube. |

### Types

```typescript
// Base API: functions that don't require tables
interface CubeApiBase {
  readonly cubeId: string; // "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"
  solvable: (src: string) => boolean;
  facecube: (maneuver: string, cube?: string) => string;
  permutation: (maneuver: string, fmt?: PermFormat) => string;
}
// Node.js
interface NodeCubeApi extends CubeApiBase {
  init: (useSymmetry?: boolean) => Promise<void>;
  solve: (src: string, options?: SolveOptions) => SolveResult;
}
// Browser (Web Worker) - solve is async. 
interface WorkerCubeApi extends CubeApiBase {
  init: (useSymmetry?: boolean) => Promise<void>;
  solve: (src: string, options?: SolveOptions) => Promise<SolveResult>;
}

interface SolveOptions {
  target?: string;      // Target state (default: cubeId)
  goodEnough?: number;  // Acceptable solution length (default: 22)
}

type SolveResult =
  | { ok: true; solution: string }
  | { ok: false; error: string }
;

// Permutation output format (default: PermFormat.Cycle).
// Also usable as a type.
const PermFormat = { Face: 0, Cubie: 1, Cycle: 2 } as const;
type PermFormat = typeof PermFormat[keyof typeof PermFormat];
```
---

## 📄 License

[MIT](LICENSE) © coshz