import { createClientSide } from './rpc.mjs';
import type { SpawnedWorker } from './rpc.mjs';
import type { CubeApi } from '../core.mjs';
import type { WorkerRequest, WorkerResponse } from './protocol.mjs';
import { loadWebWasm } from '../wasm-loader.mjs'

async function createCubeApiWith(
    opts: { opfsDir: string; workerUrl?: string | URL }
): Promise<CubeApi> {
    const spawn = (useSymmetry: boolean): SpawnedWorker => {
        const worker = new Worker(
            opts.workerUrl ?? new URL('./entry-web.mjs', import.meta.url),
            { type: 'module' }
        );
        const ready = Promise.withResolvers<boolean>();
        let solveCb: ((msg: Extract<WorkerResponse, { type: 'solve' }>) => void) | null = null;
        let isTerminated = false;

        worker.onmessage = (e: MessageEvent<WorkerResponse>) => {
            const m = e.data;
            if (m.type === 'ready') ready.resolve(m.isFirstTimeBuild);
            else if (m.type === 'init-error') ready.reject(new Error(m.message));
            else if (m.type === 'solve') solveCb?.(m);
        };

        worker.onerror = (e) => ready.reject(new Error(e.message || 'Worker error'));

        worker.postMessage({
            type: 'init',
            persistDir: opts.opfsDir,
            useSymmetry
        } satisfies WorkerRequest);

        return {
            post: (msg) => { if (!isTerminated) worker.postMessage(msg); },
            onSolve: (cb) => { solveCb = cb; },
            ready: ready.promise,
            terminate: () => {
                if (isTerminated) return;
                isTerminated = true;
                worker.terminate();
                ready.reject(new Error('Worker terminated'));
            },
        };
    };

    return createClientSide(loadWebWasm, spawn);
}

export { createCubeApiWith };
