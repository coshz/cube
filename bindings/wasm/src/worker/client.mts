import { createCubeCore, WorkerCubeApi } from '../core.mjs';
import type { SolveResult } from '../core.mjs';
import type { WorkerRequest, WorkerResponse } from './protocol.mjs';

async function createWorkerCubeApiWith(
    initModule: () => any | Promise<any>,
    opts: {vfsDir: string, opfsDirname?: string, workerUrl?: string | URL }
): Promise<WorkerCubeApi> {
    const core = await createCubeCore(initModule);

    const worker = new Worker(
        opts.workerUrl ?? new URL('./entry.mjs', import.meta.url),
        { type: 'module' }
    );

    let nextId = 1;
    const pending = new Map<number, (sr: SolveResult) => void>();
    const { promise: ready, resolve: readyResolve, reject: readyReject } = Promise.withResolvers<void>();

    worker.onmessage = (e: MessageEvent<WorkerResponse>) => {
        const m = e.data;
        if (m.type === 'ready') { readyResolve(); return; }
        if (m.type === 'init-error') { readyReject(new Error(m.message)); return; }
        if (m.type === 'solve') {
            const resolve = pending.get(m.id);
            if (!resolve) return;
            pending.delete(m.id);
            resolve(m.result);
        }
    };

    let crashed: Error | null = null;

    worker.onerror = (e) => {
        const err = new Error(e.message || 'worker error');
        crashed = err;
        readyReject(err);
        for (const resolve of pending.values()) resolve({ ok: false, error: err.message });
        pending.clear();
    };

    let initPromise: Promise<void> | null = null;

    return {
        cubeId: core.cubeId,
        solvable: core.solvable,
        facecube: core.facecube,
        permutation: core.permutation,

        init(useSymmetry = false) {
            return initPromise ??= (async () => {
                worker.postMessage({
                    type: 'init',
                    vfsDir: opts.vfsDir,
                    opfsDirname: opts.opfsDirname,
                    useSymmetry,
                } satisfies WorkerRequest);
                await ready;
            })();
        },

        solve(src, options) {
            if (crashed) return Promise.resolve({ ok: false, error: crashed.message });
            const id = nextId++;
            return new Promise<SolveResult>((resolve) => {
                pending.set(id, resolve);
                worker.postMessage({ type: 'solve', id, src, options } satisfies WorkerRequest);
            });
        },
    };
}

export { createWorkerCubeApiWith };