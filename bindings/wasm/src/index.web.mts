import type { CubeApi } from "./core.mjs";
import { SolveResult, SolveOptions, PermFormat } from "./core.mjs";
import { createCubeApiWith } from './worker/client-web.mjs';

const OPFS_DIR = "jscube_tables";

const createCubeApi = (
    opts: { opfsDir?: string; workerUrl?: string | URL } = {}
) => createCubeApiWith({
    opfsDir: opts.opfsDir ?? OPFS_DIR,
    workerUrl: opts.workerUrl
});

export type { CubeApi };
export { SolveResult, SolveOptions, PermFormat, createCubeApi };
