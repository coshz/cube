interface SolveOptions {
    target?: string; 
    maxSteps?: number; 
    best?: boolean;
}

type SolveResult =
    | { ok: true; solution: string }
    | { ok: false; error: string };


interface CubeAPI {
    solvable: (src: string) => boolean;
    facecube: (maneuver: string, cube?: string) => string;
    permutation: (maneuver: string) => string;
    trySolve: (src: string, options: SolveOptions) => SolveResult; // no exception
    solve: (src: string, options: SolveOptions) => string; // with exception
}

const CubeID = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

async function createAPI(
    initModule: () => any | Promise<any>
): Promise<CubeAPI> {
    const module_ = await initModule();

    function solvable(src:string):boolean {
        return module_.solvable(src);
    }

    function facecube(maneuver:string, cube:string = CubeID) {
        return module_.facecube(maneuver, cube);
    }
    
    function permutation(maneuver:string) {
        return module_.permutation(maneuver);
    }

    function trySolve(src:string = CubeID, options: SolveOptions = {}): SolveResult {
        const { target = CubeID, maxSteps = 30, best = true } = options;
        return module_.solveUltimate(src, target, maxSteps, best);
    }

    function solve(src:string = CubeID, options: SolveOptions = {}): string {
        const result = trySolve(src, options);
        if (result.ok) {
            return result.solution;
        } else {
            throw new Error(result.error);
        }
    }

    return {
        solvable,
        facecube,
        permutation,
        trySolve,
        solve,
    };
}

export { createAPI, SolveResult, CubeID };
export type { CubeAPI };