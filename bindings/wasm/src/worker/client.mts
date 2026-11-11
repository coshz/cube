import { createCubeCore, WorkerCubeApi } from '../core.mjs';
import type { SolveResult } from '../core.mjs';
import type { WorkerRequest, WorkerResponse } from './protocol.mjs';

async function createWorkerCubeApiWith(
    initModule: () => any | Promise<any>,
    opts: {vfsDir: string, opfsDirname?: string, workerUrl?: string | URL }
): Promise<WorkerCubeApi> {
    const core = await createCubeCore(initModule);
    const workerUrl = opts.workerUrl ?? new URL('./entry.mjs', import.meta.url);

    let activeWorker: Worker | null = null;
    let initPromise: Promise<void> | null = null;
    let nextId = 1;
    const pending = new Map<number, (sr: SolveResult) => void>();

    function spawnWorker(useSymmetry: boolean): { worker: Worker, ready: Promise<void> } {
        const worker = new Worker(workerUrl, { type: 'module' });
        const ready = Promise.withResolvers<void>();

        worker.onmessage = (e: MessageEvent<WorkerResponse>) => {
            const m = e.data;
            if (m.type === 'ready') { ready.resolve(); return; }
            if (m.type === 'init-error') { ready.reject(new Error(m.message)); return; }
            if (m.type === 'solve') {
                const resolve = pending.get(m.id);
                if (resolve)  { pending.delete(m.id); resolve(m.result); }
            }
        };
        worker.onerror = (e) => ready.reject(new Error(e.message || 'Worker error'));
        worker.postMessage({
            type: 'init',
            vfsDir: opts.vfsDir,
            opfsDirname: opts.opfsDirname,
            useSymmetry
        } satisfies WorkerRequest);

        return { worker, ready: ready.promise };
    }

    return {
        cubeId: core.cubeId,
        solvable: core.solvable,
        facecube: core.facecube,
        permutation: core.permutation,

        init(useSymmetry = false) {
            return initPromise ??= (async () => {
                // Since WASM linear memory cannot shrink, we terminate the first worker 
                // after the second one takes over, to release the memory bloat caused 
                // by table generation.

                const gen1 = spawnWorker(useSymmetry);
                await gen1.ready;

                const gen2 = spawnWorker(useSymmetry);
                await gen2.ready;

                gen1.worker.terminate();
                activeWorker = gen2.worker;
            })();
        },

        solve(src, options) {
            if (!activeWorker) return Promise.resolve({ ok: false, error: 'not initialized' });
            const id = nextId++;
            return new Promise<SolveResult>((resolve) => {
                pending.set(id, resolve);
                activeWorker!.postMessage({ type: 'solve', id, src, options } satisfies WorkerRequest);
            });
        },
    };
}

export { createWorkerCubeApiWith };