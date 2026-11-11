import { Worker } from 'node:worker_threads';
import { createClientSide } from './rpc.mjs';
import type { SpawnedWorker } from './rpc.mjs';
import type { CubeApi } from '../core.mjs';
import type { WorkerRequest, WorkerResponse } from './protocol.mjs';
import { loadNodeWasm } from '../wasm-loader.mjs'

async function createCubeApiWith(
    opts: { hostDir: string; workerUrl?: string | URL }
): Promise<CubeApi> {
    const spawn = (useSymmetry: boolean): SpawnedWorker => {
        const worker = new Worker(
            opts.workerUrl ?? new URL('./entry-node.mjs', import.meta.url)
        );
        const ready = Promise.withResolvers<boolean>();
        let solveCb: ((msg: Extract<WorkerResponse, { type: 'solve' }>) => void) | null = null;
        let isTerminated = false;

        worker.on('message', (m: WorkerResponse) => {
            if (m.type === 'ready') ready.resolve(m.isFirstTimeBuild);
            else if (m.type === 'init-error') ready.reject(new Error(m.message));
            else if (m.type === 'solve') solveCb?.(m);
        });

        worker.on('error', (e) => ready.reject(e));
        worker.on('exit', (code) => {
            if (code !== 0 && !isTerminated) {
                ready.reject(new Error(`Worker exited with code ${code}`));
            }
        });

        worker.postMessage({
            type: 'init',
            persistDir: opts.hostDir,
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

    return createClientSide(loadNodeWasm, spawn);
}

export { createCubeApiWith };
