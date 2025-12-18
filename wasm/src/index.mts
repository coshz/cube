import { createAPI, SolveResult, StatusCode, CubeID } from "./api.mjs";
import type { CubeAPI } from "./api.mjs";

const moduleLoader = async ():Promise<any> => {
    const isNode =  
        typeof process === "object" && 
        typeof process.versions === "object" && 
        typeof process.versions.node === "string";

    const loader = isNode
        ? import('#internal/jscube.node.mjs')
        : import('#internal/jscube.web.mjs');;
    return loader;
};

const createCubeApi = async () => {
    const module = await moduleLoader();
    const factory = module.default;
    return createAPI(factory);
}

export type { CubeAPI };
export { createCubeApi, SolveResult, StatusCode, CubeID };