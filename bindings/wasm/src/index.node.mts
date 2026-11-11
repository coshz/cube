import type { CubeApi } from "./core.mjs";
import { SolveResult, SolveOptions, PermFormat } from "./core.mjs";
import { createCubeApiWith } from './worker/client-node.mjs'
import path from "path";

const HOST_DIR = path.resolve(
    process.env.TABLE_DIR ?? path.join(process.cwd(), "tables")
);

const createCubeApi = (
    opts: { hostDir?: string, workerUrl?: string | URL } = {}
) => createCubeApiWith({
    hostDir: opts.hostDir ?? HOST_DIR,
    workerUrl: opts.workerUrl
});

export type { CubeApi };
export { SolveResult, SolveOptions, PermFormat, createCubeApi }
