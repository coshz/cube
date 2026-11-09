import { NodeCubeApi, createCubeCore } from "./core.mjs";
import { NodeFsTableManager } from "./TableManager/index.mjs";

async function createNodeCubeApiWith(
    initModule: () => any | Promise<any>,
    opts: { hostDir: string; vfsDir: string }
): Promise<NodeCubeApi> {
    const core = await createCubeCore(initModule);
    const tm = new NodeFsTableManager({ hostDir: opts.hostDir, vfsDir: opts.vfsDir });
    let initPromise: Promise<void> | null = null;
    return {
        cubeId: core.cubeId,
        solvable: core.solvable,
        facecube: core.facecube,
        permutation: core.permutation,
        init(useSymmetry = false) {
            if(!initPromise) {
                initPromise = core.init(tm, useSymmetry).catch(err => {
                    initPromise = null;
                    throw err;
                });
            }
            return initPromise;
        },
        solve(src, options) {
            return core.solve(src, options);
        },
    };
}

export { createNodeCubeApiWith };