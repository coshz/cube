import type { WorkerCubeApi } from "./core.mjs";
import { SolveResult, SolveOptions, PermFormat } from "./core.mjs";
import { createWorkerCubeApiWith } from './worker/index.mjs';

const VFS_MOUNT = '/coshz-jscube/tables';
const createWorkerCubeApi = async (): Promise<WorkerCubeApi> => 
    createWorkerCubeApiWith(
        async () => (await import('#internal/jscube.web.mjs')).default(),
        { vfsDir: VFS_MOUNT }
    );

export type { WorkerCubeApi };
export { SolveResult, SolveOptions, PermFormat, createWorkerCubeApi };
