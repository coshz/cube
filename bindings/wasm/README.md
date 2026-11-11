<div align="center">

# @coshz/jscube

![npm version](https://img.shields.io/npm/v/@coshz/jscube?style=flat-square&color=blue)
![license](https://img.shields.io/npm/l/@coshz/jscube?style=flat-square)
[![GitHub](https://img.shields.io/badge/github-coshz%2Fcube-blue?logo=github)](https://github.com/coshz/cube/tree/HEAD/bindings/wasm)

**High-performance, cross-platform WebAssembly Rubik's Cube core algorithm engine.**

[Changelog](https://github.com/coshz/cube/blob/main/bindings/wasm/CHANGELOG) ·
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
import { createCubeApi } from '@coshz/jscube';

async function main() {
  const cube = await createCubeApi();

  // Scramble & check legality
  const scrambled = cube.facecube("U F U' L2 R L' D2 B")
  console.log("scrambled: ", scrambled);
  console.log("solvable:", cube.solvable(scrambled));

  // useSymmetry=true: faster solving, but ~260 MB of tables.
  // Build once on first run (a few minutes), then cached via TableManager.
  await cube.init(true);

  // Solve & verify
  const solution = await cube.solve(scrambled, { target: cube.cubeId, goodEnough: 22 });
  if (solution.ok) {
    console.log("solution: ", solution.solution);
    const endState = cube.facecube(solution.solution, scrambled);
    console.log("verified:", endState === cube.cubeId);
  } else {
    console.error("solving failed:", solution.error)
  }
  // cube.abort();
}
await main();
```

Examples: See [demo-node](./examples/demo-node/), [demo-web](./examples/demo-web/) .

## 📖 API Reference

### Unified Api for Node.js & Browser

```typescript
// Node.js
import type { CubeApi } from '@coshz/jscube';
import { createCubeApi, SolveResult, SolveOptions, PermFormat } from '@coshz/jscube';

const api: CubeApi = await createCubeApi()
```

#### Usage order

| Method                                      | Order                 |
| :------------------------------------------ | :-------------------- |
| api . { solvable / permutation / facecube } | available immediately |
| api . init(useSymmetry)                     | load or build tables  |
| api . solve(cube)                           | requires init()       |

#### Trade-off

| Value               | Table Size | Build time    | Residient Mem | Solve speed |
| :------------------ | :--------- | :------------ | :------------ | :---------- |
| `useSymmetry=false` | < 10 MB    | fast          | 57 ~ 302 MB   | slower      |
| `useSymmetry=true`  | ~260 MB    | a few minutes | 57 ~ 536 MB   | faster      |

> for `useSymmetry=true`: Peak Mem ~= 927 MB, Residient Mem ~= 310 MB

### Methods

| Method                      | Description                                                            |
| :-------------------------- | :--------------------------------------------------------------------- |
| `init(useSymmetry)`         | Loads or build tables; `useSymmetry` selects the table variant.        |
| `solve(src, options?)`      | Async Solver. Returns a `Promise<SolveResult>` object.                 |
| `solvable(src)`             | Returns `true` if the state string is physically solvable.             |
| `facecube(maneuver, cube?)` | Applies move sequence (e.g., `"R U R' U'"`) and returns the new state. |
| `permutation(mOrc, fmt?)`   | Returns the permutation array/string for a move sequence or a cube.    |

### Types

```typescript
interface CubeApiBase {
    /** Solved-state 54-char facelet string. */
    readonly cubeId: string;

    /** `true` if the 54-char state string is physically reachable */
    solvable: (src: string) => boolean;

    /** Apply a move sequence (e.g. `"R U R' U'"`) to `cube` (default: `cubeId`)
     * and return the resulting 54-char state. */
    facecube: (maneuver: string, cube?: string) => string;

    /** Permutation of a move sequence or cube state.
     * `fmt` defaults to `PermFormat.Cycle`. */
    permutation: (maneuver_or_cube: string, fmt?: PermFormat) => string;
}

/** Abstract capability to abort the underlying worker */
interface Abortable {
    /**
     * Terminate the active Worker and rest the API states.
     * - If interrupted during `init`: any incomplete, newly generated tables will be cleaned up.
     * - If interrupted during `solve`: the Worker is killed. The next `solve`
     *  call will recreate a Worker, discarding the ongoing computation.
     */
    abort: () => void;
}

interface CubeCore extends CubeApiBase {
    /**
     * Load tables from persistent storage, or build and save them if missing.
     *
     * @param manager
     *   The table manager used for persistence. See @ref TableManager.
     *
     * @param useSymmetry
     *  * `false`: smaller (<10 MB), fast to build, slower solving;
     *  * `true`: symmetry-based (~260 MB), slower to build, faster solving.
     *
     * @returns A promise that resolves to `boolean`:
     *  * `true`: first-time build
     *  * `false`: subsequent load
     *
     * @note Strongly recommended to call this explicitly before `solve`.
     *  If skipped, `solve` still works, but C++ fall back to its own `useSymmetry`
     *  setting, which may not be what you want.
     */
    init: (manager: TableManager, useSymmetry: boolean) => Promise<boolean>;

    /**
     * Solve the cube from a given state.
     *
     * @param src - 54-character state string, using the standard facelet order
     *   (U1..U9, R1..R9, F1..F9, D1..D9, L1..L9, B1..B9). Obtainable from
     *   `cube.facecube()` or `cube.cubeId` (the solved state).
     *
     * @param options.target - Goal state as a 54-character string.
     *   Defaults to `cubeId` (solved state).
     *
     * @param options.goodEnough - Accept a solution once its length is <= this
     *   many moves. Smaller values search longer for a shorter solution; larger
     *   values return faster but may return a longer solution. Default `22`.
     *
     * @returns
     *   - `{ ok: true, solution }` on success. `solution` is a space-separated
     *     move sequence, e.g. `"R U R' U'"`.
     *   - `{ ok: false, error }` if no solution is found (e.g. `target` is not
     *     reachable, or search budget exhausted before hitting `goodEnough`).
     *     Check `result.ok` before reading `solution`.
     *
     * @example
     *  const r = cube.solve(cube.facecube("R U R' U'"));
     *  if (r.ok) console.log(r.solution);   // e.g. "U R U' R'"
     *  else     console.error(r.error);
     */
    solve(src: string, options?: SolveOptions): SolveResult;
}

interface CubeApi extends CubeApiBase, Abortable {
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
