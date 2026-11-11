// This file provides the two halves of the Worker RPC bridge as pure factory functions.
// It DOES NOT spawn workers. The environment-specific glue (client-web, client-node, etc.)
// is responsible for spawning threads and wiring these halves together.
//
// ⚠️ IMPORTANT:
// - `createWorkerSide` runs INSIDE the worker thread.
// - `createClientSide` runs ON THE MAIN thread.
// They should never import each other's environment-specific APIs.

import { createCubeCore } from '../core.mjs';
import type { CubeCore, TableManager, CubeApi, SolveResult } from '../core.mjs';
import type { WorkerRequest, WorkerResponse } from './protocol.mjs';

// ============================================================================
// PART 1: Worker Thread Side
// ============================================================================

function createWorkerSide(opts: {
    /** Injected by platform glue: load and create the WASM core. */
    createCore: () => Promise<CubeCore>;
    /** Injected by platform glue: create the environment-specific TableManager. */
    createTableManager: (msg: Extract<WorkerRequest, { type: 'init' }>) => TableManager;
    /** Injected by platform glue: send messages back to the main thread. */
    post: (msg: WorkerResponse) => void;
}): (msg: WorkerRequest) => Promise<void> {
    let core: CubeCore | null = null;

    return async (msg: WorkerRequest) => {
        if (msg.type === 'init') {
            try {
                const tm = opts.createTableManager(msg);
                const c = await opts.createCore();
                const isFirstTimeBuild = await c.init(tm, msg.useSymmetry ?? false);
                core = c;
                opts.post({ type: 'ready', isFirstTimeBuild });
            } catch (err) {
                core = null;
                opts.post({ type: 'init-error', message: toMessage(err) });
            }
            return;
        }

        if (msg.type === 'solve') {
            if (!core) {
                opts.post({ type: 'solve', id: msg.id, result: { ok: false, error: 'not initialized' } });
                return;
            }
            let result;
            try {
                result = core.solve(msg.src, msg.options);
            } catch (err) {
                result = { ok: false, error: toMessage(err) } as const;
            }
            opts.post({ type: 'solve', id: msg.id, result });
        }
    };
}

// ============================================================================
// PART 2: Main Thread Side
// ============================================================================

/** Abstraction of a spawned worker thread, implemented by platform glue. */
interface SpawnedWorker {
    post: (msg: WorkerRequest) => void;
    onSolve: (cb: (msg: Extract<WorkerResponse, { type: 'solve' }>) => void) => void;
    /** Resolves to a boolean indicating if this was a first-time build. */
    ready: Promise<boolean>;
    /** Forcefully terminate the worker and reject its `ready` promise if pending. */
    terminate: () => void;
}

async function createClientSide(
    initModule: () => any | Promise<any>,
    spawn: (useSymmetry: boolean) => SpawnedWorker,
): Promise<CubeApi> {
    // Main-thread core for synchronous queries only. Does not init tables.
    const core = await createCubeCore(initModule);

    let activeWorker: SpawnedWorker | null = null;
    let initPromise: Promise<void> | null = null;
    let lastUseSymmetry: boolean;
    let nextId = 1;
    const pending = new Map<number, (sr: SolveResult) => void>();
    const initializingWorkers = new Set<SpawnedWorker>();

    const handleSolve = (msg: Extract<WorkerResponse, { type: 'solve' }>) => {
        const resolve = pending.get(msg.id);
        if (resolve) {
            pending.delete(msg.id);
            resolve(msg.result);
        }
    };

    async function doInit(useSymmetry: boolean) {
        // Since WASM linear memory does not shrink automatically, we adopt
        // two-worker-relay mechanism to release memory that used in `init` but
        // not required in `solve`.
        lastUseSymmetry = useSymmetry;
        try {
            const gen1 = spawn(useSymmetry);
            initializingWorkers.add(gen1);
            gen1.onSolve(handleSolve);

            const isFirstTimeBuild = await gen1.ready;
            initializingWorkers.delete(gen1);

            if (!initPromise) { gen1.terminate(); return; }
            if (isFirstTimeBuild) {
                const gen2 = spawn(useSymmetry);
                initializingWorkers.add(gen2);
                gen2.onSolve(handleSolve);

                await gen2.ready;
                initializingWorkers.delete(gen2);

                if (!initPromise) { gen2.terminate(); gen1.terminate(); return; }

                gen1.terminate();
                activeWorker = gen2;
            } else {
                activeWorker = gen1;
            }
        } catch (err) {
            for (const w of initializingWorkers) w.terminate();
            initializingWorkers.clear();
            if (initModule === null) return; // already terminated
            initPromise = null;
            throw err;
        }
    }

    function ensureInit(useSymmetry: boolean) {
        return initPromise ??= doInit(useSymmetry);
    }

    return {
        cubeId: core.cubeId,
        solvable: core.solvable,
        facecube: core.facecube,
        permutation: core.permutation,

        init(useSymmetry = true) {
            return ensureInit(useSymmetry);
        },

        async solve(src, options) {
            if (!activeWorker) await ensureInit(lastUseSymmetry);

            const id = nextId++;
            return new Promise<SolveResult>((resolve) => {
                pending.set(id, resolve);
                activeWorker!.post({ type: 'solve', id, src, options });
            });
        },

        abort() {
            if (activeWorker) {
                activeWorker.terminate();
                activeWorker = null;
            }
            for (const w of initializingWorkers) w.terminate();
            initializingWorkers.clear();
            initPromise = null;
            for (const resolve of pending.values()) resolve({ ok: false, error: 'Aborted' });
            pending.clear();
        },
    };
}

// ============================================================================
// Shared Utilities
// ============================================================================

function toMessage(err: unknown): string {
    return err instanceof Error ? err.message : String(err);
}

export type { SpawnedWorker };
export { createClientSide, createWorkerSide };
