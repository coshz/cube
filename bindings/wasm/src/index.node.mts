import type { NodeCubeApi } from "./core.mjs";
import { SolveResult, SolveOptions, PermFormat } from "./core.mjs";
import { createNodeCubeApiWith } from './node.mjs';
import path from "path";

const VFS_MOUNT = '/coshz-jscube/tables';
const HOST_DIR = path.resolve(
    process.env.TABLE_DIR ?? path.join(process.cwd(), "tables")
);

const createNodeCubeApi = async (): Promise<NodeCubeApi> => 
    createNodeCubeApiWith(
        async () => (await import('#internal/jscube.node.mjs')).default(),
        { hostDir: HOST_DIR, vfsDir: VFS_MOUNT }
    );

export type { NodeCubeApi };
export { SolveResult, SolveOptions, PermFormat, createNodeCubeApi }