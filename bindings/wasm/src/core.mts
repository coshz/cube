interface SolveOptions {
    target?: string;
    goodEnough?: number;
}

type SolveResult =
    | { ok: true; solution: string }
    | { ok: false; error: string };

const PermFormat = { Face: 0, Cubie: 1, Cycle: 2 } as const;
type PermFormat = typeof PermFormat[keyof typeof PermFormat];

interface TableManager {
    prepare(module: any): Promise<void> | void;
    getVirtualPath(): string;
    afterInit?(module: any): Promise<void> | void;
}

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

async function createCubeCore(
    initModule: () => any | Promise<any>
): Promise<CubeCore> {
    const m = await initModule();
    const cubeId: string = m.cubeId;
    let ready = false;

    async function init(tableManager: TableManager, useSymmetry: boolean) {
        if (ready) return false;
        await tableManager.prepare(m);
        m.cube_init(useSymmetry, tableManager.getVirtualPath());
        await tableManager.afterInit?.(m);
        return ready = true;
    }

    return {
        cubeId,
        init,
        solvable: (s) => m.solvable(s),
        facecube: (s, c = cubeId) => m.facecube(s, c),
        permutation: (x, f) => m.permutation(x, f ?? PermFormat.Cycle),
        solve: (src, opts) => {
            const { target = cubeId, goodEnough = 22 } = opts ?? {};
            return m.solve(src, target, goodEnough);
        },
    };
}

export type { TableManager, CubeApiBase, CubeApi, Abortable };
export { SolveOptions, SolveResult, PermFormat, CubeCore, createCubeCore };
