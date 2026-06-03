import { createAPI, SolveResult, StatusCode, CubeID } from "./api.mjs";
import type { CubeAPI } from "./api.mjs";

const moduleLoader = async ():Promise<any> => {
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
    const module = await moduleLoader();
    const factory = module.default;
    return createAPI(factory);
}

export type { CubeAPI };
export { createCubeApi, SolveResult, StatusCode, CubeID };