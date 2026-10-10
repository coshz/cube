import { createApiFromModule, SolveResult, SolveOptions, PermFormat } from "./api.mjs";
import type { CubeApi } from "./api.mjs";

const loadWasmModule = async ():Promise<any> => {
    const isNode =  
        typeof process === "object" && 
        typeof process.versions === "object" && 
        typeof process.versions.node === "string";
    const path = isNode 
        ? './internal/jscube.node.mjs'
        : './internal/jscube.web.mjs';
    return import(path);
};

const createCubeApi = async () => {
    const wasmModule = await loadWasmModule();
    const initWasm = wasmModule.default;
    return createApiFromModule(initWasm);
}

export type { CubeApi };
export { createCubeApi, SolveResult, SolveOptions, PermFormat };