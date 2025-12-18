enum StatusCode {
    OK = 0,
    UNSOLVABLE = 1,
    NOT_FOUND = 2,
    INVALID_SRC = 3,
    INVALID_TGT = 4,
    UNKNOWN_ERR = 5
}

type SolveResult = {
    status_code: StatusCode;
    solution: string;
}

interface CubeAPI {
    solvable: (src: string) => boolean;
    get_facecube: (maneuver: string, cube?: string) => string;
    get_permutation: (maneuver: string) => string;
    solve_ultimate: (src: string, dest?: string, step?: number, best?: boolean) => SolveResult;
    try_solve: (src: string, best?: boolean) => string;
}

const CubeID = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

async function createAPI(
    initModule: () => any | Promise<any>
): Promise<CubeAPI> {
    const module_ = await initModule();

    function solvable(src:string):boolean {
        return module_.js_solvable(src);
    }

    function get_facecube(maneuver:string, cube:string=CubeID) {
        return module_.js_facecube(maneuver, cube);
    }
    
    function get_permutation(maneuver:string) {
        return module_.js_permutation(maneuver);
    }
    
    function solve_ultimate(src:string, dest:string=CubeID, step:number=30, best:boolean=true): SolveResult {
        const result = module_.js_solve_ultimate(src, dest, step, best);
        return {
            status_code: result[0].value    // object -> integer
            ,solution: result[1]            // string 
        }
    }
    
    function try_solve(src:string, best:boolean=true):string {
        const result: SolveResult = solve_ultimate(src, CubeID, 30, best);
        if (result.status_code !== StatusCode.OK) {
            throw new Error(`Failed to solve cube: ${result.status_code}`);
        }
        return result.solution;
    }

    return {
        solvable,
        get_facecube,
        get_permutation,
        solve_ultimate,
        try_solve,
    };
}

export { createAPI, StatusCode, SolveResult, CubeID };
export type { CubeAPI };